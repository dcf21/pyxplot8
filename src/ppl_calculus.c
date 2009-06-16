// ppl_calculus.c
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

#define _PPL_CALCULUS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include <gsl/gsl_deriv.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>

#include "ppl_units.h"
#include "ppl_userspace.h"

typedef struct IntComm {
 char         *expr;
 value        *dummy;
 value         first;
 unsigned char IsFirst;
 int          *errpos;
 char         *errtext;
 int           RecursionDepth;
 } IntComm;

double CalculusSlave(double x, void *params)
 {
  value output;
  IntComm *data = (IntComm *)params;

  if (*(data->errpos)>=0) return GSL_NAN; // We've previously had an error... so don't do any more work

  data->dummy->number = x;
  ppl_EvaluateAlgebra(data->expr, &output, 0, NULL, data->errpos, data->errtext, data->RecursionDepth+1);

  if (data->IsFirst)
   {
    memcpy(&data->first, &output, sizeof(value));
    data->IsFirst = 0;
   } else {
    if (!ppl_units_DimEqual(&data->first,&output))
     {
      *(data->errpos)=0;
      strcpy(data->errtext, "Error: This operand does not have consistent units across the range where calculus is being attempted.");
      return GSL_NAN;
     }
   }
  return output.number;
 }

void Integrate(char *expr, char *dummy, value *min, value *max, value *out, int *errpos, char *errtext, int RecursionDepth)
 {
  IntComm                    commlink;
  value                     *DummyVar;
  value                      DummyTemp;
  gsl_integration_workspace *ws;
  gsl_function               fn;
  double                     result, error;

  if (!ppl_units_DimEqual(min,max))
   {
    *errpos=0;
    strcpy(errtext, "Error: The minimum and maximum limits of this integration operation are not dimensionally compatible.");
    return;
   }

  commlink.expr    = expr;
  commlink.IsFirst = 1;
  commlink.errpos  = errpos;
  commlink.errtext = errtext;
  commlink.RecursionDepth = RecursionDepth;
  ppl_units_zero(&commlink.first);

  DictLookup(_ppl_UserSpace_Vars, dummy, NULL, (void **)&DummyVar);
  if (DummyVar!=NULL)
   {
    memcpy( &DummyTemp , DummyVar , sizeof(value));
    memcpy(  DummyVar  , min      , sizeof(value)); // Get units of DummyVar right
   }
  else
   {
    DictAppendValue(_ppl_UserSpace_Vars, dummy, *min); // Get units of DummyVar right
    DictLookup     (_ppl_UserSpace_Vars, dummy, NULL, (void **)&DummyVar);
    ppl_units_zero(&DummyTemp);
    DummyTemp.modified = 2;
   }
  commlink.dummy = DummyVar;

  ws          = gsl_integration_workspace_alloc(1000);
  fn.function = &CalculusSlave;
  fn.params   = &commlink;

  gsl_integration_qags (&fn, min->number, max->number, 0, 1e-7, 1000, ws, &result, &error); 

  memcpy( DummyVar  , &DummyTemp , sizeof(value)); // Restore old value of the dummy variable we've been using

  if (*errpos < 0)
   {
    ppl_units_mult( &commlink.first , min , out , errpos, errtext ); // Get units of output right
    out->number = result;
   }
  return;
 }

void Differentiate(char *expr, char *dummy, value *point, value *step, value *out, int *errpos, char *errtext, int RecursionDepth)
 {
  IntComm                    commlink;
  value                     *DummyVar;
  value                      DummyTemp;
  gsl_function               fn;
  double                     result, error;

  if (!ppl_units_DimEqual(point, step))
   {
    *errpos=0;
    strcpy(errtext, "Error: The arguments x and step to this differentiation operation are not dimensionally compatible.");
    return;
   }

  commlink.expr    = expr;
  commlink.IsFirst = 1;
  commlink.errpos  = errpos;
  commlink.errtext = errtext;
  commlink.RecursionDepth = RecursionDepth;
  ppl_units_zero(&commlink.first);

  DictLookup(_ppl_UserSpace_Vars, dummy, NULL, (void **)&DummyVar);
  if (DummyVar!=NULL)
   {
    memcpy( &DummyTemp , DummyVar , sizeof(value));
    memcpy(  DummyVar  , point    , sizeof(value)); // Get units of DummyVar right
   }
  else
   {
    DictAppendValue(_ppl_UserSpace_Vars, dummy, *point); // Get units of DummyVar right
    DictLookup     (_ppl_UserSpace_Vars, dummy, NULL, (void **)&DummyVar);
    ppl_units_zero(&DummyTemp);
    DummyTemp.modified = 2;
   }
  commlink.dummy = DummyVar;

  fn.function = &CalculusSlave;
  fn.params   = &commlink;

  gsl_deriv_central(&fn, point->number, step->number, &result, &error);

  memcpy( DummyVar  , &DummyTemp , sizeof(value)); // Restore old value of the dummy variable we've been using

  if (*errpos < 0)
   {
    point->number = 1.0;
    ppl_units_div( &commlink.first , point , out , errpos, errtext ); // Get units of output right
    out->number = result;
   }
  return;
 }

