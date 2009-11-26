// dcfmath.c
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

// A selection of useful mathematical functions which are not included in the standard C math library

#define _PPL_DCFMATH_C 1

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <math.h>

#include <gsl/gsl_cdf.h>
#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_sf_ellint.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_expint.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_legendre.h>
#include <gsl/gsl_sf_zeta.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

#include "dcfmath.h"
#include "airy_functions.h"
#include "zeta_riemann.h"

#include "dcfmath_macros.h"

double machine_epsilon;

double max(double x, double y)
 {
  if (x>y) return x;
  return y;
 }

double min(double x, double y)
 {
  if (x<y) return x;
  return y;
 }

int sgn(double x)
 {
  if (x==0) return  0;
  if (x< 0) return -1;
  return 1;
 }

void LinearRaster(double *out, double min, double max, int Nsteps)
 {
  int i;
  if (Nsteps < 2) Nsteps = 2; // Avoid division by zero
  for (i=0; i<Nsteps; i++) out[i] = min + (max - min) * ((double)i / (double)(Nsteps-1));
  return;
 }

void LogarithmicRaster(double *out, double min, double max, int Nsteps)
 {
  int i;
  if (min < 1e-200) min = 1e-200; // Avoid log of negative numbers or zero
  if (max < 1e-200) max = 1e-200;
  if (Nsteps < 2) Nsteps = 2; // Avoid division by zero
  for (i=0; i<Nsteps; i++) out[i] = min * pow(max / min , (double)i / (double)(Nsteps-1));
  return;
 }

double degrees(double rad)
 {
  return rad*180/M_PI;
 }

double radians(double degrees)
 {
  return degrees*M_PI/180;
 }

// Wrappers for mathematical functions to make them take values as inputs

void dcfmath_abs(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "abs(x)";
  CHECK_1NOTNAN;
  IF_1COMPLEX    output->real = hypot(in->real , in->imag);
  ELSE_REAL      output->real = fabs(in->real);
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in);
 }

void dcfmath_acos(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "acos(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arccos(z); }
  ELSE_REAL   { z=gsl_complex_arccos_real(in->real); }
  ENDIF
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_acosh(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "acosh(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arccosh(z); }
  ELSE_REAL   { z=gsl_complex_arccosh_real(in->real); }
  ENDIF;
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_acot(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "acot(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arccot(z);
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_acoth(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "acoth(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arccoth(z);
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_acsc(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "acsc(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arccsc(z); }
  ELSE_REAL   { z=gsl_complex_arccsc_real(in->real); }
  ENDIF;
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_acsch(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "acsch(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arccsch(z);
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_airy_ai(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "airy_ai(x)";
  gsl_complex zi,z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&zi,in->real,in->imag);
  airy_ai(zi,&z,status,errtext);
  if (*status) return;
  CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_airy_ai_diff(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "airy_ai_diff(x)";
  gsl_complex zi,z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&zi,in->real,in->imag);
  airy_ai_diff(zi,&z,status,errtext);
  if (*status) return;
  CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_airy_bi(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "airy_bi(x)";
  gsl_complex zi,z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&zi,in->real,in->imag);
  airy_bi(zi,&z,status,errtext);
  if (*status) return;
  CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_airy_bi_diff(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "airy_bi_diff(x)";
  gsl_complex zi,z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&zi,in->real,in->imag);
  airy_bi_diff(zi,&z,status,errtext);
  if (*status) return;
  CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_arg(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "arg(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  GSL_SET_COMPLEX(&z,in->real,in->imag);
  output->real = gsl_complex_arg(z);
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_asec(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "asec(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arcsec(z); }
  ELSE_REAL   { z=gsl_complex_arcsec_real(in->real); }
  ENDIF;
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_asech(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "asech(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arcsech(z);
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_asin(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "asin(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arcsin(z); }
  ELSE_REAL   { z=gsl_complex_arcsin_real(in->real); }
  ENDIF;
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_asinh(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "asinh(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arcsinh(z);
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_atan(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "atan(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arctan(z);
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_atanh(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "atanh(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_arctanh(z); }
  ELSE_REAL   { z=gsl_complex_arctanh_real(in->real); }
  ENDIF;
  CLEANUP_GSLCOMPLEX;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_atan2(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "atan2(x,y)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = atan2(in1->real, in2->real); }
  ENDIF;
  CLEANUP_APPLYUNIT(UNIT_ANGLE);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besseli(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besseli(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_il_scaled((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besselI(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besselI(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_In((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besselj(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besselj(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_jl((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besselJ(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besselJ(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_Jn((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besselk(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besselk(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_kl_scaled((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besselK(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besselK(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_Kn((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_bessely(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "bessely(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_yl((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_besselY(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "besselY(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function can only evaluate Bessel functions");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_bessel_Yn((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_beta(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "beta(a,b)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_beta(in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_binomialPDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "binomialPDF(k,p,n)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  CHECK_NEEDINT(in1 , "function's first parameter must be");
  CHECK_NEEDINT(in3 , "function's 3rd  parameter must be");
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_binomial_pdf((unsigned int)in1->real, in2->real, (unsigned int)in3->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_binomialCDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "binomialCDF(k,p,n)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  CHECK_NEEDINT(in1 , "function's first parameter must be");
  CHECK_NEEDINT(in3 , "function's 3rd  parameter must be");
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_binomial_P((unsigned int)in1->real, in2->real, (unsigned int)in3->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_planck_Bv(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "Bv(nu,T)";
  int i;
  value kelvin;

  CHECK_2NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in1, "first", "a frequency", UNIT_TIME, -1);
  CHECK_DIMLESS_OR_HAS_UNIT(in2, "second", "a temperature", UNIT_TEMPERATURE, 1);
  IF_2COMPLEX
   { QUERY_MUST_BE_REAL }
  ELSE_REAL
   {
    ppl_units_zero(&kelvin);
    kelvin.real = 1.0;
    kelvin.exponent[UNIT_TEMPERATURE] = 1;
    kelvin.TempType = 1;
    ppl_units_div(in2, &kelvin, &kelvin, status, errtext); // Convert in2 into kelvin
    if (*status) kelvin.real = GSL_NAN;
    output->dimensionless = 0;
    output->exponent[UNIT_MASS] =  1;
    output->exponent[UNIT_TIME] = -2;
    output->exponent[UNIT_ANGLE]= -2;
    output->real              =  2 * GSL_CONST_MKSA_PLANCKS_CONSTANT_H / pow(GSL_CONST_MKSA_SPEED_OF_LIGHT, 2) * pow(in1->real,3) / expm1(GSL_CONST_MKSA_PLANCKS_CONSTANT_H * in1->real / GSL_CONST_MKSA_BOLTZMANN / kelvin.real);
   }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_planck_Bvmax(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "Bvmax(T)";
  int i;
  value kelvin;

  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first" , "a temperature", UNIT_TEMPERATURE, 1);
  IF_1COMPLEX 
   { QUERY_MUST_BE_REAL }
  ELSE_REAL
   {
    ppl_units_zero(&kelvin);
    kelvin.real = 1.0;
    kelvin.exponent[UNIT_TEMPERATURE] = 1;
    kelvin.TempType = 1;
    ppl_units_div(in, &kelvin, &kelvin, status, errtext); // Convert in into kelvin
    if (*status) kelvin.real = GSL_NAN;
    output->dimensionless = 0;
    output->exponent[UNIT_TIME] = -1;
    output->real = 2.821439 * GSL_CONST_MKSA_BOLTZMANN / GSL_CONST_MKSA_PLANCKS_CONSTANT_H * kelvin.real; // Wien displacement law
   }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_ceil(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "ceil(x)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = ceil(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_chisqPDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "chisqPDF(x,nu)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_chisq_pdf(in1->real , in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_chisqCDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "chisqCDF(x,nu)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_chisq_P(in1->real , in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_chisqCDFi(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "chisqCDFi(P,nu)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_chisq_Pinv(in1->real , in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_conjugate(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "conjugate(z)";
  CHECK_1NOTNAN;
  memcpy(output, in, sizeof(value));
  output->imag *= -1;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_cos(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "cos(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_cos(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = cos(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_cosh(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "cosh(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_cosh(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = cosh(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_cot(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "cot(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_cot(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_coth(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "coth(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_coth(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_csc(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "csc(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_csc(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_csch(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "csch(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_csch(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_degrees(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "degrees(x)";
  int i;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = degrees(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_ellK(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "ellipticintK(k)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_ellint_Kcomp(in->real , GSL_PREC_DOUBLE); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_ellE(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "ellipticintE(k)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_ellint_Ecomp(in->real , GSL_PREC_DOUBLE); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_ellP(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "ellipticintP(k,n)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_ellint_Pcomp(in1->real , in2->real , GSL_PREC_DOUBLE); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_erf(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "erf(x)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_erf(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_erfc(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "erfc(x)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_erfc(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_exp(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "exp(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "dimensionless or an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_exp(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = exp(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_expm1(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "expm1(x)";
  int i;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "dimensionless or an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = expm1(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_expint(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "expint(n,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDSINT(in1, "function's first argument must be");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_expint_En((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_floor(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "floor(x)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_gamma(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "gamma(x)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_gamma(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_gaussianPDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "gaussianPDF(x,sigma)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_gaussian_pdf(in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimInverse(output, in1);
 }

void dcfmath_gaussianCDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "gaussianCDF(x,sigma)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_gaussian_P(in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_gaussianCDFi(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  value *in = in1;
  char *FunctionDescription = "gaussianCDFi(x,sigma)";
  CHECK_2NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_gaussian_Pinv(in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in2);
 }

void dcfmath_heaviside(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "heaviside(x)";
  CHECK_1NOTNAN;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { if (in->real >= 0) output->real = 1.0; }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_hypot(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  value tmp, tmp2;
  char *FunctionDescription = "hypot(x,y)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { ppl_units_mult(in1,in1,&tmp,status,errtext); if(*status)return; ppl_units_mult(in2,in2,&tmp2,status,errtext); if(*status)return; ppl_units_add(&tmp,&tmp2,&tmp2,status,errtext); if(*status)return; dcfmath_sqrt(&tmp2,output,status,errtext); if(*status)return; } // Yay for Reverse Polish!
  ELSE_REAL   { output->real = hypot(in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in1);
 }

void dcfmath_imag(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "Im(z)";
  if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status=1; sprintf(errtext, "The function %s can only be used when complex arithmetic is enabled; type 'set numerics complex' first.", FunctionDescription); return; }
    else { NULL_OUTPUT; }
   }
  CHECK_1NOTNAN;
  output->real = in->imag;
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in);
 }

void dcfmath_ldexp(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "ldexp(x,y)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDSINT(in2 , "function's second parameter must be");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = ldexp(in1->real, (int)in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_legendreP(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "legendreP(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1 , "function's first parameter must be");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_legendre_Pl((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_legendreQ(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "legendreQ(l,x)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in1 , "function's first parameter must be");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_sf_legendre_Ql((int)in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_log (value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "log(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_log(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_log10(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "log10(x)";
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_log10(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_logn(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "logn(x,n)";
  gsl_complex z;
  value base;
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  GSL_SET_COMPLEX(&z,in2->real,in2->imag); z=gsl_complex_log(z); CLEANUP_GSLCOMPLEX; base=*output;
  GSL_SET_COMPLEX(&z,in1->real,in1->imag); z=gsl_complex_log(z); CLEANUP_GSLCOMPLEX;
  ppl_units_div(output,&base,output,status,errtext);
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_lognormalPDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  value *in = in3;
  char *FunctionDescription = "lognormalPDF(x,zeta,sigma)";
  CHECK_3NOTNAN;
  CHECK_1INPUT_DIMLESS;
  CHECK_2INPUT_DIMMATCH;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_lognormal_pdf(in1->real, in2->real, in3->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimInverse(output, in1);
 }

void dcfmath_lognormalCDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  value *in = in3;
  char *FunctionDescription = "lognormalCDF(x,zeta,sigma)";
  CHECK_3NOTNAN;
  CHECK_1INPUT_DIMLESS;
  CHECK_2INPUT_DIMMATCH;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_lognormal_P(in1->real, in2->real, in3->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_lognormalCDFi(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  value *in = in3;
  char *FunctionDescription = "lognormalCDFi(x,zeta,sigma)";
  CHECK_3NOTNAN;
  CHECK_1INPUT_DIMLESS;
  CHECK_2INPUT_DIMMATCH;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_lognormal_Pinv(in1->real, in2->real, in3->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in2);
 }

void dcfmath_max (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "max(x,y)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { if (hypot(in1->real,in1->imag) < hypot(in2->real,in2->imag)) { memcpy(output,in2,sizeof(value)); } else { memcpy(output,in1,sizeof(value)); } }
  ELSE_REAL   { output->real = max(in1->real, in2->real); ppl_units_DimCpy(output, in1); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_min (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "min(x,y)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { if (hypot(in1->real,in1->imag) > hypot(in2->real,in2->imag)) { memcpy(output,in2,sizeof(value)); } else { memcpy(output,in1,sizeof(value)); } }
  ELSE_REAL   { output->real = min(in1->real, in2->real); ppl_units_DimCpy(output, in1); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_mod(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "mod(x,y)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL
   {
    if (in1->real*machine_epsilon*10 > in2->real)
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status = 1; sprintf(errtext, "Loss of accuracy in the function %s; the remainder of this division is lost in floating-point rounding.", FunctionDescription); return; }
      else { NULL_OUTPUT; }
     }
    output->real = fmod(in1->real , in2->real);
   }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in1);
 }

void dcfmath_poissonPDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "poissonPDF(x,mu)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_poisson_pdf(in1->real , in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_poissonCDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "poissonCDF(x,mu)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_cdf_poisson_P(in1->real , in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_pow (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "pow(x,y)";
  CHECK_2NOTNAN;
  ppl_units_pow(in1, in2, output, status, errtext);
 }

void dcfmath_radians(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "radians(x)";
  int i;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { if (in->dimensionless) { output->real = radians(in->real); } else { output->real = in->real; } }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

static gsl_rng *rndgen = NULL; // Random number generator for next five functions

void dcfmath_SetRandomSeed(long i)
 {
  if (rndgen==NULL) rndgen = gsl_rng_alloc(gsl_rng_default);
  gsl_rng_set(rndgen, i);
  return;
 }

void dcfmath_frandom(value *output, int *status, char *errtext)
 {
  WRAPPER_INIT;
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, 0); }
  output->real = gsl_rng_uniform(rndgen);
 }

void dcfmath_frandombin(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "random_binomial(p,n)";
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, 0); }
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in2, "function's second argument must be an integer in the range");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_binomial(rndgen, in1->real, (unsigned int)in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_frandomcs(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "random_chisq(mu)";
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, 0); }
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_chisq(rndgen, in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_frandomg(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "random_gaussian(sigma)";
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, 0); }
  CHECK_1NOTNAN;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_gaussian(rndgen, in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in);
 }

void dcfmath_frandomln(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  value *in = in2; // Only check that in2 is dimensionless
  char *FunctionDescription = "random_lognormal(zeta,sigma)";
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, 0); }
  CHECK_2NOTNAN;
  CHECK_1INPUT_DIMLESS; // THIS IS CORRECT. Only check in2
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_lognormal(rndgen, in1->real, in2->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in1);
 }

void dcfmath_frandomp(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "random_poisson(n)";
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, 0); }
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = gsl_ran_poisson(rndgen, in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_real(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "Re(z)";
  CHECK_1NOTNAN;
  output->real = in->real;
  CHECK_OUTPUT_OKAY;
  ppl_units_DimCpy(output, in);
 }

void dcfmath_root(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  value *in = in2; // Only check that in2 is dimensionless
  value x1,x2;
  double tmpdbl;
  char *FunctionDescription = "root(z,n)";
  unsigned char negated = 0;
  CHECK_2NOTNAN;
  CHECK_1INPUT_DIMLESS; // THIS IS CORRECT. Only check in2
  if ((in2->FlagComplex) || (in2->real < 2) || (in2->real >= INT_MAX))
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_ON) { *status = 1; sprintf(errtext, "The %s %s in the range 2 <= n < %d.",FunctionDescription,"function's second argument must be an integer in the range",INT_MAX); return; }
    else { NULL_OUTPUT; }
   }
  ppl_units_zero(&x2);
  x1=*in1;
  if (x1.real < 0.0) { negated=1; x1.real=-x1.real; if (x1.imag!=0.0) x1.imag=-x1.imag; }
  x2.real = 1.0 / floor(in2->real);
  ppl_units_pow(&x1, &x2, output, status, errtext);
  if (*status) return;
  if (negated)
   {
    if (fmod(floor(in2->real) , 2) == 1)
     {
      output->real=-output->real; if (output->imag!=0.0) output->imag=-output->imag;
     } else {
      if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { QUERY_OUT_OF_RANGE; }
      else
       {
        tmpdbl = output->imag;
        output->imag = output->real;
        output->real = -tmpdbl;
        output->FlagComplex = !ppl_units_DblEqual(output->imag, 0);
        if (!output->FlagComplex) output->imag=0.0; // Enforce that real numbers have positive zero imaginary components
       } 
     }
   }
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_sec(value *in, value *output, int *status, char *errtext)
 {  
  char *FunctionDescription = "sec(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_sec(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_sech(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "sech(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_sech(z); CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_sin(value *in, value *output, int *status, char *errtext)
 {  
  char *FunctionDescription = "sin(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_sin(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = sin(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_sinh(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "sinh(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_sinh(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = sinh(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_sqrt(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "sqrt(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_sqrt(z); }
  ELSE_REAL   { z=gsl_complex_sqrt_real(in->real); }
  ENDIF
  CLEANUP_GSLCOMPLEX;
  CHECK_OUTPUT_OKAY;
  output->dimensionless = in->dimensionless;
  output->TempType      = in->TempType;
  for (i=0; i<UNITS_MAX_BASEUNITS; i++) output->exponent[i] = in->exponent[i] / 2;
 }

void dcfmath_tan(value *in, value *output, int *status, char *errtext)
 {  
  char *FunctionDescription = "tan(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_tan(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = tan(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_tanh(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "tanh(x)";
  int i;
  gsl_complex z;
  CHECK_1NOTNAN;
  CHECK_DIMLESS_OR_HAS_UNIT(in , "first", "an angle", UNIT_ANGLE, 1);
  IF_1COMPLEX { GSL_SET_COMPLEX(&z,in->real,in->imag); z=gsl_complex_tanh(z); CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = tanh(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcfmath_tophat(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "tophat(x,sigma)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMMATCH;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { if ( fabs(in1->real) <= fabs(in2->real) ) output->real = 1.0; }
  ENDIF
 }

void dcfmath_zeta(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "zeta(x)";
  gsl_complex zi,z;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { GSL_SET_COMPLEX(&zi,in->real,in->imag); riemann_zeta_complex(zi,&z,status,errtext); if (*status) return; CLEANUP_GSLCOMPLEX; }
  ELSE_REAL   { output->real = gsl_sf_zeta(in->real); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

