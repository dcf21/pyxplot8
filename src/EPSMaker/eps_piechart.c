// eps_piechart.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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
  unsigned char    AutoUsingList=0;
  Dict            *tempdict;
  with_words       ww_default;
  double          *ordinate_raster, acc;
  DataBlock       *blk;

  if ((pd=x->current->plotitems)==NULL) return;

  // Malloc pointer to data table where data to be plotted will be stored
  x->current->plotdata      = (DataTable **)lt_malloc(1 * sizeof(DataTable *));
  x->current->DatasetTextID = (int *)lt_malloc(1 * sizeof(int));
  if (x->current->plotdata == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); *(x->status) = 1; return; }

  // Merge together with words to form a final set
  eps_withwords_default(&ww_default, &x->current->settings, 1, 0, 0, 0, settings_term_current.colour==SW_ONOFF_ON);
  with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1);
  pd->ww_final.linespoints = SW_STYLE_LINES; // In case FuncStyle is something bonkers like contourplot

  // Make raster on which to evaluate parametric functions
  ordinate_raster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
  if (x->current->settings.Tlog == SW_BOOL_TRUE) LogarithmicRaster(ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);
  else                                           LinearRaster     (ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);

  // Work out what label string to use
  if (pd->label != NULL) LabelString = pd->label;
  else                   LabelString = "\"Item %d\"%($0)";

  // Collect together using and every items
  UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(HASHSIZE_SMALL); DictAppendPtr(tempdict, "using_item", (void *)pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
  EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(HASHSIZE_SMALL); DictAppendPtr(tempdict, "every_item", (void *)(pd->EveryList+j), 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
  status   = 0;
  ErrCount = DATAFILE_NERRS;
  NExpect  = 1;

  if (eps_plot_AddUsingItemsForWithWords(&pd->ww_final, &NExpect, &AutoUsingList, UsingList)) { *(x->status) = 1; return; } // Add extra using items for, e.g. "linewidth $3".

  if (pd->function == 0) // Read data from file
   {
    if (DEBUG) { sprintf(temp_err_string, "Reading data from file '%s' for piechart item %d", pd->filename, x->current->id); ppl_log(temp_err_string); }
    if (pd->PersistentDataTable==NULL) DataFile_read(x->current->plotdata, &status, errbuffer, pd->filename, pd->index, pd->UsingRowCols, UsingList, AutoUsingList, EveryList, LabelString, NExpect, pd->SelectCriterion, pd->continuity, NULL, -1, 0, &ErrCount);
    else                               x->current->plotdata[0] = pd->PersistentDataTable;
   } else {
    if (DEBUG) { sprintf(temp_err_string, "Reading data from functions for piechart item %d", x->current->id); ppl_log(temp_err_string); }
    DataFile_FromFunctions(ordinate_raster, 1, x->current->settings.samples, &settings_graph_current.Tmin, NULL, 0, NULL, x->current->plotdata, &status, errbuffer, pd->functions, pd->NFunctions, UsingList, AutoUsingList, LabelString, NExpect, pd->SelectCriterion, pd->continuity, NULL, -1, &ErrCount);
   }
  if (status) { ppl_error(ERR_GENERAL, -1, -1, errbuffer); x->current->plotdata[0]=NULL; }

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
  if ((!gsl_finite(acc))||(acc<=0.0)) { sprintf(temp_err_string, "Sum of sizes of all pie wedges is not a finite number."); ppl_error(ERR_GENERAL, -1, -1,temp_err_string); x->current->plotdata[0]=NULL; *(x->status) = 1; }
  if (DEBUG) { sprintf(temp_err_string, "Sum of sizes of all pie wedges = %e", acc); ppl_log(temp_err_string); }
  pd->PieChart_total = acc;
  return;
 }

void eps_pie_YieldUpText(EPSComm *x)
 {
  int              j, k, l, Ncolumns;
  char            *FormatString, *label, DefaultFormat[64]="\"%.0f\\%% %s\"%(percentage,label)";
  canvas_plotdesc *pd;
  DataBlock       *blk;
  CanvasTextItem  *i;
  value            DummyTemp_l, DummyTemp_p, DummyTemp_w;
  value           *var_l      , *var_p     , *var_w;

  if ((pd=x->current->plotitems)==NULL) return;
  if (x->current->plotdata[0]==NULL) return;
  x->current->FirstTextID = x->NTextItems;

  // Work out what format string to use
  if (x->current->text==NULL) FormatString=DefaultFormat;
  else                        FormatString=x->current->text;

  // Get pointers to the variables [percentage, label, wedgesize]
  ppl_UserSpace_GetVarPointer("label"     , &var_l, &DummyTemp_l);
  ppl_UserSpace_GetVarPointer("percentage", &var_p, &DummyTemp_p);
  ppl_UserSpace_GetVarPointer("wedgesize" , &var_w, &DummyTemp_w);
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
      if ((label==NULL)||(label[0]=='\0')) blk->text[j]=NULL;
      YIELD_TEXTITEM(label);
     }
    blk=blk->next;
   }

  // Restore values of the variables [percentage, label, wedgesize]
  ppl_UserSpace_RestoreVarPointer(&var_l, &DummyTemp_l);
  ppl_UserSpace_RestoreVarPointer(&var_p, &DummyTemp_p);
  ppl_UserSpace_RestoreVarPointer(&var_w, &DummyTemp_w);

  // Title of piechart
  x->current->TitleTextID = x->NTextItems;
  YIELD_TEXTITEM(x->current->settings.title);
  return;
 }

void eps_pie_RenderEPS(EPSComm *x)
 {
  int              j, Ncolumns, lt, WedgeNumber, l, m;
  double           xpos,ypos,rad,angle,size,vtitle;
  canvas_plotdesc *pd;
  DataBlock       *blk;
  double           lw, lw_scale;
  with_words       ww, ww_txt;

  if ((pd=x->current->plotitems)==NULL) return;
  if (x->current->plotdata[0]==NULL) return;
  x->LaTeXpageno = x->current->FirstTextID;

  // Print label at top of postscript description of box
  fprintf(x->epsbuffer, "%% Canvas item %d [piechart]\n", x->current->id);
  eps_core_clear(x);

  // Calculate position of centre of piechart, and its radius
  xpos   = x->current->settings.OriginX.real * M_TO_PS;
  ypos   = x->current->settings.OriginX.real * M_TO_PS;
  rad    = x->current->settings.width  .real * M_TO_PS / 2;
  vtitle = ypos+rad;

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
      if (settings_palette_current[m] > 0) { ww.fillcolour  = settings_palette_current[m]; ww.USEfillcolour = 1; ww.USEfillcolour1234 = 0; }
      else                                 { ww.FillCol1234Space = settings_paletteS_current[m]; ww.fillcolour1 = settings_palette1_current[m]; ww.fillcolour2 = settings_palette2_current[m]; ww.fillcolour3 = settings_palette3_current[m]; ww.fillcolour4 = settings_palette4_current[m]; ww.USEfillcolour = 0; ww.USEfillcolour1234 = 1; }

      // Work out size of wedge and fill it
      size = fabs(blk->data_real[0 + Ncolumns*j].d) / pd->PieChart_total * 360.0;
      eps_core_SetFillColour(x, &ww);
      eps_core_SwitchTo_FillColour(x,1);
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

  // Write text on piechart
  with_words_zero(&ww_txt,0);
  if (x->current->settings.TextColour > 0) { ww_txt.colour = x->current->settings.TextColour; ww_txt.USEcolour = 1; }
  else                                     { ww_txt.Col1234Space = x->current->settings.TextCol1234Space; ww_txt.colour1 = x->current->settings.TextColour1; ww_txt.colour2 = x->current->settings.TextColour2; ww_txt.colour3 = x->current->settings.TextColour3; ww_txt.colour4 = x->current->settings.TextColour4; ww_txt.USEcolour1234 = 1; }
  eps_core_SetColour(x, &ww_txt, 1);

  IF_NOT_INVISIBLE
   {
    int ItemNo=0;
    double *TextWidth, *TextHeight, TotalHeight=0.0;
    double height1,height2,bb_left,bb_right,bb_top,bb_bottom,ab_left,ab_right,ab_top,ab_bottom;
    double label_rpos, best_label_rpos, best_label_margin, min_margin, key_vpos=0.0;
    postscriptPage *dviPage;

    // Calculate dimensions of text items
    TextWidth  = (double *)lt_malloc(x->current->plotdata[0]->Nrows * sizeof(double));
    TextHeight = (double *)lt_malloc(x->current->plotdata[0]->Nrows * sizeof(double));
    if ((TextWidth==NULL)||(TextHeight==NULL)) goto END_LABEL_PRINTING;
    blk        = x->current->plotdata[0]->first;
    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        dviPage = (postscriptPage *)ListGetItem(x->dvi->output->pages, x->LaTeXpageno+ItemNo);
        if (dviPage== NULL) { ItemNo++; continue; } // Such doom will trigger errors later
        bb_left   = dviPage->boundingBox[0];
        bb_bottom = dviPage->boundingBox[1];
        bb_right  = dviPage->boundingBox[2];
        bb_top    = dviPage->boundingBox[3];
        ab_left   = dviPage->textSizeBox[0];
        ab_bottom = dviPage->textSizeBox[1];
        ab_right  = dviPage->textSizeBox[2];
        ab_top    = dviPage->textSizeBox[3];
        height1   = fabs(ab_top - ab_bottom) * AB_ENLARGE_FACTOR;
        height2   = fabs(bb_top - bb_bottom) * BB_ENLARGE_FACTOR;
        TextWidth [ItemNo] = ((ab_right - ab_left) + MARGIN_HSIZE  ) * x->current->settings.FontSize;
        TextHeight[ItemNo] = ((height2<height1) ? height2 : height1) * x->current->settings.FontSize;
        TotalHeight += TextHeight[ItemNo];
        ItemNo++;
       }
      blk=blk->next;
     }

    // Labels of pie wedges
    Ncolumns = x->current->plotdata[0]->Ncolumns;
    blk      = x->current->plotdata[0]->first;
    angle    = 0.0;
    ItemNo   = 0;
    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        size = fabs(blk->data_real[0 + Ncolumns*j].d) / pd->PieChart_total * 2 * M_PI; // Angular size of wedge
        if (blk->text[j]!=NULL)
         {
          int    ArrowType;
          double a = angle+size/2; // Central angle of wedge

          // Test different radii where label can be placed
          best_label_rpos   = 0.7;
          best_label_margin = -1e10;
          for (label_rpos=0.7; label_rpos>=0.3; label_rpos-=0.05)
           {
            double r,theta,t;
            min_margin = 1e10;

#define CHECK_POSITION(X,Y) \
   r     = hypot(X,Y); \
   theta = fmod(atan2(X,Y) - a, 2*M_PI); if (theta<-M_PI) theta+=2*M_PI; if (theta>M_PI) theta-=2*M_PI; \
   t     = theta + size/2; \
   if (t < min_margin) min_margin = t; \
   t     = size/2 - theta; \
   if (t < min_margin) min_margin = t; \
   t     = r / rad; \
   if (t < min_margin) min_margin = t; \
   t     = (rad-r) / rad; \
   if (t < min_margin) min_margin = t; \

            CHECK_POSITION( rad*sin(a)*label_rpos-TextWidth[ItemNo]/2 , rad*cos(a)*label_rpos-TextHeight[ItemNo]/2 );
            CHECK_POSITION( rad*sin(a)*label_rpos-TextWidth[ItemNo]/2 , rad*cos(a)*label_rpos+TextHeight[ItemNo]/2 );
            CHECK_POSITION( rad*sin(a)*label_rpos+TextWidth[ItemNo]/2 , rad*cos(a)*label_rpos-TextHeight[ItemNo]/2 );
            CHECK_POSITION( rad*sin(a)*label_rpos+TextWidth[ItemNo]/2 , rad*cos(a)*label_rpos+TextHeight[ItemNo]/2 );
            if (min_margin>best_label_margin) { best_label_margin = min_margin; best_label_rpos = label_rpos; }
           }

          // Work out whether label will go inside, outside or in a key
          ArrowType = x->current->ArrowType;
          if (ArrowType==SW_PIEKEYPOS_AUTO) ArrowType = (best_label_margin<0.0) ? SW_PIEKEYPOS_OUTSIDE : SW_PIEKEYPOS_INSIDE;

          if (ArrowType==SW_PIEKEYPOS_INSIDE)
           { // Labelling pie wedges inside pie
            int pageno = x->LaTeXpageno++;
            eps_core_SetColour(x, &ww_txt, 1);
            canvas_EPSRenderTextItem(x, NULL, pageno, (xpos+rad*sin(a)*best_label_rpos)/M_TO_PS, (ypos+rad*cos(a)*best_label_rpos)/M_TO_PS, SW_HALIGN_CENT, SW_VALIGN_CENT, x->CurrentColour, x->current->settings.FontSize, 0.0, NULL, NULL);
           }
          else if (ArrowType==SW_PIEKEYPOS_OUTSIDE)
           { // Labelling pie wedges around edge of pie
            int    hal, val, pageno = x->LaTeXpageno++;
            double vtop;
            if      (a <   5*M_PI/180) { hal=SW_HALIGN_CENT ; val=SW_VALIGN_BOT;  }
            else if (a <  30*M_PI/180) { hal=SW_HALIGN_LEFT ; val=SW_VALIGN_BOT;  }
            else if (a < 150*M_PI/180) { hal=SW_HALIGN_LEFT ; val=SW_VALIGN_CENT; }
            else if (a < 175*M_PI/180) { hal=SW_HALIGN_LEFT ; val=SW_VALIGN_TOP;  }
            else if (a < 185*M_PI/180) { hal=SW_HALIGN_CENT ; val=SW_VALIGN_TOP;  }
            else if (a < 210*M_PI/180) { hal=SW_HALIGN_RIGHT; val=SW_VALIGN_TOP;  }
            else if (a < 330*M_PI/180) { hal=SW_HALIGN_RIGHT; val=SW_VALIGN_CENT; }
            else if (a < 355*M_PI/180) { hal=SW_HALIGN_RIGHT; val=SW_VALIGN_BOT;  }
            else                       { hal=SW_HALIGN_CENT ; val=SW_VALIGN_BOT;  }
            if      (val == SW_VALIGN_BOT ) vtop = ypos+rad*cos(a)*1.08 + TextHeight[ItemNo];
            else if (val == SW_VALIGN_CENT) vtop = ypos+rad*cos(a)*1.08 + TextHeight[ItemNo]/2;
            else                            vtop = ypos+rad*cos(a)*1.08;
            if (vtop > vtitle) vtitle=vtop;
            eps_core_SetColour(x, &ww, 1);
            IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\nclosepath\nstroke\n",xpos+rad*sin(a),ypos+rad*cos(a),xpos+rad*sin(a)*1.05,ypos+rad*cos(a)*1.05);
            eps_core_SetColour(x, &ww_txt, 1);
            canvas_EPSRenderTextItem(x, NULL, pageno, (xpos+rad*sin(a)*1.08)/M_TO_PS, (ypos+rad*cos(a)*1.08)/M_TO_PS, hal, val, x->CurrentColour, x->current->settings.FontSize, 0.0, NULL, NULL);
           }
          else // Labelling pie wedges in a key
           {
            int    pageno = x->LaTeXpageno++;
            double h = xpos + rad*1.2; // Position of left of key
            double v = ypos + TotalHeight/2 - key_vpos - TextHeight[ItemNo]/2; // Vertical position of centre of key item
            double s = MARGIN_HSIZE_LEFT*0.45/3; // Controls size of filled square which appears next to wedge label
            double st= MARGIN_HSIZE_LEFT*0.67;

            // Work out what fillcolour to use
            for (l=0; l<PALETTE_LENGTH; l++) if (settings_palette_current[l]==-1) break; // l now contains length of palette
            m = ItemNo % l; // m is now the palette colour number to use
            while (m<0) m+=l;
            if (settings_palette_current[m] > 0) { ww.fillcolour  = settings_palette_current[m]; ww.USEfillcolour = 1; ww.USEfillcolour1234 = 0; }
            else                                 { ww.FillCol1234Space = settings_paletteS_current[m]; ww.fillcolour1 = settings_palette1_current[m]; ww.fillcolour2 = settings_palette2_current[m]; ww.fillcolour3 = settings_palette3_current[m]; ww.fillcolour4 = settings_palette4_current[m]; ww.USEfillcolour = 0; ww.USEfillcolour1234 = 1; }

            // Fill icon
            eps_core_SetFillColour(x, &ww);
            eps_core_SwitchTo_FillColour(x,1);
            IF_NOT_INVISIBLE
             {
              fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath fill\n", h, v-s, h, v+s, h+2*s, v+s, h+2*s, v-s);
              eps_core_BoundingBox(x, h    , v-s, 0);
              eps_core_BoundingBox(x, h    , v+s, 0);
              eps_core_BoundingBox(x, h+2*s, v-s, 0);
              eps_core_BoundingBox(x, h+2*s, v+s, 0);
             }

            // Stroke icon
            eps_core_SetColour(x, &ww, 1);
            eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * ww.linewidth, ww.USElinetype ? ww.linetype : 1, 0);
            IF_NOT_INVISIBLE
             {
              fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath stroke\n", h, v-s, h, v+s, h+2*s, v+s, h+2*s, v-s);
              eps_core_BoundingBox(x, h    , v-s, ww.linewidth);
              eps_core_BoundingBox(x, h    , v+s, ww.linewidth);
              eps_core_BoundingBox(x, h+2*s, v-s, ww.linewidth);
              eps_core_BoundingBox(x, h+2*s, v+s, ww.linewidth);
             }

            // Write text next to icon
            eps_core_SetColour(x, &ww_txt, 1);
            canvas_EPSRenderTextItem(x, NULL, pageno, (h+st)/M_TO_PS, v/M_TO_PS, SW_HALIGN_LEFT, SW_VALIGN_CENT, x->CurrentColour, x->current->settings.FontSize, 0.0, NULL, NULL);
            key_vpos += TextHeight[ItemNo];
           }
         }
        angle += size;
        ItemNo++;
       }
      blk=blk->next;
     }

END_LABEL_PRINTING:

    // Title of piechart
    x->LaTeXpageno = x->current->TitleTextID;
    if ((x->current->settings.title != NULL) && (x->current->settings.title[0] != '\0'))
     {
      int pageno = x->LaTeXpageno++;
      canvas_EPSRenderTextItem(x, NULL, pageno, xpos/M_TO_PS, (vtitle+8)/M_TO_PS, SW_HALIGN_CENT, SW_VALIGN_BOT, x->CurrentColour, x->current->settings.FontSize, 0.0, NULL, NULL);
     }
   }

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

