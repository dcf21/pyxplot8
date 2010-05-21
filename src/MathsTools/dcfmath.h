// dcfmath.h
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

#ifndef _DCFMATH_H
#define _DCFMATH_H 1

#include "ppl_units.h"

#ifndef _DCFMATH_C
extern double machine_epsilon;
#endif

double max    (double x, double y);
double min    (double x, double y);
int    sgn    (double x);
void   LinearRaster     (double *out, double min, double max, int Nsteps);
void   LogarithmicRaster(double *out, double min, double max, int Nsteps);
double degrees(double rad);
double radians(double degrees);

void dcfmath_SetRandomSeed(long i);

void dcfmath_abs(value *in, value *output, int *status, char *errtext);
void dcfmath_acos(value *in, value *output, int *status, char *errtext);
void dcfmath_acosh(value *in, value *output, int *status, char *errtext);
void dcfmath_acot(value *in, value *output, int *status, char *errtext);
void dcfmath_acoth(value *in, value *output, int *status, char *errtext);
void dcfmath_acsc(value *in, value *output, int *status, char *errtext);
void dcfmath_acsch(value *in, value *output, int *status, char *errtext);
void dcfmath_airy_ai(value *in, value *output, int *status, char *errtext);
void dcfmath_airy_ai_diff(value *in, value *output, int *status, char *errtext);
void dcfmath_airy_bi(value *in, value *output, int *status, char *errtext);
void dcfmath_airy_bi_diff(value *in, value *output, int *status, char *errtext);
void dcfmath_arg(value *in, value *output, int *status, char *errtext);
void dcfmath_asec(value *in, value *output, int *status, char *errtext);
void dcfmath_asech(value *in, value *output, int *status, char *errtext);
void dcfmath_asin(value *in, value *output, int *status, char *errtext);
void dcfmath_asinh(value *in, value *output, int *status, char *errtext);
void dcfmath_atan(value *in, value *output, int *status, char *errtext);
void dcfmath_atanh(value *in, value *output, int *status, char *errtext);
void dcfmath_atan2(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besseli(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselI(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselj(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselJ(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselk(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselK(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_bessely(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_besselY(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_beta(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_binomialPDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_binomialCDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_planck_Bv(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_planck_Bvmax(value *in, value *output, int *status, char *errtext);
void dcfmath_ceil(value *in, value *output, int *status, char *errtext);
void dcfmath_chisqPDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_chisqCDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_chisqCDFi(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_conjugate(value *in, value *output, int *status, char *errtext);
void dcfmath_cos(value *in, value *output, int *status, char *errtext);
void dcfmath_cosh(value *in, value *output, int *status, char *errtext);
void dcfmath_cot(value *in, value *output, int *status, char *errtext);
void dcfmath_coth(value *in, value *output, int *status, char *errtext);
void dcfmath_csc(value *in, value *output, int *status, char *errtext);
void dcfmath_csch(value *in, value *output, int *status, char *errtext);
void dcfmath_degrees(value *in, value *output, int *status, char *errtext);
void dcfmath_ellK(value *in, value *output, int *status, char *errtext);
void dcfmath_ellE(value *in, value *output, int *status, char *errtext);
void dcfmath_ellP(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_erf(value *in, value *output, int *status, char *errtext);
void dcfmath_erfc(value *in, value *output, int *status, char *errtext);
void dcfmath_exp(value *in, value *output, int *status, char *errtext);
void dcfmath_expm1(value *in, value *output, int *status, char *errtext);
void dcfmath_expint(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_floor(value *in, value *output, int *status, char *errtext);
void dcfmath_gamma(value *in, value *output, int *status, char *errtext);
void dcfmath_gaussianPDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_gaussianCDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_gaussianCDFi(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_heaviside(value *in, value *output, int *status, char *errtext);
void dcfmath_hyperg_0F1(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_hyperg_1F1(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_hyperg_2F0(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_hyperg_2F1(value *in1, value *in2, value *in3, value *in4, value *output, int *status, char *errtext);
void dcfmath_hyperg_U(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_hypot(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_imag(value *in, value *output, int *status, char *errtext);
void dcfmath_jacobi_cn(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_jacobi_dn(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_jacobi_sn(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_lambert_W0(value *in, value *output, int *status, char *errtext);
void dcfmath_lambert_W1(value *in, value *output, int *status, char *errtext);
void dcfmath_ldexp(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_legendreP(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_legendreQ(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_log(value *in, value *output, int *status, char *errtext);
void dcfmath_log10(value *in, value *output, int *status, char *errtext);
void dcfmath_logn(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_lognormalPDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_lognormalCDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_lognormalCDFi(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_max(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_min(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_mod(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_poissonPDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_poissonCDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_pow(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_radians(value *in, value *output, int *status, char *errtext);
void dcfmath_frandom(value *output, int *status, char *errtext);
void dcfmath_frandombin(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_frandomcs(value *in, value *output, int *status, char *errtext);
void dcfmath_frandomg(value *in, value *output, int *status, char *errtext);
void dcfmath_frandomln(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_frandomp(value *in, value *output, int *status, char *errtext);
void dcfmath_frandomt(value *in, value *output, int *status, char *errtext);
void dcfmath_real(value *in, value *output, int *status, char *errtext);
void dcfmath_root(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_sec(value *in, value *output, int *status, char *errtext);
void dcfmath_sech(value *in, value *output, int *status, char *errtext);
void dcfmath_sin(value *in, value *output, int *status, char *errtext);
void dcfmath_sinc(value *in, value *output, int *status, char *errtext);
void dcfmath_sinh(value *in, value *output, int *status, char *errtext);
void dcfmath_sqrt(value *in, value *output, int *status, char *errtext);
void dcfmath_tan(value *in, value *output, int *status, char *errtext);
void dcfmath_tanh(value *in, value *output, int *status, char *errtext);
void dcfmath_tdistPDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_tdistCDF(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_tdistCDFi(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_tophat(value *in1, value *in2, value *output, int *status, char *errtext);
void dcfmath_zernike(value *in1, value *in2, value *in3, value *in4, value *output, int *status, char *errtext);
void dcfmath_zernikeR(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcfmath_zeta(value *in, value *output, int *status, char *errtext);

#endif

