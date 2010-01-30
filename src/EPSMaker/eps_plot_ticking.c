// eps_plot_ticking.c
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

// This file contains routines for working out the ranges of axes, and where to
// put axis ticks along them

#define _PPL_EPS_PLOT_TICKING_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ListTools/lt_memory.h"

#include "MathsTools/dcfmath.h"

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_plot_canvas.h"
#include "eps_plot_ticking.h"

void eps_plot_ticking(settings_axis *axis, int xyz, int axis_n, int canvas_id, double length, int AxisUnitStyle, const double *HardMin, const double *HardMax, unsigned char HardAutoMin, unsigned char HardAutoMax)
 {
  int i,j,N,xrn;
  const double logmin = 1e-10;

  axis->FinalActive = axis->FinalActive || axis->enabled || (HardMin!=NULL) || (HardMax!=NULL) || (HardAutoMin) || (HardAutoMax);
  if (!axis->FinalActive) { axis->RangeFinalised = 0; return; } // Axis is not in use

  // First of all, work out what axis range to use
  if (!axis->RangeFinalised)
   {
    // Work out axis range
    unsigned char MinSet=1;
    double min_prelim, max_prelim, OoM;

    if       (HardMin != NULL)                               axis->MinFinal = *HardMin;
    else if ((axis->MinSet==SW_BOOL_TRUE) && (!HardAutoMin)) axis->MinFinal = axis->min;
    else if  (axis->MinUsedSet)                              axis->MinFinal = axis->MinUsed;
    else                                                     MinSet = 0;

    if       (HardMax != NULL)                               axis->MaxFinal = *HardMax;
    else if ((axis->MaxSet==SW_BOOL_TRUE) && (!HardAutoMax)) axis->MaxFinal = axis->max;
    else if  (axis->MaxUsedSet)                              axis->MaxFinal = axis->MaxUsed;
    else if  (MinSet)                                        axis->MaxFinal = (axis->log == SW_BOOL_TRUE) ? axis->MinFinal * 100
                                                                                                          : axis->MinFinal +  20;
    else                                                     axis->MaxFinal = (axis->log == SW_BOOL_TRUE) ? 10.0 : 10.0;

    // Check that log axes do not venture too close to zero
    if ((axis->log == SW_BOOL_TRUE) && (axis->MaxFinal <= 1e-200)) { axis->MaxFinal = logmin; sprintf(temp_err_string, "Range for logarithmic axis %c%d set below zero; defaulting to 1e-10.", "xyz"[xyz], axis_n); ppl_warning(ERR_NUMERIC, temp_err_string); }
    if (!MinSet) axis->MinFinal = (axis->log == SW_BOOL_TRUE) ? (axis->MaxFinal / 100) : (axis->MaxFinal - 20);
    if ((axis->log == SW_BOOL_TRUE) && (axis->MinFinal <= 1e-200)) { axis->MinFinal = logmin; sprintf(temp_err_string, "Range for logarithmic axis %c%d set below zero; defaulting to 1e-10.", "xyz"[xyz], axis_n); ppl_warning(ERR_NUMERIC, temp_err_string); }

    // If there's no spread of data on the axis, make a spread up
    if (ppl_units_DblApprox(axis->MinFinal , axis->MaxFinal , 1e-14))
     {
      if (   ((HardMin != NULL) || ((axis->MinSet==SW_BOOL_TRUE) && (!HardAutoMin)))
          && ((HardMax != NULL) || ((axis->MaxSet==SW_BOOL_TRUE) && (!HardAutoMax))) )
       { sprintf(temp_err_string, "Specified minimum and maximum range limits for axis %c%d are equal; reverting to alternative limits.", "xyz"[xyz], axis_n); ppl_warning(ERR_NUMERIC, temp_err_string); }
      if (axis->log == SW_BOOL_TRUE)
       {
        axis->MinFinal -= max(1.0,1e-3*fabs(axis->MinFinal));
        axis->MaxFinal += max(1.0,1e-3*fabs(axis->MinFinal));
       }
      else
       {
        if (axis->MinFinal > 1e-300) axis->MinFinal /= 10.0;
        if (axis->MaxFinal <  1e300) axis->MaxFinal *= 10.0;
       }
     }

    // If axis does not have a user-specified range, round it outwards towards a round endpoint
    min_prelim = axis->MinFinal;
    max_prelim = axis->MaxFinal;

    if (axis->log == SW_BOOL_TRUE) { min_prelim = log10(min_prelim); max_prelim = log10(max_prelim); }

    OoM = pow(10.0, floor(log10(fabs(max_prelim - min_prelim)/5)));
    min_prelim = floor(min_prelim / OoM) * OoM;
    max_prelim = ceil (max_prelim / OoM) * OoM;

    if (axis->log == SW_BOOL_TRUE) { min_prelim = pow(10.0,min_prelim); max_prelim = pow(10.0,max_prelim); }

    if (gsl_finite(min_prelim) && (HardMin == NULL) && (axis->MinSet!=SW_BOOL_TRUE) && ((axis->log!=SW_BOOL_TRUE)||(min_prelim>1e-300))) axis->MinFinal = min_prelim;
    if (gsl_finite(max_prelim) && (HardMax == NULL) && (axis->MaxSet!=SW_BOOL_TRUE) && ((axis->log!=SW_BOOL_TRUE)||(min_prelim>1e-300))) axis->MaxFinal = max_prelim;

    // Print out debugging report
    if (DEBUG)
     {
      sprintf(temp_err_string,"Determined range for axis %c%d of plot %d. Usage was [", "xyz"[xyz], axis_n, canvas_id);
      i = strlen(temp_err_string);
      if (axis->MinUsedSet) { sprintf(temp_err_string+i, "%f", axis->MinUsed); i+=strlen(temp_err_string+i); }
      else                  temp_err_string[i++] = '*';
      temp_err_string[i++] = ':';
      if (axis->MaxUsedSet) { sprintf(temp_err_string+i, "%f", axis->MaxUsed); i+=strlen(temp_err_string+i); }
      else                  temp_err_string[i++] = '*';
      sprintf(temp_err_string+i,"]. Final range was [%f:%f].",axis->MinFinal,axis->MaxFinal);
      ppl_log(temp_err_string);
     }

    // Set flag to show that we have finalised the range of this axis
    axis->RangeFinalised = 1;
   }

  // Secondly, decide what ticks to place on this axis
  if (!axis->TickListFinalised)
   {
    double tick_sep_major , tick_sep_minor, UnitMultiplier=1.0;
    value CentralValue;

    // Work out optimal tick separation
    if (xyz!=1)
     {
      tick_sep_major = 0.025;
      tick_sep_minor = 0.008;
     }
    else
     {
      tick_sep_major = 0.015;
      tick_sep_minor = 0.008;
     }

    // Finalise the label to be placed on the axis, appending a physical unit as necessary
    CentralValue = axis->DataUnit;
    CentralValue.FlagComplex = 0;
    CentralValue.imag = 0.0;
    CentralValue.real = (axis->format==NULL) ? eps_plot_axis_InvGetPosition(0.5, axis) : 1.0;
    if (axis->DataUnit.dimensionless)
    { axis->FinalAxisLabel = axis->label; } // No units to append
    else
     {
      i = 0;
      if (axis->label != NULL) i+=strlen(axis->label);
      if (!(axis->DataUnit.dimensionless)) i+= 1024;
      axis->FinalAxisLabel = (char *)lt_malloc(i);
      if (axis->FinalAxisLabel==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return; }
      if      (AxisUnitStyle == SW_AXISUNITSTY_BRACKET) sprintf(axis->FinalAxisLabel, "%s ($%s$)", (axis->label != NULL)?axis->label:"", ppl_units_GetUnitStr(&CentralValue,&UnitMultiplier,NULL,0,SW_DISPLAY_L));
      else if (AxisUnitStyle == SW_AXISUNITSTY_RATIO)   sprintf(axis->FinalAxisLabel, "%s / $%s$", (axis->label != NULL)?axis->label:"", ppl_units_GetUnitStr(&CentralValue,&UnitMultiplier,NULL,0,SW_DISPLAY_L));
      else                                              sprintf(axis->FinalAxisLabel, "%s [$%s$]", (axis->label != NULL)?axis->label:"", ppl_units_GetUnitStr(&CentralValue,&UnitMultiplier,NULL,0,SW_DISPLAY_L));
      UnitMultiplier /= CentralValue.real;
      if (!gsl_finite(UnitMultiplier)) UnitMultiplier=1.0;
     }

    // MAJOR TICKS
    if (axis->TickList != NULL) // Ticks have been specified as an explicit list
     {
      for (N=0; axis->TickStrs[N]!=NULL; N++); // Find length of list of ticks
      axis->TickListPositions = (double  *)lt_malloc((N+1) * (axis->AxisValueTurnings+1) * sizeof(double));
      axis->TickListStrings   = (char   **)lt_malloc((N+1) * (axis->AxisValueTurnings+1) * sizeof(char *));
      if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
      for (i=j=0; i<N; i++)
       for (xrn=0; xrn<=axis->AxisValueTurnings; xrn++)
        {
         axis->TickListPositions[j] = eps_plot_axis_GetPosition(axis->TickList[i], axis, xrn, 0);
         if ( (!gsl_finite(axis->TickListPositions[j])) || (axis->TickListPositions[j]<0.0) || (axis->TickListPositions[j]>1.0) ) continue; // Filter out ticks which are off the end of the axis
         if      (axis->TickStrs[i][0]!='\xFF') axis->TickListStrings[j] = axis->TickStrs[i];
         else if (axis->format == NULL)         TickLabelAutoGen(&axis->TickListStrings[j] , axis->TickList[i] * UnitMultiplier , axis->LogBase);
         else                                   TickLabelFromFormat(&axis->TickListStrings[j], axis->format, axis->TickList[i], &axis->DataUnit, xyz);
         j++;
        }
      axis->TickListStrings[j] = NULL; // null terminate list
     }
    else if (axis->TickStepSet)
     {
      double TMin, TStep, TMax, tmp;
      TMin  = (axis->TickMinSet) ? axis->TickMin : axis->MinFinal;
      TStep = axis->TickStep;
      TMax  = (axis->TickMaxSet) ? axis->TickMax : axis->MaxFinal;
      if (TMax < TMin) { tmp=TMax; TMax=TMin; TMin=tmp; }
      if (TStep<0) TStep=-TStep;
      if (axis->log == SW_BOOL_TRUE) { if (TStep<1) TStep=1/TStep; }
      if (TMin < axis->MinFinal) TMin += ceil ((axis->MinFinal-TMin)/TStep) * TStep;
      if (TMax > axis->MaxFinal) TMax -= floor((TMax-axis->MaxFinal)/TStep) * TStep;
      axis->TickListPositions = (double  *)lt_malloc(102 * (axis->AxisValueTurnings+1) * sizeof(double));
      axis->TickListStrings   = (char   **)lt_malloc(102 * (axis->AxisValueTurnings+1) * sizeof(char *));
      if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
      tmp = TMin;
      for (i=j=0; (i<100)&&(tmp<=TMax); i++)
       {
        for (xrn=0; xrn<=axis->AxisValueTurnings; xrn++)
         {
          axis->TickListPositions[j] = eps_plot_axis_GetPosition(tmp, axis, xrn, 0);
          if ( (!gsl_finite(axis->TickListPositions[j])) || (axis->TickListPositions[j]<0.0) || (axis->TickListPositions[j]>1.0) ) continue; // Filter out ticks which are off the end of the axis
          if (axis->format == NULL) TickLabelAutoGen(&axis->TickListStrings[j] , tmp * UnitMultiplier , axis->LogBase);
          else                      TickLabelFromFormat(&axis->TickListStrings[j], axis->format, tmp, &axis->DataUnit, xyz);
          if (axis->TickListStrings[j]==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
          j++;
         }
        if (axis->log == SW_BOOL_TRUE) tmp*=TStep; else tmp+=TStep;
       }
      axis->TickListStrings[j] = NULL; // null terminate list
     }
    else
     {
      N = 1 + length/tick_sep_major; // Estimate how many ticks we want
      if (N<  3) N=  3;
      if (N>100) N=100;

      axis->TickListPositions = (double  *)lt_malloc((N+1) * sizeof(double));
      axis->TickListStrings   = (char   **)lt_malloc((N+1) * sizeof(char *));
      if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
      for (i=0; i<N; i++)
       {
        double x;
        x = ((double)i)/(N-1);
        axis->TickListPositions[i] = x;
        x = eps_plot_axis_InvGetPosition(x, axis);
        if (axis->format == NULL) TickLabelAutoGen(&axis->TickListStrings[i] , x * UnitMultiplier , axis->LogBase);
        else                      TickLabelFromFormat(&axis->TickListStrings[i], axis->format, x, &axis->DataUnit, xyz);
        if (axis->TickListStrings[i]==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
       }
      axis->TickListStrings[i] = NULL; // null terminate list
     }

    // MINOR TICKS
    if (axis->MTickList != NULL) // Ticks have been specified as an explicit list
     {
      for (N=0; axis->MTickStrs[N]!=NULL; N++); // Find length of list of ticks
      axis->MTickListPositions = (double  *)lt_malloc((N+1) * (axis->AxisValueTurnings+1) * sizeof(double));
      axis->MTickListStrings   = (char   **)lt_malloc((N+1) * (axis->AxisValueTurnings+1) * sizeof(char *));
      if ((axis->MTickListPositions==NULL) || (axis->MTickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->MTickListPositions = NULL; axis->MTickListStrings = NULL; return; }
      for (i=j=0; i<N; i++)
       for (xrn=0; xrn<=axis->AxisValueTurnings; xrn++)
        {
         axis->MTickListPositions[j] = eps_plot_axis_GetPosition( axis->MTickList[i] , axis, xrn, 0);
         if ( (!gsl_finite(axis->MTickListPositions[j])) || (axis->MTickListPositions[j]<0.0) || (axis->MTickListPositions[j]>1.0) ) continue; // Filter out ticks which are off the end of the axis
         if      (axis->TickStrs[i][0]!='\01') axis->TickListStrings[j] = axis->TickStrs[i];
         else                                  axis->TickListStrings[j] = "";
         j++;
        }
      axis->MTickListStrings[j] = NULL; // null terminate list
     }

    // Set flag to show that we have finalised the ticking of this axis
    axis->TickListFinalised = 1;
   }

  return;
 }

void TickLabelAutoGen(char **output, double x, double log_base)
 {
  int    SF = settings_term_current.SignificantFigures;
  double ApproxMargin;

  ApproxMargin = pow(10,-SF+1);
  if (ApproxMargin < 1e-15) ApproxMargin = 1e-15;

  if ((fabs(x)<DBL_MIN*100) || ((fabs(x)>=1e-3) && (fabs(x)<1e5))) { sprintf(temp_err_string,"%s",NumericDisplay(x,0,SF,1)); }
  else
   {
    double e,m;
    unsigned char sgn=0;
    if (x<0) { sgn=1; x=-x; }
    e = floor(log(x)/log(log_base));
    m = x / pow(log_base,e);
    if (ppl_units_DblApprox(m,1,pow(10,-SF+1))) sprintf(temp_err_string,"%s%d^{%s}",sgn?"-":"",(int)log_base,NumericDisplay(e,0,SF,1));
    else                                        sprintf(temp_err_string,"%s%s\\times %d^{%s}",sgn?"-":"",NumericDisplay(m,0,SF,1),(int)log_base,NumericDisplay(e,1,SF,1));
   }
  *output = (char *)lt_malloc(strlen(temp_err_string)+3);
  if ((*output)==NULL) return;
  sprintf(*output,"$%s$",temp_err_string);
  return;
 }

void TickLabelFromFormat(char **output, char *FormatStr, double x, value *xunit, int xyz)
 {
  int i,j=-1,k=-1;
  char *VarName, tmp_string[LSTR_LENGTH], err_string[LSTR_LENGTH];
  value DummyTemp, *VarVal;
  if      (xyz==0) VarName = "x";
  else if (xyz==1) VarName = "y";
  else             VarName = "z";

  // Look up variable in user space and get pointer to its value
  DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&VarVal);
  if (VarVal!=NULL)
   {
    DummyTemp = *VarVal;
    if ((VarVal->string != NULL) || ((VarVal->FlagComplex) && (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)) || (!gsl_finite(VarVal->real)) || (!gsl_finite(VarVal->imag))) { ppl_units_zero(VarVal); VarVal->real=1.0; } // Turn string variables into floats
   }
  else
   {
    ppl_units_zero(&DummyTemp);
    DictAppendValue(_ppl_UserSpace_Vars, VarName, DummyTemp);
    DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&VarVal);
    DummyTemp.modified = 2;
   }

  // Set value of x (or y/z)
  *VarVal = *xunit;
  VarVal->imag        = 0.0;
  VarVal->FlagComplex = 0;
  VarVal->real        = x;

  // Generate tick string
  i = strlen(FormatStr);
  while ((i>0)&&(FormatStr[i-1]<=' ')) i--;
  j = -1;
  ppl_GetQuotedString(FormatStr, tmp_string, 0, &j, 0, &k, err_string, 1);
  if (k>=0) { sprintf(temp_err_string, "Error encountered whilst using format string: %s",FormatStr); ppl_error(ERR_GENERAL, temp_err_string); ppl_error(ERR_GENERAL, err_string); sprintf(tmp_string, "{\\bf ?}"); }
  if (j< i) { sprintf(temp_err_string, "Error encountered whilst using format string: %s",FormatStr); ppl_error(ERR_GENERAL, temp_err_string); ppl_error(ERR_GENERAL, "Unexpected trailing matter."); sprintf(tmp_string, "{\\bf ?}"); }
  *output = (char *)lt_malloc(strlen(tmp_string)+3);
  if ((*output)==NULL) return;
  sprintf(*output,"%s",tmp_string);

  // Restore original value of x (or y/z)
  *VarVal = DummyTemp;
  return;
 }

