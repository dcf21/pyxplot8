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
#include "eps_plot_axespaint.h"
#include "eps_plot_styles.h"
#include "eps_plot_ticking.h"
#include "eps_settings.h"

// Loop through all of the datasets plotted in a single plot command.
// Initialise the datastructures for the plot command which we will fill in the
// process of deciding how to render the graph. Then read in data from
// datafiles and parametric functions. Do not read in data from non-parametric
// functions as we need to finalise axis ranges first, before we know what
// raster we will sample them on.

void eps_plot_ReadAccessibleData(EPSComm *x)
 {
  int              i, j, Ndatasets, Fcounter=0, Dcounter=0, status, ErrCount, NExpect;
  canvas_plotdesc *pd;
  settings_axis   *axes, *axissets[3];
  List            *UsingList, *EveryList;
  Dict            *tempdict;
  char             tempbuff[100];
  with_words       ww_default;
  double          *ordinate_raster;

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

  // First clear all range information from all axes
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==1) axes = x->current->YAxes;
    else if (j==2) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     {
      axes[i].MinUsedSet = axes[i].MaxUsedSet = axes[i].DataUnitSet = axes[i].RangeFinalised = axes[i].FinalActive = 0;
      axes[i].MinUsed    = axes[i].MaxUsed    = axes[i].MinFinal = axes[i].MaxFinal = 0.0;
      axes[i].OrdinateRasterLen = 0;
      axes[i].OrdinateRaster = NULL;
      axes[i].TickListFinalised = 0;
      axes[i].TickListPositions = axes[i].MTickListPositions = NULL;
      axes[i].TickListStrings   = axes[i].MTickListStrings   = NULL;
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
    if (pd->function != 0) { Fcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1); }
    else                   { Dcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.DataStyle, &ww_default, NULL, NULL, 1); }

    // Mark up axes which are going to be used for any dataset, from datafile or functions
    axissets[pd->axis1xyz][pd->axis1].FinalActive = 1;
    axissets[pd->axis2xyz][pd->axis2].FinalActive = 1;
    axissets[pd->axis3xyz][pd->axis3].FinalActive = 1;

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
        if (DEBUG) { sprintf(temp_err_string, "Reading data from file '%s' for dataset %d in plot item %d", pd->filename, i+1, x->current->id); ppl_log(temp_err_string); }
        DataFile_read(x->current->plotdata+i, &status, temp_err_string, pd->filename, pd->index, pd->UsingRowCols, UsingList, EveryList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
       } else {
        if (DEBUG) { sprintf(temp_err_string, "Reading data from parametric functions for dataset %d in plot item %d", i+1, x->current->id); ppl_log(temp_err_string); }
        DataFile_FromFunctions(ordinate_raster, 1, x->current->settings.samples, &settings_graph_current.Tmin, x->current->plotdata+i, &status, temp_err_string, pd->functions, pd->NFunctions, UsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
       }
      if (status) { ppl_error(ERR_GENERAL, temp_err_string); x->current->plotdata[i]=NULL; }
      else
       {
        // Update axes to reflect usage
        status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2], &axissets[pd->axis3xyz][pd->axis3], pd->axis1xyz, pd->axis2xyz, pd->axis3xyz, pd->axis1, pd->axis2, pd->axis3, x->current->id);
        if (status) { *(x->status) = 1; return; }
        eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis1xyz][pd->axis1], pd->axis1xyz, pd->axis1);
        eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis2xyz][pd->axis2], pd->axis2xyz, pd->axis2);
        eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis3xyz][pd->axis3], pd->axis3xyz, pd->axis3);
       }
     }
    pd=pd->next; i++;
   }
  return;
 }

// Whenever we update the usage variables MinUsed and MaxUsed for an axis, this
// procedure is called. It checks whether the axis is linked, and if so,
// updates the usage variables for the axis which it is linked to. This process
// may then iteration down a hierarchy of linked axes. As a rule, it is the
// axis at the bottom of the linkage hierarchy (i.e. at the end of the linked
// list) that has the canonical usage variables. Axes further up may not have
// complete information about the usage of the set of linked axes, since usage
// does not propagate UP the hierarchy.

void eps_plot_LinkedAxisBackPropagate(EPSComm *x, settings_axis *source, int xyz, int axis_n)
 {
  int            IterDepth;
  settings_axis *target;
  canvas_item   *item;

  if (DEBUG) { sprintf(temp_err_string, "Back-propagating axis usage for axis %c%d on plot %d", "xyz"[xyz], axis_n, x->current->id); }

  // Propagating MinUsed and MaxUsed variables along links between axes
  IterDepth = 0;
  while (1) // loop over as many iterations of linkage as may be necessary
   {
    if (IterDepth++ > 100) return;
    if (!(source->linked && (source->MinUsedSet || source->MaxUsedSet))) { break; } // proceed only if axis is linked and has usage information
    item = x->itemlist->first;
    while ((item != NULL) && ((item->id)<source->LinkedAxisCanvasID)) item=item->next;
    if ((item == NULL) || (item->id != source->LinkedAxisCanvasID)) { sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but no such plot exists.","xyz"[xyz],axis_n,x->current->id,"xyz"[source->LinkedAxisToXYZ],source->LinkedAxisToNum,source->LinkedAxisCanvasID); ppl_warning(ERR_GENERAL, temp_err_string); break; }
    if      (source->LinkedAxisToXYZ == 0) target = item->XAxes + source->LinkedAxisToNum;
    else if (source->LinkedAxisToXYZ == 1) target = item->YAxes + source->LinkedAxisToNum;
    else                                   target = item->ZAxes + source->LinkedAxisToNum;
    if ((target->MinUsedSet || target->MaxUsedSet) && (!ppl_units_DimEqual(&target->DataUnit , &source->DataUnit)))
     {
      sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but axes have data plotted against them with conflicting physical units. The former has units of <%s> whilst the latter has units of <%s>.","xyz"[xyz],axis_n,x->current->id,"xyz"[source->LinkedAxisToXYZ],source->LinkedAxisToNum,source->LinkedAxisCanvasID,ppl_units_GetUnitStr(&target->DataUnit,NULL,NULL,0,0),ppl_units_GetUnitStr(&source->DataUnit,NULL,NULL,1,0));
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
  return;
 }

// Loop through all of the axes on a single plot item, and decide its range. If
// the axis is a linked axis, must first decide the range of the axis at the
// bottom of the linkage hierarchy. The function
// eps_plot_LinkedAxisForwardPropagate deals with this.

void eps_plot_DecideAxisRanges(EPSComm *x)
 {
  int i, j;
  settings_axis *axes;

  // Decide the range of each axis in turn
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==1) axes = x->current->YAxes;
    else if (j==2) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     {
      if (!axes[i].RangeFinalised   ) { eps_plot_LinkedAxisForwardPropagate(x, &axes[i], j, i); if (*x->status) return; }
      if (!axes[i].TickListFinalised) { eps_plot_ticking(&axes[i], j, i, x->current->id, NULL, NULL, 0, 0); if (*x->status) return; }
     }
   }
  return;
 }

// Finalises the range of a particular axis, bringing together usage
// information, ranges specified in the plot command, and ranges set for the
// axis with the set xrange command.

void eps_plot_DecideAxisRange(EPSComm *x, settings_axis *axis, int xyz, int axis_n)
 {
  int k, l;
  canvas_plotrange *pr;
  double *HardMin, *HardMax;
  unsigned char HardAutoMin, HardAutoMax; // Set for "plot [:*]", where * says we must autoscale, even if there's a preexisting maximum set for the axis

  // Decide the range of each axis in turn
  k = 3*(axis_n-1) + xyz; // See if user has specified a range for this axis in the plot command itself
  if (k<0) { HardMin = HardMax = NULL; } // Ignore axis zero...
  else
   {
    pr = x->current->plotranges;
    for (l=0; ((pr!=NULL)&&(l<k)); l++) pr=pr->next;
    if (pr == NULL) { HardMin=HardMax=NULL; HardAutoMin=HardAutoMax=0; }
    else
     {
      if ((pr->MinSet || pr->MaxSet) && (axis->DataUnitSet) && (!ppl_units_DimEqual(&axis->DataUnit, &pr->unit))) { sprintf(temp_err_string, "The range specified for axis %c%d has conflicting units with the data plotting on that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->DataUnit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      if ((pr->MinSet && (!pr->MaxSet)) && (axis->MaxSet) && (!ppl_units_DimEqual(&axis->unit, &pr->unit))) { sprintf(temp_err_string, "The minimum limit specified for axis %c%d in the plot command has conflicting units with the maximum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->unit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      if (((!pr->MinSet) && pr->MaxSet) && (axis->MinSet) && (!ppl_units_DimEqual(&axis->unit, &pr->unit))) { sprintf(temp_err_string, "The maximum limit specified for axis %c%d in the plot command has conflicting units with the minimum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->unit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      if ((axis->MinSet || axis->MaxSet) && (axis->DataUnitSet) && (!ppl_units_DimEqual(&axis->unit, &axis->DataUnit))) { sprintf(temp_err_string, "The range specified for axis %c%d has conflicting units with the data plotting on that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&axis->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->DataUnit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      // Check if we have partial range which conflicts with units of range of axis
      if (pr->AutoMinSet) { HardAutoMin = 1; }
      if (pr->AutoMaxSet) { HardAutoMax = 1; }
      if (pr->MinSet)     { HardMin = &pr->min; } else { HardMin = NULL; }
      if (pr->MaxSet)     { HardMax = &pr->max; } else { HardMax = NULL; }
     }
   }
  eps_plot_ticking(axis, xyz, axis_n, x->current->id, HardMin, HardMax, HardAutoMin, HardAutoMax);
  return;
 }

// As part of the process of determining the range of axis xyz[axis_n], check
// whether the axis is linking, and if so fetch usage information from the
// bottom of the linkage hierarchy. Propagate this information up through all
// intermediate levels of the hierarchy before calling
// eps_plot_DecideAxisRange().

void eps_plot_LinkedAxisForwardPropagate(EPSComm *x, settings_axis *axis, int xyz, int axis_n)
 {
  int            IterDepth, target_xyz, target_axis_n;
  settings_axis *source, *source2, *target, *target2;
  canvas_item   *item;
  
  // Propagate MinUsed and MaxUsed variables along links
  IterDepth     = 0;
  target        = axis;
  target_xyz    = xyz;
  target_axis_n = axis_n;
  while (1) // loop over as many iterations of linkage as may be necessary to find MinFinal and MaxFinal at the bottom
   {
    if (IterDepth++ > 100) return;
    if ((!target->linked) || target->RangeFinalised) { break; } // proceed only if axis is linked
    item = x->itemlist->first;
    while ((item != NULL) && (item->id)<target->LinkedAxisCanvasID) item=item->next;
    if ((item == NULL) || (item->id != target->LinkedAxisCanvasID)) { break; }
    if      (target->LinkedAxisToXYZ == 0) target2 = item->XAxes + target->LinkedAxisToNum;
    else if (target->LinkedAxisToXYZ == 1) target2 = item->YAxes + target->LinkedAxisToNum;
    else                                   target2 = item->ZAxes + target->LinkedAxisToNum;
    if (!ppl_units_DimEqual(&target->DataUnit , &target2->DataUnit)) break; // If axes are dimensionally incompatible, stop
    target_xyz    = target->LinkedAxisToXYZ;
    target_axis_n = target->LinkedAxisToNum;
    target        = target2;
   }
  if (!target->RangeFinalised) eps_plot_DecideAxisRange(x, target, target_xyz, target_axis_n);
  if (*x->status) return;
  IterDepth = 0;
  source = axis;
  while (1) // loop over as many iterations of linkage as may be necessary
   {
    if (IterDepth++ > 100) return;
    if (source->RangeFinalised) { break; }
    source->MinFinal       = target->MinFinal;
    source->MaxFinal       = target->MaxFinal;
    source->RangeFinalised = 1;
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
  return;
 }

// Loop through all of the datasets plotting within a single plot command, and
// pick out those which are plotting functions which need to be rasterised
// along an ordinate axis (i.e. those which are not parametric). At this stage,
// we need to finalise the range of the ordinate axis, create a raster of
// values along it, and sample the function. After calling this function, all
// datasets plotted by the plot command will then have been sampled and be
// ready for plotting.

void eps_plot_SampleFunctions(EPSComm *x)
 {
  int              i, j, status, ErrCount, NExpect;
  canvas_plotdesc *pd;
  settings_axis   *OrdinateAxis, *axissets[3];
  List            *UsingList, *EveryList;
  Dict            *tempdict;
  char             tempbuff[100];

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

  // Loop through all datasets
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL)
   {
    if ((pd->function == 1) && (pd->parametric == 0))
     {
      UsingList    = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, tempdict, 0, 0, DATATYPE_VOID); }
      EveryList    = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); sprintf(tempbuff+j*10,"%d",pd->EveryList[j]); DictAppendPtr(tempdict, "every_item", tempbuff+j*10, 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, tempdict, 0, 0, DATATYPE_VOID); }
      status       = 0;
      ErrCount     = DATAFILE_NERRS;
      NExpect      = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);
      OrdinateAxis = &axissets[pd->axis1xyz][pd->axis1];

      // Fix range of ordinate axis
      eps_plot_LinkedAxisForwardPropagate(x, OrdinateAxis, pd->axis1xyz, pd->axis1);
      if (*x->status) return;

      // Make ordinate raster if we don't already have one
      if (OrdinateAxis->OrdinateRaster == NULL)
       {
        OrdinateAxis->OrdinateRaster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
        if (OrdinateAxis->OrdinateRaster == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *(x->status) = 1; return; }
        if (OrdinateAxis->log == SW_BOOL_TRUE) LogarithmicRaster(OrdinateAxis->OrdinateRaster, OrdinateAxis->MinFinal, OrdinateAxis->MaxFinal, x->current->settings.samples);
        else                            LinearRaster     (OrdinateAxis->OrdinateRaster, OrdinateAxis->MinFinal, OrdinateAxis->MaxFinal, x->current->settings.samples);
        OrdinateAxis->OrdinateRasterLen = x->current->settings.samples;
       }

      if (DEBUG) { sprintf(temp_err_string, "Reading data from functions for dataset %d in plot item %d", i+1, x->current->id); ppl_log(temp_err_string); }

      // Get data from functions
      DataFile_FromFunctions(OrdinateAxis->OrdinateRaster, 0, OrdinateAxis->OrdinateRasterLen, &OrdinateAxis->DataUnit, x->current->plotdata+i, &status, temp_err_string, pd->functions, pd->NFunctions, UsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
      if (status) { *(x->status) = 1; return; }

      // Update axes to reflect usage
      status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2], &axissets[pd->axis3xyz][pd->axis3], pd->axis1xyz, pd->axis2xyz, pd->axis3xyz, pd->axis1, pd->axis2, pd->axis3, x->current->id);
      if (status) { *(x->status) = 1; return; }
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis1xyz][pd->axis1], pd->axis1xyz, pd->axis1);
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis2xyz][pd->axis2], pd->axis2xyz, pd->axis2);
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis3xyz][pd->axis3], pd->axis3xyz, pd->axis3);
     }
    pd=pd->next; i++;
   }
  return;
 }

// Adds all of the text items which will be needed to render this plot to the
// list x->TextItems. It is vital that they be added in the exact order in
// which they will be rendered.

#define YIELD_TEXTITEM(X) \
  if ((X != NULL) && (X[0]!='\0')) \
   { \
    i = (CanvasTextItem *)lt_malloc(sizeof(CanvasTextItem)); \
    if (i==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); *(x->status) = 1; return; } \
    i->text              = X; \
    i->CanvasMultiplotID = x->current->id; \
    ListAppendPtr(x->TextItems, i, sizeof(CanvasTextItem), 0, DATATYPE_VOID); \
   }

void eps_plot_YieldUpText(EPSComm *x)
 {
  int             j, k, l, m;
  settings_axis  *axes;
  CanvasTextItem *i;

  // Axis labels and titles
  for (j=0; j<2+(x->current->ThreeDim); j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==1) axes = x->current->YAxes;
    else           axes = x->current->ZAxes;

    for (k=0; k<MAX_AXES; k++)
     if ((axes[k].FinalActive) && (!axes[k].invisible))
      {
       for (m=0; m<1+(axes[k].MirrorType == SW_AXISMIRROR_FULLMIRROR); m++) // Create second copy for mirrored axis if required
        {
         if (axes[k]. TickListPositions != NULL) for (l=0; axes[k]. TickListStrings[l]!=NULL; l++) { YIELD_TEXTITEM(axes[k]. TickListStrings[l]); } // Major tick labels
         if (axes[k].MTickListPositions != NULL) for (l=0; axes[k].MTickListStrings[l]!=NULL; l++) { YIELD_TEXTITEM(axes[k].MTickListStrings[l]); } // Minor tick labels
         YIELD_TEXTITEM(axes[k].label);
        }
      }
   }

  // Title of plot
  YIELD_TEXTITEM(x->current->settings.title);

  return;
 }

// Finally, produce a postscript plot

void eps_plot_RenderEPS(EPSComm *x)
 {
  int              i, status, xyzaxis[3];
  double           origin_x, origin_y, width, height;
  canvas_plotdesc *pd;
  settings_axis   *a1, *a2, *a3, *axissets[3];

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

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

  // Render each dataset in turn
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL) // loop over all datasets
   {
    a1 = &axissets[pd->axis1xyz][pd->axis1];
    a2 = &axissets[pd->axis2xyz][pd->axis2];
    a3 = &axissets[pd->axis3xyz][pd->axis3];
    xyzaxis[pd->axis1xyz] = 0;
    xyzaxis[pd->axis2xyz] = 1;
    xyzaxis[pd->axis3xyz] = 2;

    status = eps_plot_dataset(x, x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, a1, a2, a3, xyzaxis[0], xyzaxis[1], xyzaxis[2], &x->current->settings, pd, origin_x, origin_y, width, height);
    if (status) { *(x->status) = 1; return; }

    pd=pd->next; i++;
   }

  // Render axes
  eps_plot_axespaint(x, origin_x, origin_y, width, height);

  // Render legend

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

