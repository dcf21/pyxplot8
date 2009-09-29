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

#include <gsl/gsl_deriv.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multimin.h>
#include <gsl/gsl_permutation.h>

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
 int                 NArgs; // The number of arguments taken by the function that we're fitting
 int                 NExpect; // The total number of columns we're reading from datafile; equals NArgs, plus the target value for f(), plus possibly the error on each target
 int                 NFitVars; // The number of variables listed after via ....
 int                 NParams; // The total number of free parameters in the fitting problems; either equals NFitVars, or twice this if complex arithmetic is enabled
 long int            NDataPoints; // The number of data points read from the supplied datafile
 value             **outval; // Pointers to the values of the variables which we're fitting the values of in the user's variable space
 const gsl_vector   *ParamVals; // Trial parameter values to be tried by FitResidual in this iteration
 const gsl_vector   *BestFitParamVals; // The best fit parameter values (only set after first round of minimisation)
 gsl_vector         *ParamValsHessian; // Internal variable used by GetHessian() to vary parameter values when differentiating
 value              *FirstVals; // The first values found in each column of the supplied datafile. These determine the physical units associated with each column.
 double             *DataTable; // Two-dimensional table of the data read from the datafile.
 unsigned char       FlagYErrorBars; // If true, the user has specified errorbars for each target value. If false, we have no idea of uncertainty.
 FunctionDescriptor *funcdef; // Function descriptor for the function f() which we're trying to get to fit the data
 char               *ScratchPad, *errtext, *FunctionName; // String workspaces
 unsigned char       GoneNaN; // Used by the minimiser to keep track of when the function being minimised has returned NAN.
 double              SigmaData; // The assumed errorbar (uniform for all datapoints) on the supplied target values if errorbars are not supplied. We fit this.
 int                 diff1    , diff2; // The numbers of the free parameters currently being differentiated inside GetHessian()
 double              diff1step, diff2step; // The step size which GetHessian() recommends using for each of the parameters being differentiated
} FitComm;

// Routine for printing a GSL matrix in pythonesque format
static char *MatrixPrint(const gsl_matrix *m, const size_t size, char *out)
 {
  size_t i,j,p=0;
  strcpy(out+p, "[ [");
  p+=strlen(out+p);
  for (i=0;i<size;i++)
   {
    for (j=0;j<size;j++) { sprintf(out+p,"%s,",NumericDisplay(gsl_matrix_get(m,i,j),0,settings_term_current.SignificantFigures,0)); p+=strlen(out+p); }
    if (size>0) p--; // Delete final comma
    strcpy(out+p, "] , ["); p+=strlen(out+p); // New row
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
  for (i=0; i<p->NFitVars; i++)
   if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)   p->outval[i]->real = gsl_vector_get(p->ParamVals,   i  ); // This is setting variables in the user's variable space
   else                                                      { p->outval[i]->real = gsl_vector_get(p->ParamVals, 2*i  );
                                                               p->outval[i]->imag = gsl_vector_get(p->ParamVals, 2*i+1);
                                                             }

  accumulator = 0.0; // Add up sum of square residuals

  for (j=0; j<p->NDataPoints; j++) // Loop over all of the data points in the file that we're fitting
   {
    i=0;
    sprintf(p->ScratchPad+i, "%s(", p->FunctionName); i+=strlen(p->ScratchPad+i);
    for (k=0; k<p->NArgs; k++) { sprintf(p->ScratchPad+i, "%.20e%s,", p->DataTable[j*p->NExpect+k], ppl_units_GetUnitStr(p->FirstVals+k,NULL,NULL,0,1)); i+=strlen(p->ScratchPad+i); }
    if (p->NArgs>0) i--; // Remove final comma from list of arguments
    sprintf(p->ScratchPad+i, ")");
    ppl_EvaluateAlgebra(p->ScratchPad, &x, 0, NULL, 0, &errpos, p->errtext, 0);
    if (errpos>=0) return GSL_NAN; // Evaluation of algebra failed
    if (!ppl_units_DimEqual(&x, p->FirstVals+p->NArgs)) { sprintf(p->errtext, "The supplied function to fit produces a value which is dimensionally incompatible with its target value. The function produces a result with dimensions of <%s>, while its target value has dimensions of <%s>.", ppl_units_GetUnitStr(&x,NULL,NULL,0,0), ppl_units_GetUnitStr(p->FirstVals+p->NArgs,NULL,NULL,1,0)); return GSL_NAN; }
    residual = pow(x.real - p->DataTable[j*p->NExpect+k] , 2) + pow(x.imag , 2); // Calculate squared deviation of function result from desired result
    if (p->FlagYErrorBars) residual /= 2 * pow(p->DataTable[j*p->NExpect+k+1] , 2); // Divide square residual by 2 sigma squared.
    else                   residual /= 2 * pow(p->SigmaData                   , 2);
    accumulator += residual; // ... and sum
   }

  return accumulator;
 }

// Slave routines called by the differentiation operation when working out the Hessian matrix

static double GetHessian_diff2(double x, void *p_void)
 {
  double tmp, output;
  FitComm *p = (FitComm *)p_void;
  tmp = gsl_vector_get(p->ParamValsHessian, p->diff2); // Replace old parameter with x
  gsl_vector_set(p->ParamValsHessian, p->diff2, x);
  output = FitResidual(p); // Evaluate residual
  gsl_vector_set(p->ParamValsHessian, p->diff2, tmp); // Restore old parameter value
  return output;
 }

static double GetHessian_diff1(double x, void *p_void)
 {
  double        tmp, output, output_error;
  gsl_function  fn;
  FitComm      *p = (FitComm *)p_void;

  fn.function = &GetHessian_diff2;
  fn.params   = p_void;

  tmp = gsl_vector_get(p->ParamValsHessian, p->diff1); // Replace old parameter with x
  gsl_vector_set(p->ParamValsHessian, p->diff1, x);
  gsl_deriv_central(&fn, gsl_vector_get(p->ParamValsHessian, p->diff2), p->diff2step, &output, &output_error); // Differentiate residual a second time
  gsl_vector_set(p->ParamValsHessian, p->diff1, tmp); // Restore old parameter value
  return output;
 }

// Routine for working out the Hessian matrix
static gsl_matrix *GetHessian(FitComm *p)
 {
  gsl_matrix   *out;
  int           i,j;
  double        output, output_error;
  gsl_function  fn;

  // Allocate a vector for passing our position in free-parameter space to FitResidual()
  p->ParamValsHessian = gsl_vector_alloc(p->NParams);
  for (i=0; i<p->NParams; i++) gsl_vector_set(p->ParamValsHessian, i, gsl_vector_get(p->BestFitParamVals, i));
  p->ParamVals = p->ParamValsHessian;

  out = gsl_matrix_alloc(p->NParams, p->NParams); // Allocate memory for Hessian matrix
  if (out==NULL) return NULL;

  fn.function = &GetHessian_diff1;
  fn.params   = (void *)p;

  for (i=0; i<p->NParams; i++) for (j=0; j<p->NParams; j++) // Loop over elements of Hessian matrix
   {
    p->diff1 = i; p->diff1step = ((output = gsl_vector_get(p->ParamVals,i)*1e-6) < 1e-100) ? 1e-6 : output;
    p->diff2 = j; p->diff2step = ((output = gsl_vector_get(p->ParamVals,j)*1e-6) < 1e-100) ? 1e-6 : output;
    gsl_deriv_central(&fn, gsl_vector_get(p->ParamVals, p->diff1), p->diff1step, &output, &output_error);
    gsl_matrix_set(out,i,j,-output); // Minus sign here since FitResidual returns the negative of log(P)
   }

  gsl_vector_free(p->ParamValsHessian);
  p->ParamValsHessian = NULL;
  return out;
 }

// Slave routines called by minimisers

static double ResidualMinimiserSlave(const gsl_vector *x, void *p_void)
 {
  FitComm *p = (FitComm *)p_void;
  p->ParamVals = x;
  return FitResidual(p);
 }

static double FitSigmaData(const gsl_vector *x, void *p_void)
 {
  double term1, term2, term3;
  int    sgn;
  gsl_matrix *hessian;
  gsl_permutation *perm;

  FitComm *p = (FitComm *)p_void;
  p->SigmaData = gsl_vector_get(x,0);
  p->ParamVals = p->BestFitParamVals;
  term1 = FitResidual(p); // Likelihood for the best-fit set of parameters, without the Gaussian normalisation factor
  term2 = -(p->NDataPoints * log(1.0 / sqrt(2*M_PI) / p->SigmaData)); // Gaussian normalisation factor

  // term3 is the Occam Factor, which equals the determinant of -H
  hessian = GetHessian(p);
  gsl_matrix_scale(hessian, -1.0); // Want the determinant of -H
  // Generate the LU decomposition of the Hessian matrix
  perm = gsl_permutation_alloc(p->NParams);
  gsl_linalg_LU_decomp(hessian,perm,&sgn); // Hessian matrix is overwritten here, but we don't need it again
  // Calculate the determinant of the Hessian matrix
  term3 = gsl_linalg_LU_lndet(hessian) * 0.5; // -log(1/sqrt(det))
  gsl_matrix_free(hessian);
  gsl_permutation_free(perm);

  return term1+term2+term3; // We return the negative of the log-likelihood, which GSL then minimises by varying the assume errorbar size
 }

// Top-level routine for managing the GSL minimiser

static int FitMinimiseIterate(FitComm *commlink, double(*slave)(const gsl_vector *, void *), unsigned char FittingSigmaData)
 {
  size_t                              iter = 0,iter2 = 0;
  int                                 i, status=0, NParams;
  double                              size=0,sizelast=0,sizelast2=0,testval;
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex; // We don't use nmsimplex2 here because it was new in gsl 1.12
  gsl_multimin_fminimizer            *s;
  gsl_vector                         *x, *ss;
  gsl_multimin_function               fn;

  if (!FittingSigmaData) NParams = commlink->NParams;
  else                   NParams = 1;
  x  = gsl_vector_alloc( NParams );
  ss = gsl_vector_alloc( NParams );

  fn.n = NParams;
  fn.f = slave;
  fn.params = (void *)commlink;

  iter2=0;
  do
   {
    iter2++;
    sizelast2 = size;

    if (!FittingSigmaData)
     {
      if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)for(i=0;i<commlink->NFitVars;i++) gsl_vector_set(x,  i,commlink->outval[i]->real);
      else                                                     for(i=0;i<commlink->NFitVars;i++){gsl_vector_set(x,2*i,commlink->outval[i]->real); gsl_vector_set(x,2*i+1,commlink->outval[i]->imag); }
     } else {
      gsl_vector_set(x,0,commlink->SigmaData);
     }

    for (i=0; i<NParams; i++)
     {
      if (fabs(gsl_vector_get(x,i))>1e-6) gsl_vector_set(ss, i, 0.1 * gsl_vector_get(x,i));
      else                                gsl_vector_set(ss, i, 0.1                      ); // Avoid having a stepsize of zero
     }

    s = gsl_multimin_fminimizer_alloc (T, fn.n);
    gsl_multimin_fminimizer_set (s, &fn, x, ss);

    // If initial value we are giving the minimiser produces an algebraic error, it's not worth continuing
    testval = (*slave)(x,(void *)commlink);
    if (commlink->errtext[0]!='\0') { gsl_vector_free(x); gsl_vector_free(ss); gsl_multimin_fminimizer_free(s); return 1; }

    iter                 = 0;
    commlink->GoneNaN    = 0;
    do
     {
      iter++;
      for (i=0; i<2+NParams*2; i++) // When you're minimising over many parameters simultaneously sometimes nothing happens for a long time
       {
        status = gsl_multimin_fminimizer_iterate(s);
        if (status) break;
       }
      if (status) break;
      sizelast = size;
      size     = gsl_multimin_fminimizer_minimum(s);
     }
    while ((iter < 10) || ((size < sizelast) && (iter < 50))); // Iterate 10 times, and then see whether size carries on getting smaller

    gsl_multimin_fminimizer_free(s);
   }
  while ((iter2 < 3) || ((commlink->GoneNaN==0) && (!status) && (size < sizelast2) && (iter2 < 20))); // Iterate 2 times, and then see whether size carries on getting smaller

  if (iter2>=20) status=1;

  if (status) { sprintf(commlink->errtext, "Failed to converge. GSL returned error: %s", gsl_strerror(status)); gsl_vector_free(x); gsl_vector_free(ss); return 1; }
  sizelast = (*slave)(x,(void *)commlink); // Calling minimiser slave now sets all fitting variables to desired values

  gsl_vector_free(x);
  gsl_vector_free(ss);
  return 0;
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
  value     *min[USING_ITEMS_MAX], *max[USING_ITEMS_MAX], *outval[USING_ITEMS_MAX], FirstVals[USING_ITEMS_MAX];
  value     *DummyVar, DummyTemp;
  DataTable *data;
  DataBlock *blk;
  unsigned char InRange;
  double    *LocalDataTable, val;
  gsl_vector *BestFitParamVals;
  gsl_matrix *hessian, *hessian_lu, *hessian_inv;
  gsl_permutation *perm;
  int         sgn;
  double      StdDev[2*USING_ITEMS_MAX], tmp1, tmp2, tmp3;

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

  // Default starting point for fitting is 1.0
  ppl_units_zero(&DummyTemp);
  DummyTemp.real    = 1.0;

  // Get list of fitting variables
  DictLookup(command, "fit_variables," , NULL, (void **)&VarList);
  i = ListLen(VarList);
  DataComm.NFitVars = i;
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
  if (funcdef==NULL) { sprintf(temp_err_string,"No such function as '%s()'.",cptr); ppl_error(ERR_GENERAL, temp_err_string); return 1; }
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

  // Make a copy of the physical units of all of the columns of data
  for (j=0; j<NExpect; j++) FirstVals[j] = data->FirstEntries[j];

  // Free original data table which is no longer needed
  lt_AscendOutOfContext(ContextDataTab);
  data = NULL;

  // Populate DataComm
  DataComm.NArgs       = NArgs;
  DataComm.NExpect     = NExpect;
  DataComm.NParams     = DataComm.NFitVars * ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) ? 1 : 2);
  DataComm.NDataPoints = NDataPoints;
  DataComm.outval      = outval;
  DataComm.ParamVals   = NULL;
  DataComm.BestFitParamVals = NULL;
  DataComm.FirstVals   = FirstVals;
  DataComm.DataTable   = LocalDataTable;
  DataComm.FlagYErrorBars = (NExpect == NArgs+2);
  DataComm.SigmaData   = 1.0;
  DataComm.funcdef     = funcdef;
  DataComm.ScratchPad  = (char *)lt_malloc_incontext(LSTR_LENGTH, ContextLocalVec);
  DataComm.errtext     = (char *)lt_malloc_incontext(LSTR_LENGTH, ContextLocalVec); // FunctionName was already set above

  // Set up a minimiser
  status = FitMinimiseIterate(&DataComm, &ResidualMinimiserSlave, 0);
  if (status) { ppl_error(ERR_GENERAL, DataComm.errtext); return 1; }

  // Display the results of the minimiser
  ppl_report("\n# Best fit parameters were:\n# -------------------------\n");
  for (j=0; j<DataComm.NFitVars; j++)
   {
    if (settings_term_current.NumDisplay != SW_DISPLAY_L) sprintf(temp_err_string,  "%s = %s", FitVars[j], ppl_units_NumericDisplay(outval[j],0,0,0)  );
    else                                                  sprintf(temp_err_string, "$%s = %s", FitVars[j], ppl_units_NumericDisplay(outval[j],0,0,0)+1);
    ppl_report(temp_err_string);
   }

  // Store best-fit position
  BestFitParamVals = gsl_vector_alloc(DataComm.NParams);
  for (i=0; i<DataComm.NFitVars; i++)
   if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)   gsl_vector_set(BestFitParamVals,   i  , outval[i]->real);
   else                                                      { gsl_vector_set(BestFitParamVals, 2*i  , outval[i]->real);
                                                               gsl_vector_set(BestFitParamVals, 2*i+1, outval[i]->imag);
                                                             }
  DataComm.BestFitParamVals = BestFitParamVals;

  // Estimate the size of the errorbars on the supplied data if no errorbars were supplied (this doesn't affect best fit position, but does affect error estimates)
  if (!DataComm.FlagYErrorBars)
   {
    status = FitMinimiseIterate(&DataComm, &FitSigmaData, 1);
    if (status) { ppl_error(ERR_GENERAL, DataComm.errtext); gsl_vector_free(BestFitParamVals); return 1; }
    FirstVals[NArgs].real = DataComm.SigmaData;
    FirstVals[NArgs].imag = 0.0;
    FirstVals[NArgs].FlagComplex = 0;
    sprintf(temp_err_string, "\n# Estimate of error bars on supplied data, based on their fit to model function = %s", ppl_units_NumericDisplay(FirstVals+NArgs,0,0,0)); ppl_report(temp_err_string);
   }

  // Calculate and print the Hessian matrix
  hessian = GetHessian(&DataComm);
  MatrixPrint(hessian, DataComm.NParams, DataComm.ScratchPad);
  sprintf(temp_err_string, "\n# Hessian matrix of log-probability distribution:\n# -----------------------------------------------\n\nhessian = %s", DataComm.ScratchPad);
  ppl_report(temp_err_string);

  // Set variables in user's variable space to best-fit values
  for (i=0; i<DataComm.NFitVars; i++)
   if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)   outval[i]->real = gsl_vector_get(DataComm.BestFitParamVals,   i  );
   else                                                      { outval[i]->real = gsl_vector_get(DataComm.BestFitParamVals, 2*i  );
                                                               outval[i]->imag = gsl_vector_get(DataComm.BestFitParamVals, 2*i+1);
                                                             }

  // Calculate and print the covariance matrix
  hessian_lu  = gsl_matrix_alloc(DataComm.NParams, DataComm.NParams);
  hessian_inv = gsl_matrix_alloc(DataComm.NParams, DataComm.NParams);
  gsl_matrix_memcpy(hessian_lu, hessian);
  gsl_matrix_scale(hessian_lu, -1.0); // Want the inverse of -H
  perm = gsl_permutation_alloc(DataComm.NParams);
  gsl_linalg_LU_decomp(hessian_lu,perm,&sgn);
  gsl_linalg_LU_invert(hessian_lu,perm,hessian_inv);
  MatrixPrint(hessian_inv, DataComm.NParams, DataComm.ScratchPad);
  sprintf(temp_err_string, "\n# Covariance matrix of probability distribution:\n# ----------------------------------------------\n\ncovariance = %s", DataComm.ScratchPad);
  ppl_report(temp_err_string);

  // Calculate the standard deviation of each parameter
  for (i=0; i<DataComm.NParams; i++)
   {
    if ((gsl_matrix_get(hessian_inv, i, i) <= 0.0) || (!gsl_finite(gsl_matrix_get(hessian_inv, i, i))))
     {
      sprintf(temp_err_string, "One of the calculated variances for the fitted parameters is negative. This strongly suggests that the fitting process has failed.");
      ppl_warning(ERR_NUMERIC, temp_err_string);
      StdDev[i] = 1e-100;
     } else {
      StdDev[i] = sqrt( gsl_matrix_get(hessian_inv, i, i) );
     }
   }

  // Calculate the correlation matrix
  for (i=0; i<DataComm.NParams; i++) for (j=0; j<DataComm.NParams; j++)
   gsl_matrix_set(hessian_inv, i, j, gsl_matrix_get(hessian_inv, i, j) / StdDev[i] / StdDev[j]);
  MatrixPrint(hessian_inv, DataComm.NParams, DataComm.ScratchPad);
  sprintf(temp_err_string, "\n# Correlation matrix of probability distribution:\n# ----------------------------------------------\n\ncorrelation = %s", DataComm.ScratchPad);
  ppl_report(temp_err_string);

  // Print a list of standard deviations
  ppl_report("\n# Uncertainties in best-fit parameters are:\n# -----------------------------------------\n");
  for (i=0; i<DataComm.NFitVars; i++)
   {
    DummyTemp = *(outval[i]);
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)
     {
      DummyTemp.real = StdDev[i] ; DummyTemp.imag = 0.0; DummyTemp.FlagComplex = 0; // Apply appropriate unit to standard deviation, which is currently just a double
      if (settings_term_current.NumDisplay != SW_DISPLAY_L)
       {
        sprintf(DataComm.ScratchPad, "sigma_%s", FitVars[i]);
        sprintf(temp_err_string, "%22s = %s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,0,0,0));
       } else {
        sprintf(DataComm.ScratchPad, "$\\sigma_\\textrm{");
        j = strlen(DataComm.ScratchPad);
        for (k=0; FitVars[i][k]!='\0'; k++) { if (FitVars[i][k]=='_') DataComm.ScratchPad[j++]='\\'; DataComm.ScratchPad[j++]=FitVars[i][k]; }
        DataComm.ScratchPad[j++]='\0';
        sprintf(temp_err_string, "%33s} = %s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,0,0,0)+1);
       }
      ppl_report(temp_err_string);
     }
    else
     {
      if (settings_term_current.NumDisplay != SW_DISPLAY_L)
       {
        DummyTemp.real = StdDev[2*i  ] ; DummyTemp.imag = 0.0; DummyTemp.FlagComplex = 0;
        sprintf(DataComm.ScratchPad, "sigma_%s_real", FitVars[i]);
        sprintf(temp_err_string, "%27s = %s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,0,0,0));
        ppl_report(temp_err_string);
        DummyTemp.real = StdDev[2*i+1] ; DummyTemp.imag = 0.0; DummyTemp.FlagComplex = 0;
        sprintf(DataComm.ScratchPad, "sigma_%s_imag", FitVars[i]);
        sprintf(temp_err_string, "%27s = %s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,0,0,0));
        ppl_report(temp_err_string);
       }
      else
       {
        DummyTemp.real = StdDev[2*i  ] ; DummyTemp.imag = 0.0; DummyTemp.FlagComplex = 0;
        sprintf(DataComm.ScratchPad, "$\\sigma_\\textrm{");
        j = strlen(DataComm.ScratchPad);
        for (k=0; FitVars[i][k]!='\0'; k++) { if (FitVars[i][k]=='_') DataComm.ScratchPad[j++]='\\'; DataComm.ScratchPad[j++]=FitVars[i][k]; }
        DataComm.ScratchPad[j++]='\0';
        sprintf(temp_err_string, "%38s,real} = %s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,0,0,0)+1);
        ppl_report(temp_err_string);
        DummyTemp.real = StdDev[2*i+1] ; DummyTemp.imag = 0.0; DummyTemp.FlagComplex = 0;
        sprintf(temp_err_string, "%38s,imag} = %s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,0,0,0)+1);
        ppl_report(temp_err_string);
       }
     }
   }

  // Print summary information
  ppl_report("\n# Summary:\n# --------\n");
  for (i=0; i<DataComm.NFitVars; i++)
   {
    cptr      = ppl_units_GetUnitStr(outval[i], &tmp1, &tmp2, 0, 0); // Work out what unit the best-fit value is best displayed in
    if      (fabs(outval[i]->real)>1e-200) tmp3 = tmp1 / outval[i]->real; // Set tmp3 to be the multiplicative size of this unit relative to its SI counterpart
    else if (fabs(outval[i]->imag)>1e-200) tmp3 = tmp2 / outval[i]->imag; // Can't do this if magnitude of best-fit value is zero, though...
    else
     {
      DummyTemp = *(outval[i]);
      DummyTemp.real = 1.0; DummyTemp.imag = 0.0; DummyTemp.FlagComplex = 0; // If best-fit value is zero, use the unit we would use for unity instead.
      cptr = ppl_units_GetUnitStr(&DummyTemp, &tmp1, &tmp2, 0, 0);
      tmp3 = tmp1;
     }
    ppl_units_zero(&DummyTemp);
    DummyTemp.real = tmp1; DummyTemp.imag = tmp2 ; DummyTemp.FlagComplex = outval[i]->FlagComplex ; DummyTemp.dimensionless = 0; // Want to display value without unit, and put unit later on line
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)
     {
      if (settings_term_current.NumDisplay != SW_DISPLAY_L)
       sprintf(temp_err_string, "%16s = (%s +/- %s) %s", FitVars[i], ppl_units_NumericDisplay(&DummyTemp,1,0,0), NumericDisplay(StdDev[i]*tmp3,0,settings_term_current.SignificantFigures,0), cptr);
      else
       {
        DataComm.ScratchPad[0]='$';
        for (j=1,k=0; FitVars[i][k]!='\0'; k++) { if (FitVars[i][k]=='_') DataComm.ScratchPad[j++]='\\'; DataComm.ScratchPad[j++]=FitVars[i][k]; }
        DataComm.ScratchPad[j++]='\0';
        sprintf(temp_err_string, "%17s = (%s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,1,0,0)+1);
        j=strlen(temp_err_string)-1; // Remove final $
        sprintf(temp_err_string+j, "\\pm%s)%s$", NumericDisplay(StdDev[i]*tmp3,0,settings_term_current.SignificantFigures,0), cptr);
       }
     }
    else
     {
      if      (settings_term_current.NumDisplay == SW_DISPLAY_T)
       sprintf(temp_err_string, "%16s = (%s +/- (%s+%s*sqrt(-1)))%s", FitVars[i], ppl_units_NumericDisplay(&DummyTemp,1,0,0), NumericDisplay(StdDev[2*i]*tmp3,0,settings_term_current.SignificantFigures,0), NumericDisplay(StdDev[2*i+1]*tmp3,2,settings_term_current.SignificantFigures,0), cptr);
      else if (settings_term_current.NumDisplay == SW_DISPLAY_L)
       {
        DataComm.ScratchPad[0]='$';
        for (j=1,k=0; FitVars[i][k]!='\0'; k++) { if (FitVars[i][k]=='_') DataComm.ScratchPad[j++]='\\'; DataComm.ScratchPad[j++]=FitVars[i][k]; }
        DataComm.ScratchPad[j++]='\0';
        sprintf(temp_err_string, "%17s = (%s", DataComm.ScratchPad, ppl_units_NumericDisplay(&DummyTemp,1,0,0)+1);
        j=strlen(temp_err_string)-1; // Remove final $
        sprintf(temp_err_string+j, "\\pm(%s+%si))%s$", NumericDisplay(StdDev[2*i]*tmp3,0,settings_term_current.SignificantFigures,0), NumericDisplay(StdDev[2*i+1]*tmp3,2,settings_term_current.SignificantFigures,0), cptr);
       }
      else
       sprintf(temp_err_string, "%16s = (%s +/- (%s+%si)) %s", FitVars[i], ppl_units_NumericDisplay(&DummyTemp,1,0,0), NumericDisplay(StdDev[2*i]*tmp3,0,settings_term_current.SignificantFigures,0), NumericDisplay(StdDev[2*i+1]*tmp3,2,settings_term_current.SignificantFigures,0), cptr);
     }
    ppl_report(temp_err_string);
   }

  // We're finished... can now free DataTable
  gsl_vector_free(BestFitParamVals);
  gsl_matrix_free(hessian_inv);
  gsl_matrix_free(hessian_lu);
  gsl_matrix_free(hessian);
  gsl_permutation_free(perm);
  lt_AscendOutOfContext(ContextLocalVec);
  return 0;
 }

