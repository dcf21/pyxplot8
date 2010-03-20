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

#define MAX_ARGS 32
#define STEP_DUPLICITY 100
#define MAX_TICKS_PER_INTERVAL 10
#define MAX_FACTORS 32

typedef struct ArgumentInfo {
 int      id, score, NValueChanges, Throw, FactorsThrow[MAX_FACTORS], NFactorsThrow;
 unsigned char StringArg, ContinuousArg, MinValueSet, MaxValueSet, vetoed;
 double   MinValue, MaxValue;
 double  *NumericValues;
 char   **StringValues;
} ArgumentInfo;

typedef struct ArgLeagueTableEntry {
 int id;
 double score;
} ArgLeagueTableEntry;

typedef struct PotentialTick {
 int ArgNo; // The argument whose changing is marked by this tick
 int DivOfThrow; // This tick divides up throw of argument in one of its factors
 int OoM; // The order of magnitude of the digit which changes on this tick
 int DivOfOoM; // Order of magnitude is being split up into factors of log base
 int IntervalNum; // Number of the interval in which this tick lies
 double TargetValue; // Target value of argument where this tick should be placed
} PotentialTick;

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

int compare_ArgLeagueEntry(const void *x, const void *y)
 {
  const ArgLeagueTableEntry *xalte, *yalte;
  xalte = (const ArgLeagueTableEntry *)x;
  yalte = (const ArgLeagueTableEntry *)y;
  if (xalte->score > yalte->score) return -1;
  if (xalte->score < yalte->score) return  1;
  return 0;
 }

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

static void factorise(int in, int *out, int MaxFactors, int *NFactors)
 {
  int i,j=0,N=sqrt(in);
  for (i=2; i<=N; i++) if (in%i==0) { out[j]=i; out[MaxFactors-1-j]=in/i; j++; if (j>=MaxFactors/2) break; }
  for (i=MaxFactors-j; i<=MaxFactors-1; i++) { out[j] = out[i]; if (out[j]!=out[j-1]) j++; }
  *NFactors=j;
  return;
 }

void eps_plot_ticking_auto(settings_axis *axis, int xyz, double UnitMultiplier, unsigned char *AutoTicks, double length, double tick_sep_major, double tick_sep_minor)
 {
  int    i, j, k, l, start, N, NArgs, OutContext, ContextRough=-1, CommaPositions[MAX_ARGS+2], NFactorsLogBase, LogBase;
  int    FactorsLogBase[MAX_FACTORS];
  int    N_STEPS;
  char  *format, VarName[2]="\0\0", FormatTemp[32], QuoteType, *DummyStr;
  value  CentralValue, *VarVal=NULL, DummyTemp, DummyVal;
  ArgumentInfo *args;
  ArgLeagueTableEntry *ArgLeagueTable;
  PotentialTick *PotTickList;
  int NPotTicks, NPotTicksThis;

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
   }
  for (j=0; j<N_STEPS; j++)
   {
    VarVal->real = eps_plot_axis_InvGetPosition(j/(N_STEPS-1.0), axis);
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

  // Work out throw of each argument
  for (i=0; i<NArgs; i++)
   {
    double throw;
    throw = args[i].MaxValue - args[i].MinValue;
    if ((fabs(throw - floor(throw+0.5))<1e-10) && (fabs(fmod(throw,2.0)-1.0)<0.1)) throw = ceil(throw+0.5);
    throw = ceil(throw - 1e-10);
    if ((throw>1e6) || (args[i].MinValue < INT_MIN+10) || (args[i].MaxValue > INT_MAX-10)) args[i].Throw = 0.0;
    else                                                                                   args[i].Throw = throw;
    args[i].MinValue = floor(args[i].MinValue);
    factorise(args[i].Throw, args[i].FactorsThrow, MAX_FACTORS, &args[i].NFactorsThrow);
   }

  // Work out factors of log base of axis
  LogBase = (axis->log == SW_BOOL_TRUE) ? axis->LogBase : 10;
  factorise(LogBase, FactorsLogBase, MAX_FACTORS, &NFactorsLogBase);

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
  NPotTicks   = 0;
  PotTickList = (PotentialTick *)lt_malloc(NArgs * N_STEPS * MAX_TICKS_PER_INTERVAL * sizeof(PotentialTick));

  // Generate list of potential ticks
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
           PotTickList[NPotTicks].ArgNo = i;
           PotTickList[NPotTicks].DivOfThrow = PotTickList[NPotTicks].OoM = PotTickList[NPotTicks].DivOfOoM = 0;
           PotTickList[NPotTicks].TargetValue = 0.0;
           PotTickList[NPotTicks].IntervalNum = j;
           NPotTicks++;
          }
        }
       else // Continuous arguments... study where they change OoM
        {
         NPotTicksThis = 0;

         // Ticks which mark factors of throw
         for (k=0; k<args[i].NFactorsThrow; k++)
          {
           int n,m;
           if (NPotTicksThis>=MAX_TICKS_PER_INTERVAL) break;
           n = floor((args[i].NumericValues[j-1] - args[i].MinValue) / args[i].FactorsThrow[k]);
           m = floor((args[i].NumericValues[j  ] - args[i].MinValue) / args[i].FactorsThrow[k]);
           if (n!=m)
            {
             PotTickList[NPotTicks].ArgNo       = i;
             PotTickList[NPotTicks].DivOfThrow  = k;
             PotTickList[NPotTicks].OoM = PotTickList[NPotTicks].DivOfOoM = 0;
             PotTickList[NPotTicks].TargetValue = args[i].MinValue + ((n>m)?n:m) * args[i].FactorsThrow[k];
             PotTickList[NPotTicks].IntervalNum = j;
             NPotTicks++;
             NPotTicksThis++;
            }
          }

         // Ticks which mark the changes of the Nth significant digit

         // Ticks which mark the passing of fractions of the Nth significant digit

        }
      }
   }

  // Sort list of potential ticks
  qsort((void *)PotTickList, NPotTicks, sizeof(PotentialTick), compare_PotentialTicks);

//printf("\n%d\n",NArgs);
//for (i=0; i<NArgs; i++) printf("-- %d %d %d %d %d %d %d\n",i,(int)args[i].id,(int)args[i].score,(int)args[i].StringArg,(int)args[i].ContinuousArg,(int)args[i].vetoed,args[i].NValueChanges);
//printf("\n%d\n",NPotTicks);
//for (i=0; i<NPotTicks; i++) printf("## %d %d %d %d %d %e\n",PotTickList[i].ArgNo,PotTickList[i].DivOfThrow,PotTickList[i].OoM,PotTickList[i].DivOfOoM,PotTickList[i].IntervalNum,PotTickList[i].TargetValue);

FAIL:

  // Delete rough workspace
  if (ContextRough>0) lt_AscendOutOfContext(ContextRough);
  ContextRough=-1;

  // A very simple way of putting ticks on axes when clever logic fails
  N = 1 + length/tick_sep_major; // Estimate how many ticks we want
  if (N<  3) N=  3;
  if (N>100) N=100;
  
  axis->TickListPositions = (double  *)lt_malloc_incontext((N+1) * sizeof(double), OutContext);
  axis->TickListStrings   = (char   **)lt_malloc_incontext((N+1) * sizeof(char *), OutContext);
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

  // Delete rough workspace
  if (ContextRough>0) lt_AscendOutOfContext(ContextRough);
  return;
 }

