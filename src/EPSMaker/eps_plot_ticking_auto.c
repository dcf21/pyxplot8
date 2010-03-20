// eps_plot_ticking_auto.c
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

#define _PPL_EPS_PLOT_TICKING_AUTO_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ListTools/lt_memory.h"

#include "MathsTools/dcfmath.h"

#include "StringTools/asciidouble.h"

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_plot_canvas.h"
#include "eps_plot_ticking.h"
#include "eps_plot_ticking_auto.h"

#define MAX_ARGS   32

void eps_plot_ticking_auto(settings_axis *axis, int xyz, double UnitMultiplier, unsigned char *AutoTicks, double length, double tick_sep_major, double tick_sep_minor)
 {
  int            i, j, k, l, start, N, NArgs, OutContext, ContextRough, CommaPositions[MAX_ARGS+2], NValueChanges[MAX_ARGS];
  int            N_STEPS;
  unsigned char  StringArg[MAX_ARGS], ContinuousArg[MAX_ARGS], MinValueSet[MAX_ARGS], MaxValueSet[MAX_ARGS];
  char          *format, VarName[2]="\0\0", FormatTemp[32], QuoteType, *DummyStr, **StringValues[MAX_ARGS];
  value          CentralValue, *VarVal=NULL, DummyTemp, DummyVal;
  double        *NumericValues[MAX_ARGS], MinValue[MAX_ARGS], MaxValue[MAX_ARGS];

  N_STEPS = (2 + length/tick_sep_major) * 100; // Number of intervals into which we divide axis

  // Make temporary rough workspace
  OutContext   = lt_GetMemContext();
  ContextRough = lt_DescendIntoNewContext();
  DummyStr     = (char *)lt_malloc(LSTR_LENGTH);

  // Work through format string identifying substitution expressions
  VarName[0] = "xyz"[xyz];
  if (axis->format == NULL) { sprintf(FormatTemp, "\"%%s\"%%(%s)", VarName); format=FormatTemp; }
  else                      { format=axis->format; }
  for (i=0; ((format[i]!='\0')&&(format[i]!='\'')&&(format[i]!='\"')); i++);
  QuoteType = format[i];
  if (QuoteType=='\0') goto FAIL;
  i++;
  for (; ((format[i]!='\0')&&((format[i]!=QuoteType)||(format[i-1]=='\\'))); i++);
  if (format[i]!=QuoteType) goto FAIL;
  i++;
  for (;((format[i]<=' ')&&(format[i]!='\0'));i++);
  if (format[i]!='%') goto FAIL;
  i++;
  for (;((format[i]<=' ')&&(format[i]!='\0'));i++);
  if (format[i]!='(') goto FAIL;
  StrBracketMatch(format+i, CommaPositions, &NArgs, &j, MAX_ARGS);
  if (j<0) goto FAIL;
  if (NArgs<1) goto FAIL;
  start=i;

  // Look up variable x/y/z in user space and get pointer to its value
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

  // Identify string versus numeric arguments
  CentralValue = axis->DataUnit;
  CentralValue.FlagComplex = 0;
  CentralValue.imag = 0.0;
  CentralValue.real = eps_plot_axis_InvGetPosition(0.5, axis);
  *VarVal = CentralValue;
  for (i=0; i<NArgs; i++)
   {
    j=k=-1;
    ppl_EvaluateAlgebra(format+start+CommaPositions[i]+1, &DummyVal, 0, &k, 0, &j, temp_err_string, 1);
    if (j<0) { StringArg[i]=0; continue; }
    j=k=-1;
    ppl_GetQuotedString(format+start+CommaPositions[i]+1,  DummyStr, 0, &k, 0, &j, temp_err_string, 1);
    if (j<0) { StringArg[i]=1; continue; }
    goto FAIL;
   }

  // Sample arguments at equally-spaced intervals along axis
  for (i=0; i<NArgs; i++)
   {
    NValueChanges[i] = 0;
    MinValueSet[i]   = 0;
    MaxValueSet[i]   = 0;
    StringValues[i]  = (char  **)lt_malloc(N_STEPS * sizeof(char *));
    NumericValues[i] = (double *)lt_malloc(N_STEPS * sizeof(double));
   }
  for (j=0; j<N_STEPS; j++)
   {
    VarVal->real = eps_plot_axis_InvGetPosition(j/(N_STEPS-1.0), axis);
    for (i=0; i<NArgs; i++)
     {
      k=l=-1;
      if (StringArg[i])
       {
        ppl_GetQuotedString(format+start+CommaPositions[i]+1,  DummyStr, 0, &k, 0, &l, temp_err_string, 1);
        if (l>=0)   StringValues[i][j]="";
        else      { StringValues[i][j]=(char *)lt_malloc(strlen(DummyStr)+1); strcpy(StringValues[i][j], DummyStr); }
        if ((j>0) && (strcmp(StringValues[i][j],StringValues[i][j-1])!=0)) NValueChanges[i]++;
       }
      else
       {
        ppl_EvaluateAlgebra(format+start+CommaPositions[i]+1, &DummyVal, 0, &k, 0, &l, temp_err_string, 1);
        if (l>=0) DummyVal.real = GSL_NAN;
        NumericValues[i][j] = DummyVal.real;
        if ((j>0) && (NumericValues[i][j]!=NumericValues[i][j-1])) NValueChanges[i]++;
        if ((gsl_finite(DummyVal.real)) && ((!MinValueSet[i]) || (DummyVal.real < MinValue[i]))) { MinValue[i] = DummyVal.real; MinValueSet[i]=1; }
        if ((gsl_finite(DummyVal.real)) && ((!MaxValueSet[i]) || (DummyVal.real > MaxValue[i]))) { MaxValue[i] = DummyVal.real; MaxValueSet[i]=1; }
       }
     }
   }
  for (i=0; i<NArgs; i++) { ContinuousArg[i] = (NValueChanges[i]>N_STEPS/4); }

//printf("\n%d\n",NArgs);
//for (i=0; i<NArgs+1; i++) printf("-- %d %d %d\n",CommaPositions[i],(int)StringArg[i],NValueChanges[i]);

  // Delete rough workspace
  lt_AscendOutOfContext(ContextRough);


FAIL:
  // A very simple way of putting ticks on axes when clever logic fails
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

//CLEANUP:
  // Restore original value of x (or y/z)
  if (VarVal!=NULL) *VarVal = DummyTemp;
  return;
 }

