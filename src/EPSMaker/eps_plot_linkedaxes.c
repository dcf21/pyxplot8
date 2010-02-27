// eps_plot_linkedaxes.c
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

#define _PPL_EPS_PLOT_LINKEDAXES 1

#include <stdlib.h>
#include <stdio.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "MathsTools/dcfmath.h"

#include "ppl_error.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_canvas.h"
#include "eps_plot_linkedaxes.h"
#include "eps_plot_ticking.h"
#include "eps_settings.h"

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
  double *HardMin, *HardMax, width, height;
  unsigned char HardAutoMin, HardAutoMax; // Set for "plot [:*]", where * says we must autoscale, even if there's a preexisting maximum set for the axis

  HardAutoMin = axis->HardAutoMinSet;
  HardAutoMax = axis->HardAutoMaxSet;
  HardMin     = axis->HardMinSet ? &axis->HardMin : NULL;
  HardMax     = axis->HardMaxSet ? &axis->HardMax : NULL;

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

