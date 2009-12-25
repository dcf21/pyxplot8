// eps_plot.c
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

#define _PPL_EPS_PLOT 1

#include <stdlib.h>
#include <stdio.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "ppl_datafile.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

#include "eps_comm.h"
#include "eps_plot.h"
#include "eps_plot_styles.h"
#include "eps_plot_ticking.h"
#include "eps_settings.h"

void eps_plot_ReadAccessibleData(EPSComm *x)
 {
  int              i, j, Ndatasets, Fcounter=0, Dcounter=0, status, ErrCount, NExpect;
  canvas_plotdesc *pd;
  settings_axis   *axes;
  List            *UsingList, *EveryList;
  Dict            *tempdict;
  char             tempbuff[100];
  with_words       ww_default;

  // First clear all range information from all axes
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==0) axes = x->current->YAxes;
    else if (j==0) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     {
      axes[i].MinUsedSet = axes[i].MaxUsedSet = axes[i].DataUnitSet = axes[i].RangeFinalised = axes[i].FinalActive = 0;
      axes[i].MinUsed    = axes[i].MaxUsed    = axes[i].MinFinal = axes[i].MaxFinal = 0.0;
      axes[i].OrdinateRasterLen = 0;
      axes[i].OrdinateRaster = NULL;
      axes[i].TickListFinal  = NULL;
      ppl_units_zero(&axes[i].DataUnit);
     }
   }

  // Count number of datasets which we are plotting
  pd = x->current->plotitems;
  Ndatasets = 0;
  while (pd != NULL) { pd=pd->next; Ndatasets++; }

  // Malloc pointers to data tables where data to be plotted will be stored
  if (Ndatasets>0)
   {
    x->current->plotdata = (DataTable **)lt_malloc(Ndatasets * sizeof(DataTable *));
    if (x->current->plotdata == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *(x->status) = 1; return; }
   } else {
    x->current->plotdata = NULL;
   }

  // Loop through all datasets
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL)
   {
    // Merge together with words to form a final set
    eps_withwords_default(&ww_default, pd->function, Fcounter, Dcounter, settings_term_current.colour==SW_ONOFF_ON);
    if (pd->functions == 0) { Fcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1); }
    else                    { Dcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.DataStyle, &ww_default, NULL, NULL, 1); }

    // If plotting a datafile, can read in data now, so do so
    if (pd->functions == 0)
     {
      UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); sprintf(tempbuff+j*10,"%d",pd->EveryList[j]); DictAppendPtr(tempdict, "every_item", tempbuff+j*10, 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, tempdict, 0, 0, DATATYPE_VOID); }
      status   = 0;
      ErrCount = DATAFILE_NERRS;
      NExpect  = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);

      // Read data from file
      DataFile_read(x->current->plotdata+i, &status, temp_err_string, pd->filename, pd->index, pd->UsingRowCols, UsingList, EveryList, NULL, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
      if (status) { ppl_error(ERR_GENERAL, temp_err_string); x->current->plotdata[i]=NULL; }
      else
       {
        // Update axes to reflect usage
        status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &x->current->XAxes[pd->axisX], &x->current->YAxes[pd->axisY], &x->current->ZAxes[pd->axisZ], pd);
        if (status) { *(x->status) = 1; return; }
       }
     }
    pd=pd->next; i++;
   }
  return;
 }

void eps_plot_DecideAxisRanges(EPSComm *x)
 {
  int i, j, k, l;
  settings_axis *axes;
  canvas_plotrange *pr;
  double *HardMin, *HardMax;
  unsigned char HardAutoMin, HardAutoMax; // Set for "plot [:*]", where * says we must autoscale, even if there's a preexisting maximum set for the axis

  // Decide the range of each axis in turn
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==0) axes = x->current->YAxes;
    else if (j==0) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     {
      k = 3*(i-1) + j; // See if user has specified a range for this axis in the plot command itself
      if (k<0) { HardMin = HardMax = NULL; } // Ignore axis zero...
      else
       {
        pr = x->current->plotranges;
        for (l=0; ((pr!=NULL)&&(l<k)); l++) pr=pr->next;
        if (pr == NULL) { HardMin=HardMax=NULL; HardAutoMin=HardAutoMax=0; }
        else
         {
          if ((pr->MinSet || pr->MaxSet) && (axes[i].DataUnitSet) && (!ppl_units_DimEqual(&axes[i].DataUnit, &pr->unit))) { sprintf(temp_err_string, "The range specified for axis %c%d has conflicting units with the data plotting on that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[j], i, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axes[i].DataUnit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
          // Check if we have partial range which conflicts with units of range of axis
          if (pr->AutoMinSet) { HardAutoMin = 1; }
          if (pr->AutoMaxSet) { HardAutoMax = 1; }
          if (pr->MinSet)     { HardMin = &pr->min; } else { HardMin = NULL; }
          if (pr->MaxSet)     { HardMax = &pr->max; } else { HardMax = NULL; }
         }
       }
      eps_plot_ticking(axes+i, HardMin, HardMax, HardAutoMin, HardAutoMax);
     }
   }
  return;
 }

void eps_plot_LinkedAxesPropagate(EPSComm *x)
 {
  return;
 }

void eps_plot_SampleFunctions(EPSComm *x)
 {
  return;
 }

void eps_plot_YieldUpText(EPSComm *x)
 {
  return;
 }

void eps_plot_RenderEPS(EPSComm *x)
 {
  fprintf(x->epsbuffer, "%% Canvas item %d [plot]\n", x->current->id);
  return;
 }

