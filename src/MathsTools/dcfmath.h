// dcfmath.h
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

#ifndef _DCFMATH_H
#define _DCFMATH_H 1

#include "ppl_units.h"

double max    (double x, double y);
double min    (double x, double y);
int    sgn    (double x);
double degrees(double rad);
double radians(double degrees);
double frandom();

void dcfmath_acos(value *in, value *output, int *status, char *errtext);
void dcfmath_asin(value *in, value *output, int *status, char *errtext);
void dcfmath_atan(value *in, value *output, int *status, char *errtext);
void dcfmath_atan2(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besseli(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselI(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselj(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselJ(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselk(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselK(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_bessely(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselY(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_ceil(value *in, value *output, int *status, char *errtext);
void dcfmath_cos (value *in, value *output, int *status, char *errtext);
void dcfmath_cosh(value *in, value *output, int *status, char *errtext);
void dcfmath_degrees(value *in, value *output, int *status, char *errtext);
void dcfmath_erf(value *in, value *output, int *status, char *errtext);
void dcfmath_exp (value *in, value *output, int *status, char *errtext);
void dcfmath_fabs(value *in, value *output, int *status, char *errtext);
void dcfmath_floor(value *in, value *output, int *status, char *errtext);
void dcfmath_fmod(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_frexp(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_gamma(value *in, value *output, int *status, char *errtext);
void dcfmath_hypot(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_ldexp(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_legendreP(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_legendreQ(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_log (value *in, value *output, int *status, char *errtext);
void dcfmath_log10(value *in, value *output, int *status, char *errtext);
void dcfmath_max (value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_min (value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_pow (value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_radians(value *in, value *output, int *status, char *errtext);
void dcfmath_frandom(value *output, int *status, char *errtext);
void dcfmath_frandomg(value *output, int *status, char *errtext);
void dcfmath_sin (value *in, value *output, int *status, char *errtext);
void dcfmath_sinh(value *in, value *output, int *status, char *errtext);
void dcfmath_sqrt(value *in, value *output, int *status, char *errtext);
void dcfmath_tan (value *in, value *output, int *status, char *errtext);
void dcfmath_tanh(value *in, value *output, int *status, char *errtext);

#endif

