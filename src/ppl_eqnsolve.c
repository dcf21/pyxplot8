// ppl_eqnsolve.c
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

#define _PPL_EQNSOLVE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <gsl/gsl_errno.h>
#include <gsl/gsl_multimin.h>

#include "MathsTools/dcfmath.h"

#include "StringTools/str_constants.h"

#include "ppl_constants.h"
#include "ppl_eqnsolve.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Functions which we use to optimise in a log-log type space to help when
// solutions are many orders of magnitude different from initial guess
double optimise_RealToLog(double in, int iter, double *norm)
 {
  double a=-2;

  if (iter >= 3) { *norm = max(1e-200, fabs(in)); return in/(*norm); }
  if (iter >=2) a = -500;

  if (in >= exp(a)) return log(in);
  else              return 2*a - log(2*exp(a)-in);
 }

double optimise_LogToReal(double in, int iter, double *norm)
 {
  double a=-2;

  if (iter >=3) return in*(*norm);
  if (iter >=2) a = -500;

  if (in >= a) return exp(in);
  else         return 2*exp(a)-exp(2*a-in);
 }

// Structure used to communicate between top-level optimiser, and the
// evaluation slave called by GSL
typedef struct MMComm {
 char         *expr1  [EQNSOLVE_MAXDIMS];
 char         *expr2  [EQNSOLVE_MAXDIMS];
 char         *fitvarname[EQNSOLVE_MAXDIMS]; // Name of nth fit variable
 value        *fitvar    [EQNSOLVE_MAXDIMS];
 double        norm      [EQNSOLVE_MAXDIMS];
 int           Nfitvars , Nexprs , GoneNaN, iter2;
 double        sign;
 value         first  [EQNSOLVE_MAXDIMS];
 unsigned char IsFirst[EQNSOLVE_MAXDIMS];
 int          *errpos;
 char          errtext[LSTR_LENGTH];
 int           WarningPos; // One final algebra error is allowed to produce a warning
 char          warntext[LSTR_LENGTH];
 } MMComm;

char *PrintParameterValues(const gsl_vector *x, MMComm *data, char *output)
 {
  int i=0, j=0;

  sprintf(output+j, "( ");
  j += strlen(output+j);
  for (i=0; i<data->Nfitvars; i++)
   {
    sprintf(output+j, "%s=%s; ", data->fitvarname[i], ppl_units_NumericDisplay(data->fitvar[i],0,0,0));
    j += strlen(output+j);
   }
  if (j>3) sprintf(output+j-2, " )");
  else     sprintf(output    , "()");

  return output;
 }

double MultiMinSlave(const gsl_vector *x, void *params)
 {
  value output1, output2;
  int i;
  unsigned char squareroot=0;
  double accumulator=0.0;
  MMComm *data = (MMComm *)params;

  if (*(data->errpos)>=0) return GSL_NAN; // We've previously had a serious error... so don't do any more work

  if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)
   {
    for (i=0; i<data->Nfitvars; i++)
      data->fitvar[i]->real = optimise_LogToReal( gsl_vector_get(x, i) , data->iter2, &data->norm[i] );
   }
  else
   {
    for (i=0; i<data->Nfitvars; i++)
     {
      data->fitvar[i]->real        = optimise_LogToReal( gsl_vector_get(x,2*i  ) , data->iter2, &data->norm[2*i  ] );
      data->fitvar[i]->imag        = optimise_LogToReal( gsl_vector_get(x,2*i+1) , data->iter2, &data->norm[2*i+1] );
      data->fitvar[i]->FlagComplex = !ppl_units_DblEqual(data->fitvar[i]->imag, 0);
      if (!data->fitvar[i]->FlagComplex) data->fitvar[i]->imag=0.0; // Enforce that real numbers have positive zero imaginary components
     }
   }

  for (i=0; i<data->Nexprs; i++)
   {
    ppl_EvaluateAlgebra(data->expr1[i], &output1, 0, NULL, 0, data->errpos, data->errtext, 0);
    // If a numerical error happened; ignore it for now, but return NAN
    if (*(data->errpos) >= 0) { data->WarningPos=*(data->errpos); sprintf(data->warntext, "An algebraic error was encountered at %s: %s", PrintParameterValues(x,data,temp_err_string), data->errtext); *(data->errpos)=-1; return GSL_NAN; }

    if (data->expr2[i] != NULL)
     {
      ppl_EvaluateAlgebra(data->expr2[i], &output2, 0, NULL, 0, data->errpos, data->errtext, 0);
      // If a numerical error happened; ignore it for now, but return NAN
      if (*(data->errpos) >= 0) { data->WarningPos=*(data->errpos); sprintf(data->warntext, "An algebraic error was encountered at %s: %s", PrintParameterValues(x,data,temp_err_string), data->errtext); *(data->errpos)=-1; return GSL_NAN; }

      if (!ppl_units_DimEqual(&output1, &output2))
       {
        *(data->errpos)=0;
        sprintf(data->errtext, "The two sides of the equation which is being solved are not dimensionally compatible. The left side has dimensions of <%s> while the right side has dimensions of <%s>.",ppl_units_GetUnitStr(&output1, NULL, NULL, 0, 1, 0),ppl_units_GetUnitStr(&output2, NULL, NULL, 1, 1, 0));
        return GSL_NAN;
       }

#define TWINLOG(X) ((X>1e-200) ? log(X) : (2*log(1e-200) - log(2e-200-X)))
      accumulator += pow(TWINLOG(output1.real) - TWINLOG(output2.real) , 2); // Minimise sum of square deviations of many equations
      accumulator += pow(TWINLOG(output1.imag) - TWINLOG(output2.imag) , 2);
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
          strcpy(data->errtext, "The function being minimised or maximised does not have consistent units.");
          return GSL_NAN;
         }
       }
      accumulator += output1.real; // ... or just the raw values in minimise and maximise commands
     }
   }
  if (squareroot) accumulator = sqrt(accumulator);
  if (!gsl_finite(accumulator)) data->GoneNaN=1;
  return accumulator * data->sign;
 }

void MultiMinIterate(MMComm *commlink)
 {
  size_t                              iter = 0,iter2 = 0;
  int                                 i, Nparams, status=0;
  double                              size=0,sizelast=0,sizelast2=0,testval;
  const gsl_multimin_fminimizer_type *T = gsl_multimin_fminimizer_nmsimplex; // We don't use nmsimplex2 here because it was new in gsl 1.12
  gsl_multimin_fminimizer            *s;
  gsl_vector                         *x, *ss;
  gsl_multimin_function               fn;

  Nparams = commlink->Nfitvars * ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) ? 1:2);

  fn.n = Nparams;
  fn.f = &MultiMinSlave;
  fn.params = (void *)commlink;

  x  = gsl_vector_alloc( Nparams );
  ss = gsl_vector_alloc( Nparams );

  iter2=0;
  do
   {
    iter2++;
    commlink->iter2 = iter2;
    sizelast2 = size;

    // Transfer starting guess values from fitting variables into gsl_vector x
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)
     {
      for (i=0; i<commlink->Nfitvars; i++)
        gsl_vector_set(x,  i  ,optimise_RealToLog(commlink->fitvar[i]->real , iter2, &commlink->norm[  i  ]));
     }
    else
     {
      for (i=0; i<commlink->Nfitvars; i++)
       {
        gsl_vector_set(x,2*i  ,optimise_RealToLog(commlink->fitvar[i]->real , iter2, &commlink->norm[2*i  ]));
        gsl_vector_set(x,2*i+1,optimise_RealToLog(commlink->fitvar[i]->imag , iter2, &commlink->norm[2*i+1]));
       }
     }

    for (i=0; i<Nparams; i++)
     {
      if (fabs(gsl_vector_get(x,i))>1e-6) gsl_vector_set(ss, i, 0.1 * gsl_vector_get(x,i));
      else                                gsl_vector_set(ss, i, 0.1                      ); // Avoid having a stepsize of zero
     }

    s = gsl_multimin_fminimizer_alloc (T, fn.n);
    gsl_multimin_fminimizer_set (s, &fn, x, ss);

    // If initial value we are giving the minimiser produces an algebraic error, it's not worth continuing
    testval = MultiMinSlave(x,(void *)commlink);
    if (commlink->WarningPos>=0) { *(commlink->errpos) = commlink->WarningPos; commlink->WarningPos=-1; sprintf(commlink->errtext, "%s", commlink->warntext); return; }

    iter                 = 0;
    commlink->GoneNaN    = 0;
    do 
     {
      iter++;
      // When you're minimising over many parameters simultaneously sometimes nothing happens for a long time
      for (i=0; i<2+Nparams*2; i++) { status = gsl_multimin_fminimizer_iterate(s); if (status) break; }
      if (status) break;
      sizelast = size;
      size     = gsl_multimin_fminimizer_minimum(s);
     }
    while ((iter < 10) || ((size < sizelast) && (iter < 50))); // Iterate 10 times, and then see whether size carries on getting smaller

    // Transfer best-fit values from s->x into fitting variables
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)
     {
      for (i=0; i<commlink->Nfitvars; i++)
        commlink->fitvar[i]->real = optimise_LogToReal(gsl_vector_get(s->x,  i  ) , iter2, &commlink->norm[  i  ]);
     }
    else
     {
      for (i=0; i<commlink->Nfitvars; i++)
       {
        commlink->fitvar[i]->real = optimise_LogToReal(gsl_vector_get(s->x,2*i  ) , iter2, &commlink->norm[2*i  ]);
        commlink->fitvar[i]->imag = optimise_LogToReal(gsl_vector_get(s->x,2*i+1) , iter2, &commlink->norm[2*i+1]);
       }
     }

    gsl_multimin_fminimizer_free(s);
   }
  while ((iter2 < 3) || ((commlink->GoneNaN==0) && (!status) && (size < sizelast2) && (iter2 < 20))); // Iterate 2 times, and then see whether size carries on getting smaller

  if (iter2>=20) status=1;
  if (status) { *(commlink->errpos)=0; sprintf(commlink->errtext, "Failed to converge. GSL returned error: %s", gsl_strerror(status)); }
  gsl_vector_free(x);
  gsl_vector_free(ss);
  return;
 }

void MinOrMax(Dict *command, double sign)
 {
  MMComm        commlink;
  char         *VarName;
  List         *ViaList;
  ListIterator *ListIter;
  value        *DummyVar, DummyTemp;
  int           i, errpos=-1;

  ppl_units_zero(&DummyTemp);
  DummyTemp.real    = 1.0; // Default starting point is 1.0
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
      if ((DummyVar->string != NULL) || ((DummyVar->FlagComplex) && (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)) || (!gsl_finite(DummyVar->real)) || (!gsl_finite(DummyVar->imag))) { ppl_units_zero(DummyVar); DummyVar->real=1.0; } // Turn string variables into floats
      commlink.fitvar    [ commlink.Nfitvars ] = DummyVar;
      commlink.fitvarname[ commlink.Nfitvars ] = VarName;
     }
    else
     {
      DictAppendValue(_ppl_UserSpace_Vars, VarName, DummyTemp);
      DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&DummyVar);
      commlink.fitvar    [ commlink.Nfitvars ] = DummyVar;
      commlink.fitvarname[ commlink.Nfitvars ] = VarName;
     }

    commlink.Nfitvars += 1;
    ListIter = ListIterate(ListIter, NULL);
    if (commlink.Nfitvars >= EQNSOLVE_MAXDIMS)
     {
      sprintf(temp_err_string, "Too many via variables; the maximum allowed number is %d.", EQNSOLVE_MAXDIMS);
      ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
      return;
     }
   }

  DictLookup(command, "expression", NULL, (void *)&commlink.expr1[0]);
  commlink.expr2[0]   = NULL;
  commlink.sign       = sign;
  commlink.IsFirst[0] = 1;
  commlink.errpos     = &errpos;
  commlink.WarningPos =-1;
  commlink.GoneNaN    = 0;
  ppl_units_zero(&commlink.first[0]);

  MultiMinIterate(&commlink);

  if (commlink.WarningPos >= 0) ppl_warning(ERR_NUMERIC, commlink.warntext);
  if (errpos >= 0) ppl_error(ERR_NUMERIC, -1, -1, commlink.errtext);

  if ((errpos >= 0) || (commlink.GoneNaN==1))
   {
    for (i=0; i<commlink.Nfitvars; i++) { commlink.fitvar[i]->real=GSL_NAN; commlink.fitvar[i]->imag=0; commlink.fitvar[i]->FlagComplex=0; } // We didn't produce a sensible answer
   }
  return;
 }

void directive_solve(Dict *command)
 {
  MMComm        commlink;
  char         *VarName;
  List         *ViaList;
  ListIterator *ListIter;
  value        *DummyVar, DummyTemp;
  int           i, errpos=-1;

  ppl_units_zero(&DummyTemp);
  DummyTemp.real    = 1.0; // Default starting point is 1.0
  commlink.Nfitvars = 0;
  commlink.Nexprs   = 0;

  // Fetch the names of the variables which we are fitting
  DictLookup(command, "fit_variables,", NULL, (void *)&ViaList);
  ListIter = ListIterateInit(ViaList);
  while (ListIter != NULL)
   {
    DictLookup(ListIter->data, "fit_variable", NULL, (void *)&VarName); // Look up each fitting variable in the user's variable space
    DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&DummyVar);

    if (DummyVar!=NULL)
     {
      if ((DummyVar->string != NULL) || ((DummyVar->FlagComplex) && (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)) || (!gsl_finite(DummyVar->real)) || (!gsl_finite(DummyVar->imag))) { ppl_units_zero(DummyVar); DummyVar->real=1.0; } // Turn string variables into floats
      commlink.fitvar    [ commlink.Nfitvars ] = DummyVar;
      commlink.fitvarname[ commlink.Nfitvars ] = VarName;
     }
    else // If variable is not defined, create it now
     {
      DictAppendValue(_ppl_UserSpace_Vars, VarName, DummyTemp);
      DictLookup(_ppl_UserSpace_Vars, VarName, NULL, (void **)&DummyVar);
      commlink.fitvar    [ commlink.Nfitvars ] = DummyVar;
      commlink.fitvarname[ commlink.Nfitvars ] = VarName;
     }

    commlink.Nfitvars += 1;
    ListIter = ListIterate(ListIter, NULL);
    if (commlink.Nfitvars >= EQNSOLVE_MAXDIMS)
     {
      sprintf(temp_err_string, "Too many via variables; the maximum allowed number is %d.", EQNSOLVE_MAXDIMS);
      ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
      return;
     }
   }

  DictLookup(command, "expressions,", NULL, (void *)&ViaList);
  ListIter = ListIterateInit(ViaList);
  while (ListIter != NULL)
   {
    DictLookup(ListIter->data, "left_expression" , NULL, (void *)&commlink.expr1[ commlink.Nexprs ]);
    DictLookup(ListIter->data, "right_expression", NULL, (void *)&commlink.expr2[ commlink.Nexprs ]);
    commlink.Nexprs += 1;
    ListIter = ListIterate(ListIter, NULL);
    if (commlink.Nexprs >= EQNSOLVE_MAXDIMS)
     {
      sprintf(temp_err_string, "Too many simultaneous equations to solve; the maximum allowed number is %d.", EQNSOLVE_MAXDIMS);
      ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
      return;
     }
   }

 if (commlink.Nexprs < 1)
  {
   sprintf(temp_err_string, "No equations supplied to solve.");
    ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
    return;
   }

  commlink.sign       = 1.0;
  commlink.errpos     = &errpos;
  commlink.WarningPos =-1;
  commlink.GoneNaN    = 0;
  for (i=0; i<commlink.Nexprs; i++) { commlink.IsFirst[i]=1; ppl_units_zero(&commlink.first[i]); }

  MultiMinIterate(&commlink);

  if (commlink.WarningPos >= 0) ppl_warning(ERR_NUMERIC, commlink.warntext);
  if (errpos >= 0) ppl_error(ERR_NUMERIC, -1, -1, commlink.errtext);

  if ((errpos >= 0) || (commlink.GoneNaN==1))
   {
    for (i=0; i<commlink.Nfitvars; i++) { commlink.fitvar[i]->real=GSL_NAN; commlink.fitvar[i]->imag=0; commlink.fitvar[i]->FlagComplex=0; } // We didn't produce a sensible answer
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

