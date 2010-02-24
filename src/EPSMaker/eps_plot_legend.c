// eps_plot_legend.c
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

// This file contains routines for adding legends to plots

#define _PPL_EPS_PLOT_LEGEND_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "StringTools/asciidouble.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_texify.h"

#include "eps_colours.h"
#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_legend.h"
#include "eps_plot_styles.h"

void GraphLegend_YieldUpText(EPSComm *x)
 {
  canvas_plotdesc *pd;
  CanvasTextItem  *i;
  char *cptr, *buffer;
  int j, k, BracketLevel, inlen, status;

  if (x->current->settings.key != SW_ONOFF_ON) return;

  pd = x->current->plotitems;
  while (pd != NULL) // loop over all datasets
   {
    if      (pd->NoTitleSet) { pd=pd->next; continue; } // notitle set
    else if (pd->TitleSet  ) { pd->TitleFinal=pd->title; YIELD_TEXTITEM(pd->title); } // title for dataset manually set
    else // generate automatic title for dataset
     {
      pd->TitleFinal = cptr = (char *)lt_malloc(LSTR_LENGTH);
      buffer = (char *)lt_malloc(LSTR_LENGTH);
      if (buffer==NULL) cptr=NULL;
      if (cptr!=NULL)
       {
        k=0;
        if (pd->parametric) { sprintf(cptr+k, "parametric"); k+=strlen(cptr+k); }
        if (!pd->function) { cptr[k++]=' '; StrEscapify(pd->filename, buffer); inlen=strlen(buffer); BracketLevel=0; status=-1; texify_generic(buffer, &inlen, cptr+k, 0, &status, temp_err_string, 1, &BracketLevel); if (status>=0) cptr[k++]='?'; else k+=strlen(cptr+k); } // Filename of datafile we are plotting
        else
         for (j=0; j<pd->NFunctions; j++) // Print out the list of functions which we are plotting
          {
           cptr[k++]=(j!=0)?':':' ';
           inlen=strlen(pd->functions[j]); BracketLevel=0; status=-1;
           texify_generic(pd->functions[j], &inlen, cptr+k, 1, &status, temp_err_string, 1, &BracketLevel);
           if (status>=0) cptr[k++]='?'; else k+=strlen(cptr+k);
          }
        if (pd->ContinuitySet) // Print continuous / discontinuous flag
         {
          if (pd->continuity == DATAFILE_DISCONTINUOUS) { sprintf(cptr+k, " discontinuous"); k+=strlen(cptr+k); }
          else                                          { sprintf(cptr+k,    " continuous"); k+=strlen(cptr+k); }
         }
        if (pd->axis1set || pd->axis2set || pd->axis3set) // Print axes to use
         {           
          strcpy(cptr+k, " axes "); k+=strlen(cptr+k);
          if (pd->axis1set) { sprintf(cptr+k, "$%c%d$", "xyz"[pd->axis1xyz], pd->axis1); k+=strlen(cptr+k); }
          if (pd->axis2set) { sprintf(cptr+k, "$%c%d$", "xyz"[pd->axis2xyz], pd->axis2); k+=strlen(cptr+k); }
          if (pd->axis3set) { sprintf(cptr+k, "$%c%d$", "xyz"[pd->axis3xyz], pd->axis3); k+=strlen(cptr+k); }
         }
        if (pd->EverySet>0) { sprintf(cptr+k, " every $%d$", pd->EveryList[0]); k+=strlen(cptr+k); } // Print out 'every' clause of plot command
        if (pd->EverySet>1) { sprintf(cptr+k, ":$%d$", pd->EveryList[1]); k+=strlen(cptr+k); }
        if (pd->EverySet>2) { sprintf(cptr+k, ":$%d$", pd->EveryList[2]); k+=strlen(cptr+k); }
        if (pd->EverySet>3) { sprintf(cptr+k, ":$%d$", pd->EveryList[3]); k+=strlen(cptr+k); }
        if (pd->EverySet>4) { sprintf(cptr+k, ":$%d$", pd->EveryList[4]); k+=strlen(cptr+k); }
        if (pd->EverySet>5) { sprintf(cptr+k, ":$%d$", pd->EveryList[5]); k+=strlen(cptr+k); }
        if (pd->IndexSet) { sprintf(cptr+k, " index $%d$", pd->index); k+=strlen(cptr+k); } // Print index to use
        if (pd->label!=NULL) { sprintf(cptr+k, " label "); k+=strlen(cptr+k); inlen=strlen(pd->label); BracketLevel=0; status=-1; texify_generic(pd->label, &inlen, cptr+k, 0, &status, temp_err_string, 1, &BracketLevel); if (status>=0) cptr[k++]='?'; else k+=strlen(cptr+k); } // Print label string
        if (pd->SelectCriterion!=NULL) { sprintf(cptr+k, " select "); inlen=strlen(pd->SelectCriterion); BracketLevel=0; status=-1; texify_generic(pd->SelectCriterion, &inlen, cptr+k, 0, &status, temp_err_string, 1, &BracketLevel); if (status>=0) cptr[k++]='?'; else k+=strlen(cptr+k); } // Print select criterion
        if ((pd->NUsing>0)||(pd->UsingRowCols!=DATAFILE_COL))
         {
          sprintf(cptr+k, " using %s", (pd->UsingRowCols==DATAFILE_COL)?"":"rows"); k+=strlen(cptr+k); // Print using list
          for (j=0; j<pd->NUsing; j++)
           {
            cptr[k++]=(j!=0)?':':' ';
            inlen=strlen(pd->UsingList[j]); BracketLevel=0; status=-1;
            texify_generic(pd->UsingList[j], &inlen, cptr+k, 0, &status, temp_err_string, 1, &BracketLevel);
            if (status>=0) cptr[k++]='?'; else k+=strlen(cptr+k);
           }
         }
        cptr[k]='\0';
        YIELD_TEXTITEM(cptr);
       }
     }
    pd=pd->next;
   }
  return;
 }

#define LOOP_OVER_DATASETS \
  pd = x->current->plotitems; \
  while (pd != NULL) \
   { \
    if ((pd->NoTitleSet) || (pd->TitleFinal==NULL) || (pd->TitleFinal[0]=='\0')) { pd=pd->next; continue; } /* no title set */

#define END_LOOP_OVER_DATASETS \
    pd = pd->next; \
   }

// Lay out all of the items in the current legend with a maximum allowed column height of TrialHeight.
void GraphLegend_ArrangeToHeight(EPSComm *x, double TrialHeight, double *AttainedHeight, int *Ncolumns, double *ColumnX, double *ColumnHeight, int *ColumnNItems)
 {
  canvas_plotdesc *pd;
  int i, ColumnNo = 0;
  double ColumnYPos = 0.0, ColumnWidth = 0.0;

  *AttainedHeight = 0.0;
  for (i=0; i<MAX_LEGEND_COLUMNS; i++) ColumnNItems[i]=0;
  ColumnX[0]=0.0;
  LOOP_OVER_DATASETS;
    if ((ColumnYPos>0) && (pd->TitleFinal_height>TrialHeight-ColumnYPos) && (ColumnNo<MAX_LEGEND_COLUMNS-1)) { ColumnHeight[ColumnNo]=ColumnYPos; ColumnX[ColumnNo+1]=ColumnX[ColumnNo] + ColumnWidth; ColumnYPos=ColumnWidth=0.0; ColumnNo++; }
    ColumnNItems[ColumnNo]++;
    pd->TitleFinal_col  = ColumnNo;
    pd->TitleFinal_xpos = ColumnX[ColumnNo];
    pd->TitleFinal_ypos = -ColumnYPos; // Minus sign since postscript measures height from bottom, and legend runs down page
    ColumnYPos+=pd->TitleFinal_height;
    if (ColumnWidth < pd->TitleFinal_width) ColumnWidth=pd->TitleFinal_width;
    if (ColumnYPos > *AttainedHeight) *AttainedHeight=ColumnYPos;
  END_LOOP_OVER_DATASETS;
  ColumnHeight[ColumnNo]=ColumnYPos;
  ColumnX[ColumnNo+1] = ColumnX[ColumnNo] + ColumnWidth;
  *Ncolumns = ColumnNo+1;
  return;
 }

void GraphLegend_Render(EPSComm *x, double width, double height)
 {
  double fs=x->current->settings.FontSize, CombinedHeight=0.0, MinimumHeight=0.0;
  double xoff, yoff;
  double ColumnX[MAX_LEGEND_COLUMNS], ColumnHeight[MAX_LEGEND_COLUMNS];
  double BestHeight, AttainedHeight, TrialHeight;
  int    NDataSet, Ncolumns, ColumnNItems[MAX_LEGEND_COLUMNS];
  double height1,height2,bb_left,bb_right,bb_top,bb_bottom,ab_left,ab_right,ab_top,ab_bottom;
  unsigned char hfixed=0, vfixed=0;
  canvas_plotdesc *pd;
  int    pageno, j, kp=x->current->settings.KeyPos;
  postscriptPage *dviPage;
  with_words ww;

  if (x->current->settings.key != SW_ONOFF_ON) return;

  // Loop over all legend items to calculate their individual heights and widths, as well as the combined height of all of them
  pageno = x->LaTeXpageno = x->current->LegendTextID;
  LOOP_OVER_DATASETS;
    // Fetch dimensions of requested page of postscript
    if (x->dvi == NULL) { pd->TitleFinal_width=0; pd->TitleFinal_height=0; pd=pd->next; pageno++; continue; }
    dviPage = (postscriptPage *)ListGetItem(x->dvi->output->pages, pageno+1);
    if (dviPage== NULL) { pd->TitleFinal_width=0; pd->TitleFinal_height=0; pd=pd->next; pageno++; continue; } // Such doom will trigger errors later
    bb_left   = dviPage->boundingBox[0];
    bb_bottom = dviPage->boundingBox[1];
    bb_right  = dviPage->boundingBox[2];
    bb_top    = dviPage->boundingBox[3];
    ab_left   = dviPage->textSizeBox[0];
    ab_bottom = dviPage->textSizeBox[1];
    ab_right  = dviPage->textSizeBox[2];
    ab_top    = dviPage->textSizeBox[3];
    height1 = fabs(ab_top - ab_bottom) * AB_ENLARGE_FACTOR;
    height2 = fabs(bb_top - bb_bottom) * BB_ENLARGE_FACTOR;
    pd->TitleFinal_height = (height2<height1) ? height2 : height1;
    pd->TitleFinal_width  = (ab_right - ab_left) + MARGIN_HSIZE;
    CombinedHeight += pd->TitleFinal_height;
    if (MinimumHeight < pd->TitleFinal_height) MinimumHeight = pd->TitleFinal_height;
    pageno++;
  END_LOOP_OVER_DATASETS;

  // If number of columns is manually specified, repeatedly reduce height of legend until the desired number of columns is exceeded.
  if      (x->current->settings.KeyColumns > 0)
   {
    BestHeight = TrialHeight = CombinedHeight+2;
    while (TrialHeight>MinimumHeight)
     {
      GraphLegend_ArrangeToHeight(x, TrialHeight, &AttainedHeight, &Ncolumns, ColumnX, ColumnHeight, ColumnNItems);
      if (Ncolumns > x->current->settings.KeyColumns) break;
      if (AttainedHeight>TrialHeight) break;
      BestHeight = TrialHeight;
      TrialHeight = AttainedHeight-1;
     }
   }

  // In ABOVE and BELOW alignment modes, repeatedly reduce height of legend until its width exceeds that of the plot
  else if ((kp == SW_KEYPOS_ABOVE) || (kp == SW_KEYPOS_BELOW))
   {
    hfixed = 1;
    BestHeight = TrialHeight = CombinedHeight+2;
    while (TrialHeight>MinimumHeight)
     {
      GraphLegend_ArrangeToHeight(x, TrialHeight, &AttainedHeight, &Ncolumns, ColumnX, ColumnHeight, ColumnNItems);
      if (ColumnX[Ncolumns] > width-2*LEGEND_MARGIN) break;
      if (AttainedHeight>TrialHeight) break;
      BestHeight = TrialHeight;
      TrialHeight = AttainedHeight-1;
     }
   }

  // In all other modes, make maximum height of legend equal height of plot.
  else
   {
    vfixed = 1;
    BestHeight = height-2*LEGEND_MARGIN;
   }

  // Adjust spacing between legend items to justify them as necessary
  GraphLegend_ArrangeToHeight(x, BestHeight, &AttainedHeight, &Ncolumns, ColumnX, ColumnHeight, ColumnNItems);
  TrialHeight = vfixed ? (height-2*LEGEND_MARGIN) : AttainedHeight; // Vertical justification
  for (j=0; j<Ncolumns; j++) if ((ColumnNItems[j]<2) || (ColumnHeight[j] < TrialHeight-LEGEND_VGAP_MAXIMUM*(ColumnNItems[j]-1))) { TrialHeight = AttainedHeight; break; }
  for (j=0; j<Ncolumns; j++)
   if (ColumnHeight[j] >= TrialHeight-LEGEND_VGAP_MAXIMUM*(ColumnNItems[j]-1))
    {
     double GapPerItem = (TrialHeight-ColumnHeight[j])/(ColumnNItems[j]-1), gap=0.0;
     if (gsl_finite(GapPerItem))
      {
       LOOP_OVER_DATASETS; if (pd->TitleFinal_col==j) { pd->TitleFinal_ypos-=gap; gap+=GapPerItem; } END_LOOP_OVER_DATASETS;
      }
    }
  AttainedHeight = TrialHeight;
  // Horizonal justification
  if (hfixed && (ColumnX[Ncolumns] < width-2*LEGEND_MARGIN) && (Ncolumns>1) && (ColumnX[Ncolumns] >= width-2*LEGEND_MARGIN-LEGEND_HGAP_MAXIMUM*(Ncolumns-1)))
   {
    double GapPerColumn = (width - 2*LEGEND_MARGIN - ColumnX[Ncolumns])/Ncolumns;
    if (gsl_finite(GapPerColumn))
     {
      LOOP_OVER_DATASETS; pd->TitleFinal_xpos+=pd->TitleFinal_col*GapPerColumn; END_LOOP_OVER_DATASETS;
      for (j=0; j<Ncolumns; j++) ColumnX[j] += j*GapPerColumn;
      ColumnX[Ncolumns] += (Ncolumns-1)*GapPerColumn;
     }
   }
  
  // Translate legend to desired place on canvas (2D case)
  if (!x->current->ThreeDim)
   switch (kp)
    {
     case SW_KEYPOS_TR:      xoff = width   - ColumnX[Ncolumns]   - LEGEND_MARGIN; yoff = height                     - LEGEND_MARGIN; break;
     case SW_KEYPOS_TM:      xoff = width/2 - ColumnX[Ncolumns]/2                ; yoff = height                     - LEGEND_MARGIN; break;
     case SW_KEYPOS_TL:      xoff =                                 LEGEND_MARGIN; yoff = height                     - LEGEND_MARGIN; break;
     case SW_KEYPOS_MR:      xoff = width   - ColumnX[Ncolumns]   - LEGEND_MARGIN; yoff = height/2 + AttainedHeight/2               ; break;
     case SW_KEYPOS_MM:      xoff = width/2 - ColumnX[Ncolumns]/2                ; yoff = height/2 + AttainedHeight/2               ; break;
     case SW_KEYPOS_ML:      xoff =                                 LEGEND_MARGIN; yoff = height/2 + AttainedHeight/2               ; break;
     case SW_KEYPOS_BR:      xoff = width   - ColumnX[Ncolumns]   - LEGEND_MARGIN; yoff =            AttainedHeight  + LEGEND_MARGIN; break;
     case SW_KEYPOS_BM:      xoff = width/2 - ColumnX[Ncolumns]/2                ; yoff =            AttainedHeight  + LEGEND_MARGIN; break;
     case SW_KEYPOS_BL:      xoff =                                 LEGEND_MARGIN; yoff =            AttainedHeight  + LEGEND_MARGIN; break;
     case SW_KEYPOS_ABOVE:   xoff = width/2 - ColumnX[Ncolumns]/2                ; yoff = height   + AttainedHeight  + LEGEND_MARGIN; break;
     case SW_KEYPOS_BELOW:   xoff = width/2 - ColumnX[Ncolumns]/2                ; yoff =                            - LEGEND_MARGIN; break;
     case SW_KEYPOS_OUTSIDE: xoff = width                         + LEGEND_MARGIN; yoff = height                     - LEGEND_MARGIN; break;
    }

  // Translate legend to desired place on canvas (3D case)
  else
   {
   }

  xoff += (x->current->settings.OriginX.real + x->current->settings.KeyXOff.real) * M_TO_PS;
  yoff += (x->current->settings.OriginY.real + x->current->settings.KeyYOff.real) * M_TO_PS;
  LOOP_OVER_DATASETS;
  pd->TitleFinal_xpos += xoff;
  pd->TitleFinal_ypos += yoff;
  END_LOOP_OVER_DATASETS;

  // Finally loop over all datasets to display legend items
  NDataSet=0;
  LOOP_OVER_DATASETS
    int xyzaxis[3];
    settings_axis *a1, *a2, *a3, *axissets[3];
    axissets[0] = x->current->XAxes;
    axissets[1] = x->current->YAxes;
    axissets[2] = x->current->ZAxes;
    a1 = &axissets[pd->axis1xyz][pd->axis1];
    a2 = &axissets[pd->axis2xyz][pd->axis2];
    a3 = &axissets[pd->axis3xyz][pd->axis3];
    xyzaxis[pd->axis1xyz] = 0;
    xyzaxis[pd->axis2xyz] = 1;
    xyzaxis[pd->axis3xyz] = 2;
    eps_plot_LegendIcon(x, NDataSet, pd, pd->TitleFinal_xpos + MARGIN_HSIZE_LEFT/2, pd->TitleFinal_ypos - pd->TitleFinal_height/2, MARGIN_HSIZE_LEFT, a1, a2, a3, xyzaxis[0], xyzaxis[1], xyzaxis[2]);
    pageno = x->LaTeXpageno++;
    with_words_zero(&ww,0);
    if (x->current->settings.TextColour > 0) { ww.USEcolour = 1; ww.colour = x->current->settings.TextColour; }
    else                                     { ww.USEcolourRGB = 1; ww.colourR = x->current->settings.TextColourR; ww.colourG = x->current->settings.TextColourG; ww.colourB = x->current->settings.TextColourB; }
    eps_core_SetColour(x, &ww, 1);
    IF_NOT_INVISIBLE canvas_EPSRenderTextItem(x, pageno, (pd->TitleFinal_xpos+MARGIN_HSIZE_LEFT)/M_TO_PS, (pd->TitleFinal_ypos - pd->TitleFinal_height/2)/ M_TO_PS, SW_HALIGN_LEFT, SW_VALIGN_CENT, x->LastEPSColour, fs, 0.0, NULL, NULL);
   NDataSet++;
  END_LOOP_OVER_DATASETS
  return;
 }
