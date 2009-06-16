// ppl_eqnsolve.c
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

#define _PPL_EQNSOLVE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <gsl/gsl_multimin.h>

#include "StringTools/str_constants.h"

#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_units.h"
#include "ppl_userspace.h"

typedef struct MMComm {
 char         *expr1  [EQNSOLVE_MAXDIMS];
 char         *expr2  [EQNSOLVE_MAXDIMS];
 value        *fitvar [EQNSOLVE_MAXDIMS];
 int           Nfitvars , Nexprs , GoneNaN;
 double        sign;
 value         first  [EQNSOLVE_MAXDIMS];
 unsigned char IsFirst[EQNSOLVE_MAXDIMS];
 int          *errpos;
 char          errtext[LSTR_LENGTH];
 } MMComm;

double MultiMinSlave(const gsl_vector *x, void *params)
 {
  value output1, output2;
  int i;
  unsigned char squareroot=0;
  double accumulator=0.0;
  MMComm *data = (MMComm *)params;

  if (*(data->errpos)>=0) return GSL_NAN; // We've previously had an error... so don't do any more work

  for (i=0; i<data->Nfitvars; i++) data->fitvar[i]->number = gsl_vector_get(x, i);

  for (i=0; i<data->Nexprs; i++)
   {
    ppl_EvaluateAlgebra(data->expr1[i], &output1, 0, NULL, data->errpos, data->errtext, 0);

    if (data->expr2[i] != NULL)
     {
      ppl_EvaluateAlgebra(data->expr2[i], &output2, 0, NULL, data->errpos, data->errtext, 0);
      if (!ppl_units_DimEqual(&output1, &output2))
       {
        *(data->errpos)=0;
        strcpy(data->errtext, "Error: The two sides of the equation which is being solved are not dimensionally compatible.");
        return GSL_NAN;
       }
      accumulator += pow(output1.number - output2.number , 2); // Minimise sum of square deviations of many equations
      squareroot = 1;
     } else {
      if (data->IsFirst[i])
       {
        memcpy(&data->first[i], &output1, sizeof(value));
        data->IsFirst[i] = 0;
       } else {
        if (!ppl_units_DimEqual(&data->first[i],&output1))
         {
          *(data->errpos)=0;
          strcpy(data->errtext, "Error: The function being minimised or maximised does not have consistent units.");
          return GSL_NAN;
         }
       }
      accumulator += output1.number; // ... or just the raw values in minimise and maximise commands
     }
   }
  if (squareroot) accumulator = sqrt(accumulator);
  if (!gsl_finite(accumulator)) data->GoneNaN=1;
  return accumulator * data->sign;
 }

void MinOrMax(Dict *command, double sign)
 {
  MMComm                             commlink;
  char    /*   -    -    -    - */  *VarName;
  List                              *ViaList;
  ListIterator                      *ListIter;
  value                             *DummyVar, DummyTemp;
  size_t                             iter = 0;
  int                                i, status, errpos=-1;
  double                             size=0,sizelast=0;
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex2;
  gsl_multimin_fminimizer           *s;
  gsl_vector                        *x, *ss;
  gsl_multimin_function              fn;

  ppl_units_zero(&DummyTemp);
  DummyTemp.number  = 1.0; // Default starting point is 1.0
  commlink.Nfitvars = 0;
  commlink.Nexprs   = 1;

  DictLookup(command, "fit_variables,", NULL, (void *)&ViaList);
  ListIter = ListIterateInit(ViaList);
  while (ListIter != NULL)
   {
    DictLookup(ListIter->data, "fit_variable", NULL, (void *)&VarName);
    DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&DummyVar);

    if (DummyVar!=NULL)
     {
      if (DummyVar->string != NULL) { ppl_units_zero(DummyVar); DummyVar->number=1.0; } // Turn string variables into floats
      commlink.fitvar[ commlink.Nfitvars ] = DummyVar;
     }
    else
     {
      DictAppendValue(_ppl_UserSpace_Vars, VarName, DummyTemp);
      DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&DummyVar);
      commlink.fitvar[ commlink.Nfitvars ] = DummyVar;
     }

    commlink.Nfitvars += 1;
    ListIter = ListIterate(ListIter, NULL);
    if (commlink.Nfitvars >= EQNSOLVE_MAXDIMS)
     {
      sprintf(temp_err_string, "Error: Too many via variables; the maximum allowed number is %d.", EQNSOLVE_MAXDIMS);
      ppl_error(temp_err_string);
      return;
     }
   }

  DictLookup(command, "expression", NULL, (void *)&commlink.expr1[0]);
  commlink.expr2[0]   = NULL;
  commlink.sign       = sign;
  commlink.IsFirst[0] = 1;
  commlink.errpos     = &errpos;
  commlink.GoneNaN    = 0;
  ppl_units_zero(&commlink.first[0]);

  fn.n = commlink.Nfitvars;
  fn.f = &MultiMinSlave;
  fn.params = (void *)&commlink;

  x = gsl_vector_alloc( commlink.Nfitvars );
  ss = gsl_vector_alloc( commlink.Nfitvars );

  for (i=0; i<commlink.Nfitvars; i++) gsl_vector_set(x , i,     commlink.fitvar[i]->number);
  for (i=0; i<commlink.Nfitvars; i++) gsl_vector_set(ss, i, 0.1*commlink.fitvar[i]->number);

  s = gsl_multimin_fminimizer_alloc (T, fn.n);
  gsl_multimin_fminimizer_set (s, &fn, x, ss);

  do
   {
    iter++;
    for (i=0; i<2+commlink.Nfitvars*2; i++) // When you're minimising over many parameters simultaneously sometimes nothing happens for a long time
     {
      status = gsl_multimin_fminimizer_iterate(s);           
      if (status) break;
     }
    if (status) break;
    if (iter == 8) commlink.GoneNaN=0;
    sizelast = size;   
    size     = gsl_multimin_fminimizer_minimum(s);
   }
  while ((iter < 10) || ((size < sizelast) && (iter < 100))); // Iterate 10 times, and then see whether size carries on getting smaller

  gsl_multimin_fminimizer_free(s);

  gsl_vector_free(x);
  gsl_vector_free(ss);

  if (errpos >= 0) ppl_error(commlink.errtext);

  if ((status) || (errpos >= 0) || (iter>=100) || (commlink.GoneNaN==1))
   {
    for (i=0; i<commlink.Nfitvars; i++) commlink.fitvar[i]->number=GSL_NAN; // We didn't produce a sensible answer
   }

  return;
 }

void directive_minimise(Dict *command)
 {
  MinOrMax(command, 1.0);
 }

void directive_maximise(Dict *command)
 {
  MinOrMax(command,-1.0);
 }

