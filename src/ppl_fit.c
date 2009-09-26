// ppl_fit.c
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

#define _PPL_FIT_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <glob.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multimin.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Structure used for passing data around
typedef struct FitComm {
 int                 NArgs, NExpect;
 long int            NDataPoints;
 value             **outval;
 gsl_vector         *ParamVals;
 value              *FirstVals;
 double             *DataTable;
 unsigned char       FlagYErrorBars;
 FunctionDescriptor *funcdef;
 char               *ScratchPad, *errtext, *FunctionName;
} FitComm;

// Routine for printing a GSL matrix in pythonesque format
char *MatrixPrint(gsl_matrix *m, size_t size, char *out)
 {
  int i,j,p=0;
  strcpy(out+p, "[ [");
  p+=strlen(out+p);
  for (i=0;i<size;i++)
   {
    for (j=0;j<size;j++) { sprintf(out+p,"%s,",NumericDisplay(gsl_matrix_get(m,size,size),0,settings_term_current.SignificantFigures,0)); p+=strlen(out+p); }
    if (size>0) p--; // Delete final comma
    strcpy(out+p, "] , ["); // New row
   }
  if (size>0) p-=3; // Delete final comma and open bracket
  strcpy(out+p, "]");
  return out;
 }

// Low-level routine for working out the mismatch between function and data for a given set of free parameter values
static double FitResidual(FitComm *p)
 {
  int      i, k, errpos=-1;
  long int j;
  double   accumulator, residual;
  value    x;

  // Set free parameter values
  for (i=0; i<p->NArgs; i++) p->outval[i]->real = gsl_vector_get(p->ParamVals, i);

  accumulator = 0.0; // Add up sum of square residuals

  for (j=0; j<p->NDataPoints; j++)
   {
    i=0;
    sprintf(p->ScratchPad+i, "%s(", p->FunctionName); i+=strlen(p->ScratchPad+i);
    for (k=0; k<p->NArgs; k++) { sprintf(p->ScratchPad+i, "%e%s,", gsl_vector_get(p->ParamVals,k), ppl_units_GetUnitStr((*(p->outval))+k,NULL,NULL,0,1)); i+=strlen(p->ScratchPad+i); }
    sprintf(p->ScratchPad+i, ")");
    ppl_EvaluateAlgebra(p->ScratchPad, &x, 0, NULL, 0, &errpos, p->errtext, 0);
    if (!ppl_units_DimEqual(&x, p->FirstVals+p->NArgs)) { sprintf(p->errtext, "The supplied function to fit produces a value which is dimensionally incompatible with its target value. The function produces a result with dimensions of <%s>, while its target value has dimensions of <%s>.", ppl_units_GetUnitStr(&x,NULL,NULL,0,0), ppl_units_GetUnitStr(p->FirstVals+p->NArgs,NULL,NULL,1,0)); return GSL_NAN; }
    residual = hypot(x.real - gsl_vector_get(p->ParamVals,k) , x.imag);
    accumulator += residual;
   }

  return accumulator;
 }

static double ResidualMinimiserSlave(gsl_vector *x, void *p_void)
 {
  FitComm *p = (FitComm *)p_void;
  p->ParamVals = x;
  return FitResidual(p);
 }

// Routine for working out the Hessian matrix
static gsl_matrix *GetHessian(FitComm *p)
 {
  gsl_matrix *out;
  int         i,j;

  out = gsl_matrix_alloc(p->NArgs, p->NArgs); // Allocate memory for Hessian matrix
  if (out==NULL) return NULL;

  for (i=0; i<p->NArgs; i++) for (j=0; j<p->NArgs; j++) // Loop over elements of Hessian matrix
   {
   }

  return out;
 }

// Main entry point for the implementation of the fit command
int directive_fit(Dict *command)
 {
  int        status, NArgs, NExpect;
  char      *cptr, *filename;
  wordexp_t  WordExp;
  glob_t     GlobData;
  long int   i, j, k, NDataPoints;
  int        ContextOutput, ContextLocalVec, ContextDataTab, index=-1, *indexptr, rowcol=DATAFILE_COL, ErrCount=DATAFILE_NERRS;
  char       errtext[LSTR_LENGTH], *FitVars[USING_ITEMS_MAX], *tempstr=NULL, *SelectCrit=NULL;
  List      *UsingList=NULL, *EveryList=NULL;
  value     *min[USING_ITEMS_MAX], *max[USING_ITEMS_MAX], *outval[USING_ITEMS_MAX];
  value     *DummyVar, DummyTemp;
  DataTable *data;
  DataBlock *blk;
  unsigned char InRange;
  double    *LocalDataTable, val;

  FunctionDescriptor *funcdef;
  List         *RangeList, *VarList;
  ListIterator *ListIter;
  Dict         *TempDict;

  FitComm       DataComm; // Structure which we fill with variables we need to pass to the minimiser

  // Expand filename if it contains wildcards
  DictLookup(command,"filename",NULL,(void **)(&cptr));
  if (cptr==NULL) ppl_error(ERR_INTERNAL, "File attribute not found in fit command.");
  status=0;
  if ((wordexp(cptr, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
  if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
  if ((glob(WordExp.we_wordv[0], 0, NULL, &GlobData) != 0) || (GlobData.gl_pathc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[j]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return 1; }
  if  (GlobData.gl_pathc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return 1; }
  filename = lt_malloc(strlen(GlobData.gl_pathv[0])+1);
  if (filename==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); wordfree(&WordExp); globfree(&GlobData); return 1; }
  strcpy(filename, GlobData.gl_pathv[0]);
  wordfree(&WordExp);
  globfree(&GlobData);

  // Get list of fitting variables
  DictLookup(command, "fit_variables," , NULL, (void **)&VarList);
  i = ListLen(VarList);
  if ((i<0) || (i>USING_ITEMS_MAX)) { sprintf(temp_err_string,"The fit command must be supplied a list of between %d and %d free parameters to fit.", 1, USING_ITEMS_MAX); ppl_error(ERR_SYNTAX, temp_err_string); return 1; }
  ListIter = ListIterateInit(VarList);
  for (j=0; j<i; j++)
   {
    TempDict = (Dict *)ListIter->data;
    DictLookup(TempDict,"fit_variable",NULL,(void **)(FitVars+j)); // Read variable name into FitVars[j]

    // Look up variable in user space and get pointer to its value
    DictLookup(_ppl_UserSpace_Vars, FitVars[j], NULL, (void **)&DummyVar);
    if (DummyVar!=NULL)
     {
      if ((DummyVar->string != NULL) || ((DummyVar->FlagComplex) && (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)) || (!gsl_finite(DummyVar->real)) || (!gsl_finite(DummyVar->imag))) { ppl_units_zero(DummyVar); DummyVar->real=1.0; } // Turn string variables into floats
      outval[j] = DummyVar;
     }
    else
     {
      DictAppendValue(_ppl_UserSpace_Vars, FitVars[j], DummyTemp);
      DictLookup(_ppl_UserSpace_Vars, FitVars[j], NULL, (void **)&DummyVar);
      outval[j] = DummyVar;
     }
    ListIter = ListIterate(ListIter, NULL);
   }

  // Get name of function to fit
  DictLookup(command,"fit_function", NULL, (void **)&cptr);
  DataComm.FunctionName = cptr;
  if (cptr   ==NULL) ppl_error(ERR_INTERNAL, "Fitting function name not found in fit command.");
  DictLookup(_ppl_UserSpace_Funcs, cptr, NULL, (void **)&funcdef);
  if (funcdef==NULL) { sprintf(temp_err_string,"No such function as '%s'.",cptr); ppl_error(ERR_GENERAL, temp_err_string); return 1; }
  NArgs = funcdef->NumberArguments;

  // Look up index , using , every modifiers to datafile reading
  DictLookup(command, "index"      , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_rows"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_cols"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:", NULL, (void **)&UsingList);
  DictLookup(command, "every_list:", NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  // Work out how many columns of data we're going to read
  if ((UsingList==NULL) || (ListLen(UsingList) != NArgs+2)) NExpect = NArgs+1;
  else                                                      NExpect = NArgs+2; // Only expect weights to go with fitting data if using list has exactly the right length

  DictLookup(command, "range_list", NULL, (void **)&RangeList);
  ListIter = ListIterateInit(RangeList);
  for (j=0; j<NExpect; j++) // Can have up to NArgs+2 ranges (argument values, desired expression value, and optional weight)
   if (ListIter == NULL) { min[j]=NULL; max[j]=NULL; }
   else
    {
     TempDict = (Dict *)ListIter->data;
     DictLookup(TempDict,"min",NULL,(void **)(min+j));
     DictLookup(TempDict,"max",NULL,(void **)(max+j));
     if ((min[j]!=NULL)&&(max[j]!=NULL)&&(!ppl_units_DimEqual(min[j],max[j]))) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %ld in the fit command have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", j+1, ppl_units_GetUnitStr(min[j],NULL,NULL,0,0), ppl_units_GetUnitStr(max[j],NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
     ListIter = ListIterate(ListIter, NULL);
    }
   if (ListIter != NULL) { sprintf(temp_err_string, "Too many ranges supplied to the fit command. %d ranges were supplied, but only a maximum of %ld were expected.", ListLen(RangeList), i); ppl_error(ERR_SYNTAX, temp_err_string); return 1; }

  // Allocate a new memory context for the data file we're about to read
  ContextOutput  = lt_GetMemContext();
  ContextLocalVec= lt_DescendIntoNewContext();
  ContextDataTab = lt_DescendIntoNewContext();

  // Read data from file
  DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, NExpect, SelectCrit, DATAFILE_CONTINUOUS, &ErrCount);
  if (status) { ppl_error(ERR_GENERAL, errtext); return 1; }

  // Check that the FirstEntries above have the same units as any supplied ranges
  for (j=0; j<NExpect; j++)
   if (min[j] != NULL)
    {
     if (!ppl_units_DimEqual(min[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %ld in the fit command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ppl_units_GetUnitStr(min[j],NULL,NULL,0,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    }
   else if (max[j] != NULL)
    {
     if (!ppl_units_DimEqual(max[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %ld in the fit command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ppl_units_GetUnitStr(max[j],NULL,NULL,0,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    }

  // Work out how many data points we have within the specified ranges
  NDataPoints = 0;
  blk = data->first;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      InRange=1;
      for (k=0; k<NExpect; k++)
       {
        val = blk->data_real[k + NExpect*j];
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
       }
      if (InRange) NDataPoints++;
     }
    blk=blk->next;
   }

  // Copy data into a new table and apply the specified ranges to it
  LocalDataTable = (double *)lt_malloc_incontext(NDataPoints * NExpect * sizeof(double), ContextLocalVec);
  if (LocalDataTable==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); return 1; } 
  i=0;
  blk = data->first;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      InRange=1;
      for (k=0; k<NExpect; k++)
       {
        val = blk->data_real[k + NExpect*j];
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
        LocalDataTable[i * NExpect + k] = val;
       }
      if (InRange) i++;
     }
    blk=blk->next;
   }

  // Free original data table which is no longer needed
  lt_AscendOutOfContext(ContextDataTab);

  // Populate DataComm
  DataComm.NArgs       = NArgs;
  DataComm.NExpect     = NExpect;
  DataComm.NDataPoints = NDataPoints;
  DataComm.outval      = outval;
  DataComm.ParamVals   = NULL;
  DataComm.FirstVals   = data->FirstEntries;
  DataComm.DataTable   = LocalDataTable;
  DataComm.FlagYErrorBars = (NExpect == NArgs+2);
  DataComm.funcdef     = funcdef;
  DataComm.ScratchPad  = (char *)lt_malloc_incontext(LSTR_LENGTH, ContextLocalVec);
  DataComm.errtext     = (char *)lt_malloc_incontext(LSTR_LENGTH, ContextLocalVec); // FunctionName was already set above

  // Set up a minimiser
  // ResidualMinimiserSlave()

  // We're finished... can now free DataTable
  lt_AscendOutOfContext(ContextLocalVec);
  return 0;
 }

