// ppl_interpolation.c
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

#define _PPL_INTERPOLATION_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_interp.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_spline.h>

#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_memory.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_glob.h"
#include "ppl_interpolation.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#define COUNTEDERR1 if (ErrCount > 0) { ErrCount--;
#define COUNTEDERR2 if (ErrCount==0) { sprintf(temp_err_string, "%s: Too many errors: no more errors will be shown.",filename); ppl_warning(ERR_STACKED, temp_err_string); } }

static long int __compare_offset;

int __compare(const void *x, const void *y)
 {
  if      (*(((double *)x)+__compare_offset) > *(((double *)y)+__compare_offset)) return  1.0;
  else if (*(((double *)x)+__compare_offset) < *(((double *)y)+__compare_offset)) return -1.0;
  else                                                                            return  0.0;
 }

int directive_interpolate(Dict *command, int mode)
 {
  DataTable *data;
  DataBlock *blk;
  long int   i, j, k, Nrows;
  int        ContextOutput, ContextLocalVec, ContextDataTab, status=0, index=-1, *indexptr, rowcol=DATAFILE_COL, continuity, ErrCount=DATAFILE_NERRS;
  char       errtext[LSTR_LENGTH], *cptr, *filename=NULL, *fitfunc=NULL, *tempstr=NULL, *SelectCrit=NULL;
  double    *xdata, *ydata;
  List      *UsingList=NULL, *EveryList=NULL;
  FunctionDescriptor *FuncPtr, *FuncPtrNext, *FuncPtr2;
  value      v, FirstEntries[2];
  SplineDescriptor *desc;

  List         *RangeList;
  ListIterator *ListIter;
  Dict         *TempDict;
  value        *xmin=NULL, *ymin=NULL, *xmax=NULL, *ymax=NULL;

  const gsl_interp_type *SplineType;
  gsl_spline            *SplineObj;

  // Expand filename if it contains wildcards
  DictLookup(command,"filename",NULL,(void **)(&cptr));
  if (cptr==NULL) ppl_error(ERR_INTERNAL, "File attribute not found in interpolate command.");
  filename = ppl_glob_oneresult(cptr);
  if (filename == NULL) return 1;

  status=0;
  DictLookup(command, "fit_function",NULL, (void **)&fitfunc);    if (fitfunc  == NULL) { ppl_error(ERR_INTERNAL, "ppl_interpolation could not read name of function for output."); return 1; }
  DictLookup(command, "index"      , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_rows"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_cols"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:", NULL, (void **)&UsingList);
  DictLookup(command, "every_list:", NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  DictLookup(command, "range_list", NULL, (void **)&RangeList);
  if (RangeList == NULL) goto RANGES_DONE;
  ListIter = ListIterateInit(RangeList);
  if (ListIter == NULL) goto RANGES_DONE;
  TempDict = (Dict *)ListIter->data;
  DictLookup(TempDict,"min",NULL,(void **)&xmin);
  DictLookup(TempDict,"max",NULL,(void **)&xmax);
  if ((xmin!=NULL)&&(xmax!=NULL)&&(!ppl_units_DimEqual(xmin,xmax))) { sprintf(temp_err_string, "The minimum and maximum limits specified in the interpolate command for the x axis have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", ppl_units_GetUnitStr(xmin,NULL,NULL,0,0), ppl_units_GetUnitStr(xmax,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
  ListIter = ListIterate(ListIter, NULL);
  if (ListIter == NULL) goto RANGES_DONE;
  TempDict = (Dict *)ListIter->data;
  DictLookup(TempDict,"min",NULL,(void **)&ymin);
  DictLookup(TempDict,"max",NULL,(void **)&ymax);
  if ((ymin!=NULL)&&(ymax!=NULL)&&(!ppl_units_DimEqual(ymin,ymax))) { sprintf(temp_err_string, "The minimum and maximum limits specified in the interpolate command for the y axis have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", ppl_units_GetUnitStr(ymin,NULL,NULL,0,0), ppl_units_GetUnitStr(ymax,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
  ListIter = ListIterate(ListIter, NULL);
  if (ListIter != NULL) { ppl_error(ERR_SYNTAX, "Too many ranges have been supplied to the interpolate command. Only two are allowed: one for each ordinate."); return 1; }

RANGES_DONE:

  continuity = DATAFILE_CONTINUOUS;

  if      (mode == INTERP_LINEAR) SplineType = gsl_interp_linear;
  else if (mode == INTERP_LOGLIN) SplineType = gsl_interp_linear;
  else if (mode == INTERP_SPLINE) SplineType = gsl_interp_cspline;
  else if (mode == INTERP_AKIMA ) SplineType = gsl_interp_akima;
  else if (mode == INTERP_POLYN ) SplineType = gsl_interp_polynomial;
  else                            { ppl_error(ERR_INTERNAL, "interpolate command requested to perform unknown type of interpolation."); return 1; }

  // Check that the function we're about to replace isn't a system function
  DictLookup(_ppl_UserSpace_Funcs, fitfunc, NULL, (void *)&FuncPtr); // Check whether we are going to overwrite an existing function
  if ((FuncPtr!=NULL)&&((FuncPtr->FunctionType==PPL_USERSPACE_SYSTEM)||(FuncPtr->FunctionType==PPL_USERSPACE_STRFUNC)||(FuncPtr->FunctionType==PPL_USERSPACE_UNIT)))
   { sprintf(temp_err_string, "Attempt to redefine a core system function %s()", fitfunc); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  ContextOutput  = lt_GetMemContext();
  ContextLocalVec= lt_DescendIntoNewContext();
  ContextDataTab = lt_DescendIntoNewContext();

  DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, 2, SelectCrit, continuity, &ErrCount);
  if (status) { ppl_error(ERR_GENERAL, errtext); return 1; }

  xdata = (double *)lt_malloc_incontext(2 * data->Nrows * sizeof(double), ContextLocalVec); // Transfer data from multiple data tables into single vectors
  ydata = xdata + data->Nrows;
  Nrows = data->Nrows;

  if ((xdata==NULL)||(ydata==NULL)) { ppl_error(ERR_MEMORY, "Out of memory whilst reading data from input file."); return 1; }

  // Copy data table into an array of x values, and an array of y values
  blk = data->first; i=0;
  while (blk != NULL)
   {
    k=i; for (j=0; j<blk->BlockPosition; j++) xdata[i++] = blk->data_real[0 + 2*j].d;
    i=k; for (j=0; j<blk->BlockPosition; j++) ydata[i++] = blk->data_real[1 + 2*j].d;
    blk=blk->next;
   }
  FirstEntries[0] = data->FirstEntries[0];
  FirstEntries[1] = data->FirstEntries[1];

  // Check that the FirstEntries above have the same units as any supplied ranges
  if      (xmin != NULL)
   {
    if (!ppl_units_DimEqual(xmin,FirstEntries+0)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the interpolate command for the x axis have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", ppl_units_GetUnitStr(xmin,NULL,NULL,0,0), ppl_units_GetUnitStr(FirstEntries+0,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
   }
  else if (xmax != NULL)
   {
    if (!ppl_units_DimEqual(xmax,FirstEntries+0)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the interpolate command for the x axis have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", ppl_units_GetUnitStr(xmax,NULL,NULL,0,0), ppl_units_GetUnitStr(FirstEntries+0,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
   }
  if      (ymin != NULL)
   {
    if (!ppl_units_DimEqual(ymin,FirstEntries+1)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the interpolate command for the y axis have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", ppl_units_GetUnitStr(ymin,NULL,NULL,0,0), ppl_units_GetUnitStr(FirstEntries+1,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
   }
  else if (ymax != NULL)
   {
    if (!ppl_units_DimEqual(ymax,FirstEntries+1)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the interpolate command for the y axis have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", ppl_units_GetUnitStr(ymax,NULL,NULL,0,0), ppl_units_GetUnitStr(FirstEntries+1,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
   }

  // Free original data table which is no longer needed
  lt_AscendOutOfContext(ContextDataTab);

  // Sort data vectors according to x values
  __compare_offset = -Nrows;
  qsort((void *)ydata, i, sizeof(double), __compare);
  __compare_offset = 0;
  qsort((void *)xdata, i, sizeof(double), __compare);

  // Filter out repeat values of x
  for (j=k=0; j<i; j++)
   {
    if ( (!gsl_finite(xdata[j])) || (!gsl_finite(ydata[j])) ) continue; // Ignore non-finite datapoints
    if ( (xmin!=NULL) && (xdata[j]<xmin->real) )              continue; // Ignore out-of-range datapoints
    if ( (xmax!=NULL) && (xdata[j]>xmax->real) )              continue; // Ignore out-of-range datapoints
    if ( (ymin!=NULL) && (ydata[j]<ymin->real) )              continue; // Ignore out-of-range datapoints
    if ( (ymax!=NULL) && (ydata[j]>ymax->real) )              continue; // Ignore out-of-range datapoints

    if ((j>0) && (xdata[j]==xdata[j-1])) { COUNTEDERR1; v=FirstEntries[0]; v.real=xdata[j]; sprintf(temp_err_string,"Repeat values for interpolation have been supplied at x=%s.",ppl_units_NumericDisplay(&v, 0, 0, 0)); ppl_warning(ERR_GENERAL, temp_err_string); COUNTEDERR2; continue; }

    if (mode == INTERP_LOGLIN)
     {
      if ((xdata[j]<=0.0) || (ydata[j]<=0.0)) { COUNTEDERR1; v=FirstEntries[0]; v.real=xdata[j]; sprintf(temp_err_string,"Negative or zero values are not allowed in power-law interpolation; negative values supplied at x=%s will be ignored.",ppl_units_NumericDisplay(&v, 0, 0, 0)); ppl_warning(ERR_NUMERIC, temp_err_string); COUNTEDERR2; continue; }
      xdata[k]=log(xdata[j]);
      ydata[k]=log(ydata[j]);
      if ( (!gsl_finite(xdata[k])) || (!gsl_finite(ydata[k])) ) continue;
     } else {
      xdata[k]=xdata[j];
      ydata[k]=ydata[j];
     }
    k++;
   }

  // Check that we have at least three points to interpolate
  if (k<3) { ppl_error(ERR_NUMERIC, "Interpolation is only possible on data sets with members at at least three distinct values of x."); return 1; }

  // Create GSL interpolation object
  SplineObj = gsl_spline_alloc(SplineType, k);
  if (SplineObj==NULL) { ppl_error(ERR_INTERNAL, "Failed to make interpolation object."); return 1; }
  status    = gsl_spline_init(SplineObj, xdata, ydata, k);
  if (status) { sprintf(temp_err_string, "Error whilst creating interpolation object: %s", gsl_strerror(status)); ppl_error(ERR_INTERNAL, temp_err_string); return 1; }

  // Generate a function descriptor for this spline
  desc              = (SplineDescriptor *)lt_malloc_incontext(sizeof(SplineDescriptor), 0);
  if (desc == NULL) { ppl_error(ERR_MEMORY, "Out of memory whilst adding interpolation object to function dictionary."); return 1; }
  desc->UnitX       = FirstEntries[0];
  desc->UnitY       = FirstEntries[1];
  desc->LogInterp   = (mode == INTERP_LOGLIN);
  desc->SplineObj   = SplineObj;
  desc->accelerator = gsl_interp_accel_alloc();
  if (desc->accelerator == NULL) { ppl_error(ERR_MEMORY, "Out of memory whilst adding interpolation object to function dictionary."); return 1; }
  desc->filename    = (char *)lt_malloc_incontext(strlen(filename)+1, 0);
  if (desc->filename == NULL) { ppl_error(ERR_MEMORY, "Out of memory whilst adding interpolation object to function dictionary."); return 1; }
  strcpy(desc->filename, filename);

  if      (mode == INTERP_LINEAR) desc->SplineType="Linear";
  else if (mode == INTERP_LOGLIN) desc->SplineType="Power-law";
  else if (mode == INTERP_SPLINE) desc->SplineType="Cubic spline";
  else if (mode == INTERP_AKIMA ) desc->SplineType="Akima spline";
  else if (mode == INTERP_POLYN ) desc->SplineType="Polynomial";
  else                            { ppl_error(ERR_INTERNAL, "interpolate command requested to perform unknown type of interpolation."); return 1; }

  // Make a new function descriptor
  FuncPtr2 = (FunctionDescriptor *)lt_malloc_incontext(sizeof(FunctionDescriptor), 0);
  if (FuncPtr2 == NULL) { ppl_error(ERR_MEMORY, "Out of memory whilst adding interpolation object to function dictionary."); return 1; }
  FuncPtr2->FunctionType    = PPL_USERSPACE_SPLINE;
  FuncPtr2->modified        = 1;
  FuncPtr2->NumberArguments = 1;
  FuncPtr2->FunctionPtr     = (void *)desc;
  FuncPtr2->ArgList         = NULL;
  FuncPtr2->min       = FuncPtr2->max       = NULL;
  FuncPtr2->MinActive = FuncPtr2->MaxActive = NULL;
  FuncPtr2->next            = NULL;
  FuncPtr2->description = FuncPtr2->LaTeX = NULL;

  // Supersede any previous function descriptor
  while (FuncPtr != NULL)
   {
    FuncPtrNext = FuncPtr->next;
    ppl_UserSpace_FuncDestroy(FuncPtr);
    FuncPtr = FuncPtrNext;
   }

  // Add entry to function dictionary
  DictAppendPtr(_ppl_UserSpace_Funcs, fitfunc, (void *)FuncPtr2, sizeof(FunctionDescriptor), 0, DATATYPE_VOID);

  // Free copies of data vectors defined within our local context
  lt_AscendOutOfContext(ContextLocalVec);
  return 0;
 }

void ppl_spline_evaluate(char *FuncName, SplineDescriptor *desc, value *in, value *out, int *status, char *errout)
 {
  double dblin, dblout;

  if (!ppl_units_DimEqual(in, &desc->UnitX))
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "The %s(x) function expects an argument with dimensions of <%s>, but has instead received an argument with dimensions of <%s>.", FuncName, ppl_units_GetUnitStr(&desc->UnitX, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in, NULL, NULL, 1, 0)); }
    else { ppl_units_zero(out); out->real = GSL_NAN; out->imag = 0; }
    *status=1;
    return;
   }
  if (in->FlagComplex)
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "The %s(x) function expects a real argument, but the supplied argument has an imaginary component.", FuncName); }
    else { ppl_units_zero(out); out->real = GSL_NAN; out->imag = 0; }
    *status=1;
    return;
   }

  dblin = in->real;
  if (desc->LogInterp) dblin = log(dblin);
  *status = gsl_spline_eval_e(desc->SplineObj, dblin, desc->accelerator, &dblout);
  if (*status!=0)
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "Error whilst evaluating the %s(x) function: %s", FuncName, gsl_strerror(*status)); }
    else { ppl_units_zero(out); out->real = GSL_NAN; out->imag = 0; }
    *status=1;
    return;
   }
  if (desc->LogInterp) dblout = exp(dblout);

  if (!gsl_finite(dblout))
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "Error whilst evaluating the %s(x) function: result was not a finite number.", FuncName); }
    else { ppl_units_zero(out); out->real = GSL_NAN; out->imag = 0; }
    *status=1;
    return;
   }

  // Return output
  ppl_units_zero(out);
  out->real = dblout;
  ppl_units_DimCpy(out, &desc->UnitY);
  return;
 }

