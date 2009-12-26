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

#include "MathsTools/dcfmath.h"

#include "ppl_datafile.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

#include "eps_comm.h"
#include "eps_core.h"
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
  double          *ordinate_raster;

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

  // Make raster on which to evaluate parametric functions
  ordinate_raster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
  if (x->current->settings.Tlog == SW_BOOL_TRUE) LogarithmicRaster(ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);
  else                                           LinearRaster     (ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);

  // Loop through all datasets
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL)
   {
    // Merge together with words to form a final set
    eps_withwords_default(&ww_default, pd->function, Fcounter, Dcounter, settings_term_current.colour==SW_ONOFF_ON);
    if (pd->function == 0) { Fcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1); }
    else                   { Dcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.DataStyle, &ww_default, NULL, NULL, 1); }

    // Mark up axes which are going to be used for any dataset, from datafile or functions
    x->current->XAxes[pd->axisX].FinalActive = 1;
    x->current->YAxes[pd->axisY].FinalActive = 1;
    x->current->ZAxes[pd->axisZ].FinalActive = 1;

    // If plotting a datafile, can read in data now, so do so
    if ((pd->function == 0) || (pd->parametric == 1))
     {
      UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); sprintf(tempbuff+j*10,"%d",pd->EveryList[j]); DictAppendPtr(tempdict, "every_item", tempbuff+j*10, 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, tempdict, 0, 0, DATATYPE_VOID); }
      status   = 0;
      ErrCount = DATAFILE_NERRS;
      NExpect  = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);

      if (pd->function == 0) // Read data from file
       {
        DataFile_read(x->current->plotdata+i, &status, temp_err_string, pd->filename, pd->index, pd->UsingRowCols, UsingList, EveryList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
       } else {
        DataFile_FromFunctions(ordinate_raster, 1, x->current->settings.samples, &settings_graph_current.Tmin, x->current->plotdata+i, &status, temp_err_string, pd->functions, pd->NFunctions, UsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
       }
      if (status) { ppl_error(ERR_GENERAL, temp_err_string); x->current->plotdata[i]=NULL; }
      else
       {
        // Update axes to reflect usage
        status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &x->current->XAxes[pd->axisX], &x->current->YAxes[pd->axisY], &x->current->ZAxes[pd->axisZ], pd->axisX, pd->axisY, pd->axisZ, x->current->id);
        if (status) { *(x->status) = 1; return; }
       }
     }
    pd=pd->next; i++;
   }
  return;
 }

void eps_plot_LinkedAxesBackPropagate(EPSComm *x)
 {
  int            i, j, IterDepth;
  settings_axis *axes, *source, *target;
  canvas_item   *item;

  // Loop over all axes, propagating MinUsed and MaxUsed variables along links
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==0) axes = x->current->YAxes;
    else if (j==0) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     {
      IterDepth = 0;
      source = axes+i;
      while (1) // loop over as many iterations of linkage as may be necessary
       {
        if (IterDepth++ > 100) return;
        if (!(source->linked && (source->MinUsedSet || source->MaxUsedSet))) { break; } // proceed only if axis is linked and has usage information
        item = x->itemlist->first;
        while ((item != NULL) && ((item->id)<source->LinkedAxisCanvasID)) item=item->next;
        if ((item == NULL) || (item->id != source->LinkedAxisCanvasID)) { sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but no such plot exists.","xyz"[j],i,x->current->id,"xyz"[source->LinkedAxisToXYZ],source->LinkedAxisToNum,source->LinkedAxisCanvasID); ppl_warning(ERR_GENERAL, temp_err_string); break; }
        if      (source->LinkedAxisToXYZ == 0) target = item->XAxes + source->LinkedAxisToNum;
        else if (source->LinkedAxisToXYZ == 1) target = item->YAxes + source->LinkedAxisToNum;
        else                                   target = item->ZAxes + source->LinkedAxisToNum;
        if ((target->MinUsedSet || target->MaxUsedSet) && (!ppl_units_DimEqual(&target->DataUnit , &source->DataUnit)))
         {
          sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but axes have data plotted against them with conflicting physical units. The former has units of <%s> whilst the latter has units of <%s>.","xyz"[j],i,x->current->id,"xyz"[source->LinkedAxisToXYZ],source->LinkedAxisToNum,source->LinkedAxisCanvasID,ppl_units_GetUnitStr(&target->DataUnit,NULL,NULL,0,0),ppl_units_GetUnitStr(&source->DataUnit,NULL,NULL,1,0));
          ppl_warning(ERR_GENERAL, temp_err_string);
          break;
         }
        else
         {
          if ((!target->MinUsedSet) || (target->MinUsed > source->MinUsed)) { target->MinUsed=source->MinUsed; target->MinUsedSet=1; }
          if ((!target->MaxUsedSet) || (target->MaxUsed < source->MaxUsed)) { target->MaxUsed=source->MaxUsed; target->MaxUsedSet=1; }
          target->DataUnit = source->DataUnit;
         }
        source = target;
       }
     }
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
          if ((pr->MinSet && (!pr->MaxSet)) && (axes[i].MaxSet) && (!ppl_units_DimEqual(&axes[i].unit, &pr->unit))) { sprintf(temp_err_string, "The minimum limit specified for axis %c%d in the plot command has conflicting units with the maximum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[j], i, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axes[i].unit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
          if (((!pr->MinSet) && pr->MaxSet) && (axes[i].MinSet) && (!ppl_units_DimEqual(&axes[i].unit, &pr->unit))) { sprintf(temp_err_string, "The maximum limit specified for axis %c%d in the plot command has conflicting units with the minimum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[j], i, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axes[i].unit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
          if ((axes[i].MinSet || axes[i].MaxSet) && (axes[i].DataUnitSet) && (!ppl_units_DimEqual(&axes[i].unit, &axes[i].DataUnit))) { sprintf(temp_err_string, "The range specified for axis %c%d has conflicting units with the data plotting on that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[j], i, ppl_units_GetUnitStr(&axes[i].unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axes[i].DataUnit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
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

void eps_plot_LinkedAxesForwardPropagate(EPSComm *x)
 {
  int            i, j, IterDepth;
  settings_axis *axes, *source, *source2, *target, *target2;
  canvas_item   *item;
  
  // Loop over all axes, propagating MinUsed and MaxUsed variables along links
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==0) axes = x->current->YAxes;
    else if (j==0) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     if (!axes[i].linked) // proceed only if axis is linked
      {
       IterDepth = 0;
       target = axes+i;
       while (1) // loop over as many iterations of linkage as may be necessary to find MinFinal and MaxFinal at the bottom
        {
         if (IterDepth++ > 100) return;
         if (!target->linked) { break; } // proceed only if axis is linked
         item = x->itemlist->first;
         while ((item != NULL) && (item->id)<target->LinkedAxisCanvasID) item=item->next;
         if ((item == NULL) || (item->id != target->LinkedAxisCanvasID)) { break; }
         if      (target->LinkedAxisToXYZ == 0) target2 = item->XAxes + target->LinkedAxisToNum;
         else if (target->LinkedAxisToXYZ == 1) target2 = item->YAxes + target->LinkedAxisToNum;
         else                                   target2 = item->ZAxes + target->LinkedAxisToNum;
         if (!ppl_units_DimEqual(&target->DataUnit , &target2->DataUnit)) break; // If axes are dimensionally incompatible, stop
         target = target2;
        }
       IterDepth = 0;
       source = axes+i;
       while (1) // loop over as many iterations of linkage as may be necessary
        {
         if (IterDepth++ > 100) return;
         source->MinFinal      = target->MinFinal;
         source->MaxFinal      = target->MaxFinal;
         source->TickListFinal = target->TickListFinal;
         if (!source->linked) { break; } // proceed only if axis is linked
         item = x->itemlist->first;
         while ((item != NULL) && (item->id)<source->LinkedAxisCanvasID) item=item->next;
         if ((item == NULL) || (item->id != source->LinkedAxisCanvasID)) { break; }
         if      (source->LinkedAxisToXYZ == 0) source = item->XAxes + source->LinkedAxisToNum;
         else if (source->LinkedAxisToXYZ == 1) source = item->YAxes + source->LinkedAxisToNum;
         else                                   source = item->ZAxes + source->LinkedAxisToNum;
         if (!ppl_units_DimEqual(&source->DataUnit , &source2->DataUnit)) break; // If axes are dimensionally incompatible, stop
         source = source2;
        }
      }
   }
  return;
 }

void eps_plot_SampleFunctions(EPSComm *x)
 {
  int              i, j, status, ErrCount, NExpect;
  canvas_plotdesc *pd;
  settings_axis   *Xaxis;
  List            *UsingList, *EveryList;
  Dict            *tempdict;
  char             tempbuff[100];

  // Loop through all datasets
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL)
   {
    if ((pd->function == 1) && (pd->parametric == 0))
     {
      UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); sprintf(tempbuff+j*10,"%d",pd->EveryList[j]); DictAppendPtr(tempdict, "every_item", tempbuff+j*10, 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, tempdict, 0, 0, DATATYPE_VOID); }
      status   = 0;
      ErrCount = DATAFILE_NERRS;
      NExpect  = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);
      Xaxis    = x->current->XAxes + pd->axisX;

      // Make ordinate raster if we don't already have one
      if (Xaxis->OrdinateRaster == NULL)
       {
        Xaxis->OrdinateRaster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
        if (Xaxis->OrdinateRaster == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *(x->status) = 1; return; }
        if (Xaxis->log == SW_BOOL_TRUE) LogarithmicRaster(Xaxis->OrdinateRaster, Xaxis->MinFinal, Xaxis->MaxFinal, x->current->settings.samples);
        else                            LinearRaster     (Xaxis->OrdinateRaster, Xaxis->MinFinal, Xaxis->MaxFinal, x->current->settings.samples);
       }

      DataFile_FromFunctions(Xaxis->OrdinateRaster, 0, Xaxis->OrdinateRasterLen, &Xaxis->DataUnit, x->current->plotdata+i, &status, temp_err_string, pd->functions, pd->NFunctions, UsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
      if (status) { *(x->status) = 1; return; }
     }
    pd=pd->next; i++;
   }
  return;
 }

void eps_plot_YieldUpText(EPSComm *x)
 {
  return;
 }

void eps_plot_RenderEPS(EPSComm *x)
 {
  int i;
  double origin_x, origin_y, width, height;
  canvas_plotdesc *pd;

  // Write header at top of postscript
  fprintf(x->epsbuffer, "%% Canvas item %d [plot]\n", x->current->id);

  // Add four corners of graph to postscript bounding box, even if there are no axes
  origin_x = x->current->settings.OriginX.real * M_TO_PS;
  origin_y = x->current->settings.OriginY.real * M_TO_PS;
  width    = x->current->settings.width  .real * M_TO_PS;
  if (x->current->settings.AutoAspect == SW_ONOFF_ON) height = width * 2.0/(1.0+sqrt(5));
  else                                                height = width * x->current->settings.aspect;
  eps_core_BoundingBox(x, origin_x      , origin_y       , 0.0);
  eps_core_BoundingBox(x, origin_x+width, origin_y       , 0.0);
  eps_core_BoundingBox(x, origin_x      , origin_y+height, 0.0);
  eps_core_BoundingBox(x, origin_x+width, origin_y+height, 0.0);

  // Render gridlines

  // Render axes

  // Render each dataset in turn
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL) // loop over all datasets
   {
    pd=pd->next; i++;
   }

  // Render legend

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

