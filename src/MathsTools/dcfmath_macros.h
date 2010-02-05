// dcfmath_macros.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
//
// $Id: dcfmath.c 347 2009-09-09 14:59:19Z dcf21 $
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

// Wrappers for mathematical functions to make them take values as inputs

#define NULL_OUTPUT \
 { output->real = GSL_NAN; output->imag = 0; output->FlagComplex=0; return; }

#define QUERY_OUT_OF_RANGE \
 { \
  if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status=1; sprintf(errtext, "The function %s is not defined at the requested point in parameter space.", FunctionDescription); return; } \
  else { NULL_OUTPUT; } \
 }

#define QUERY_MUST_BE_REAL \
 { \
  if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status=1; sprintf(errtext, "The function %s only accepts real arguments; the supplied arguments are complex.", FunctionDescription); return; } \
  else { NULL_OUTPUT; } \
 }

#define CHECK_NEEDINT(X, DESCRIPTION) \
 { \
  if (((X)->FlagComplex) || ((X)->real < 0) || ((X)->real >= INT_MAX)) \
   { \
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status = 1; sprintf(errtext, "The %s %s in the range 0 <= k < %d.",FunctionDescription,DESCRIPTION,INT_MAX); return; } \
    else { NULL_OUTPUT; } \
   } \
 }

#define CHECK_NEEDSINT(X, DESCRIPTION) \
 { \
  if (((X)->real <= INT_MIN) || ((X)->real >= INT_MAX)) \
   { \
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status = 1; sprintf(errtext, "The %s %s in the range %d <= k < %d.",FunctionDescription,DESCRIPTION,INT_MIN,INT_MAX); return; } \
    else { NULL_OUTPUT; } \
   } \
 }

#define WRAPPER_INIT \
 { \
  *status = 0; \
  ppl_units_zero(output); \
 }

#define NAN_CHECK_FAIL \
 { \
  if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status = 1; sprintf(errtext, "The function %s has received a non-finite input.",FunctionDescription); return; } \
  else { NULL_OUTPUT; } \
 }

#define CHECK_1NOTNAN \
 { \
  WRAPPER_INIT; \
  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && (in->FlagComplex)) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in->real)) || (!gsl_finite(in->imag))) { NAN_CHECK_FAIL; } \
 }

#define CHECK_2NOTNAN \
 { \
  WRAPPER_INIT; \
  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && ((in1->FlagComplex) || (in2->FlagComplex))) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in1->real)) || (!gsl_finite(in1->imag))) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in2->real)) || (!gsl_finite(in2->imag))) { NAN_CHECK_FAIL; } \
 }

#define CHECK_3NOTNAN \
 { \
  CHECK_2NOTNAN; \
  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && (in3->FlagComplex)) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in3->real)) || (!gsl_finite(in3->imag))) { NAN_CHECK_FAIL; } \
 }

#define CHECK_4NOTNAN \
 { \
  CHECK_3NOTNAN; \
  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && (in4->FlagComplex)) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in4->real)) || (!gsl_finite(in4->imag))) { NAN_CHECK_FAIL; } \
 }

#define CHECK_6NOTNAN \
 { \
  CHECK_4NOTNAN; \
  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && ((in5->FlagComplex) || (in6->FlagComplex))) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in5->real)) || (!gsl_finite(in5->imag))) { NAN_CHECK_FAIL; } \
  if ((!gsl_finite(in6->real)) || (!gsl_finite(in6->imag))) { NAN_CHECK_FAIL; } \
 }

#define CHECK_1INPUT_DIMLESS \
 { \
  if (!(in->dimensionless)) \
   { \
    *status = 1; \
    sprintf(errtext, "The %s function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", FunctionDescription, ppl_units_GetUnitStr(in, NULL, NULL, 1, 0)); \
    return; \
   } \
 }

#define CHECK_2INPUT_DIMLESS \
 { \
  if (!(in1->dimensionless && in2->dimensionless)) \
   { \
    *status = 1; \
    sprintf(errtext, "The %s function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", FunctionDescription, ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0)); \
    return; \
   } \
 }

#define CHECK_3INPUT_DIMLESS \
 { \
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless)) \
   { \
    *status = 1; \
    sprintf(errtext, "The %s function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s>, <%s> and <%s>.", FunctionDescription, ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0), ppl_units_GetUnitStr(in3, NULL, NULL, 2, 0)); \
    return; \
   } \
 }

#define CHECK_4INPUT_DIMLESS \
 { \
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless && in4->dimensionless)) \
   { \
    *status = 1; \
    sprintf(errtext, "The %s function can only act upon dimensionless inputs.", FunctionDescription); \
    return; \
   } \
 }


#define CHECK_6INPUT_DIMLESS \
 { \
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless && in4->dimensionless && in5->dimensionless && in6->dimensionless)) \
   { \
    *status = 1; \
    sprintf(errtext, "The %s function can only act upon dimensionless inputs.", FunctionDescription); \
    return; \
   } \
 }

#define CHECK_2INPUT_DIMMATCH \
 { \
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2)))) \
   { \
    *status = 1; \
    sprintf(errtext, "The %s function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", FunctionDescription, ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0)); \
    return; \
   } \
 }

#define CHECK_DIMLESS_OR_HAS_UNIT(X, DESCRIPTION, UNITNAME, UNIT, UNITN) \
 { \
  if (!((X)->dimensionless)) \
   for (i=0; i<UNITS_MAX_BASEUNITS; i++) \
    if ((X)->exponent[i] != UNITN*(i==UNIT)) \
     { \
      *status = 1; \
      sprintf(errtext, "The %s argument to the %s function must be %s. Supplied input has dimensions of <%s>.", DESCRIPTION, FunctionDescription, UNITNAME, ppl_units_GetUnitStr((X), NULL, NULL, 1, 0)); \
      return; \
     } \
 } \

#define IF_1COMPLEX if (in->FlagComplex) {
#define IF_2COMPLEX if ((in1->FlagComplex) || (in2->FlagComplex)) {
#define IF_3COMPLEX if ((in1->FlagComplex) || (in2->FlagComplex) || (in3->FlagComplex)) {
#define IF_4COMPLEX if ((in1->FlagComplex) || (in2->FlagComplex) || (in3->FlagComplex) || (in4->FlagComplex)) {
#define IF_6COMPLEX if ((in1->FlagComplex) || (in2->FlagComplex) || (in3->FlagComplex) || (in4->FlagComplex) || (in5->FlagComplex) || (in6->FlagComplex)) {
#define ELSE_REAL   } else {
#define ENDIF       }

#define CLEANUP_GSLCOMPLEX \
  output->real = GSL_REAL(z); \
  output->imag = GSL_IMAG(z); \
  output->FlagComplex = !ppl_units_DblEqual(output->imag,0); \
  if (!output->FlagComplex) output->imag=0.0;

#define CLEANUP_APPLYUNIT(UNIT) \
  output->dimensionless = 0; \
  output->exponent[UNIT] = 1; \


#define CHECK_OUTPUT_OKAY \
 if ((!gsl_finite(output->real)) || (!gsl_finite(output->imag)) || ((output->FlagComplex) && (settings_term_current.ComplexNumbers == SW_ONOFF_OFF))) \
  { QUERY_OUT_OF_RANGE; }

