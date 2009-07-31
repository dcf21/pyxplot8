// ppl_interpolation.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-9 Dominic Ford <coders@pyxplot.org.uk>
//               2008-9 Ross Church
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
#include "ppl_interpolation.h"
#include "ppl_userspace.h"

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
  int        ContextOutput, ContextLocalVec, ContextDataTab, status=0, index=-1, *indexptr, rowcol=DATAFILE_COL, continuity, ErrCount=500;
  char       errtext[LSTR_LENGTH], *filename=NULL, *fitfunc=NULL, *tempstr=NULL, *SelectCrit=NULL;
  double    *xdata, *ydata;
  List      *UsingList=NULL, *EveryList=NULL;
  FunctionDescriptor *FuncPtr;
  value      v, FirstEntries[2];

  const gsl_interp_type *SplineType;
  gsl_spline            *SplineObj;

  DictLookup(command, "filename"   , NULL, (void **)&filename);   if (filename == NULL) { ppl_error("Internal error: ppl_interpolation could not read filename."); return 1; }
  DictLookup(command, "fit_function",NULL, (void **)&fitfunc);    if (fitfunc  == NULL) { ppl_error("Internal error: ppl_interpolation could not read name of function for output."); return 1; }
  DictLookup(command, "index"      , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_rows"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_cols"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:", NULL, (void **)&UsingList);
  DictLookup(command, "every_list:", NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  continuity = DATAFILE_CONTINUOUS;

  if      (mode == INTERP_LINEAR) SplineType=gsl_interp_linear;
  else if (mode == INTERP_LOGLIN) SplineType=gsl_interp_linear;
  else if (mode == INTERP_SPLINE) SplineType=gsl_interp_cspline;
  else if (mode == INTERP_AKIMA ) SplineType=gsl_interp_akima;
  else if (mode == INTERP_POLYN ) SplineType=gsl_interp_polynomial;
  else                            { ppl_error("Internal Error: interpolate command requested to perform unknown type of interpolation."); return 1; }

  // Check that the function we're about to replace isn't a system function
  DictLookup(_ppl_UserSpace_Funcs, fitfunc, NULL, (void *)&FuncPtr); // Check whether we are going to overwrite an existing function
  if ((FuncPtr!=NULL)&&((FuncPtr->FunctionType==PPL_USERSPACE_SYSTEM)||(FuncPtr->FunctionType==PPL_USERSPACE_STRFUNC)||(FuncPtr->FunctionType==PPL_USERSPACE_UNIT)))
   { sprintf(temp_err_string, "Error: Attempt to redefine a core system function %s()", fitfunc); ppl_error(temp_err_string); return 1; }

  ContextOutput  = lt_GetMemContext();
  ContextLocalVec= lt_DescendIntoNewContext();
  ContextDataTab = lt_DescendIntoNewContext();

  DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, 2, SelectCrit, continuity, &ErrCount);
  if (status) { ppl_error(errtext); return 1; }

  xdata = (double *)lt_malloc_incontext(2 * data->Nrows * sizeof(double), ContextLocalVec); // Transfer data from multiple data tables into single vectors
  ydata = xdata + data->Nrows;
  Nrows = data->Nrows;

  if ((xdata==NULL)||(ydata==NULL)) { ppl_error("Error: Out of memory whilst reading data from input file."); return 1; }

  // Copy data table into an array of x values, and an array of y values
  blk = data->first; i=0;
  while (blk != NULL)
   {
    k=i; for (j=0; j<blk->BlockPosition; j++) xdata[i++] = blk->data_real[0 + 2*j];
    i=k; for (j=0; j<blk->BlockPosition; j++) ydata[i++] = blk->data_real[1 + 2*j];
    blk=blk->next;
   }
  FirstEntries[0] = data->FirstEntries[0];
  FirstEntries[1] = data->FirstEntries[1];

  // Free original data table which is no longer needed
  lt_AscendOutOfContext(ContextDataTab);

  // Sort data vectors according to x values
  __compare_offset = -Nrows;
  qsort((void *)ydata, i, sizeof(double), __compare);
  __compare_offset = 0;
  qsort((void *)xdata, i, sizeof(double), __compare);

  // Filter out repeat values of x
  for (j=k=1; j<i; j++)
   {
    if (xdata[j]==xdata[j-1]) { if (ErrCount > 0) { ErrCount--; v=FirstEntries[0]; v.real=xdata[j]; sprintf(temp_err_string,"Warning: Repeat values for interpolation have been supplied at x=%s.",ppl_units_NumericDisplay(&v, 0, 0)); ppl_warning(temp_err_string); } continue; }
    if (mode == INTERP_LOGLIN)
     {
      if ((xdata[j]<=0.0) || (ydata[j]<=0.0)) { if (ErrCount > 0) { ErrCount--; v=FirstEntries[0]; v.real=xdata[j]; sprintf(temp_err_string,"Warning: Negative or zero values are not allowed in power-law interpolation; negative values supplied at x=%s will be ignored.",ppl_units_NumericDisplay(&v, 0, 0)); ppl_warning(temp_err_string); } continue; }
      xdata[k]=log(xdata[j]);
      ydata[k]=log(ydata[j]);
     }
    xdata[k]=xdata[j];
    ydata[k]=ydata[j];
    if (gsl_finite(xdata[k]) && gsl_finite(ydata[k])) k++; // Ignore non-finite data points
   }

  // Check that we have at least three points to interpolate
  if (k<3) { ppl_error("Error: interpolation is only possible on data sets with members at at least three distinct values of x."); return 1; }

  // Create GSL interpolation object
  SplineObj = gsl_spline_alloc(SplineType, k);
  if (SplineObj==NULL) { ppl_error("Internal Error: Failed to make interpolation object."); return 1; }
  status    = gsl_spline_init(SplineObj, xdata, ydata, k);
  if (status) { sprintf(temp_err_string, "Error whilst creating interpolation object:\n%s", gsl_strerror(status)); ppl_error(temp_err_string); return 1; }

  // Generate a function descriptor for this spline

  // Free copies of data vectors defined within our local context
  lt_AscendOutOfContext(ContextLocalVec);
  return 0;
 }

