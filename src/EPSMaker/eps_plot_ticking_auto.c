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

#define MAX_ARGS 32 /* Maximum number of substitution arguments in 'set format' which we analyse */
#define STEP_DUPLICITY 100 /* Controls how many steps we divide axis into, multiplied by max number of ticks which fit on axis */
#define MAX_TICKS_PER_INTERVAL 10 /* Maximum number of ticks which we file in any interval */
#define MAX_FACTORS 32 /* Maximum number of factors of LogBase which we consider */
#define FACTOR_MULTIPLY 10.0 /* Factorise 10*LogBase, so that 0.00,0.25,0.50,0.75,1.00 is a valid factorisation */
#define NOT_THROW 9999 /* Value we put in DivOfThrow when a tick isn't dividing throw (to get sorting right) */

// Structure used for storing information about a substitution argument in 'set format'
typedef struct ArgumentInfo {
 int      id, score, NValueChanges, Throw, FactorsThrow[MAX_FACTORS], NFactorsThrow;
 unsigned char StringArg, ContinuousArg, MinValueSet, MaxValueSet, vetoed, OoM_RangeSet;
 int      OoM_min, OoM_max;
 double   MinValue, MaxValue; // Work out range of values which this argument takes (aka the 'throw')
 double  *NumericValues; // Raster of values sampled along axis
 char   **StringValues;
} ArgumentInfo;

// Structure used when sorting a list of ints according to some double-precision 'score'
typedef struct ArgLeagueTableEntry {
 int id;
 double score;
} ArgLeagueTableEntry;

// Structure used for storing information about a potential tick which we might put on the axis
typedef struct PotentialTick {
 int ArgNo; // The argument whose changing is marked by this tick
 int DivOfThrow; // This tick divides up throw of argument in one of its factors
 int OoM; // The order of magnitude of the digit which changes on this tick
 int DivOfOoM; // Order of magnitude is being split up into factors of log base
 int IntervalNum; // Number of the interval in which this tick lies
 double TargetValue; // Target value of argument where this tick should be placed
 int OrderPosition; // Position of this tick in TickOrder
} PotentialTick;

// Compare two ArgumentInfo structures; used by qsort
int compare_arg(const void *x, const void *y)
 {
  const ArgumentInfo *xai, *yai;
  xai = (const ArgumentInfo *)x;
  yai = (const ArgumentInfo *)y;
  if ((!xai->vetoed) && ( yai->vetoed)) return -1;
  if (( xai->vetoed) && (!yai->vetoed)) return  1;
  if ((!xai->ContinuousArg) && ( yai->ContinuousArg)) return -1; // Discrete arguments more important than continuous
  if (( xai->ContinuousArg) && (!yai->ContinuousArg)) return  1;
  if (!xai->ContinuousArg)
   {
    if (xai->NValueChanges < yai->NValueChanges) return -1; // Slow-moving continuous arguments most important
    if (xai->NValueChanges > yai->NValueChanges) return  1;
   }
  else
   {
    if (xai->score < yai->score) return -1; // Slow-moving continuous arguments most important
    if (xai->score > yai->score) return  1;
   }
  return 0;
 }

// Compare two ArgLeagueTableEntry structures; used by qsort
int compare_ArgLeagueEntry(const void *x, const void *y)
 {
  const ArgLeagueTableEntry *xalte, *yalte;
  xalte = (const ArgLeagueTableEntry *)x;
  yalte = (const ArgLeagueTableEntry *)y;
  if (xalte->score > yalte->score) return -1;
  if (xalte->score < yalte->score) return  1;
  return 0;
 }

// Compare two PotentialTick structures; used by qsort
int compare_PotentialTicks(const void *x, const void *y)
 {
  const PotentialTick *xpt, *ypt;
  xpt = (const PotentialTick *)x;
  ypt = (const PotentialTick *)y;
  if (xpt->ArgNo      > ypt->ArgNo     ) return  1;
  if (xpt->ArgNo      < ypt->ArgNo     ) return -1;
  if (xpt->DivOfThrow > ypt->DivOfThrow) return  1;
  if (xpt->DivOfThrow < ypt->DivOfThrow) return -1;
  if (xpt->OoM        > ypt->OoM       ) return -1;
  if (xpt->OoM        < ypt->OoM       ) return  1;
  if (xpt->DivOfOoM   > ypt->DivOfOoM  ) return  1;
  if (xpt->DivOfOoM   < ypt->DivOfOoM  ) return -1;
  return 0;
 }

// Return up to a maximum of MaxFactors factors of in. Factors are returned to array out.
static void factorise(int in, int *out, int MaxFactors, int *NFactors)
 {
  int i,j=0,N=sqrt(in);
  for (i=2; i<=N; i++) if (in%i==0) { out[j]=i; out[MaxFactors-1-j]=in/i; j++; if (j>=MaxFactors/2) break; }
  for (i=MaxFactors-j; i<=MaxFactors-1; i++) { out[j] = out[i]; if (out[j]!=out[j-1]) j++; }
  *NFactors=j;
  return;
 }

// Add a tick scheme to a list of accepted ticks
void AddTickScheme(const PotentialTick *PotTickList, const int NPotTicks, const ArgLeagueTableEntry *TickOrder, const int NIntervals, const int ArgNo, const int DivOfThrow, const int OoM, const int DivOfOoM, const unsigned char *TicksAcceptedIn, unsigned char *TicksAcceptedOut, const unsigned char MAJORminor, const double AxisLength, const double TickSepMin, unsigned char *FLAGacceptable, int *NTicksOut)
 {
  int i, j, id, imin=-1, imax=-1;
  *NTicksOut      = 0;
  *FLAGacceptable = 1;
  for (i=0; i<NPotTicks; i++)
   {
    TicksAcceptedOut[i] = (TicksAcceptedIn == NULL) ? 0 : TicksAcceptedIn[i];
    id = TickOrder[i].id;
    if (    (PotTickList[id].ArgNo      == ArgNo     )
         && (PotTickList[id].DivOfThrow == DivOfThrow)
         && (PotTickList[id].OoM        == OoM       )
         && (PotTickList[id].DivOfOoM   == DivOfOoM  )
         && (MAJORminor || (TicksAcceptedOut[i]==0)) )
     {
      TicksAcceptedOut[i] = MAJORminor ? 1 : 2;
      if ((imin<0) || (imin>PotTickList[id].IntervalNum)) imin = PotTickList[id].IntervalNum;
      if ((imax<0) || (imax<PotTickList[id].IntervalNum)) imax = PotTickList[id].IntervalNum;
     }
    if (TicksAcceptedOut[i] != 0) (*NTicksOut)++;
    if (MAJORminor && (TicksAcceptedOut[i]==1)) // Check that no two major ticks are too close together
     {
      for (j=i-1; j>=0; j--)
       if (TicksAcceptedOut[j]==1)
        {
         double gap = fabs(PotTickList[id].IntervalNum - PotTickList[TickOrder[j].id].IntervalNum) * AxisLength / (NIntervals-1);
         if (gap < TickSepMin) *FLAGacceptable = 0; // Gap between this tick and that to our left is too small
         break;
        }
     }
   }
  if (!MAJORminor) // Check that minor ticks don't exceed maximum allowed density
   {
    int Nnewticks=0;
    for (i=0; i<NPotTicks; i++)
     {
      id = PotTickList[ TickOrder[i].id ].IntervalNum;
      if ((TicksAcceptedOut[i] != 0) && (id>=imin) && (id<=imax)) Nnewticks++;
     }
    *FLAGacceptable = (unsigned char)(AxisLength*(imax-imin)/(NIntervals-1)/Nnewticks > TickSepMin);
   }
  return;
 }

// Take a list of accepted ticks and convert these into a final TickList to associate with axis
static int AutoTickListFinalise(settings_axis *axis, const int xyz, const double UnitMultiplier, value *VarVal, const int NIntervals, char *format, const int start, const int *CommaPositions, const ArgumentInfo *args, const PotentialTick *PotTickList, const int NPotTicks, const ArgLeagueTableEntry *TickOrder, const unsigned char *TicksAccepted, int OutContext)
 {
  int    i, jMAJ, jMIN, k, l, Nmajor=0, Nminor=0;
  double axispos, x;
  value  DummyVal;

  // Count number of accepted ticks
  for (i=0; i<NPotTicks; i++)
   {
    if      (TicksAccepted[i]==1) Nmajor++;
    else if (TicksAccepted[i]==2) Nminor++;
   }

  // Malloc list of accepted ticks
  axis-> TickListPositions = (double  *)lt_malloc_incontext((Nmajor+1) * sizeof(double), OutContext);
  axis-> TickListStrings   = (char   **)lt_malloc_incontext((Nmajor+1) * sizeof(char *), OutContext);
  axis->MTickListPositions = (double  *)lt_malloc_incontext((Nminor+1) * sizeof(double), OutContext);
  axis->MTickListStrings   = (char   **)lt_malloc_incontext((Nminor+1) * sizeof(char *), OutContext);
  if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL) || (axis->MTickListPositions==NULL) || (axis->MTickListStrings==NULL)) goto FAIL;
  axis->TickListStrings[Nmajor] = axis->MTickListStrings[Nminor] = NULL; // null terminate lists

  // Make ticks
  for (i=jMAJ=jMIN=0; i<NPotTicks; i++)
   if (TicksAccepted[i]>0)
    {
     const PotentialTick *tick;
     const ArgumentInfo  *arg;
     double axispos_min, axispos_max, axispos_mid;
     unsigned char DiscreteMoveMin, SlopePositive;

     // Find precise location of tick
     tick          = &PotTickList[ TickOrder[i].id ];
     arg           = &args[ tick->ArgNo ];
     axispos_min   = ((double)tick->IntervalNum - 1.0)/(NIntervals-1);
     axispos_max   = ((double)tick->IntervalNum      )/(NIntervals-1);
     SlopePositive = (arg->NumericValues[tick->IntervalNum] >= arg->NumericValues[tick->IntervalNum-1]);

     while ((axispos_max - axispos_min) > 1e-15*axispos_max)
      {
       axispos_mid = (axispos_min+axispos_max)/2;
       VarVal->real = eps_plot_axis_InvGetPosition(axispos_mid, axis) * UnitMultiplier;

       k=l=-1;
       if (arg->StringArg) // Evaluate argument at midpoint of the interval we know it to be in
        {
         ppl_GetQuotedString(format+start+CommaPositions[tick->ArgNo]+1, temp_err_string, 0, &k, 0, &l, temp_err_string, 1);
         if (l>=0) temp_err_string[0]='\0';
         DiscreteMoveMin = (strcmp(temp_err_string, arg->StringValues[tick->IntervalNum-1])==0);
        }
       else
        {
         ppl_EvaluateAlgebra(format+start+CommaPositions[tick->ArgNo]+1, &DummyVal, 0, &k, 0, &l, temp_err_string, 1);
         if (l>=0) DummyVal.real = GSL_NAN;
         DiscreteMoveMin = (DummyVal.real == arg->NumericValues[tick->IntervalNum-1]);
        }

       // Decide whether to pick left half of interval or right half
       if (!arg->ContinuousArg)
        {
         if (DiscreteMoveMin) axispos_min = axispos_mid;
         else                 axispos_max = axispos_mid;
        }
       else
        {
         if (SlopePositive ^ (DummyVal.real >= tick->TargetValue)) axispos_min = axispos_mid;
         else                                                      axispos_max = axispos_mid;
        }
      }
     axispos = (axispos_min+axispos_max)/2;
     x       = eps_plot_axis_InvGetPosition(axispos, axis);

     // File this tick
     if (TicksAccepted[i]==1) // Major tick, with label
      {
       axis->TickListPositions[jMAJ] = axispos;
       if (axis->format == NULL) TickLabelAutoGen   (&axis->TickListStrings[jMAJ], x * UnitMultiplier, axis->LogBase, OutContext);
       else                      TickLabelFromFormat(&axis->TickListStrings[jMAJ], axis->format, x, &axis->DataUnit, xyz, OutContext);
       if (axis->TickListStrings[jMAJ]==NULL) goto FAIL;
       jMAJ++;
      }
     else if (TicksAccepted[i]==2) // Minor tick, without label
      {
       axis->MTickListPositions[jMIN] = axispos;
       axis->MTickListStrings  [jMIN] = "";
       jMIN++;
      }
    }

  return 0;

FAIL:
  axis-> TickListPositions = NULL;
  axis-> TickListStrings   = NULL;
  axis->MTickListPositions = NULL;
  axis->MTickListStrings   = NULL;
  return 1;
 }

// Main entry point for automatic ticking of axes
void eps_plot_ticking_auto(settings_axis *axis, int xyz, double UnitMultiplier, unsigned char *AutoTicks, double length, double tick_sep_major, double tick_sep_minor)
 {
  int    i, j, k, l, start, N, NArgs, OutContext, ContextRough=-1, CommaPositions[MAX_ARGS+2], NFactorsLogBase, LogBase;
  int    FactorsLogBase[MAX_FACTORS];
  int    N_STEPS;
  char  *format, VarName[2]="\0\0", FormatTemp[32], QuoteType, *DummyStr;
  value  CentralValue, *VarVal=NULL, DummyTemp, DummyVal;
  ArgumentInfo *args;
  ArgLeagueTableEntry *ArgLeagueTable, *TickOrder;
  PotentialTick *PotTickList;
  int NPotTicks, NPotTicksMax;
  unsigned char *TicksAccepted, *TicksAcceptedNew, *TicksAcceptedNew2;

  N_STEPS = (2 + length/tick_sep_major) * STEP_DUPLICITY; // Number of intervals into which we divide axis

  // Make temporary rough workspace
  OutContext   = lt_GetMemContext();
  ContextRough = lt_DescendIntoNewContext();
  DummyStr     = (char *)lt_malloc(LSTR_LENGTH);
  if (DummyStr==NULL) goto FAIL;

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

  // Malloc structure to hold argument info
  args = (ArgumentInfo *)lt_malloc(NArgs * sizeof(ArgumentInfo));
  if (args==NULL) goto FAIL;
  ArgLeagueTable = (ArgLeagueTableEntry *)lt_malloc(NArgs * sizeof(ArgLeagueTableEntry));
  if (ArgLeagueTable==NULL) goto FAIL;

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
    if (j<0) { args[i].StringArg=0; continue; }
    j=k=-1;
    ppl_GetQuotedString(format+start+CommaPositions[i]+1,  DummyStr, 0, &k, 0, &j, temp_err_string, 1);
    if (j<0) { args[i].StringArg=1; continue; }
    goto FAIL;
   }

  // Sample arguments at equally-spaced intervals along axis
  for (i=0; i<NArgs; i++)
   {
    args[i].id            = i;
    args[i].score         = 0;
    args[i].vetoed        = 0;
    args[i].NValueChanges = 0;
    args[i].MinValueSet   = 0;
    args[i].MaxValueSet   = 0;
    args[i].StringValues  = (char  **)lt_malloc(N_STEPS * sizeof(char *));
    args[i].NumericValues = (double *)lt_malloc(N_STEPS * sizeof(double));
    args[i].OoM_RangeSet  = 0;
    args[i].OoM_min       = 0;
    args[i].OoM_max       = 0;
   }
  for (j=0; j<N_STEPS; j++)
   {
    VarVal->real = eps_plot_axis_InvGetPosition(j/(N_STEPS-1.0), axis) * UnitMultiplier;
    for (i=0; i<NArgs; i++)
     {
      k=l=-1;
      if (args[i].StringArg)
       {
        ppl_GetQuotedString(format+start+CommaPositions[i]+1,  DummyStr, 0, &k, 0, &l, temp_err_string, 1);
        if (l>=0)   args[i].StringValues[j]="";
        else      { args[i].StringValues[j]=(char *)lt_malloc(strlen(DummyStr)+1); strcpy(args[i].StringValues[j], DummyStr); }
        if ((j>0) && (strcmp(args[i].StringValues[j],args[i].StringValues[j-1])!=0)) args[i].NValueChanges++;
       }
      else
       {
        ppl_EvaluateAlgebra(format+start+CommaPositions[i]+1, &DummyVal, 0, &k, 0, &l, temp_err_string, 1);
        if (l>=0) DummyVal.real = GSL_NAN;
        args[i].NumericValues[j] = DummyVal.real;
        if ((j>0) && (args[i].NumericValues[j]!=args[i].NumericValues[j-1])) args[i].NValueChanges++;
        if ((gsl_finite(DummyVal.real)) && ((!args[i].MinValueSet) || (DummyVal.real < args[i].MinValue))) { args[i].MinValue = DummyVal.real; args[i].MinValueSet=1; }
        if ((gsl_finite(DummyVal.real)) && ((!args[i].MaxValueSet) || (DummyVal.real > args[i].MaxValue))) { args[i].MaxValue = DummyVal.real; args[i].MaxValueSet=1; }
       }
     }
   }
  for (i=0; i<NArgs; i++)
   {
    args[i].ContinuousArg = (!args[i].StringArg) && (args[i].NValueChanges>N_STEPS/4);
    args[i].vetoed        = (!args[i].ContinuousArg) && (args[i].NValueChanges>N_STEPS/STEP_DUPLICITY); // Discrete argument changes too fast to be useful
   }

  // Work out throw of each argument (i.e. the spread of values that it takes)
  for (i=0; i<NArgs; i++)
   {
    double throw;
    throw = args[i].MaxValue - args[i].MinValue;
    if ((fabs(throw - floor(throw+0.5))<1e-10) && (fabs(fmod(throw,2.0)-1.0)<0.1)) throw = ceil(throw+0.5);
    throw = ceil(throw - 1e-10);
    if ((throw>1e6) || (args[i].MinValue < INT_MIN+10) || (args[i].MaxValue > INT_MAX-10)) args[i].Throw = 0.0;
    else                                                                                   args[i].Throw = throw;
    args[i].MinValue = floor(args[i].MinValue);
    factorise(args[i].Throw*FACTOR_MULTIPLY, args[i].FactorsThrow, MAX_FACTORS, &args[i].NFactorsThrow);
   }

  // Work out factors of log base of axis. Multiply by FACTOR_MULTIPLY so that ten divides by four.
  LogBase = (axis->log == SW_BOOL_TRUE) ? axis->LogBase : 10;
  factorise(LogBase*FACTOR_MULTIPLY, FactorsLogBase, MAX_FACTORS, &NFactorsLogBase);

  // Compile scores of how fast each continuous argument moves
  for (j=1; j<N_STEPS; j++)
   {
    for (i=0; i<NArgs; i++)
     {
      ArgLeagueTable[i].id    = i;
      ArgLeagueTable[i].score = args[i].StringArg ? 0.0 : fabs(args[i].NumericValues[j] - args[i].NumericValues[j-1]);
     }
    qsort((void *)ArgLeagueTable, NArgs, sizeof(ArgLeagueTableEntry), compare_ArgLeagueEntry);
    for (i=0; i<NArgs; i++) args[ArgLeagueTable[i].id].score += NArgs-i;
   }

  // Sort arguments
  qsort((void *)args, NArgs, sizeof(ArgumentInfo), compare_arg);

  // Malloc table of potential ticks
  NPotTicks    = 0;
  NPotTicksMax = NArgs * N_STEPS * MAX_TICKS_PER_INTERVAL;
  PotTickList  = (PotentialTick *)lt_malloc(NPotTicksMax * sizeof(PotentialTick));

  // Generate list of potential ticks

#define ADDTICK(A,B,C,D,E,F) \
   if (NPotTicks<NPotTicksMax) \
    { \
     PotTickList[NPotTicks].ArgNo       = A; \
     PotTickList[NPotTicks].DivOfThrow  = B; \
     PotTickList[NPotTicks].OoM         = C; \
     PotTickList[NPotTicks].DivOfOoM    = D; \
     PotTickList[NPotTicks].TargetValue = E; \
     PotTickList[NPotTicks].IntervalNum = F; \
     NPotTicks++; \
    }

  for (j=1; j<N_STEPS; j++)
   {
    for (i=0; i<NArgs; i++)
     if (!args[i].vetoed)
      {
       if (!args[i].ContinuousArg) // Discrete arguments... note everywhere where they change
        {
         if (   (( args[i].StringArg) && (strcmp(args[i].StringValues[j],args[i].StringValues[j-1])!=0))
             || ((!args[i].StringArg) && (args[i].NumericValues[j]!=args[i].NumericValues[j-1]))         )
          {
           ADDTICK(i, 0, 0, 0, 0.0, j);
          }
        }
       else // Continuous arguments... study where they change OoM
        {
         // Ticks which mark factors of throw
         for (k=0; k<args[i].NFactorsThrow; k++)
          {
           int n,m; double nd;
           if (NPotTicks>=NPotTicksMax) break;
           nd = (args[i].NumericValues[j-1] - args[i].MinValue) * FACTOR_MULTIPLY / args[i].FactorsThrow[k];
           n = floor(nd + 0.5);
           if ((j==1) && (fabs(nd-n)<1e-12)) // A tick should go on the left extreme of the axis
            { ADDTICK(i,k,0,0,(args[i].MinValue + n/FACTOR_MULTIPLY*args[i].FactorsThrow[k]),j); }
           n = floor((args[i].NumericValues[j-1] - args[i].MinValue) * FACTOR_MULTIPLY / args[i].FactorsThrow[k]);
           m = floor((args[i].NumericValues[j  ] - args[i].MinValue) * FACTOR_MULTIPLY / args[i].FactorsThrow[k]);
           if (n!=m) { ADDTICK(i,k,0,0,(args[i].MinValue + ((n>m)?n:m) / FACTOR_MULTIPLY * args[i].FactorsThrow[k]),j); }
          }

         // Ticks which mark the changes of the Nth significant digit
         if (    ((args[i].NumericValues[j-1]!=0)&&(args[i].NumericValues[j]==0))
              || ((args[i].NumericValues[j-1]==0) && (j==1))
              || ((args[i].NumericValues[j-1]< 0)&&(args[i].NumericValues[j]> 0))
              || ((args[i].NumericValues[j-1]> 0)&&(args[i].NumericValues[j]< 0))
            )
         {
          ADDTICK(i,NOT_THROW,INT_MAX,-3,0.0,j); // Zero should be marked in this interval
         }
         else if ((args[i].NumericValues[j-1]!=0)||(args[i].NumericValues[j]!=0))
         {
          const double xn     = args[i].NumericValues[j-1];
          const double yn     = args[i].NumericValues[j  ];
          const double OoM_n  = log(fabs(args[i].NumericValues[j-1])) / log(LogBase);
          const double OoM_m  = log(fabs(args[i].NumericValues[j  ])) / log(LogBase);
          const int    Nsteps = fabs(log(1e-15) / log(LogBase));
          double OoM;
          int    n;
          if      (!gsl_finite(OoM_n)) OoM = floor(OoM_m);
          else if (!gsl_finite(OoM_m)) OoM = floor(OoM_n);
          else                         OoM = floor(max(OoM_n , OoM_m));

          // Should a tick go on the left extreme of the axis?
          if (j==1)
           {
            for (n=0; n<Nsteps; n++)
             {
              double divisor = pow(LogBase , OoM-n);
              if ((gsl_finite(divisor)) && (fabs(floor(xn/divisor+0.5)-xn/divisor)<1e-12))
               {
                int priority = 0;
                if (n==0) { priority = -1; if (fabs(floor(xn/divisor+0.5))<2) priority = -2; }
                if ((!args[i].OoM_RangeSet)||(OoM-n<args[i].OoM_min)) args[i].OoM_min = OoM-n;
                if ((!args[i].OoM_RangeSet)||(OoM-n>args[i].OoM_max)) args[i].OoM_max = OoM-n;
                args[i].OoM_RangeSet = 1;
                ADDTICK(i,NOT_THROW,OoM-n,priority,(floor(xn/divisor+0.5)*divisor),j);
                break;
               }
             }
           }
          // Should tick go within body of interval j-1 -> j?
          for (n=0; n<Nsteps; n++)
           {
            double divisor = pow(LogBase , OoM-n);
            if ((gsl_finite(divisor)) && (floor(xn/divisor) != floor(yn/divisor)))
             {
              int priority = 0;
              if (n==0) { priority = -1; if ((floor(xn/divisor)==0) || (floor(xn/divisor)==-1) || (floor(yn/divisor)==0) || (floor(xn/divisor)==-1)) priority = -2; }
              if ((!args[i].OoM_RangeSet)||(OoM-n<args[i].OoM_min)) args[i].OoM_min = OoM-n;
              if ((!args[i].OoM_RangeSet)||(OoM-n>args[i].OoM_max)) args[i].OoM_max = OoM-n;
              args[i].OoM_RangeSet = 1;
              ADDTICK(i,NOT_THROW,OoM-n,priority,(floor(max(xn,yn)/divisor)*divisor),j);
              break;
             }
           }

          // Ticks which mark the passing of fractions of the Nth significant digit
          {
          }
         }

        }
      }
   }

  // Sort list of potential ticks
  qsort((void *)PotTickList, NPotTicks, sizeof(PotentialTick), compare_PotentialTicks);

  // Create look-up table of ticks in order of position along axis
  TickOrder = (ArgLeagueTableEntry *)lt_malloc(NPotTicks * sizeof(ArgLeagueTableEntry));
  if (TickOrder==NULL) goto FAIL;
  for (i=0; i<NPotTicks; i++)
   {
    TickOrder[i].id    =  i;
    TickOrder[i].score = -PotTickList[i].IntervalNum;
   }
  qsort((void *)TickOrder, NPotTicks, sizeof(ArgLeagueTableEntry), compare_ArgLeagueEntry);
  for (i=0; i<NPotTicks; i++) PotTickList[ TickOrder[i].id ].OrderPosition = i;

  // Debugging lines
  if (DEBUG && (VarName[0]!='z'))
   {
    sprintf(temp_err_string,"Potential ticks for %s axis (NArgs = %d)",VarName,NArgs); ppl_log(temp_err_string);
    for (i=0; i<NArgs; i++) { sprintf(temp_err_string, "Argument %d: id %d score %d StringArg %d ContinuousArg %d Vetoed %d NValueChanges %d",i,(int)args[i].id,(int)args[i].score,(int)args[i].StringArg,(int)args[i].ContinuousArg,(int)args[i].vetoed,args[i].NValueChanges); ppl_log(temp_err_string); }
    sprintf(temp_err_string, "Number of potential ticks: %d", NPotTicks); ppl_log(temp_err_string);
    for (i=0; ((i<NPotTicks)&&(i<64)); i++) { sprintf(temp_err_string, "Tick %4d: Arg %3d DivOfThrow %4d OoM %3d DivOfOoM %3d IntervalNum %3d TargetVal %e",PotTickList[i].OrderPosition,PotTickList[i].ArgNo,PotTickList[i].DivOfThrow,PotTickList[i].OoM,PotTickList[i].DivOfOoM,PotTickList[i].IntervalNum,PotTickList[i].TargetValue); ppl_log(temp_err_string); }
   }

  // Make arrays for noting which ticks have been accepted
  TicksAccepted     = (unsigned char *)lt_malloc(NPotTicks);
  TicksAcceptedNew  = (unsigned char *)lt_malloc(NPotTicks);
  TicksAcceptedNew2 = (unsigned char *)lt_malloc(NPotTicks);
  for (i=0; i<NPotTicks; i++) TicksAccepted    [i] = 0;
  for (i=0; i<NPotTicks; i++) TicksAcceptedNew [i] = 0;
  for (i=0; i<NPotTicks; i++) TicksAcceptedNew2[i] = 0;

  // Start investigating which tick schemes to accept
  for (i=0; i<NArgs; i++)
   if (!args[i].vetoed)
    {
     unsigned char acceptable=1;

     if (!args[i].ContinuousArg) // Discrete arguments... note everywhere where they change
      {
       int Nticks;

       // Try marking discrete changes to arguments with major ticks
       AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, 0, 0, 0, TicksAccepted, TicksAcceptedNew,
                     1, length, tick_sep_major, &acceptable, &Nticks);
       if (acceptable)
        {
         if (DEBUG) { sprintf(temp_err_string, "Accepted major ticks to mark changes of argument %d (now %d ticks).", i, Nticks); ppl_log(temp_err_string); }
         memcpy(TicksAccepted, TicksAcceptedNew, NPotTicks); // Update TicksAccepted
         continue;
        }

       // Try marking discrete changes to arguments with minor ticks
       AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, 0, 0, 0, TicksAccepted, TicksAcceptedNew,
                     0, length, tick_sep_minor, &acceptable, &Nticks);
       if (acceptable)
        {
         if (DEBUG) { sprintf(temp_err_string, "Accepted minor ticks to mark changes of argument %d (now %d ticks).", i, Nticks); ppl_log(temp_err_string); }
         memcpy(TicksAccepted, TicksAcceptedNew, NPotTicks); // Update TicksAccepted
         continue; 
        } 
      }
     else // Continuous arguments are more complicated
      {
       int ThrowFactors_Nticks=-1, ThrowFactors_FactNum=-1;
       int Divide_LogBase_Nticks=-1, Nticks_new, Nticks_new_accepted, Nticks_new_prev, l;
       unsigned char MAJORminor;

       // Option 1: Divide throw
       for (k=0; k<args[i].NFactorsThrow; k++)
        {
         acceptable=1;
         AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, k, 0, 0, TicksAccepted, TicksAcceptedNew,
                       1, length, tick_sep_major, &acceptable, &ThrowFactors_Nticks);
         if (acceptable) { ThrowFactors_FactNum =  k; break; }
         else            { ThrowFactors_Nticks  = -1; }
        }

       // Option 2: Divide log base
       if (DEBUG) { sprintf(temp_err_string, "Checking for ticks which divide OoMs %d to %d.",args[i].OoM_max,args[i].OoM_min); ppl_log(temp_err_string); }
       if (args[i].OoM_RangeSet)
        {
         memcpy(TicksAcceptedNew, TicksAccepted, NPotTicks);
         Nticks_new_prev=-1;
         for (k=args[i].OoM_max; k>=args[i].OoM_min; k--)
          {
           acceptable=1;
           Nticks_new_accepted=-1;
           MAJORminor = 1;
           if (k==args[i].OoM_max)
              AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, NOT_THROW, INT_MAX, -3, TicksAcceptedNew, TicksAcceptedNew,
                            1, length, tick_sep_major, &acceptable, &Nticks_new); // Must zero, if present, with highest OoM
           for (l=-2; l<=0; l++)
            {
             AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, NOT_THROW, k, l, TicksAcceptedNew, TicksAcceptedNew2,
                           MAJORminor, length, MAJORminor ? tick_sep_major : tick_sep_minor, &acceptable, &Nticks_new);
             if (DEBUG) { sprintf(temp_err_string, "OoM %3d Priority %2d %s %s (Nticks: %d).",k,l,MAJORminor?"major":"minor",acceptable?"pass":"fail",Nticks_new); ppl_log(temp_err_string); }
             if (acceptable) { Nticks_new_accepted=Nticks_new; memcpy(TicksAcceptedNew, TicksAcceptedNew2, NPotTicks); }
             else            { if (MAJORminor) { MAJORminor=0; l--; continue; } else { break; } }
            }
           if ((Nticks_new_accepted<=0) || (Nticks_new_accepted<=Nticks_new_prev)) break;
           Nticks_new_prev = Nticks_new_accepted;
          }
         Divide_LogBase_Nticks = Nticks_new_prev;
        }

       // Choose which option produced best result
       if (DEBUG) { sprintf(temp_err_string,"Dividing by factors of throw produced %d major ticks.", ThrowFactors_Nticks); ppl_log(temp_err_string); }
       if (DEBUG) { sprintf(temp_err_string,"Dividing by factors of log base produced %d major ticks.", Divide_LogBase_Nticks); ppl_log(temp_err_string); }
       if (ThrowFactors_Nticks > Divide_LogBase_Nticks)
        {
         if (ThrowFactors_Nticks > 0) // Mark factors of throw of argument
          {
           acceptable=1;
           AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, ThrowFactors_FactNum, 0, 0, TicksAccepted, TicksAcceptedNew,
                         1, length, tick_sep_major, &acceptable, &ThrowFactors_Nticks);
           if (acceptable) memcpy(TicksAccepted, TicksAcceptedNew, NPotTicks); // Update TicksAccepted

           // Try other factors of throw for minor ticks
           for (k=0; k<ThrowFactors_FactNum; k++)
            if ((args[i].FactorsThrow[ThrowFactors_FactNum] % args[i].FactorsThrow[k])==0)
             {
              acceptable=1;
              AddTickScheme(PotTickList, NPotTicks, TickOrder, N_STEPS, i, k, 0, 0, TicksAccepted, TicksAcceptedNew,
                            0, length, tick_sep_minor, &acceptable, &ThrowFactors_Nticks);
              if (acceptable) { memcpy(TicksAccepted, TicksAcceptedNew, NPotTicks); break; }
             }
          }
        }
       else
        {
         if (Divide_LogBase_Nticks > 0) // Mark places where argument is a significant round number
          {
           memcpy(TicksAccepted, TicksAcceptedNew, NPotTicks);
          }
        }
      }
    }

  // Finalise list of ticks
  AutoTickListFinalise(axis, xyz, UnitMultiplier, VarVal, N_STEPS, format, start, CommaPositions, args, PotTickList, NPotTicks, TickOrder, TicksAccepted, OutContext);
  goto CLEANUP;

FAIL:

  // A very simple way of putting ticks on axes when clever logic fails
  N = 1 + length/tick_sep_major; // Estimate how many ticks we want
  if (N<  3) N=  3;
  if (N>100) N=100;
  
  axis->TickListPositions = (double  *)lt_malloc_incontext((N+1) * sizeof(double), OutContext);
  axis->TickListStrings   = (char   **)lt_malloc_incontext((N+1) * sizeof(char *), OutContext);
  if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; goto CLEANUP; }
  for (i=0; i<N; i++)
   {
    double x;
    x = ((double)i)/(N-1);
    axis->TickListPositions[i] = x;
    x = eps_plot_axis_InvGetPosition(x, axis);
    if (axis->format == NULL) TickLabelAutoGen(&axis->TickListStrings[i] , x * UnitMultiplier , axis->LogBase, OutContext);
    else                      TickLabelFromFormat(&axis->TickListStrings[i], axis->format, x, &axis->DataUnit, xyz, OutContext);
    if (axis->TickListStrings[i]==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; goto CLEANUP; }
   } 
  axis->TickListStrings[i] = NULL; // null terminate list

CLEANUP:
  // Restore original value of x (or y/z)
  if (VarVal!=NULL) *VarVal = DummyTemp;

  // Delete rough workspace
  if (ContextRough>0) lt_AscendOutOfContext(ContextRough);
  return;
 }

