// ppl_histogram.c
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

#define _PPL_HISTOGRAM_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_glob.h"
#include "ppl_histogram.h"
#include "ppl_settings.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#define COUNTEDERR1 if (ErrCount > 0) { ErrCount--;
#define COUNTEDERR2 if (ErrCount==0) { sprintf(temp_err_string, "%s: Too many errors: no more errors will be shown.",filename); ppl_warning(ERR_STACKED, temp_err_string); } }

int __hcompare(const void *x, const void *y)
 {
  if      (*((double *)x) > *((double *)y)) return  1.0;
  else if (*((double *)x) < *((double *)y)) return -1.0;
  else                                      return  0.0;
 }

// Main entry point for the implementation of the histogram command
int directive_histogram(Dict *command)
 {
  DataTable *data;
  DataBlock *blk;
  long int   i, j, k, Nrows;
  int        ContextOutput, ContextLocalVec, ContextDataTab, status=0, index=-1, *indexptr, rowcol=DATAFILE_COL, continuity, ErrCount=DATAFILE_NERRS;
  char       errtext[LSTR_LENGTH], *cptr, *filename=NULL, *histfunc=NULL, *tempstr=NULL, *SelectCrit=NULL;
  double    *xdata, xbinmin, xbinmax, BinOriginDbl, BinWidthDbl;
  List      *UsingList=NULL, *EveryList=NULL, *templist;
  Dict      *tempdict;
  ListIterator *listiter;
  FunctionDescriptor *FuncPtr, *FuncPtrNext, *FuncPtr2;
  HistogramDescriptor *output;
  value      FirstEntry, v, *tempval1, *tempval2, *BinWidth, *BinOrigin;
  value     *xmin=NULL, *xmax=NULL;
  unsigned char logaxis, BinOriginSet;

  // Expand filename if it contains wildcards
  DictLookup(command,"filename",NULL,(void **)(&cptr));
  if (cptr==NULL) ppl_error(ERR_INTERNAL, -1, -1, "File attribute not found in histogram command.");
  filename = ppl_glob_oneresult(cptr);
  if (filename == NULL) return 1;

  status=0;
  DictLookup(command, "hist_function", NULL, (void **)&histfunc);   if (histfunc == NULL) { ppl_error(ERR_INTERNAL, -1, -1, "ppl_histogram could not read name of function for output."); return 1; }
  DictLookup(command, "index"        , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_rows"     , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_cols"     , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:"  , NULL, (void **)&UsingList);
  DictLookup(command, "every_list:"  , NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  DictLookup(command,"min",NULL,(void **)&xmin);
  DictLookup(command,"max",NULL,(void **)&xmax);
  if ((xmin!=NULL)&&(xmax!=NULL)&&(!ppl_units_DimEqual(xmin,xmax))) { sprintf(temp_err_string, "The minimum and maximum limits specified in the histogram command have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", ppl_units_GetUnitStr(xmin,NULL,NULL,0,1,0), ppl_units_GetUnitStr(xmax,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }

  continuity = DATAFILE_CONTINUOUS;

  // Check that the function we're about to replace isn't a system function
  DictLookup(_ppl_UserSpace_Funcs, histfunc, NULL, (void *)&FuncPtr); // Check whether we are going to overwrite an existing function
  if ((FuncPtr!=NULL)&&((FuncPtr->FunctionType==PPL_USERSPACE_SYSTEM)||(FuncPtr->FunctionType==PPL_USERSPACE_STRFUNC)||(FuncPtr->FunctionType==PPL_USERSPACE_UNIT)))
   { sprintf(temp_err_string, "Attempt to redefine a core system function %s()", histfunc); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return 1; }

  ContextOutput  = lt_GetMemContext();
  ContextLocalVec= lt_DescendIntoNewContext();
  ContextDataTab = lt_DescendIntoNewContext();

  DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, 1, SelectCrit, continuity, &ErrCount);
  if (status) { ppl_error(ERR_GENERAL, -1, -1, errtext); return 1; }
  if (data->Nrows<3) { ppl_error(ERR_NUMERIC, -1, -1, "Histogram construction is only possible on data sets with members at at least three values of x."); return 1; }

  // Check that the FirstEntries above have the same units as any supplied ranges
  FirstEntry = data->FirstEntries[0];
  if      (xmin != NULL)
   {
    if (!ppl_units_DimEqual(xmin,&FirstEntry)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the histogram command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", ppl_units_GetUnitStr(xmin,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&FirstEntry,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
   }
  else if (xmax != NULL)
   {
    if (!ppl_units_DimEqual(xmax,&FirstEntry)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the histogram command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", ppl_units_GetUnitStr(xmax,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&FirstEntry,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
   }

  // Transfer data from multiple data tables into a single vector
  xdata = (double *)lt_malloc_incontext(data->Nrows * sizeof(double), ContextLocalVec);
  Nrows = data->Nrows;

  if (xdata==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst reading data from input file."); return 1; }

  // Copy data table into an array of x values
  blk = data->first; i=0;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++) xdata[i++] = blk->data_real[j].d;
    blk=blk->next;
   }

  // Free original data table which is no longer needed
  lt_AscendOutOfContext(ContextDataTab);

  // Sort data vector
  qsort((void *)xdata, i, sizeof(double), __hcompare);

  // Work out whether we are constructing histogram in linear space or log space
  DictLookup(command,"bin_list,",NULL,(void **)&templist); // Don't use log axis if bins are explicitly given to us
  logaxis = ((templist == NULL) && (XAxes[1].log == SW_BOOL_TRUE)); // Read from axis x1

  // Filter out any values of x which are out-of-range
  for (j=k=0; j<i; j++)
   {
    if ( !gsl_finite(xdata[j])                 ) continue; // Ignore non-finite datapoints
    if ( (xmin!=NULL) && (xdata[j]<xmin->real) ) continue; // Ignore out-of-range datapoints
    if ( (xmax!=NULL) && (xdata[j]>xmax->real) ) continue; // Ignore out-of-range datapoints

    if (logaxis)
     {
      if (xdata[j]<=0.0) { COUNTEDERR1; v=FirstEntry; v.real=xdata[j]; sprintf(temp_err_string,"Negative or zero values are not allowed in the construction of histograms in log space; value of x=%s will be ignored.",ppl_units_NumericDisplay(&v, 0, 0, 0)); ppl_warning(ERR_NUMERIC, temp_err_string); COUNTEDERR2; continue; }
      xdata[k]=log(xdata[j]); // If we're constructing histogram in log space, log data now
      if (!gsl_finite(xdata[k])) continue;
     } else {
      xdata[k]=xdata[j];
     }
    k++;
   }

  // Check that we have at least three points to interpolate
  if (k<3) { ppl_error(ERR_NUMERIC, -1, -1, "Histogram construction is only possible on data sets with members at at least three values of x."); return 1; }

  // Make HistogramDescriptor data structure
  output = (HistogramDescriptor *)lt_malloc_incontext(sizeof(HistogramDescriptor), 0);
  if (output == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); return 1; }
  output->unit     = FirstEntry;
  output->filename = (char *)lt_malloc_incontext(strlen(filename)+1, 0);
  if (output->filename == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); return 1; }
  strcpy(output->filename , filename);

  // Now need to work out what bins we're going to use
  DictLookup(command,"bin_list,",NULL,(void **)&templist);
  DictLookup(command,"binwidth" ,NULL,(void **)&tempval1);
  DictLookup(command,"binorigin",NULL,(void **)&tempval2);
  if (templist != NULL)
   {
    output->bins = (double *)malloc(ListLen(templist)*sizeof(double));
    if (output->bins == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); return 1; }
    j=0;
    listiter = ListIterateInit(templist);
    while (listiter != NULL)
     {
      tempdict = (Dict *)listiter->data;
      DictLookup(tempdict,"x",NULL,(void **)&tempval1);
      if (!ppl_units_DimEqual(&FirstEntry,tempval1)) { sprintf(temp_err_string, "The supplied bin boundary at x=%s has conflicting physical dimensions with the data supplied, which has units of <%s>. Ignoring this bin boundary.", ppl_units_GetUnitStr(tempval1,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&FirstEntry,NULL,NULL,1,1,0)); ppl_warning(ERR_NUMERIC, temp_err_string); }
      else { output->bins[j++] = tempval1->real; }
      listiter = ListIterate(listiter, NULL);
     }
    output->Nbins = j;
    qsort((void *)output->bins, j, sizeof(double), __hcompare); // Make sure that bins are in ascending order
   }
  else
   {
    if (tempval1 != NULL) BinWidth  = tempval1;
    else                  BinWidth  = &(settings_term_current.BinWidth);
    if (tempval2 != NULL) { BinOrigin = tempval2;                           BinOriginSet = 1;                                    }
    else                  { BinOrigin = &(settings_term_current.BinOrigin); BinOriginSet = !settings_term_current.BinOriginAuto; }
    if ((!logaxis) && (!ppl_units_DimEqual(&FirstEntry,BinWidth))) { sprintf(temp_err_string, "The bin width supplied to the histogram command has conflicting physical dimensions with the data supplied. The former has units of <%s>, whilst the latter has units of <%s>.", ppl_units_GetUnitStr(BinWidth,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&FirstEntry,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
    if ((logaxis) && (BinWidth->dimensionless==0)) { sprintf(temp_err_string, "For logarithmically spaced bins, the multiplicative spacing between bins must be dimensionless. The supplied spacing has units of <%s>.", ppl_units_GetUnitStr(BinWidth,NULL,NULL,0,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
    if ((BinOriginSet) && (!ppl_units_DimEqual(&FirstEntry,BinOrigin))) { sprintf(temp_err_string, "The bin origin supplied to the histogram command has conflicting physical dimensions with the data supplied. The former has units of <%s>, whilst the latter has units of <%s>.", ppl_units_GetUnitStr(BinOrigin,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&FirstEntry,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
    if ((logaxis) && (BinWidth->real <= 1.0)) { sprintf(temp_err_string, "For logarithmically spaced bins, the multiplicative spacing between bins must be greater than 1.0. Value supplied was %s.", ppl_units_NumericDisplay(BinWidth,0,0,0)); return 1; }
    if (BinWidth->real <= 0.0) { sprintf(temp_err_string, "The bin width supplied to the histogram command must be greater than zero. Value supplied was %s.", ppl_units_NumericDisplay(BinWidth,0,0,0)); return 1; }
    if ((logaxis) && (BinOriginSet) && (BinOrigin->real <= 0.0)) { sprintf(temp_err_string, "For logarithmically spaced bins, the specified bin origin must be greater than zero. Value supplied was %s.", ppl_units_NumericDisplay(BinOrigin,0,0,0)); return 1; }

    if (logaxis) { if (BinOriginSet) BinOriginDbl = log(BinOrigin->real); BinWidthDbl = log(BinWidth->real); }
    else         { if (BinOriginSet) BinOriginDbl =     BinOrigin->real ; BinWidthDbl =     BinWidth->real ; }

    // Generate a series of bins to use based on supplied BinWidth and BinOrigin
    xbinmin = xdata[0];
    xbinmax = xdata[k-1];
    if (xmin != NULL) xbinmin = xmin->real;
    if (xmax != NULL) xbinmax = xmax->real;
    if (BinOriginSet) BinOriginDbl = BinOriginDbl - BinWidthDbl * floor(BinOriginDbl / BinWidthDbl);
    else              BinOriginDbl = 0.0;
    xbinmin = floor((xbinmin-BinOriginDbl)/BinWidthDbl)*BinWidthDbl + BinOriginDbl;
    xbinmax = ceil ((xbinmax-BinOriginDbl)/BinWidthDbl)*BinWidthDbl + BinOriginDbl;

    if (((xbinmax-xbinmin)/BinWidthDbl + 1.0001) > 1e7) { sprintf(temp_err_string, "The supplied value of BinWidth produces a binning scheme with more than 1e7 bins. This is probably not sensible."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return 1; }
    output->Nbins = (long)((xbinmax-xbinmin)/BinWidthDbl + 1.0001);
    output->bins = (double *)malloc(output->Nbins*sizeof(double));
    if (output->bins == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); return 1; }

    for (j=0; j<output->Nbins; j++) output->bins[j] = xbinmin + j*BinWidthDbl;
   }

  // Allocate vector for storing histogram values
  output->binvals = (double *)malloc(output->Nbins * sizeof(double));
  if (output->binvals == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); free(output->bins); return 1; }
  for (i=0; i<output->Nbins; i++) output->binvals[i]=0.0;

  // Count number of data values in each histogram bin
  i=0;
  for (j=0; j<k; j++) // Loop over all of the data points we have read in
   {
    while ((i < output->Nbins) && (xdata[j] > output->bins[i])) i++;
    if (i>=output->Nbins) break; // We gone off the top of the last bin
    if (i==0) continue; // We've not yet arrived in the bottom bin
    output->binvals[i-1] += 1.0;
   }

  // If this is a logarithmic set of bins, unlog bin boundaries now
  if (logaxis) for (i=0; i<output->Nbins; i++) output->bins[i] = exp(output->bins[i]);

  // Divide each bin's number of counts by its width
  for (i=0; i<output->Nbins-1; i++) output->binvals[i] /= output->bins[i+1] - output->bins[i] + 1e-200;

  // Debugging lines
  if (DEBUG)
   for (i=0; i<output->Nbins-1; i++)
    {
     sprintf(temp_err_string, "Bin %ld [%e:%e] --> %e", i+1, output->bins[i], output->bins[i+1], output->binvals[i]);
     ppl_log(temp_err_string);
    }

  // Make a new function descriptor
  FuncPtr2 = (FunctionDescriptor *)lt_malloc_incontext(sizeof(FunctionDescriptor), 0);
  if (FuncPtr2 == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst adding interpolation object to function dictionary."); return 1; }
  FuncPtr2->FunctionType    = PPL_USERSPACE_HISTOGRAM;
  FuncPtr2->modified        = 1;
  FuncPtr2->NumberArguments = 1;
  FuncPtr2->FunctionPtr     = (void *)output;
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
  DictAppendPtr(_ppl_UserSpace_Funcs, histfunc, (void *)FuncPtr2, sizeof(FunctionDescriptor), 0, DATATYPE_VOID);

  // Free copies of data vectors defined within our local context
  lt_AscendOutOfContext(ContextLocalVec);
  return 0;
 }

void ppl_histogram_evaluate(char *FuncName, HistogramDescriptor *desc, value *in, value *out, int *status, char *errout)
 {
  int i;
  double dblin;

  if (!ppl_units_DimEqual(in, &desc->unit))
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "The %s(x) function expects an argument with dimensions of <%s>, but has instead received an argument with dimensions of <%s>.", FuncName, ppl_units_GetUnitStr(&desc->unit, NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr(in, NULL, NULL, 1, 1, 0)); }
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

  *out = desc->unit;
  out->imag = 0.0;
  out->real = 0.0;
  out->FlagComplex = 0;
  for (i=0; i<UNITS_MAX_BASEUNITS; i++) if (out->exponent[i]!=0.0) out->exponent[i]*=-1; // Output has units of 'per x'
  if ((desc->Nbins<1) || (dblin<desc->bins[0]) || (dblin>desc->bins[desc->Nbins-1])) return; // Query is outside range of histogram

  for (i=1; i<desc->Nbins; i++) if (desc->bins[i]>dblin) { out->real = desc->binvals[i-1]; return; }
  return;
 }

