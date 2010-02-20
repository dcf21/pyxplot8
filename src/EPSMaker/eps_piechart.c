// eps_piechart.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
//
// $Id$
//
// PyXPlot is free software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
//
// You should have received a copy of the GNU General Public License along with
// PyXPlot; if not, write to the Free Software Foundation, Inc., 51 Franklin
// Street, Fifth Floor, Boston, MA  02110-1301, USA

// ----------------------------------------------------------------------------

#define _PPL_EPS_PIECHART 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

#include "MathsTools/dcfmath.h"

#include "ppl_canvasdraw.h"
#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_piechart.h"
#include "eps_plot.h"
#include "eps_plot_legend.h"
#include "eps_plot_styles.h"
#include "eps_settings.h"

void eps_pie_ReadAccessibleData(EPSComm *x)
 {
  int              j, status, ErrCount, NExpect, Ncolumns;
  canvas_plotdesc *pd;
  char             errbuffer[LSTR_LENGTH];
  char            *LabelString;
  List            *UsingList, *EveryList;
  Dict            *tempdict;
  with_words       ww_default;
  double          *ordinate_raster, acc;
  DataBlock       *blk;

  if ((pd=x->current->plotitems)==NULL) return;

  // Malloc pointer to data table where data to be plotted will be stored
  x->current->plotdata      = (DataTable **)lt_malloc(1 * sizeof(DataTable *));
  x->current->DatasetTextID = (int *)lt_malloc(1 * sizeof(int));
  if (x->current->plotdata == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *(x->status) = 1; return; }

  // Merge together with words to form a final set
  eps_withwords_default(&ww_default, &x->current->settings, 1, 0, 0, settings_term_current.colour==SW_ONOFF_ON);
  with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1);

  // Make raster on which to evaluate parametric functions
  ordinate_raster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
  if (x->current->settings.Tlog == SW_BOOL_TRUE) LogarithmicRaster(ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);
  else                                           LinearRaster     (ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);

  // Work out what label string to use
  if (pd->label != NULL) LabelString = pd->label;
  else                   LabelString = "\"Item %d\"%($0)";

  // Collect together using and every items
  UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
  EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "every_item", (void *)(pd->EveryList+j), 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
  status   = 0;
  ErrCount = DATAFILE_NERRS;
  NExpect  = 1;

  if (eps_plot_AddUsingItemsForWithWords(&pd->ww_final, &NExpect, UsingList)) { *(x->status) = 1; return; } // Add extra using items for, e.g. "linewidth $3".

  if (pd->function == 0) // Read data from file
   {
    if (DEBUG) { sprintf(temp_err_string, "Reading data from file '%s' for piechart item %d", pd->filename, x->current->id); ppl_log(temp_err_string); }
    DataFile_read(x->current->plotdata, &status, errbuffer, pd->filename, pd->index, pd->UsingRowCols, UsingList, EveryList, LabelString, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
   } else {
    if (DEBUG) { sprintf(temp_err_string, "Reading data from functions for piechart item %d", x->current->id); ppl_log(temp_err_string); }
    DataFile_FromFunctions(ordinate_raster, 1, x->current->settings.samples, &settings_graph_current.Tmin, x->current->plotdata, &status, errbuffer, pd->functions, pd->NFunctions, UsingList, LabelString, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
   }
  if (status) { ppl_error(ERR_GENERAL, errbuffer); x->current->plotdata[0]=NULL; }

  // Work out sum of all pie segment sizes
  if (x->current->plotdata[0]==NULL) return;
  Ncolumns = x->current->plotdata[0]->Ncolumns;
  blk      = x->current->plotdata[0]->first;
  acc      = 0.0;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++) { acc += fabs(blk->data_real[0 + Ncolumns*j].d); }
    blk=blk->next;
   }
  if ((!gsl_finite(acc))||(acc<=0.0)) { sprintf(temp_err_string, "Sum of sizes of all pie wedges is not a finite number."); ppl_error(ERR_GENERAL,temp_err_string); x->current->plotdata[0]=NULL; }
  if (DEBUG) { sprintf(temp_err_string, "Sum of sizes of all pie wedges = %e", acc); ppl_log(temp_err_string); }
  pd->PieChart_total = acc;
  return;
 }

void eps_pie_YieldUpText(EPSComm *x)
 {
  int              j, k, l, Ncolumns;
  char            *FormatString, *label, DefaultFormat[64]="\"%.1d\\%% %s\"%(percentage,label)";
  canvas_plotdesc *pd;
  DataBlock       *blk;
  CanvasTextItem  *i;
  value            DummyTemp_l, DummyTemp_p, DummyTemp_w;
  value           *var_l      , *var_p     , *var_w;

  if ((pd=x->current->plotitems)==NULL) return;
  x->current->FirstTextID = x->NTextItems;

  // Work out what format string to use
  if (x->current->text==NULL) FormatString=DefaultFormat;
  else                        FormatString=x->current->text;

  // Get pointers to the variables [percentage, label, wedgesize]
  DictLookup(_ppl_UserSpace_Vars, "label"     , NULL, (void **)&var_l);
  if (var_l != NULL) { DummyTemp_l = *var_l; }
  else  { ppl_units_zero(&DummyTemp_l); DictAppendValue(_ppl_UserSpace_Vars, "label"     , DummyTemp_l); DictLookup(_ppl_UserSpace_Vars, "label"     , NULL, (void **)&var_l); DummyTemp_l.modified = 2; }
  DictLookup(_ppl_UserSpace_Vars, "percentage", NULL, (void **)&var_p);
  if (var_p != NULL) { DummyTemp_p = *var_p; }
  else  { ppl_units_zero(&DummyTemp_p); DictAppendValue(_ppl_UserSpace_Vars, "percentage", DummyTemp_p); DictLookup(_ppl_UserSpace_Vars, "percentage", NULL, (void **)&var_p); DummyTemp_p.modified = 2; }
  DictLookup(_ppl_UserSpace_Vars, "wedgesize" , NULL, (void **)&var_w);
  if (var_w != NULL) { DummyTemp_w = *var_w; }
  else  { ppl_units_zero(&DummyTemp_w); DictAppendValue(_ppl_UserSpace_Vars, "wedgesize" , DummyTemp_w); DictLookup(_ppl_UserSpace_Vars, "wedgesize" , NULL, (void **)&var_w); DummyTemp_w.modified = 2; }
  ppl_units_zero(var_l);
  ppl_units_zero(var_p);
  *var_w = x->current->plotdata[0]->FirstEntries[0];

  // Labels of pie wedges
  Ncolumns = x->current->plotdata[0]->Ncolumns;
  blk      = x->current->plotdata[0]->first;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      // Set values of variables [percentage, label, wedgesize] for this wedge
      var_w->real   = blk->data_real[0 + Ncolumns*j].d;
      var_p->real   = var_w->real / pd->PieChart_total * 100.0;
      var_l->string = blk->text[j];
      label         = (char *)lt_malloc((strlen(FormatString)+10)*8);
      k             = strlen(FormatString)-1;
      l             = -1;
      if (label!=NULL)
       {
        ppl_GetQuotedString(FormatString, label, 0, &k, 0, &l, temp_err_string, 1);
        if (l>=0) strcpy(label, "?");
       }
      blk->text[j] = label;
      YIELD_TEXTITEM(label);
     }
    blk=blk->next;
   }

  // Restore values of the variables [percentage, label, wedgesize]
  *var_l = DummyTemp_l;
  *var_p = DummyTemp_p;
  *var_w = DummyTemp_w;

  // Title of piechart
  x->current->TitleTextID = x->NTextItems;
  YIELD_TEXTITEM(x->current->settings.title);
  return;
 }

void eps_pie_RenderEPS(EPSComm *x)
 {
  int              j, Ncolumns, lt, WedgeNumber, l, m;
  double           xpos,ypos,rad,angle,size;
  canvas_plotdesc *pd;
  DataBlock       *blk;
  double           lw, lw_scale;
  with_words       ww;

  if ((pd=x->current->plotitems)==NULL) return;
  x->LaTeXpageno = x->current->FirstTextID;

  // Print label at top of postscript description of box
  fprintf(x->epsbuffer, "%% Canvas item %d [piechart]\n", x->current->id);
  eps_core_clear(x);

  // Calculate position of centre of piechart, and its radius
  xpos = x->current->settings.OriginX.real * M_TO_PS;
  ypos = x->current->settings.OriginX.real * M_TO_PS;
  rad  = x->current->settings.width  .real * M_TO_PS / 2;

  // Expand any numbered styles which may appear in the with words we are passed
  with_words_merge(&ww, &x->current->with_data, NULL, NULL, NULL, NULL, 1);

  // Fill piechart segments
  Ncolumns = x->current->plotdata[0]->Ncolumns;
  blk      = x->current->plotdata[0]->first;
  angle    = 0.0;
  WedgeNumber = 0;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      // Work out what fillcolour to use
      for (l=0; l<PALETTE_LENGTH; l++) if (settings_palette_current[l]==-1) break; // l now contains length of palette
      m = WedgeNumber % l; // m is now the palette colour number to use
      while (m<0) m+=l;
      if (settings_palette_current[m] > 0) { ww.fillcolour  = settings_palette_current[m]; ww.USEfillcolour = 1; ww.USEfillcolourRGB = 0; }
      else                                 { ww.fillcolourR = settings_paletteR_current[m]; ww.fillcolourG = settings_paletteG_current[m]; ww.fillcolourB = settings_paletteB_current[m]; ww.USEfillcolour = 0; ww.USEfillcolourRGB = 1; }

      // Work out size of wedge and fill it
      size = fabs(blk->data_real[0 + Ncolumns*j].d) / pd->PieChart_total * 360.0;
      eps_core_SetFillColour(x, &ww);
      eps_core_SwitchTo_FillColour(x);
      IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f %.2f %.2f %.2f arc\n%.2f %.2f lineto\nclosepath\nfill\n",xpos,ypos,rad,90-angle-size,90-angle,xpos,ypos);
      angle += size;
      WedgeNumber++;
     }
    blk=blk->next;
   }

  // Set colour of outline of piechart
  eps_core_SetColour(x, &ww, 1);

  // Set linewidth and linetype of outline
  if (ww.USElinewidth) lw_scale = ww.linewidth;
  else                 lw_scale = x->current->settings.LineWidth;
  lw = EPS_DEFAULT_LINEWIDTH * lw_scale;

  if (ww.USElinetype)  lt = ww.linetype;
  else                 lt = 1;

  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, lw, lt, 0.0);

  // Draw circumference of piechart
  IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f %.2f 0 360 arc\nclosepath\nstroke\n", xpos,ypos,rad);

  // Draw pie wedges one-by-one
  Ncolumns = x->current->plotdata[0]->Ncolumns;
  blk      = x->current->plotdata[0]->first;
  angle    = 0.0;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      size = fabs(blk->data_real[0 + Ncolumns*j].d) / pd->PieChart_total * 360.0;
      IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\nclosepath\nstroke\n",xpos,ypos,xpos+rad*sin(angle*M_PI/180),ypos+rad*cos(angle*M_PI/180));
      angle += size;
     }
    blk=blk->next;
   }

  // Update bounding box
  eps_core_BoundingBox(x, xpos-rad, ypos-rad, lw);
  eps_core_BoundingBox(x, xpos+rad, ypos-rad, lw);
  eps_core_BoundingBox(x, xpos-rad, ypos+rad, lw);
  eps_core_BoundingBox(x, xpos+rad, ypos+rad, lw);

  // Title of piechart
  x->LaTeXpageno = x->current->TitleTextID;
  if ((x->current->settings.title != NULL) && (x->current->settings.title[0] != '\0'))
   {
    int pageno = x->LaTeXpageno++;
    with_words_zero(&ww,0);
    if (x->current->settings.TextColour > 0) { ww.USEcolour = 1; ww.colour = x->current->settings.TextColour; }
    else                                     { ww.USEcolourRGB = 1; ww.colourR = x->current->settings.TextColourR; ww.colourG = x->current->settings.TextColourG; ww.colourB = x->current->settings.TextColourB; }
    eps_core_SetColour(x, &ww, 1);
    IF_NOT_INVISIBLE canvas_EPSRenderTextItem(x, pageno, xpos/M_TO_PS, (ypos+rad+8)/M_TO_PS, SW_HALIGN_CENT, SW_VALIGN_BOT, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
   }

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

