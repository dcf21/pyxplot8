// ppl_fft.c
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

#define _PPL_FFT_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wordexp.h>
#include <glob.h>

#include <gsl/gsl_math.h>

#ifdef HAVE_FFTW3
#include <fftw3.h>
#else
#include <fftw.h>
#endif

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_fft.h"
#include "ppl_settings.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Window functions for FFTs
static void fftwindow_rectangular (value *x, int Ndim, int *Npos, int *Nstep) { }
static void fftwindow_hamming     (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; int i; for (i=0; i<Ndim; i++) { y*=0.54-0.46*cos(2*M_PI*((double)Npos[i])/((double)(Nstep[i]-1))); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_hann        (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; int i; for (i=0; i<Ndim; i++) { y*=0.5*(1.0-cos(2*M_PI*((double)Npos[i])/((double)(Nstep[i]-1)))); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_cosine      (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; int i; for (i=0; i<Ndim; i++) { y*=sin(M_PI*((double)Npos[i])/((double)(Nstep[i]-1))); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_lanczos     (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0,z; int i; for (i=0; i<Ndim; i++) { z=2*M_PI*((double)Npos[i])/((double)(Nstep[i]-1))-M_PI; y*=sin(z)/z; } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_bartlett    (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; int i; for (i=0; i<Ndim; i++) { y*=(2.0/(Nstep[i]-1))*((Nstep[i]-1)/2.0-fabs(Npos[i]-(Nstep[i]-1)/2.0)); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_triangular  (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; int i; for (i=0; i<Ndim; i++) { y*=(2.0/(Nstep[i]  ))*((Nstep[i]  )/2.0-fabs(Npos[i]-(Nstep[i]-1)/2.0)); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_gauss       (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; const double sigma=0.5; int i; for (i=0; i<Ndim; i++) { y*=exp(-0.5*pow((Npos[i]-((Nstep[i]-1)/2.0))/(sigma*(Nstep[i]-1)/2.0),2.0)); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_bartletthann(value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; int i; for (i=0; i<Ndim; i++) { y*=0.62 - 0.48*(((double)Npos[i])/((double)(Nstep[i]-1))-0.5) - 0.38*cos(2*M_PI*((double)Npos[i])/((double)(Nstep[i]-1))); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }
static void fftwindow_blackman    (value *x, int Ndim, int *Npos, int *Nstep) { double y=1.0; const double alpha=0.16; int i; for (i=0; i<Ndim; i++) { y*=(1.0-alpha)/2.0 - 0.5*cos(2*M_PI*((double)Npos[i])/((double)(Nstep[i]-1))) + alpha/2.0*cos(4*M_PI*((double)Npos[i])/((double)(Nstep[i]-1))); } x->real*=y; x->imag*=y; if (x->imag==0.0) { x->imag=0.0; x->FlagComplex=0; } }

// Main entry point for the implementation of the fft command
int directive_fft(Dict *command)
 {
  int           i, j, k, l, status, Ndims, Nsamples, Nsteps[USING_ITEMS_MAX], Npos[USING_ITEMS_MAX];
  int           ContextOutput, ContextLocalVec, ContextDataTab;
  FunctionDescriptor *FuncPtr, *FuncPtrNext, *FuncPtr2;
  FFTDescriptor      *output;
  double        TempDbl, pos[USING_ITEMS_MAX];
  value        *min[USING_ITEMS_MAX], *max[USING_ITEMS_MAX], *step[USING_ITEMS_MAX], x, FirstVal;
  unsigned char inverse;
  char         *cptr, *filename, *outfunc, *infunc, *scratchpad, *errtext;
  void (*WindowType)(value *,int,int *,int *);
  wordexp_t     WordExp;
  glob_t        GlobData;
  List         *RangeList;
  ListIterator *ListIter;
  Dict         *TempDict;
  fftw_complex *datagrid;

  #ifdef HAVE_FFTW3
  fftw_plan     fftwplan; // FFTW 3.x
  #else
  fftwnd_plan   fftwplan; // FFTW 2.x
  #endif

  // Check whether we are doing a forward or a backward FFT
  DictLookup(command, "directive", NULL, (void **)&cptr);
  if (strcmp(cptr,"ifft")==0) inverse=1;
  else                        inverse=0;

  // Read in specified data ranges
  DictLookup(command, "range_list", NULL, (void **)&RangeList);
  ListIter = ListIterateInit(RangeList);
  for (Ndims=0; ((Ndims<USING_ITEMS_MAX)&&(ListIter!=NULL)); Ndims++) // Can have up to USING_ITEMS_MAX dimensions
   {
    TempDict = (Dict *)ListIter->data;
    DictLookup(TempDict,"min" ,NULL,(void **)(min +Ndims));
    DictLookup(TempDict,"max" ,NULL,(void **)(max +Ndims));
    DictLookup(TempDict,"step",NULL,(void **)(step+Ndims));

    if (!ppl_units_DimEqual(min[Ndims],max [Ndims])) { sprintf(temp_err_string, "The minimum and maximum specified for dimension %d to the fft command have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>."  , Ndims+1, ppl_units_GetUnitStr(min[Ndims],NULL,NULL,0,0), ppl_units_GetUnitStr(max [Ndims],NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    if (!ppl_units_DimEqual(min[Ndims],step[Ndims])) { sprintf(temp_err_string, "The minimum and step size specified for dimension %d to the fft command have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", Ndims+1, ppl_units_GetUnitStr(min[Ndims],NULL,NULL,0,0), ppl_units_GetUnitStr(step[Ndims],NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    if (min[Ndims]->real >= max[Ndims]->real) { sprintf(temp_err_string, "The maximum ordinate value supplied for dimension %d to the fft command is not greater than the minimum ordinate value, which it must be.", Ndims+1); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    if (step[Ndims]->real <= 0.0) { sprintf(temp_err_string, "The ordinate step size supplied for dimension %d to the fft command is not positive and non-zero, which it must be.", Ndims+1); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    TempDbl = 1.0 + floor((max[Ndims]->real-min[Ndims]->real) / step[Ndims]->real + 0.5); // Add one because this is a fencepost problem
    if (TempDbl<2.0) { sprintf(temp_err_string, "The number of samples produced by the range and step size specified for dimension %d to the fft command is fewer than two; a single data sample cannot be FFTed.", Ndims+1); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    if (TempDbl>1e8) { sprintf(temp_err_string, "The number of samples produced by the range and step size specified for dimension %d to the fft command is in excess of 1e8; PyXPlot is not the right tool to do this FFT in.", Ndims+1); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    Nsteps[Ndims] = (int)TempDbl;
    ListIter = ListIterate(ListIter, NULL);
   }
   if (ListIter != NULL) { sprintf(temp_err_string, "Too many ranges supplied to the fft command. A %d-dimensional fft was attempted, but only a maximum of %d dimensions are supported.", ListLen(RangeList), USING_ITEMS_MAX); ppl_error(ERR_SYNTAX, temp_err_string); return 1; }

  // Work out total size of FFT data grid
  TempDbl = 1.0;
  for (i=0; i<Ndims; i++) TempDbl *= Nsteps[i];
  if (TempDbl > 1e8) { sprintf(temp_err_string, "The total number of samples in the requested %d-dimensional FFT is in excess of 1e8; PyXPlot is not the right tool to do this FFT in.", Ndims); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
  Nsamples = (int)TempDbl;

  // Check that the function we're about to replace isn't a system function
  DictLookup(command, "fft_function", NULL, (void **)&outfunc);   if (outfunc == NULL) { ppl_error(ERR_INTERNAL, "ppl_fft could not read name of function for output."); return 1; }
  DictLookup(_ppl_UserSpace_Funcs, outfunc, NULL, (void *)&FuncPtr); // Check whether we are going to overwrite an existing function
  if ((FuncPtr!=NULL)&&((FuncPtr->FunctionType==PPL_USERSPACE_SYSTEM)||(FuncPtr->FunctionType==PPL_USERSPACE_STRFUNC)||(FuncPtr->FunctionType==PPL_USERSPACE_UNIT)))
   { sprintf(temp_err_string, "Attempt to redefine a core system function %s()", outfunc); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  ContextOutput  = lt_GetMemContext();
  ContextLocalVec= lt_DescendIntoNewContext();
  ContextDataTab = lt_DescendIntoNewContext();

  // Work out what type of window we're going to use
  WindowType = fftwindow_rectangular;
  DictLookup(command,"window",NULL,(void **)(&cptr));
  if (cptr!=NULL)
   {
    if      (strcmp(cptr,"hamming"     )!=0) WindowType = fftwindow_hamming;
    else if (strcmp(cptr,"hann"        )!=0) WindowType = fftwindow_hann;
    else if (strcmp(cptr,"cosine"      )!=0) WindowType = fftwindow_cosine;
    else if (strcmp(cptr,"lanczos"     )!=0) WindowType = fftwindow_lanczos;
    else if (strcmp(cptr,"bartlett"    )!=0) WindowType = fftwindow_bartlett;
    else if (strcmp(cptr,"triangular"  )!=0) WindowType = fftwindow_triangular;
    else if (strcmp(cptr,"gauss"       )!=0) WindowType = fftwindow_gauss;
    else if (strcmp(cptr,"bartletthann")!=0) WindowType = fftwindow_bartletthann;
    else if (strcmp(cptr,"blackman"    )!=0) WindowType = fftwindow_blackman;
   }

  // Fetch filename of potential datafile to FFT
  DictLookup(command,"filename",NULL,(void **)(&cptr));

  // If we are FFTing data from a file, glob filename now
  if (cptr != NULL)
   {
    if ((wordexp(cptr, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
    if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
    if ((glob(WordExp.we_wordv[0], 0, NULL, &GlobData) != 0) || (GlobData.gl_pathc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return 1; }
    if  (GlobData.gl_pathc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return 1; }
    filename = lt_malloc(strlen(GlobData.gl_pathv[0])+1);
    if (filename==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); wordfree(&WordExp); globfree(&GlobData); return 1; }
    strcpy(filename, GlobData.gl_pathv[0]);
    wordfree(&WordExp);
    globfree(&GlobData);
   }

  // Allocate workspace in which to do FFT
  datagrid = (fftw_complex *)fftw_malloc(Nsamples * sizeof(fftw_complex));
  if (datagrid == NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return 1; }

  // Fetch data which we are going to FFT
  if (cptr != NULL) // We are FFTing data from a file
   {
   }
  else // We are FFTing data from a function
   {
    DictLookup(command, "input_function", NULL, (void **)&infunc);
    if (infunc == NULL) { ppl_error(ERR_INTERNAL, "ppl_fft could not read name of function for input."); return 1; }
    DictLookup(_ppl_UserSpace_Funcs, infunc, NULL, (void *)&FuncPtr2);
    if (FuncPtr2 == NULL) { sprintf(temp_err_string, "No such function as %s()", infunc); ppl_error(ERR_GENERAL, temp_err_string); return 1; }
    if (FuncPtr2->NumberArguments != Ndims) { sprintf(temp_err_string, "%d-dimensional Fourier transforms can only be performed upon functions which take %d inputs. The supplied function %s() takes %d inputs.", Ndims, Ndims, infunc, FuncPtr2->NumberArguments); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

    scratchpad = (char *)lt_malloc(LSTR_LENGTH);
    errtext    = (char *)lt_malloc(LSTR_LENGTH);
    if ((scratchpad == NULL) || (errtext == NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); return 1; }

    status=-1;
    for (i=0; i<Nsamples; i++)
     {
      sprintf(scratchpad,"%s(",infunc);
      j=strlen(scratchpad);
      k=i;
      for (l=Ndims-1; l>=0; l--) { Npos[l] = (k % Nsteps[l]); pos[l] = min[l]->real+step[l]->real*Npos[l]; k /= Nsteps[l]; }
      for (l=0; l<Ndims; l++) { x=*(min[l]); x.real=pos[l]; sprintf(scratchpad+j,"%s,",ppl_units_NumericDisplay(&x,0,1,20)); j+=strlen(scratchpad+j); }
      sprintf(scratchpad+j-(Ndims>0),")");
      ppl_EvaluateAlgebra(scratchpad, &x, 0, NULL, 0, &status, errtext, 0);
      if ((status>=0)||(!gsl_finite(x.real))||(!gsl_finite(x.imag))) { sprintf(temp_err_string, "Could not evaluate input function at position %s(", infunc); j=strlen(temp_err_string); for (l=0; l<Ndims; l++) { x=*(min[l]); x.real=pos[l]; sprintf(temp_err_string+j,"%s,",ppl_units_NumericDisplay(&x,0,1,0)); j+=strlen(temp_err_string+j); } sprintf(temp_err_string+j-(Ndims>0),")"); ppl_error(ERR_NUMERIC, temp_err_string); return 1; } // Evaluation of algebra failed
      if (i==0) { FirstVal=x; FirstVal.real=1.0; FirstVal.imag=0.0; FirstVal.FlagComplex=0; }
      else if (!ppl_units_DimEqual(&x, &FirstVal)) { sprintf(temp_err_string, "The supplied function to FFT does not produce values with consistent units; has produced values with units of <%s> and of <%s>.", ppl_units_GetUnitStr(&FirstVal,NULL,NULL,0,0), ppl_units_GetUnitStr(&x,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
      (*WindowType)(&x, Ndims, Npos, Nsteps); // Apply window function to data
      #ifdef HAVE_FFTW3
      datagrid[i][0] = x.real; datagrid[i][1] = x.imag;
      #else
      datagrid[i].re = x.real; datagrid[i].im = x.imag;
      #endif
     }
   }

  // FFT data
  #ifdef HAVE_FFTW3
  fftwplan = fftw_plan_dft(Ndims, Nsteps, datagrid, datagrid, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE); // FFTW 3.x
  fftw_execute(fftwplan);
  fftw_destroy_plan(fftwplan);
  #else
  fftwplan = fftwnd_create_plan(Ndims, Nsteps, inverse ? FFTW_BACKWARD : FFTW_FORWARD, FFTW_ESTIMATE);                // FFTW 2.x
  fftwnd_one(fftwplan, datagrid, datagrid);
  fftwnd_destroy_plan(fftwplan);
  #endif

  // Make FFTDescriptor data structure
  output = (FFTDescriptor *)lt_malloc_incontext(sizeof(FFTDescriptor), 0);
  if (output == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return 1; }
  output->Ndims    = Ndims;
  output->XSize    = (int   *)malloc(Ndims * sizeof(int  ));
  output->range    = (value *)malloc(Ndims * sizeof(value));
  output->invrange = (value *)malloc(Ndims * sizeof(value));
  if ((output->XSize==NULL)||(output->range==NULL)||(output->invrange==NULL)) { free(output); fftw_free(datagrid); ppl_error(ERR_MEMORY,"Out of memory"); return 1; }
  for (i=0; i<Ndims; i++) output->XSize[i] = Nsteps[i];
  status=0;
  for (i=0; i<Ndims; i++) { ppl_units_sub(max[i], min[i], &output->range[i], &status, temp_err_string); if (status) break; }
  if (status) { ppl_error(ERR_INTERNAL,temp_err_string); free(output); fftw_free(datagrid); free(output->XSize); free(output->range); free(output->invrange); return 1; }
  for (i=0; i<Ndims; i++) { ppl_units_sub(max[i], min[i], &output->range[i], &status, temp_err_string); if (status) break; }
  if (status) { ppl_error(ERR_INTERNAL,temp_err_string); free(output); fftw_free(datagrid); free(output->XSize); free(output->range); free(output->invrange); return 1; }
  for (i=0; i<Ndims; i++) ppl_units_DimInverse(&output->invrange[i], &output->range[i]);
  output->datagrid = datagrid;
  output->normalisation = 1.0/sqrt((double)Nsamples);

  // Make output unit
  output->OutputUnit = FirstVal; // Output of an FFT has units of fn being FFTed, multiplied by the reciprocal of the units of all arguments being FFTed
  for (i=0; i<Ndims; i++) { ppl_units_mult(&output->OutputUnit, &output->invrange[i], &output->OutputUnit, &status, temp_err_string); if (status>=0) break; }
  if (status) { ppl_error(ERR_INTERNAL,temp_err_string); free(output); fftw_free(datagrid); free(output->XSize); free(output->range); free(output->invrange); return 1; }
  output->OutputUnit.real = output->OutputUnit.imag = 0.0;
  output->OutputUnit.FlagComplex = 0; // Output unit has zero magnitude

  // Make a new function descriptor
  FuncPtr2 = (FunctionDescriptor *)lt_malloc_incontext(sizeof(FunctionDescriptor), 0);
  if (FuncPtr2 == NULL) { ppl_error(ERR_MEMORY, "Out of memory whilst adding fft object to function dictionary."); return 1; }
  FuncPtr2->FunctionType    = PPL_USERSPACE_FFT;
  FuncPtr2->modified        = 1;
  FuncPtr2->NumberArguments = Ndims;
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
  DictAppendPtr(_ppl_UserSpace_Funcs, outfunc, (void *)FuncPtr2, sizeof(FunctionDescriptor), 0, DATATYPE_VOID);

  // Free copies of data vectors defined within our local context
  lt_AscendOutOfContext(ContextLocalVec);
  return 0;
 }

// Function which is called whenever an FFT function is evaluated, to extract value out of data grid
void ppl_fft_evaluate(char *FuncName, FFTDescriptor *desc, value *in, value *out, int *status, char *errout)
 {
  int i, j;
  double TempDbl;

  *out = desc->OutputUnit;

  // Issue user with a warning if complex arithmetic is not enabled
  if (settings_term_current.ComplexNumbers != SW_ONOFF_ON) ppl_warning(ERR_NUMERIC, "Attempt to evaluate a Fourier transform function whilst complex arithmetic is disabled. Fourier transforms are almost invariably complex and so this is unlikely to work.");

  // Check dimensions of input arguments and ensure that they are all real
  for (i=0; i<desc->Ndims; i++)
   {
    if (!ppl_units_DimEqual(in+i, &desc->invrange[i]))
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "The %s(x) function expects argument %d to have dimensions of <%s>, but has instead received an argument with dimensions of <%s>.", FuncName, i+1, ppl_units_GetUnitStr(&desc->invrange[i], NULL, NULL, 0, 0), ppl_units_GetUnitStr(in+i, NULL, NULL, 1, 0)); }
      else { ppl_units_zero(out); out->real = GSL_NAN; out->imag = 0; }
      *status=1;
      return;
     }
    if ((in+i)->FlagComplex)
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { sprintf(errout, "The %s(x) function expects argument %d to be real, but the supplied argument has an imaginary component.", FuncName, i+1); }
      else { ppl_units_zero(out); out->real = GSL_NAN; out->imag = 0; }
      *status=1;
      return;
     }
   }

  // Work out closest datapoint in FFT datagrid to the one we want
  j=0;
  for (i=0; i<desc->Ndims; i++)
   {
    TempDbl = floor((in+i)->real * desc->range[i].real + 0.5);
    if      ((TempDbl >= 0.0) && (TempDbl <= desc->XSize[i]/2)) { } // Positive frequencies stored in lower half of array
    else if ((TempDbl <  0.0) && (TempDbl >=-desc->XSize[i]/2)) { TempDbl += desc->XSize[i]; } // Negative frequencies stored in upper half of array
    else                                                        { return; } // Query out of range; return zero with appropriate output unit
    j *= desc->XSize[i];
    j += (int)TempDbl;
   }

  // Write output value to out
  #ifdef HAVE_FFTW3
  out->real = desc->datagrid[j][0] * desc->normalisation;
  if (desc->datagrid[j][1] == 0.0) { out->FlagComplex = 0; out->imag = 0.0;                  }
  else                             { out->FlagComplex = 1; out->imag = desc->datagrid[j][1] * desc->normalisation; }
  #else
  out->real = desc->datagrid[j].re * desc->normalisation;
  if (desc->datagrid[j].im == 0.0) { out->FlagComplex = 0; out->imag = 0.0;                  }
  else                             { out->FlagComplex = 1; out->imag = desc->datagrid[j].im * desc->normalisation; }
  #endif
  return;
 }

