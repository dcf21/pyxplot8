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

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_multimin.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "MathsTools/dcfmath.h"

#include "ppl_eqnsolve.h"
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
  int            IterDepth, source_xyz, source_num, source_CID;
  settings_axis *target;
  canvas_item   *item;

  if (DEBUG) { sprintf(temp_err_string, "Back-propagating axis usage for axis %c%d on plot %d", "xyz"[xyz], axis_n, x->current->id); }
  source_xyz = xyz;
  source_num = axis_n;
  source_CID = x->current->id;
  item       = x->current;

  // Propagating MinUsed and MaxUsed variables along links between axes
  IterDepth = 0;
  while (1) // loop over as many iterations of linkage as may be necessary
   {
    if (IterDepth++ > 100) return;
    if (!(source->linked && (source->MinUsedSet || source->MaxUsedSet || source->DataUnitSet))) { break; } // proceed only if axis is linked and has usage information
    if (source->LinkedAxisCanvasID <= 0)
     { } // Linked to an axis on the same graph; do not change item
    else // Linked to an axis on a different canvas item
     {
      item = x->itemlist->first;
      while ((item != NULL) && ((item->id)<source->LinkedAxisCanvasID)) item=item->next;
      if ((item == NULL) || (item->id != source->LinkedAxisCanvasID) || (item->type != CANVAS_PLOT) || (item->XAxes==NULL) || (item->YAxes==NULL) || (item->ZAxes==NULL)) { break; }
     }
    if      (source->LinkedAxisToXYZ == 0) target = item->XAxes + source->LinkedAxisToNum;
    else if (source->LinkedAxisToXYZ == 1) target = item->YAxes + source->LinkedAxisToNum;
    else                                   target = item->ZAxes + source->LinkedAxisToNum;
    if (source->linkusing != NULL)
     {
      eps_plot_LinkUsingBackPropagate(x, source->MinUsed, target, source->LinkedAxisToXYZ, source->LinkedAxisToNum, item->id, source, source_xyz, source_num, source_CID);
      eps_plot_LinkUsingBackPropagate(x, source->MaxUsed, target, source->LinkedAxisToXYZ, source->LinkedAxisToNum, item->id, source, source_xyz, source_num, source_CID);
     }
    else
     {
      if ((target->DataUnitSet && source->DataUnitSet) && (!ppl_units_DimEqual(&target->DataUnit , &source->DataUnit)))
       {
        sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but axes have data plotted against them with conflicting physical units. The former has units of <%s> whilst the latter has units of <%s>.","xyz"[xyz],axis_n,x->current->id,"xyz"[source->LinkedAxisToXYZ],source->LinkedAxisToNum,source->LinkedAxisCanvasID,ppl_units_GetUnitStr(&target->DataUnit,NULL,NULL,0,1,0),ppl_units_GetUnitStr(&source->DataUnit,NULL,NULL,1,1,0));
        ppl_warning(ERR_GENERAL, temp_err_string);
        break;
       }
      else
       {
        if ((source->MinUsedSet) && ((!target->MinUsedSet) || (target->MinUsed > source->MinUsed))) { target->MinUsed=source->MinUsed; target->MinUsedSet=1; }
        if ((source->MaxUsedSet) && ((!target->MaxUsedSet) || (target->MaxUsed < source->MaxUsed))) { target->MaxUsed=source->MaxUsed; target->MaxUsedSet=1; }
        if (source->DataUnitSet) { target->DataUnit = source->DataUnit; target->DataUnitSet = 1; }
       }
     }
    source     = target;
    source_xyz = source->LinkedAxisToXYZ;
    source_num = source->LinkedAxisToNum;
    source_CID = item->id;
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
      if (!axes[i].RangeFinalised   ) { eps_plot_LinkedAxisForwardPropagate(x, &axes[i], j, i, 1); if (*x->status) return; }
      if (!axes[i].TickListFinalised) { eps_plot_ticking(&axes[i], j, i, x->current->id, (j==1)?height:width, x->current->settings.AxisUnitStyle); if (*x->status) return; }
     }
   }
  return;
 }

// Finalises the range of a particular axis, bringing together usage
// information, ranges specified in the plot command, and ranges set for the
// axis with the set xrange command.

void eps_plot_DecideAxisRange(EPSComm *x, settings_axis *axis, int xyz, int axis_n)
 {
  double width, height;

  // Work out lengths of x and y axes
  width    = x->current->settings.width.real;
  if (x->current->settings.AutoAspect == SW_ONOFF_ON) height = width * 2.0/(1.0+sqrt(5));
  else                                                height = width * x->current->settings.aspect;

  eps_plot_ticking(axis, xyz, axis_n, x->current->id, (xyz==1)?height:width, x->current->settings.AxisUnitStyle);
  return;
 }

// As part of the process of determining the range of axis xyz[axis_n], check
// whether the axis is linking, and if so fetch usage information from the
// bottom of the linkage hierarchy. Propagate this information up through all
// intermediate levels of the hierarchy before calling
// eps_plot_DecideAxisRange().

void eps_plot_LinkedAxisForwardPropagate(EPSComm *x, settings_axis *axis, int xyz, int axis_n, int mode)
 {
  int            IterDepth, target_xyz, target_axis_n, source_xyz, OriginalMode=mode;
  settings_axis *source, *target, *target2;
  canvas_item   *item;
  
  // Propagate MinUsed and MaxUsed variables along links
  IterDepth     = 0;
  target        = axis;
  target_xyz    = xyz;
  target_axis_n = axis_n;
  item          = x->current;

  while (1) // loop over as many iterations of linkage as may be necessary to find MinFinal and MaxFinal at the bottom
   {
    if (IterDepth++ > 100) break;
    if ((!target->linked) || target->RangeFinalised) { break; } // proceed only if axis is linked
    if (target->LinkedAxisCanvasID <= 0)
     { } // Linked to an axis on the same graph; do not change item
    else // Linked to an axis on a different canvas item
     {
      item = x->itemlist->first;
      while ((item != NULL) && (item->id)<target->LinkedAxisCanvasID) item=item->next;
      if ((item == NULL) || (item->id != target->LinkedAxisCanvasID)) { if ((IterDepth==1)&&(mode==0)) { sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but no such plot exists.","xyz"[xyz],axis_n,x->current->id,"xyz"[target->LinkedAxisToXYZ],target->LinkedAxisToNum,target->LinkedAxisCanvasID); ppl_warning(ERR_GENERAL, temp_err_string); } break; }
      if (item->type != CANVAS_PLOT) { if ((IterDepth==1)&&(mode==0)) { sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but this canvas item is not a plot.","xyz"[xyz],axis_n,x->current->id,"xyz"[target->LinkedAxisToXYZ],target->LinkedAxisToNum,target->LinkedAxisCanvasID); ppl_warning(ERR_GENERAL, temp_err_string); } break; }
      if ((item->XAxes==NULL)||(item->YAxes==NULL)||(item->ZAxes==NULL)) { if ((IterDepth==1)&&(mode==0)) { sprintf(temp_err_string,"Axis %c%d of plot %d is linked to axis %c%d of plot %d, but this item has NULL axes.","xyz"[xyz],axis_n,x->current->id,"xyz"[target->LinkedAxisToXYZ],target->LinkedAxisToNum,target->LinkedAxisCanvasID); ppl_warning(ERR_INTERNAL, temp_err_string); } break; }
     }
    if      (target->LinkedAxisToXYZ == 0) target2 = item->XAxes + target->LinkedAxisToNum;
    else if (target->LinkedAxisToXYZ == 1) target2 = item->YAxes + target->LinkedAxisToNum;
    else                                   target2 = item->ZAxes + target->LinkedAxisToNum;
    if (target->DataUnitSet && target2->DataUnitSet && (!ppl_units_DimEqual(&target->DataUnit , &target2->DataUnit))) break; // If axes are dimensionally incompatible, stop
    target_xyz    = target->LinkedAxisToXYZ;
    target_axis_n = target->LinkedAxisToNum;
    target        = target2;
   }
  if ((mode==1) && (!target->RangeFinalised)) { eps_plot_DecideAxisRange(x, target, target_xyz, target_axis_n); if (*x->status) return; }
  IterDepth -= 2;
  source     = target;
  source_xyz = target_xyz;
  target     = axis;
  for ( ; IterDepth>=0 ; IterDepth--) // loop over as many iterations of linkage as may be necessary
   {
    int k;
    target = axis;
    for (k=0; k<IterDepth; k++)
     {
      if (target->LinkedAxisCanvasID <= 0)
       { item = x->current; } // Linked to an axis on the same graph
      else // Linked to an axis on a different canvas item
       {
        item = x->itemlist->first;
        while ((item != NULL) && (item->id)<target->LinkedAxisCanvasID) item=item->next;
        if ((item == NULL) || (item->id != target->LinkedAxisCanvasID) || (item->type != CANVAS_PLOT) || (item->XAxes==NULL) || (item->YAxes==NULL) || (item->ZAxes==NULL)) { break; }
       }
      if      (target->LinkedAxisToXYZ == 0) target2 = item->XAxes + target->LinkedAxisToNum;
      else if (target->LinkedAxisToXYZ == 1) target2 = item->YAxes + target->LinkedAxisToNum;
      else                                   target2 = item->ZAxes + target->LinkedAxisToNum;
      target        = target2;
     }

    if (target->RangeFinalised) { break; }
    if (!target->linked) { break; } // proceed only if axis is linked
    if ((OriginalMode==0)&&(target->Mode0BackPropagated)) continue;
    target->Mode0BackPropagated = 1;
    if (mode==0) // MODE 0: Propagate HardMin, HardMax, HardUnit
     {
      if (target->linkusing != NULL)
       {
        if (source->HardMinSet && source->HardMaxSet)
         {
          source->MinFinal    = source->HardMin;
          source->MaxFinal    = source->HardMax;
          source->DataUnit    = source->HardUnit;
          source->DataUnitSet = 1;
          mode=1;
         }
        else
         {
          target->HardMinSet = target->HardMaxSet = target->HardUnitSet = 0;
         }
       }
      else
       {
        target->HardMinSet     = source->HardMinSet;
        target->HardMin        = source->HardMin;
        target->HardMaxSet     = source->HardMaxSet;
        target->HardMax        = source->HardMax;
        target->HardUnitSet    = source->HardUnitSet;
        target->HardUnit       = source->HardUnit;
        target->HardAutoMinSet = source->HardAutoMinSet;
        target->HardAutoMaxSet = source->HardAutoMaxSet;
       }
     }
    if (mode==1) // MODE 1: Propagate finalised ranges
     {
      target->MinFinal = source->MinFinal;
      target->MaxFinal = source->MaxFinal;
      if (target->linkusing != NULL)
       {
        if (eps_plot_LinkedAxisLinkUsing(target, source, source_xyz)) break;
       }
      else
       {
        if (target->DataUnitSet && source->DataUnitSet && (!ppl_units_DimEqual(&target->DataUnit , &source->DataUnit))) break; // If axes are dimensionally incompatible, stop
        target->DataUnit = source->DataUnit;
        target->DataUnitSet = source->DataUnitSet;
        target->AxisValueTurnings = source->AxisValueTurnings;
        target->AxisLinearInterpolation = source->AxisLinearInterpolation;
        target->AxisTurnings = source->AxisTurnings;
       }
      target->RangeFinalised = 1;
     }
    source = target;
   }
  return;
 }

int eps_plot_LinkedAxisLinkUsing(settings_axis *out, settings_axis *in, int xyz)
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

  if ((out->AxisLinearInterpolation==NULL)||(out->AxisTurnings==NULL)) goto FAIL;
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
    if (!ppl_units_DimEqual(&out->DataUnit,&OutVal))  { sprintf(temp_err_string, "Error encountered whilst evaluating axis linkage expression: %s",out->linkusing); ppl_error(ERR_GENERAL, temp_err_string); sprintf(temp_err_string, "Axis linkage function produces axis values with dimensions of <%s> whilst data plotted on this axis has dimensions of <%s>.", ppl_units_GetUnitStr(&OutVal,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&out->DataUnit,NULL,NULL,1,1,0)); ppl_error(ERR_GENERAL, temp_err_string); goto FAIL; }
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
  return 0;

FAIL:
  out->AxisLinearInterpolation = NULL;
  out->AxisValueTurnings       = 0;
  out->AxisTurnings            = NULL;
  return 1;
 }

// Routines for the back propagation of usage information along non-linear links

typedef struct LAUComm {
 char         *expr;
 char         *VarName;
 value        *VarValue;
 int           GoneNaN, mode, iter2;
 double        norm;
 value         target;
 int          *errpos;
 char          errtext[LSTR_LENGTH];
 int           WarningPos; // One final algebra error is allowed to produce a warning
 char          warntext[LSTR_LENGTH];
 } LAUComm;

double eps_plot_LAUSlave(const gsl_vector *x, void *params)
 {
  int      i;
  value    OutValue;
  LAUComm *data = (LAUComm *)params;

  if (*(data->errpos)>=0) return GSL_NAN; // We've previously had a serious error... so don't do any more work
  if (data->mode==0) data->VarValue->real                   = optimise_LogToReal(gsl_vector_get(x, 0) , data->iter2 , &data->norm);
  else               data->VarValue->exponent[data->mode-1] = optimise_LogToReal(gsl_vector_get(x, 0) , data->iter2 , &data->norm);

  data->VarValue->dimensionless=0;
  for (i=0; i<UNITS_MAX_BASEUNITS; i++) if (ppl_units_DblEqual(data->VarValue->exponent[i], 0) == 0) { data->VarValue->dimensionless=0; break; }
  ppl_EvaluateAlgebra(data->expr, &OutValue, 0, NULL, 0, data->errpos, data->errtext, 0);

  // If a numerical error happened; ignore it for now, but return NAN
  if (*(data->errpos) >= 0) { data->WarningPos=*(data->errpos); sprintf(data->warntext, "An algebraic error was encountered at %s=%s: %s", data->VarName, ppl_units_NumericDisplay(data->VarValue,0,0,0), data->errtext); *(data->errpos)=-1; return GSL_NAN; }

#define TWINLOG(X) ((X>1e-200) ? log(X) : (2*log(1e-200) - log(2e-200-X)))
  if (data->mode==0) return pow( TWINLOG(data->target.real                  ) - TWINLOG(OutValue.real                   ) ,2);
  else               return pow( TWINLOG(data->target.exponent[data->mode-1]) - TWINLOG(OutValue.exponent[data->mode-1] ) ,2);
 }

void eps_plot_LAUFitter(LAUComm *commlink)
 {
  size_t                              iter = 0,iter2 = 0;
  int                                 i, status=0;
  double                              size=0,sizelast=0,sizelast2=0,testval;
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex; // We don't use nmsimplex2 here because it was new in gsl 1.12
  gsl_multimin_fminimizer            *s;
  gsl_vector                         *x, *ss;
  gsl_multimin_function               fn;

  fn.n = 1;
  fn.f = &eps_plot_LAUSlave;
  fn.params = (void *)commlink;

  x  = gsl_vector_alloc( 1 );
  ss = gsl_vector_alloc( 1 );

  iter2=0;
  do
   {
    iter2++;
    if (commlink->mode!=0) iter2=999;
    commlink->iter2 = iter2;
    sizelast2 = size;
    gsl_vector_set(x , 0, optimise_RealToLog( (commlink->mode==0) ? commlink->VarValue->real : 1.0 , iter2, &commlink->norm));
    gsl_vector_set(ss, 0, (fabs(gsl_vector_get(x,i))>1e-6) ? 0.1 * (gsl_vector_get(x,0)) : 0.1);
    s = gsl_multimin_fminimizer_alloc (T, fn.n);
    gsl_multimin_fminimizer_set (s, &fn, x, ss);

    // If initial value we are giving the minimiser produces an algebraic error, it's not worth continuing
    testval = eps_plot_LAUSlave(x,(void *)commlink);
    if (commlink->WarningPos>=0) { *(commlink->errpos) = commlink->WarningPos; commlink->WarningPos=-1; sprintf(commlink->errtext, "%s", commlink->warntext); return; }
    iter              = 0;
    commlink->GoneNaN = 0;
    do
     {
      iter++;
      for (i=0; i<4; i++) { status = gsl_multimin_fminimizer_iterate(s); if (status) break; } // Run optimiser four times for good luck
      if (status) break;
      sizelast = size;
      size     = gsl_multimin_fminimizer_minimum(s); // Do this ten times, or up to 50, until fit stops improving
     }
    while ((iter < 10) || ((size < sizelast) && (iter < 50))); // Iterate 10 times, and then see whether size carries on getting smaller

    // Read off best-fit value from s->x
    if (commlink->mode==0) commlink->VarValue->real                       = optimise_LogToReal(gsl_vector_get(s->x, 0), iter2, &commlink->norm);
    else                   commlink->VarValue->exponent[commlink->mode-1] = optimise_LogToReal(gsl_vector_get(s->x, 0), iter2, &commlink->norm);
    gsl_multimin_fminimizer_free(s);
    if (iter2==999) { iter2=2; break; }
   }
  while ((iter2 < 3) || ((commlink->GoneNaN==0) && (!status) && (size < sizelast2) && (iter2 < 20))); // Iterate 2 times, and then see whether size carries on getting smaller

  if (iter2>=20) status=1;
  if (status) { *(commlink->errpos)=0; sprintf(commlink->errtext, "Failed to converge. GSL returned error: %s", gsl_strerror(status)); }
  gsl_vector_free(x);
  gsl_vector_free(ss);
  return;
 }

void eps_plot_LinkUsingBackPropagate(EPSComm *x, double val, settings_axis *target, int target_xyz, int target_n, int target_canvasID, settings_axis *source, int source_xyz, int source_n, int source_canvasID)
 {
  LAUComm commlink;
  int     errpos = -1;
  char   *VarName;
  value   DummyTemp, *VarVal;

  if (target->HardMinSet && target->HardMaxSet) return;

  if      (target_xyz==0) VarName = "x";
  else if (target_xyz==1) VarName = "y";
  else                    VarName = "z";

  // Look up xyz dummy variable in user space and get pointer to its value
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

  // Set up commlink structure to use in minimiser
  commlink.expr        = source->linkusing;
  commlink.VarName     = VarName;
  commlink.VarValue    = VarVal;
  commlink.GoneNaN     = commlink.mode = 0;
  commlink.target      = source->DataUnit;
  commlink.target.real = val;
  commlink.errpos      = &errpos;
  commlink.WarningPos  = -1;
  commlink.errtext[0]  = commlink.warntext[0] = '\0';

  // First run minimiser to get numerical value of xyz. Then fit each dimension in turn.
  ppl_units_zero(VarVal); VarVal->real = 1.0;
  for (commlink.mode=0; commlink.mode<UNITS_MAX_BASEUNITS+1; commlink.mode++)
   {
    eps_plot_LAUFitter(&commlink);
    if ((errpos>=0) || (commlink.WarningPos>=0)) break;
   }

  // If there was a problem, throw a warning and proceed no further
  if ((commlink.WarningPos>=0) || (errpos>=0))
   {
    sprintf(temp_err_string, "Could not propagate axis range information from axis %c%d of plot %d to axis %c%d of plot %d using expression <%s>. Recommend setting an explicit range for axis %c%d of plot %d.", "xyz"[source_xyz], source_n, source_canvasID, "xyz"[target_xyz], target_n, target_canvasID, source->linkusing, "xyz"[target_xyz], target_n, target_canvasID);
    ppl_warning(ERR_GENERAL, temp_err_string);
   }
  else
   {
    int i;
    VarVal->dimensionless=1; // Cycle through powers of final value of xyz dummy value making things which are nearly ints into ints.
    for (i=0; i<UNITS_MAX_BASEUNITS; i++)
     {
      if      (fabs(floor(VarVal->exponent[i]) - VarVal->exponent[i]) < 1e-12) VarVal->exponent[i] = floor(VarVal->exponent[i]);
      else if (fabs(ceil (VarVal->exponent[i]) - VarVal->exponent[i]) < 1e-12) VarVal->exponent[i] = ceil (VarVal->exponent[i]);
      if (VarVal->exponent[i] != 0) VarVal->dimensionless=0;
     }

    // Check that dimension of propagated value fits with existing unit of axis
    if      ((target->HardUnitSet) && (!ppl_units_DimEqual(&target->HardUnit, VarVal)))
     {
      sprintf(temp_err_string, "Could not propagate axis range information from axis %c%d of plot %d to axis %c%d of plot %d using expression <%s>. Propagated axis range has units of <%s>, but axis %c%d of plot %d has a range set with units of <%s>.", "xyz"[source_xyz], source_n, source_canvasID, "xyz"[target_xyz], target_n, target_canvasID, source->linkusing, ppl_units_GetUnitStr(VarVal,NULL,NULL,0,1,0), "xyz"[target_xyz], target_n, target_canvasID, ppl_units_GetUnitStr(&target->HardUnit,NULL,NULL,1,1,0));
      ppl_warning(ERR_GENERAL, temp_err_string);
     }
    else if ((target->DataUnitSet) && (!ppl_units_DimEqual(&target->DataUnit, VarVal)))
     {
      sprintf(temp_err_string, "Could not propagate axis range information from axis %c%d of plot %d to axis %c%d of plot %d using expression <%s>. Propagated axis range has units of <%s>, but axis %c%d of plot %d has data plotted against it with units of <%s>.", "xyz"[source_xyz], source_n, source_canvasID, "xyz"[target_xyz], target_n, target_canvasID, source->linkusing, ppl_units_GetUnitStr(VarVal,NULL,NULL,0,1,0), "xyz"[target_xyz], target_n, target_canvasID, ppl_units_GetUnitStr(&target->DataUnit,NULL,NULL,1,1,0));
      ppl_warning(ERR_GENERAL, temp_err_string);
     }
    else if (VarVal->FlagComplex)
     {
      sprintf(temp_err_string, "Could not propagate axis range information from axis %c%d of plot %d to axis %c%d of plot %d using expression <%s>. Axis usage was a complex number.", "xyz"[source_xyz], source_n, source_canvasID, "xyz"[target_xyz], target_n, target_canvasID, source->linkusing);
      ppl_warning(ERR_GENERAL, temp_err_string);
     }
    else if (!gsl_finite(VarVal->real))
     {
      sprintf(temp_err_string, "Could not propagate axis range information from axis %c%d of plot %d to axis %c%d of plot %d using expression <%s>. Axis usage was a non-finite number.", "xyz"[source_xyz], source_n, source_canvasID, "xyz"[target_xyz], target_n, target_canvasID, source->linkusing);
      ppl_warning(ERR_GENERAL, temp_err_string);
     }
    else
     {
      if ((!target->MinUsedSet) || (target->MinUsed > VarVal->real)) { target->MinUsed=VarVal->real; target->MinUsedSet=1; }
      if ((!target->MaxUsedSet) || (target->MaxUsed < VarVal->real)) { target->MaxUsed=VarVal->real; target->MaxUsedSet=1; }
      if (source->DataUnitSet) { target->DataUnit = *VarVal; target->DataUnitSet = 1; }
     }
   }

  // Restore original value of x (or y/z)
  *VarVal = DummyTemp;
  return;
 }

