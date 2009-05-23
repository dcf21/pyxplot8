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

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_units.h"

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

// Wrappers for mathematical functions to make them take values as inputs

void dcfmath_acos(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_asin(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_atan(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_atan2(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_ceil(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_cos (value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_cosh(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_degrees(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_gsl_sf_erf(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_exp (value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_fabs(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_floor(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_fmod(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_frexp(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_gsl_sf_gamma(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_hypot(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_ldexp(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_log (value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_log10(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_max (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_min (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_pow (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_radians(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_frandom(value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_sin (value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_sinh(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_sqrt(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_tan (value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_tanh(value *in, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

double degrees(double rad)
 {
  return rad*180/M_PI;
 }

double radians(double degrees)
 {
  return degrees*M_PI/180;
 }

double frandom()
 {
  return (double)rand() / RAND_MAX;
 }
