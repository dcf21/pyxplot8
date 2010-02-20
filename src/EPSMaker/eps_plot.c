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
#include "ppl_error.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot.h"
#include "eps_plot_axespaint.h"
#include "eps_plot_canvas.h"
#include "eps_plot_labelsarrows.h"
#include "eps_plot_legend.h"
#include "eps_plot_styles.h"
#include "eps_plot_threedimbuff.h"
#include "eps_plot_ticking.h"
#include "eps_settings.h"

// If a plot dataset has any with_words of the form "with linewidth $4", these
// need to be evaluated for every datapoint. We do this by adding additional
// items to the UsingList for these datasets. First, we need to check that the
// UsingList supplied by the user is of an acceptable form. If it is of the
// wrong length, we do nothing; it will fail in due course in ppl_datafile
// anyway. If the list is empty, we auto-generate a default list.

int eps_plot_AddUsingItemsForWithWords(with_words *ww, int *NExpect, List *UsingList)
 {
  int i, UsingLen;
  char *AutoItem, *temp, *temp2;
  Dict *tempdict;

  UsingLen = ListLen(UsingList);

  // If using list was empty, generate an automatic list before we start
  if (UsingLen==0)
   {
    for (i=0; i<*NExpect; i++)
     {
      AutoItem = (char *)lt_malloc(10);
      if (AutoItem == NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return 1; }
      sprintf(AutoItem, "%d", i+1);
      tempdict = DictInit();
      DictAppendPtr(tempdict, "using_item", (void *)AutoItem, 0, 0, DATATYPE_VOID);
      ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID);
     }
    UsingLen = *NExpect;
   }
  else if ((UsingLen==1) && (*NExpect==2)) // Prepend data point number if only one number specified in using statement
   {
    temp = (char *)ListPop(UsingList);
    tempdict = DictInit();
    temp2 = (char *)lt_malloc(2);
    if (temp2==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return 1; }
    strcpy(temp2, "0");
    DictAppendPtr(tempdict, "using_item", temp2, 0, 0, DATATYPE_VOID);
    ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID);
    ListAppendPtr(UsingList, (void *)temp, 0, 0, DATATYPE_VOID);
    UsingLen++;
   }

  // If using list is wrong length, give up and let ppl_datafile return an error
  if (UsingLen != *NExpect) return 0;

  // Now cycle through all with_words which can be item-specific
  if (ww->STRlinetype       != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRlinetype      , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRlinewidth      != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRlinewidth     , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRpointlinewidth != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRpointlinewidth, 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRpointsize      != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRpointsize     , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRpointtype      != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRpointtype     , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRcolourR        != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRcolourR       , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRcolourG        != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRcolourG       , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRcolourB        != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRcolourB       , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRfillcolourR    != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRfillcolourR   , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRfillcolourG    != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRfillcolourG   , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }
  if (ww->STRfillcolourB    != NULL) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)ww->STRfillcolourB   , 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); (*NExpect)++; }

  return 0;
 }

#define PROJ_DBL(X) \
 dbl = DataRow[i--]; \
 if (i<0) i=0; \
 if (!gsl_finite(dbl)) dbl=0.0;

#define PROJ0_255(X) \
 PROJ_DBL(X); \
 if (dbl <   0.0) dbl=  0.0; \
 if (dbl > 255.0) dbl=255.0;

#define PROJ_INT(X) \
 PROJ_DBL(X); \
 if (dbl < INT_MIN) dbl=INT_MIN+1; \
 if (dbl > INT_MAX) dbl=INT_MAX-1;

void eps_plot_WithWordsFromUsingItems(with_words *ww, double *DataRow, int Ncolumns)
 {
  int i = Ncolumns-1;
  double dbl;

  if (ww->STRfillcolourB    != NULL) { PROJ0_255(i); ww->USEfillcolourRGB  = 1; ww->fillcolourB    = (int)dbl; }
  if (ww->STRfillcolourG    != NULL) { PROJ0_255(i); ww->USEfillcolourRGB  = 1; ww->fillcolourG    = (int)dbl; }
  if (ww->STRfillcolourR    != NULL) { PROJ0_255(i); ww->USEfillcolourRGB  = 1; ww->fillcolourR    = (int)dbl; }
  if (ww->STRcolourB        != NULL) { PROJ0_255(i); ww->USEcolourRGB      = 1; ww->colourB        = (int)dbl; }
  if (ww->STRcolourG        != NULL) { PROJ0_255(i); ww->USEcolourRGB      = 1; ww->colourG        = (int)dbl; }
  if (ww->STRcolourR        != NULL) { PROJ0_255(i); ww->USEcolourRGB      = 1; ww->colourR        = (int)dbl; }
  if (ww->STRpointtype      != NULL) { PROJ_INT (i); ww->USEpointtype      = 1; ww->pointtype      = (int)dbl; }
  if (ww->STRpointsize      != NULL) { PROJ_DBL (i); ww->USEpointsize      = 1; ww->pointsize      =      dbl; }
  if (ww->STRpointlinewidth != NULL) { PROJ_DBL (i); ww->USEpointlinewidth = 1; ww->pointlinewidth =      dbl; }
  if (ww->STRlinewidth      != NULL) { PROJ_DBL (i); ww->USElinewidth      = 1; ww->linewidth      =      dbl; }
  if (ww->STRlinetype       != NULL) { PROJ_INT (i); ww->USElinetype       = 1; ww->linetype       = (int)dbl; }

  return;
 }

#define WWCUID(X) \
 if (!FirstValues[i].dimensionless) { sprintf(temp_err_string, "The expression specified for the %s should have been dimensionless, but instead had units of <%s>. Cannot plot this dataset.", X, ppl_units_GetUnitStr(FirstValues+i, NULL, NULL, 0, 0)); ppl_error(ERR_NUMERIC,temp_err_string); return 1; } \
 i--; \
 if (i<0) i=0;

int eps_plot_WithWordsCheckUsingItemsDimLess(with_words *ww, value *FirstValues, int Ncolumns)
 {
  int i = Ncolumns-1;

  if (ww->STRfillcolourB    != NULL) { WWCUID("blue component of the fillcolour"); }
  if (ww->STRfillcolourG    != NULL) { WWCUID("green component of the fillcolour"); }
  if (ww->STRfillcolourR    != NULL) { WWCUID("red component of the fillcolour"); }
  if (ww->STRcolourB        != NULL) { WWCUID("blue component of the colour"); }
  if (ww->STRcolourG        != NULL) { WWCUID("green component of the colour"); }
  if (ww->STRcolourR        != NULL) { WWCUID("red component of the colour"); }
  if (ww->STRpointtype      != NULL) { WWCUID("point type"); }
  if (ww->STRpointsize      != NULL) { WWCUID("point size"); }
  if (ww->STRpointlinewidth != NULL) { WWCUID("point line width"); }
  if (ww->STRlinewidth      != NULL) { WWCUID("line width"); }
  if (ww->STRlinetype       != NULL) { WWCUID("line type"); }
  return 0;
 }


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
  char             errbuffer[LSTR_LENGTH];
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
      axes[i].AxisValueTurnings = 0;
      axes[i].AxisLinearInterpolation = NULL;
      axes[i].MinUsedSet = axes[i].MaxUsedSet = axes[i].DataUnitSet = axes[i].RangeFinalised = axes[i].FinalActive = 0;
      axes[i].MinUsed    = axes[i].MaxUsed    = axes[i].MinFinal = axes[i].MaxFinal = 0.0;
      axes[i].OrdinateRasterLen = 0;
      axes[i].OrdinateRaster = NULL;
      axes[i].FinalAxisLabel = NULL;
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
    x->current->plotdata      = (DataTable **)lt_malloc(Ndatasets * sizeof(DataTable *));
    x->current->DatasetTextID = (int *)lt_malloc(Ndatasets * sizeof(int));
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
    pd->TitleFinal=NULL;
    pd->TitleFinal_col=0;
    pd->TitleFinal_height = pd->TitleFinal_width = pd->TitleFinal_xpos = pd->TitleFinal_ypos = 0;

    // Merge together with words to form a final set
    eps_withwords_default(&ww_default, &x->current->settings, pd->function, Fcounter, Dcounter, settings_term_current.colour==SW_ONOFF_ON);
    if (pd->function != 0) { Fcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1); }
    else                   { Dcounter++; with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.DataStyle, &ww_default, NULL, NULL, 1); }

    // Mark up axes which are going to be used for any dataset, from datafile or functions
    axissets[pd->axis1xyz][pd->axis1].FinalActive = 1;
    axissets[pd->axis2xyz][pd->axis2].FinalActive = 1;
    axissets[pd->axis3xyz][pd->axis3].FinalActive = 1;

    // If plotting a datafile, can read in data now, so do so
    if ((pd->function == 0) || (pd->parametric == 1))
     {
      UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "every_item", (void *)(pd->EveryList+j), 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      status   = 0;
      ErrCount = DATAFILE_NERRS;
      NExpect  = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);

      if (eps_plot_AddUsingItemsForWithWords(&pd->ww_final, &NExpect, UsingList)) { *(x->status) = 1; return; } // Add extra using items for, e.g. "linewidth $3".

      if (pd->function == 0) // Read data from file
       {
        if (DEBUG) { sprintf(temp_err_string, "Reading data from file '%s' for dataset %d in plot item %d", pd->filename, i+1, x->current->id); ppl_log(temp_err_string); }
        DataFile_read(x->current->plotdata+i, &status, errbuffer, pd->filename, pd->index, pd->UsingRowCols, UsingList, EveryList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
       } else {
        if (DEBUG) { sprintf(temp_err_string, "Reading data from parametric functions for dataset %d in plot item %d", i+1, x->current->id); ppl_log(temp_err_string); }
        DataFile_FromFunctions(ordinate_raster, 1, x->current->settings.samples, &settings_graph_current.Tmin, x->current->plotdata+i, &status, errbuffer, pd->functions, pd->NFunctions, UsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
       }
      if (status) { ppl_error(ERR_GENERAL, errbuffer); x->current->plotdata[i]=NULL; }
      else
       {
        // Update axes to reflect usage
        status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2], &axissets[pd->axis3xyz][pd->axis3], &x->current->settings, pd->axis1xyz, pd->axis2xyz, pd->axis3xyz, pd->axis1, pd->axis2, pd->axis3, x->current->id);
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
    if (source->LinkedAxisCanvasID <= 0)
     { item = x->current; } // Linked to an axis on the same graph
    else // Linked to an axis on a different canvas item
     {
      item = x->itemlist->first;
      while ((item != NULL) && ((item->id)<source->LinkedAxisCanvasID)) item=item->next;
      if ((item == NULL) || (item->id != source->LinkedAxisCanvasID)) { break; }
     }
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
  int            i, j;
  double         width, height;
  settings_axis *axes;

  // Work out lengths of x and y axes
  width    = x->current->settings.width.real;
  if (x->current->settings.AutoAspect == SW_ONOFF_ON) height = width * 2.0/(1.0+sqrt(5));
  else                                                height = width * x->current->settings.aspect;

  // Decide the range of each axis in turn
  for (j=0; j<3; j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==1) axes = x->current->YAxes;
    else if (j==2) axes = x->current->ZAxes;
    for (i=0; i<MAX_AXES; i++)
     {
      if (!axes[i].RangeFinalised   ) { eps_plot_LinkedAxisForwardPropagate(x, &axes[i], j, i); if (*x->status) return; }
      if (!axes[i].TickListFinalised) { eps_plot_ticking(&axes[i], j, i, x->current->id, (j==1)?height:width, x->current->settings.AxisUnitStyle, NULL, NULL, 0, 0); if (*x->status) return; }
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
  double *HardMin, *HardMax, width, height;
  unsigned char HardAutoMin, HardAutoMax; // Set for "plot [:*]", where * says we must autoscale, even if there's a preexisting maximum set for the axis

  // Decide the range of each axis in turn
  k = 3*(axis_n-1) + xyz; // See if user has specified a range for this axis in the plot command itself
  if (k<0) { HardMin = HardMax = NULL; HardAutoMin = HardAutoMax = 0; } // Ignore axis zero...
  else
   {
    pr = x->current->plotranges;
    for (l=0; ((pr!=NULL)&&(l<k)); l++) pr=pr->next;
    if (pr == NULL) { HardMin=HardMax=NULL; HardAutoMin=HardAutoMax=0; }
    else
     {
      // Check if we have partial range which conflicts with units of range of axis
      if ((pr->MinSet || pr->MaxSet) && (axis->DataUnitSet) && (!ppl_units_DimEqual(&axis->DataUnit, &pr->unit))) { sprintf(temp_err_string, "The range specified for axis %c%d has conflicting units with the data plotting on that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->DataUnit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      if ((pr->MinSet && (!pr->MaxSet)) && (axis->MaxSet==SW_BOOL_TRUE) && (!ppl_units_DimEqual(&axis->unit, &pr->unit))) { sprintf(temp_err_string, "The minimum limit specified for axis %c%d in the plot command has conflicting units with the maximum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->unit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      if (((!pr->MinSet) && pr->MaxSet) && (axis->MinSet==SW_BOOL_TRUE) && (!ppl_units_DimEqual(&axis->unit, &pr->unit))) { sprintf(temp_err_string, "The maximum limit specified for axis %c%d in the plot command has conflicting units with the minimum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->unit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }
      if (((axis->MinSet==SW_BOOL_TRUE) || (axis->MaxSet==SW_BOOL_TRUE)) && (axis->DataUnitSet) && (!ppl_units_DimEqual(&axis->unit, &axis->DataUnit))) { sprintf(temp_err_string, "The range specified for axis %c%d has conflicting units with the data plotting on that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&axis->unit,NULL,NULL,0,0), ppl_units_GetUnitStr(&axis->DataUnit,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *(x->status) = 1; return; }

      // Read information about axis range out of list of ranges supplied to the plot command, ready to pass to eps_plot_ticking
      if (pr->AutoMinSet) { HardAutoMin = 1; }
      if (pr->AutoMaxSet) { HardAutoMax = 1; }
      if (pr->MinSet)     { HardMin = &pr->min; } else { HardMin = NULL; }
      if (pr->MaxSet)     { HardMax = &pr->max; } else { HardMax = NULL; }
     }

    // Update axis->DataUnit to be the canonical reference for the units of the axis
    if (!axis->DataUnitSet)
     {
      if      ((HardMin     !=NULL        ) || (HardMax     !=NULL        )) { axis->DataUnitSet=1; axis->DataUnit=pr  ->unit; }
      else if ((axis->MinSet==SW_BOOL_TRUE) || (axis->MaxSet==SW_BOOL_TRUE)) { axis->DataUnitSet=1; axis->DataUnit=axis->unit; }
     }
   }

  // Work out lengths of x and y axes
  width    = x->current->settings.width.real;
  if (x->current->settings.AutoAspect == SW_ONOFF_ON) height = width * 2.0/(1.0+sqrt(5));
  else                                                height = width * x->current->settings.aspect;

  eps_plot_ticking(axis, xyz, axis_n, x->current->id, (xyz==1)?height:width, x->current->settings.AxisUnitStyle, HardMin, HardMax, HardAutoMin, HardAutoMax);
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
    if (target->LinkedAxisCanvasID <= 0)
     { item = x->current; } // Linked to an axis on the same graph
    else // Linked to an axis on a different canvas item
     {
      item = x->itemlist->first;
      while ((item != NULL) && (item->id)<target->LinkedAxisCanvasID) item=item->next;
      if ((item == NULL) || (item->id != target->LinkedAxisCanvasID)) { if (IterDepth==1) { sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but no such plot exists.","xyz"[xyz],axis_n,x->current->id,"xyz"[target->LinkedAxisToXYZ],target->LinkedAxisToNum,target->LinkedAxisCanvasID); ppl_warning(ERR_GENERAL, temp_err_string); } break; }
     }
    if      (target->LinkedAxisToXYZ == 0) target2 = item->XAxes + target->LinkedAxisToNum;
    else if (target->LinkedAxisToXYZ == 1) target2 = item->YAxes + target->LinkedAxisToNum;
    else                                   target2 = item->ZAxes + target->LinkedAxisToNum;
    if (target->DataUnitSet && target2->DataUnitSet && (!ppl_units_DimEqual(&target->DataUnit , &target2->DataUnit))) break; // If axes are dimensionally incompatible, stop
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
    if (!source->linked) { break; } // proceed only if axis is linked
    source->MinFinal = target->MinFinal;
    source->MaxFinal = target->MaxFinal;
    if (source->linkusing != NULL) eps_plot_LinkedAxisLinkUsing(source, target, target_xyz);
    else                           { source->DataUnit = target->DataUnit; source->DataUnitSet = target->DataUnitSet; }
    source->RangeFinalised = 1;
    if (source->LinkedAxisCanvasID <= 0)
     { item = x->current; } // Linked to an axis on the same graph
    else // Linked to an axis on a different canvas item
     {
      item = x->itemlist->first;
      while ((item != NULL) && (item->id)<source->LinkedAxisCanvasID) item=item->next;
      if ((item == NULL) || (item->id != source->LinkedAxisCanvasID)) { break; }
     }
    if      (source->LinkedAxisToXYZ == 0) source2 = item->XAxes + source->LinkedAxisToNum;
    else if (source->LinkedAxisToXYZ == 1) source2 = item->YAxes + source->LinkedAxisToNum;
    else                                   source2 = item->ZAxes + source->LinkedAxisToNum;
    if (source->DataUnitSet && source2->DataUnitSet && (!ppl_units_DimEqual(&source->DataUnit , &source2->DataUnit))) break; // If axes are dimensionally incompatible, stop
    source = source2;
   }
  return;
 }

void eps_plot_LinkedAxisLinkUsing(settings_axis *out, settings_axis *in, int xyz)
 {
  int i,j=-1,k=-1,l,xrn=0;
  int oldsgn=-10,newsgn;
  double p,x;
  char *VarName, err_string[LSTR_LENGTH];
  value DummyTemp, OutVal, *VarVal;
  if      (xyz==0) VarName = "x";
  else if (xyz==1) VarName = "y";
  else             VarName = "z";

  // Look up variable in user space and get pointer to its value
  DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&VarVal);
  if (VarVal!=NULL)
   {
    DummyTemp = *VarVal;
   }
  else
   {
    ppl_units_zero(&DummyTemp);
    DictAppendValue(_ppl_UserSpace_Vars, VarName, DummyTemp);
    DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&VarVal);
    DummyTemp.modified = 2;
   }

  out->AxisLinearInterpolation = (double *)lt_malloc(AXISLINEARINTERPOLATION_NPOINTS * sizeof(double));
  out->AxisValueTurnings       = 0;
  out->AxisTurnings            = (int *)lt_malloc((AXISLINEARINTERPOLATION_NPOINTS+2) * sizeof(int));

  if ((out->AxisLinearInterpolation==NULL)||(out->AxisTurnings==NULL)) return;
  out->AxisTurnings[xrn++] = 0;

  for (l=0; l<AXISLINEARINTERPOLATION_NPOINTS; l++) // Loop over samples we are going to take from axis linkage function
   {
    p = ((double)l)/(AXISLINEARINTERPOLATION_NPOINTS-1);
    x = eps_plot_axis_InvGetPosition(p,in);

    // Set value of x (or y/z)
    *VarVal = in->DataUnit;
    VarVal->imag        = 0.0;
    VarVal->FlagComplex = 0;
    VarVal->real        = x;

    // Generate a sample from the axis linkage function
    i = strlen(out->linkusing);
    while ((i>0)&&(out->linkusing[i-1]<=' ')) i--;
    j = -1;
    ppl_EvaluateAlgebra(out->linkusing, &OutVal, 0, &j, 0, &k, err_string, 1);
    if (k>=0) { sprintf(temp_err_string, "Error encountered whilst evaluating axis linkage expression: %s",out->linkusing); ppl_error(ERR_GENERAL, temp_err_string); ppl_error(ERR_GENERAL, err_string); goto FAIL; }
    if (j< i) { sprintf(temp_err_string, "Error encountered whilst evaluating axis linkage expression: %s",out->linkusing); ppl_error(ERR_GENERAL, temp_err_string); ppl_error(ERR_GENERAL, "Unexpected trailing matter."); goto FAIL; }
    if (OutVal.FlagComplex) { sprintf(temp_err_string, "Error encountered whilst evaluating axis linkage expression: %s",out->linkusing); ppl_error(ERR_GENERAL, temp_err_string); ppl_error(ERR_GENERAL, "Received a complex number; axes must have strictly real values at all points."); goto FAIL; }
    if (!gsl_finite(OutVal.real)) { sprintf(temp_err_string, "Error encountered whilst evaluating axis linkage expression: %s",out->linkusing); ppl_error(ERR_GENERAL, temp_err_string); ppl_error(ERR_GENERAL, "Expression returned non-finite result."); goto FAIL; }
    if (!out->DataUnitSet) { out->DataUnit = OutVal; out->DataUnitSet = 1; }
    if (!ppl_units_DimEqual(&out->DataUnit,&OutVal))  { sprintf(temp_err_string, "Error encountered whilst evaluating axis linkage expression: %s",out->linkusing); ppl_error(ERR_GENERAL, temp_err_string); sprintf(temp_err_string, "Axis linkage function produces axis values with dimensions of <%s> whilst data plotted on this axis has dimensions of <%s>.", ppl_units_GetUnitStr(&OutVal,NULL,NULL,0,0), ppl_units_GetUnitStr(&out->DataUnit,NULL,NULL,1,0)); ppl_error(ERR_GENERAL, temp_err_string); goto FAIL; }
    out->AxisLinearInterpolation[l] = OutVal.real;
    if (l>0) // Check for turning points
     {
      newsgn = sgn(out->AxisLinearInterpolation[l] - out->AxisLinearInterpolation[l-1]);
      if (newsgn==0) continue;
      if ((newsgn!=oldsgn) && (oldsgn>-10)) out->AxisTurnings[xrn++] = l-1;
      oldsgn = newsgn;
     }
   }

  // Restore original value of x (or y/z)
  *VarVal = DummyTemp;
  out->AxisTurnings[xrn--] = AXISLINEARINTERPOLATION_NPOINTS-1;
  out->AxisValueTurnings = xrn;
  return;

FAIL:
  out->AxisLinearInterpolation = NULL;
  out->AxisValueTurnings       = 0;
  out->AxisTurnings            = NULL;
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
  char             errbuffer[LSTR_LENGTH];

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
      UsingList    = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "using_item", (void *)pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(); DictAppendPtr(tempdict, "every_item", (void *)(pd->EveryList+j), 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      status       = 0;
      ErrCount     = DATAFILE_NERRS;
      NExpect      = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);
      OrdinateAxis = &axissets[pd->axis1xyz][pd->axis1];

      if (eps_plot_AddUsingItemsForWithWords(&pd->ww_final, &NExpect, UsingList)) { *(x->status) = 1; return; } // Add extra using items for, e.g. "linewidth $3".

      // Fix range of ordinate axis
      eps_plot_LinkedAxisForwardPropagate(x, OrdinateAxis, pd->axis1xyz, pd->axis1);
      if (*x->status) return;

      // Make ordinate raster if we don't already have one
      if (OrdinateAxis->OrdinateRaster == NULL)
       {
        OrdinateAxis->OrdinateRaster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
        if (OrdinateAxis->OrdinateRaster == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *(x->status) = 1; return; }
        for (j=0; j<x->current->settings.samples; j++)
          OrdinateAxis->OrdinateRaster[j] = eps_plot_axis_InvGetPosition(((double)j)/(x->current->settings.samples-1), OrdinateAxis);
        OrdinateAxis->OrdinateRasterLen = x->current->settings.samples;
       }

      if (DEBUG) { sprintf(temp_err_string, "Reading data from functions for dataset %d in plot item %d", i+1, x->current->id); ppl_log(temp_err_string); }

      // Get data from functions
      DataFile_FromFunctions(OrdinateAxis->OrdinateRaster, 0, OrdinateAxis->OrdinateRasterLen, &OrdinateAxis->DataUnit, x->current->plotdata+i, &status, errbuffer, pd->functions, pd->NFunctions, UsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, &ErrCount);
      if (status) { ppl_error(ERR_GENERAL, errbuffer); x->current->plotdata[i]=NULL; }

      // Update axes to reflect usage
      status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2], &axissets[pd->axis3xyz][pd->axis3], &x->current->settings, pd->axis1xyz, pd->axis2xyz, pd->axis3xyz, pd->axis1, pd->axis2, pd->axis3, x->current->id);
      if (status) { *(x->status) = 1; return; }
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis1xyz][pd->axis1], pd->axis1xyz, pd->axis1);
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis2xyz][pd->axis2], pd->axis2xyz, pd->axis2);
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis3xyz][pd->axis3], pd->axis3xyz, pd->axis3);
      axissets[pd->axis1xyz][pd->axis1].RangeFinalised = axissets[pd->axis1xyz][pd->axis1].TickListFinalised = 0;
     }
    pd=pd->next; i++;
   }
  return;
 }

void eps_plot_YieldUpText(EPSComm *x)
 {
  int              j, k, l, m;
  canvas_plotdesc *pd;
  DataBlock       *blk;
  settings_axis   *axes;
  CanvasTextItem  *i;

  x->current->FirstTextID = x->NTextItems;

  // Labels attached to data points
  pd = x->current->plotitems;
  k  = 0;
  while (pd != NULL) // loop over all datasets
   {
    if (x->current->plotdata[k] != NULL)
     {
      x->current->DatasetTextID[k] = x->NTextItems;
      blk = x->current->plotdata[k]->first;
      while (blk != NULL)
       {
        if (blk->text != NULL) for (j=0; j<blk->BlockPosition; j++) { YIELD_TEXTITEM(blk->text[j]); }
        blk=blk->next;
       }
     }
    pd=pd->next; k++;
   }

  // Graph legend
  x->current->LegendTextID = x->NTextItems;
  GraphLegend_YieldUpText(x);

  // Axis labels and titles
  x->current->AxesTextID = x->NTextItems;
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
         YIELD_TEXTITEM(axes[k].FinalAxisLabel);
        }
      }
   }

  // Title of plot
  x->current->TitleTextID = x->NTextItems;
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

  x->LaTeXpageno = x->current->FirstTextID;

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

  // Write header at top of postscript
  fprintf(x->epsbuffer, "%% Canvas item %d [plot]\n", x->current->id);

  // Calculate positions of the four corners of graph
  origin_x = x->current->settings.OriginX.real * M_TO_PS;
  origin_y = x->current->settings.OriginY.real * M_TO_PS;
  width    = x->current->settings.width  .real * M_TO_PS;
  if (x->current->settings.AutoAspect == SW_ONOFF_ON) height = width * 2.0/(1.0+sqrt(5));
  else                                                height = width * x->current->settings.aspect;

  // Turn on clipping if 'set clip' is set
  if (x->current->settings.clip == SW_ONOFF_ON)
   fprintf(x->epsbuffer, "gsave\nnewpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nclip newpath\n",origin_x,origin_y,origin_x+width,origin_y,origin_x+width,origin_y+height,origin_x,origin_y+height);

  // Render gridlines

  // Activate three-dimensional buffer if graph is 3D
  if (x->current->ThreeDim) ThreeDimBuffer_Activate(x);

  // Render each dataset in turn
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL) // loop over all datasets
   {
    x->LaTeXpageno = x->current->DatasetTextID[i];
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

  // Render text labels and arrows
  eps_plot_labelsarrows(x, origin_x, origin_y, width, height);

  // Turn off clipping if 'set clip' is set
  if (x->current->settings.clip == SW_ONOFF_ON)
   { fprintf(x->epsbuffer, "grestore\n"); x->LastLinewidth = -1; x->LastLinetype = -1; x->LastEPSColour[0]='\0'; }

  // Render axes
  eps_plot_axespaint(x, origin_x, origin_y, width, height);

  // Deactivate three-dimensional buffer
  ThreeDimBuffer_Deactivate(x);

  // Render legend
  GraphLegend_Render(x, width, height);

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

