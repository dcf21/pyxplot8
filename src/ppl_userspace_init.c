// ppl_userspace_init.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <gsl/gsl_const_mksa.h>
#include <gsl/gsl_const_num.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf.h>

#include "MathsTools/dcfast.h"
#include "MathsTools/dcfmath.h"
#include "MathsTools/dcftime.h"

#include "ListTools/lt_memory.h"

#include "ppl_error.h"
#include "ppl_texify.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

void ppl_UserSpaceInit()
 {
  value v;

  _ppl_UserSpace_Vars  = DictInit();
  _ppl_UserSpace_Funcs = DictInit();

  // Function descriptors for the mathematical functions which are built into PyXPlot
  FunctionDescriptor fd_abs           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_abs         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{abs}@<@1@>", "abs(z) returns the absolute magnitude of z"};
  FunctionDescriptor fd_acos          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acos        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{acos}@<@1@>", "acos(x) returns the arccosine of x"};
  FunctionDescriptor fd_acosh         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acosh       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{acosh}@<@1@>", "acosh(x) returns the hyperbolic arccosine of x"};
  FunctionDescriptor fd_acot          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acot        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{acot}@<@1@>", "acot(x) returns the arccotangent of x"};
  FunctionDescriptor fd_acoth         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acoth       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{acoth}@<@1@>", "acoth(x) returns the hyperbolic arccotangent of x"};
  FunctionDescriptor fd_acsc          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acsc        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{acsc}@<@1@>", "acsc(x) returns the arccosecant of x"};
  FunctionDescriptor fd_acsch         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acsch       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{acsch}@<@1@>", "acsch(x) returns the hyperbolic arccosecant of x"};
  FunctionDescriptor fd_airy_ai       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_airy_ai     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{airy\\_ai}@<@1@>", "airy_ai(z) returns the Airy function Ai evaluated at z"};
  FunctionDescriptor fd_airy_ai_diff  = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_airy_ai_diff, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{airy\\_ai\\_diff}@<@1@>", "airy_ai_diff(z) returns the first derivative of the Airy function Ai evaluated at z"};
  FunctionDescriptor fd_airy_bi       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_airy_bi     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{airy\\_bi}@<@1@>", "airy_bi(z) returns the Airy function Bi evaluated at z"};
  FunctionDescriptor fd_airy_bi_diff  = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_airy_bi_diff, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{airy\\_bi\\_diff}@<@1@>", "airy_bi_diff(z) returns the first derivative of the Airy function Bi evaluated at z"};
  FunctionDescriptor fd_arg           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_arg         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{arg}@<@1@>", "arg(z) returns the argument of the complex number z"};
  FunctionDescriptor fd_asec          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_asec        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{asec}@<@1@>", "asec(x) returns the arcsecant of x"};
  FunctionDescriptor fd_asech         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_asech       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{asech}@<@1@>", "asech(x) returns the hyperbolic arcsecant of x"};
  FunctionDescriptor fd_asin          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_asin        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{asin}@<@1@>", "asin(x) returns the arcsine of x"};
  FunctionDescriptor fd_asinh         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_asinh       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{asinh}@<@1@>", "asinh(x) returns the hyperbolic arcsine of x"};
  FunctionDescriptor fd_ast_Lcdm_age      = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfast_Lcdm_age     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_age@<@1,@2,@3@>", "ast_Lcdm_age(H0,w_m,w_l) returns the current age of the Universe in an L_CDM cosmology"};
  FunctionDescriptor fd_ast_Lcdm_angscale = { PPL_USERSPACE_SYSTEM , 0 , 4 , (void *)&dcfast_Lcdm_angscale, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_angscale@<@1,@2,@3,@4@>", "ast_Lcdm_angscale(z,H0,w_m,w_l) returns the angular scale of the sky in distance per unit angle for an L_CDM cosmology"};
  FunctionDescriptor fd_ast_Lcdm_DA       = { PPL_USERSPACE_SYSTEM , 0 , 4 , (void *)&dcfast_Lcdm_DA      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_D_A@<@1,@2,@3,@4@>", "ast_Lcdm_DA(z,H0,w_m,w_l) returns the angular size distance corresponding to redshift z in an L_CDM cosmology"};
  FunctionDescriptor fd_ast_Lcdm_DL       = { PPL_USERSPACE_SYSTEM , 0 , 4 , (void *)&dcfast_Lcdm_DL      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_D_L@<@1,@2,@3,@4@>", "ast_Lcdm_DL(z,H0,w_m,w_l) returns the luminosity distance corresponding to redshift z in an L_CDM cosmology"};
  FunctionDescriptor fd_ast_Lcdm_DM       = { PPL_USERSPACE_SYSTEM , 0 , 4 , (void *)&dcfast_Lcdm_DM      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_D_M@<@1,@2,@3,@4@>", "ast_Lcdm_DM(z,H0,w_m,w_l) returns the comoving distance corresponding to redshift z in an L_CDM cosmology"};
  FunctionDescriptor fd_ast_Lcdm_t        = { PPL_USERSPACE_SYSTEM , 0 , 4 , (void *)&dcfast_Lcdm_t       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_t@<@1,@2,@3,@4@>", "ast_Lcdm_t(z,H0,w_m,w_l) returns the lookback time corresponding to redshift z in an L_CDM cosmology"};
  FunctionDescriptor fd_ast_Lcdm_z        = { PPL_USERSPACE_SYSTEM , 0 , 4 , (void *)&dcfast_Lcdm_z       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_\\Lambda_{CDM}\\_z@<@1,@2,@3,@4@>", "ast_Lcdm_z(t,H0,w_m,w_l) returns the redshift corresponding to a lookback time t in an L_CDM cosmology"};
  FunctionDescriptor fd_ast_moonphase     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfast_moonphase    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_moonphase}@<@1@>", "ast_moonphase(JD) returns the phase on the Moon at Julian Date JD"};
  FunctionDescriptor fd_ast_sidereal_time = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfast_sidereal_time, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ast\\_sidereal\\_time}@<@1@>", "ast_sidereal_time(JD) returns the sidereal time at Greenwich at Julian Date JD"};
  FunctionDescriptor fd_atan          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_atan        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{atan}@<@1@>", "atan(x) returns the arctangent of x"};
  FunctionDescriptor fd_atanh         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_atanh       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{atanh}@<@1@>", "atanh(x) returns the hyperbolic arctangent of x"};
  FunctionDescriptor fd_atan2         = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_atan2       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{atan2}@<@1,@2@>", "atan2(x,y) returns the arctangent of x/y. Unlike atan(y/x), atan2(x,y) takes account of the signs of both x and y to remove the degeneracy between (1,1) and (-1,-1)"};
  FunctionDescriptor fd_besseli       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besseli     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besseli}@<@1,@2@>", "besseli(l,x) evaluates the lth regular modified spherical Bessel function at x"};
  FunctionDescriptor fd_besselI       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselI     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besselI}@<@1,@2@>", "besselI(l,x) evaluates the lth regular modified cylindrical Bessel function at x"};
  FunctionDescriptor fd_besselj       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselj     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besselj}@<@1,@2@>", "besselj(l,x) evaluates the lth regular spherical Bessel function at x"};
  FunctionDescriptor fd_besselJ       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselJ     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besselJ}@<@1,@2@>", "besselJ(l,x) evaluates the lth regular cylindrical Bessel function at x"};
  FunctionDescriptor fd_besselk       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselk     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besselk}@<@1,@2@>", "besselk(l,x) evaluates the lth irregular modified spherical Bessel function at x"};
  FunctionDescriptor fd_besselK       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselK     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besselK}@<@1,@2@>", "besselK(l,x) evaluates the lth irregular modified cylindrical Bessel function at x"};
  FunctionDescriptor fd_bessely       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_bessely     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{bessely}@<@1,@2@>", "bessely(l,x) evaluates the lth irregular spherical Bessel function at x"};
  FunctionDescriptor fd_besselY       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselY     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{besselY}@<@1,@2@>", "besselY(l,x) evaluates the lth irregular cylindrical Bessel function at x"};
  FunctionDescriptor fd_beta          = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_beta        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{B}@<@1,@2@>", "beta(a,b) evaluates the beta function B(a,b)"};
  FunctionDescriptor fd_binomialPDF   = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_binomialPDF , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{binomialPDF}@<@1,@2,@3@>", "binomialPDF(k,p,n) evaulates the probability of getting k successes out of n trials in a binomial distribution with success probability p"};
  FunctionDescriptor fd_binomialCDF   = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_binomialCDF , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{binomialCDF}@<@1,@2,@3@>", "binomialCDF(k,p,n) evaulates the probability of getting fewer than or exactly k successes out of n trials in a binomial distribution with success probability p"};
  FunctionDescriptor fd_Bv            = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_planck_Bv   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{B_\\nu}@<@1,@2@>", "Bv(nu,T) returns the power emitted by a blackbody of temperature T per unit area, per unit solid angle, per unit frequency"};
  FunctionDescriptor fd_Bvmax         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_planck_Bvmax, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{B_{\\nu,max}}@<@1@>", "Bvmax(T) returns the frequency of the maximum of the function Bv(nu,T)"};
  FunctionDescriptor fd_ceil          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ceil        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ceil}@<@1@>", "ceil(x) returns the smallest integer value greater than or equal to x"};
  FunctionDescriptor fd_chisqPDF      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_chisqPDF    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{\\chi^2 PDF}@<@1,@2@>", "chisqPDF(x,nu) returns the probability desity at x in a chi-squared distribution with nu degrees of freedom"};
  FunctionDescriptor fd_chisqCDF      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_chisqCDF    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{\\chi^2 CDF}@<@1,@2@>", "chisqCDF(x,nu) returns the cumulative probability desity at x in a chi-squared distribution with nu degrees of freedom"};
  FunctionDescriptor fd_chisqCDFi     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_chisqCDFi   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{\\chi^2 CDFi}@<@1,@2@>", "chisqCDFi(P,nu) returns the point x at which the cumulative probability desity in a chi-squared distribution with nu degrees of freedom is P"};
  FunctionDescriptor fd_conjugate     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_conjugate   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{conjugate}@<@1@>", "conjugate(z) returns the complex conjugate of z"};
  FunctionDescriptor fd_cos           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cos         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{cos}@<@1@>", "cos(x) returns the cosine of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_cosh          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cosh        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{cosh}@<@1@>", "cosh(x) returns the hyperbolic cosine of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_cot           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cot         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{cot}@<@1@>", "cot(x) returns the cotangent of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_coth          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_coth        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{coth}@<@1@>", "coth(x) returns the hyperbolic cotangent of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_csc           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_csc         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{csc}@<@1@>", "csc(x) returns the cosecant of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_csch          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_csch        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{csch}@<@1@>", "csch(x) returns the hyperbolic cosecant of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_degrees       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_degrees     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{degrees}@<@1@>", "degrees(x) converts angles measured in radians into degrees"};
  FunctionDescriptor fd_ellK          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ellK        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ellipticintK}@<@1@>", "ellipticintK(k) evaluates the complete elliptic integral K(k)"};
  FunctionDescriptor fd_ellE          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ellE        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ellipticintE}@<@1@>", "ellipticintE(k) evaluates the complete elliptic integral E(k)"};
  FunctionDescriptor fd_ellP          = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_ellP        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ellipticintP}@<@1,@2@>", "ellipticintP(k,n) evaluates the complete elliptic integral P(k,n)"};
  FunctionDescriptor fd_erf           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_erf         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{erf}@<@1@>", "erf(x) evaluates the error function at x"};
  FunctionDescriptor fd_erfc          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_erfc        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{erfc}@<@1@>", "erfc(x) evaluates the complimentary error function at x"};
  FunctionDescriptor fd_exp           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_exp         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{exp}@<@1@>", "exp(x) returns e to the power of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_expm1         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_expm1       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{expm1}@<@1@>", "expm1(x) accurately evaluates exp(x)-1"};
  FunctionDescriptor fd_expint        = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_expint      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{expint}@<@1,@2@>", "expint(n,x) evaluates the integral of exp(-xt)/t**n between one and infinity"};
  FunctionDescriptor fd_floor         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_floor       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{floor}@<@1@>", "floor(x) returns the largest integer value smaller than or equal to x"};
  FunctionDescriptor fd_gamma         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_gamma       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{\\Gamma}@<@1@>", "gamma(x) evaluates the gamma function at x"};
  FunctionDescriptor fd_gaussianPDF   = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_gaussianPDF , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{gaussianPDF}@<@1,@2@>", "gaussianPDF(x,sigma) evaluates the Gaussian probability density function of standard deviation sigma at x"};
  FunctionDescriptor fd_gaussianCDF   = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_gaussianCDF , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{gaussianCDF}@<@1,@2@>", "gaussianCDF(x,sigma) evaluates the Gaussian cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_gaussianCDFi  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_gaussianCDFi, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{gaussianCDFi}@<@1,@2@>", "gaussianCDFi(x,sigma) evaluates the inverse Gaussian cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_heaviside     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_heaviside   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{heaviside}@<@1@>", "heaviside(x) returns the Heaviside function, defined to be one for x>=0 and zero otherwise"};
  FunctionDescriptor fd_hypot         = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_hypot       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{hypot}@<@1,@2@>", "hypot(x,y) returns the quadrature sum of x and y"};
  FunctionDescriptor fd_imag          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_imag        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{Im}@<@1@>", "Im(z) returns the magnitude of the imaginary part of z"};
  FunctionDescriptor fd_ldexp         = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_ldexp       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ldexp}@<@1,@2@>", "ldexp(x,y) returns x times 2 to the power of an integer y"};
  FunctionDescriptor fd_legendreP     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_legendreP   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{legendreP}@<@1,@2@>", "legendreP(l,x) evaluates the lth Legendre polynomial at x"};
  FunctionDescriptor fd_legendreQ     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_legendreQ   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{legendreQ}@<@1,@2@>", "legendreQ(l,x) evaluates the lth Legendre function at x"};
  FunctionDescriptor fd_log           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{log}@<@1@>", "log(x) returns the natural logarithm of x"};
  FunctionDescriptor fd_log10         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log10       , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{log_{10}}@<@1@>", "log10(x) returns the logarithm of x to base 10"};
  FunctionDescriptor fd_logn          = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_logn        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{log}_n@<@1@>", "logn(x,n) returns the logarithm of x to base n"};
  FunctionDescriptor fd_ln            = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ln}@<@1@>", "ln(x) is an alias for log(x): it returns the natural logarithm of x"};
  FunctionDescriptor fd_lognormalPDF  = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_lognormalPDF, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{lognormalPDF}@<@1,@2,@3@>", "lognormalPDF(x,zeta,sigma) evaluates the log normal probability density function of standard deviation sigma at x"};
  FunctionDescriptor fd_lognormalCDF  = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_lognormalCDF, NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{lognormalCDF}@<@1,@2,@3@>", "lognormalCDF(x,zeta,sigma) evaluates the log normal cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_lognormalCDFi = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_lognormalCDFi,NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{lognormalCDFi}@<@1,@2,@3@>", "lognormalCDFi(x,zeta,sigma) evaluates the inverse log normal cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_max           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_max         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{max}@<@1,@2@>", "max(x,y) returns the greater of the two values x and y"};
  FunctionDescriptor fd_min           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_min         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{min}@<@1,@2@>", "min(x,y) returns the lesser of the two values x and y"};
  FunctionDescriptor fd_mod           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_mod         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{mod}@<@1,@2@>", "mod(x,y) returns the remainder of x/y"};
  FunctionDescriptor fd_ordinal       = { PPL_USERSPACE_STRFUNC, 0 , 1 , (void *)&dcftime_ordinal     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{ordinal}@<@1@>", "ordinal(x) converts the integer x into an ordinal number string, e.g. '1st', '2nd', etc"};
  FunctionDescriptor fd_poissonPDF    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_poissonPDF  , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{poissonPDF}@<@1,@2@>", "poissonPDF(x,mu) returns the probability of getting x from a Poisson distribution with mean mu"};
  FunctionDescriptor fd_poissonCDF    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_poissonCDF  , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{poissonCDF}@<@1,@2@>", "poissonCDF(x,mu) returns the probability of getting <= x from a Poisson distribution with mean mu"};
  FunctionDescriptor fd_pow           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_pow         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{pow}@<@1,@2@>", "pow(x,y) returns x to the power of y"};
  FunctionDescriptor fd_radians       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_radians     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{radians}@<@1@>", "radians(x) converts angles measured in degrees into radians"};
  FunctionDescriptor fd_random        = { PPL_USERSPACE_SYSTEM , 0 , 0 , (void *)&dcfmath_frandom     , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{random}@<@1@>", "random(x) returns a random number between 0 and 1"};
  FunctionDescriptor fd_randombin     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_frandombin  , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{random\\_binomial}@<@1,@2@>", "random_binomial(p,n) returns a random sample from a binomial distribution with n independent trials and a success probability p"};
  FunctionDescriptor fd_randomcs      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_frandomcs   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{random\\_\\chi^2}@<@1@>", "random_chisq(mu) returns a random sample from a chi-squared distribution with mu degrees of freedom"};
  FunctionDescriptor fd_randomg       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_frandomg    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{random\\_gaussian}@<@1@>", "random_gaussian(sigma) returns a random sample from a Gaussian (normal) distribution of standard deviation sigma"};
  FunctionDescriptor fd_randomln      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_frandomln   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{random\\_lognormal}@<@1,@2@>", "random_lognormal(zeta,sigma) returns a random sample from the log normal distribution centred on zeta, and of width sigma"};
  FunctionDescriptor fd_randomp       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_frandomp    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{random\\_poisson}@<@1@>", "random_poisson(n) returns a random integer from a Poisson distribution with mean n"};
  FunctionDescriptor fd_real          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_real        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{Re}@<@1@>", "Re(z) returns the magnitude of the real part of z"};
  FunctionDescriptor fd_root          = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_root        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{root}@<@1,@2@>", "root(z,n) returns the nth root of z"};
  FunctionDescriptor fd_sec           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sec         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{sec}@<@1@>", "sec(x) returns the secant of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_sech          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sech        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{sech}@<@1@>", "sech(x) returns the hyperbolic secant of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_sin           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sin         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{sin}@<@1@>", "sin(x) returns the sine of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_sinh          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sinh        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{sinh}@<@1@>", "sinh(x) returns the hyperbolic sine of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_sqrt          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sqrt        , NULL, NULL, NULL, NULL, NULL, NULL, "\\sqrt{@1}", "sqrt(x) returns the square root of x"};
  FunctionDescriptor fd_tan           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_tan         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{tan}@<@1@>", "tan(x) returns the tangent of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_tanh          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_tanh        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{tanh}@<@1@>", "tanh(x) returns the hyperbolic tangent of x. x may either be a dimensionless number or may have units of angle"};
  FunctionDescriptor fd_texify        = { PPL_USERSPACE_STRFUNC, 0 ,-1 , (void *)&wrapper_texify      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{texify}@<@0@>", "texify(str) converts an algebraic expression into a LaTeX command string representation"};
  FunctionDescriptor fd_time_juliandate={ PPL_USERSPACE_SYSTEM , 0 , 6 , (void *)&dcftime_juliandate  , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_juliandate}@<@1,@2,@3,@4,@5,@6@>", "time_juliandate(year,month,day,hour,min,sec) returns the Julian Date corresponding to the calendar date supplied"};
  FunctionDescriptor fd_time_now      = { PPL_USERSPACE_SYSTEM , 0 , 0 , (void *)&dcftime_now         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_now}@<@>", "time_now() returns the present Julian Date"};
  FunctionDescriptor fd_time_year     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_year        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_year}@<@1@>", "time_year(JD) returns the calendar year corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_monthnum = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_monthnum    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_monthnum}@<@1@>", "time_monthnum(JD) returns the number (1-12) of the calendar month corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_monthname= { PPL_USERSPACE_STRFUNC, 0 , 2 , (void *)&dcftime_monthname   , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_monthname}@<@1,@2@>", "time_monthname(JD,length) returns the name, truncated to length characters, of the calendar month corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_daymonth = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_daymonth    , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_daymonth}@<@1@>", "time_daymonth(JD) returns the day of the calendar month corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_dayweeknum={ PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_dayweeknum  , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_dayweeknum}@<@1@>", "time_dayweeknum(JD) returns the number (1=Monday - 7=Sunday) of the day of the week corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_dayweekname={PPL_USERSPACE_STRFUNC, 0 , 2 , (void *)&dcftime_dayweekname , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_dayweekname{@1,@2}", "time_dayweekname(JD,length) returns the name, truncated to length characters, of the day of the week corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_hour     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_hour        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_hour}@<@1@>", "time_hour(JD) returns the hour of the day corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_min      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_min         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_min}@<@1@>", "time_min(JD) returns the minutes within the hour corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_sec      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcftime_sec         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_sec}@<@1@>", "time_sec(JD) returns the seconds within the minute corresponding to the Julian Date JD"};
  FunctionDescriptor fd_time_string   = { PPL_USERSPACE_STRFUNC, 0 ,-1 , (void *)&dcftime_string      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_string@<@0@>", "time_string(JD,format) returns a textual representation of the Julian Date JD"};
  FunctionDescriptor fd_time_diff     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcftime_diff        , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_diff}@<@1,@2@>", "time_diff(JD1,JD2) returns the time interval elapsed between Julian Dates JD1 and JD2."};
  FunctionDescriptor fd_time_diff_string={PPL_USERSPACE_STRFUNC, 0 ,-1 , (void *)&dcftime_diff_string , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{time\\_diff\\_string@<@0@>", "time_diff_string(JD1,JD2,JD,format) returns a textual representation of the time interval elapsed between Julian Dates JD1 and JD2."};
  FunctionDescriptor fd_tophat        = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_tophat      , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{tophat}@<@1,@2@>", "tophat(x,sigma) returns one if |x| <= |sigma|, and zero otherwise"};
  FunctionDescriptor fd_unit          = { PPL_USERSPACE_UNIT   , 0 ,-1 , NULL                         , NULL, NULL, NULL, NULL, NULL, NULL, "\\mathrm{unit}@<@0@>", "unit(...) multiplies a number by a physical unit"};
  FunctionDescriptor fd_zeta          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_zeta        , NULL, NULL, NULL, NULL, NULL, NULL, "\\zeta@<@1@>", "zeta(x) evaluates the Riemann zeta function at x"};
  FunctionDescriptor fd_int           = { PPL_USERSPACE_INT    , 0 , 3 , NULL                         , NULL, NULL, NULL, NULL, NULL, NULL, "\\int_{@2}^{@3}@<@1@>\\,\\mathrm{d}@?", "int_dx(e,min,max) numerically integrates an expression e wrt x between min and max"};
  FunctionDescriptor fd_diff          = { PPL_USERSPACE_INT    , 0 , 3 , NULL                         , NULL, NULL, NULL, NULL, NULL, NULL, "\\left.\\frac{\\mathrm{d}}{\\mathrm{d}@?}\\right|_{@?=@2}@<@1@>", "diff_da(e,x,step) numerically differentiates an expression e wrt a at x, using a step size of step. 'a' can be any variable name with fewer than 16 characters."};

  // Set up default variables
  ppl_units_zero(&v);
  v.real = M_PI;
  DictAppendValue(_ppl_UserSpace_Vars , "pi"            , v); // pi
  v.real = M_E;
  DictAppendValue(_ppl_UserSpace_Vars , "e"             , v); // e
  v.real = M_EULER;
  DictAppendValue(_ppl_UserSpace_Vars , "euler"         , v); // Euler constant
  v.real = (1.0+sqrt(5))/2.0;
  DictAppendValue(_ppl_UserSpace_Vars , "GoldenRatio"   , v); // Golden Ratio
  v.real = 0.0;
  v.imag = 1.0;
  v.FlagComplex = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "i"             , v); // i
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_SPEED_OF_LIGHT;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH]=1 ; v.exponent[UNIT_TIME]=-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_c"         , v); // Speed of light
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_VACUUM_PERMEABILITY;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1; v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] = -2; v.exponent[UNIT_CURRENT] = -2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_0"      , v); // The permeability of free space
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_VACUUM_PERMITTIVITY;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] =-3; v.exponent[UNIT_MASS] =-1; v.exponent[UNIT_TIME] =  4; v.exponent[UNIT_CURRENT] =  2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_epsilon_0" , v); // The permittivity of free space
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_ELECTRON_CHARGE;
  v.dimensionless = 0;
  v.exponent[UNIT_CURRENT] = 1; v.exponent[UNIT_TIME] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_q"         , v); // The fundamental charge
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_PLANCKS_CONSTANT_H;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_h"         , v); // The Planck constant
  v.real = GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_hbar"      , v); // The Planck constant / 2pi
  ppl_units_zero(&v);
  v.real = GSL_CONST_NUM_AVOGADRO;
  v.dimensionless = 0;
  v.exponent[UNIT_MOLE] = -1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_NA"        , v); // The Avogadro constant
  ppl_units_zero(&v);
  v.real = 3.839e26;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-3;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Lsun"      , v); // The solar luminosity
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_UNIFIED_ATOMIC_MASS;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_u"       , v); // The universal mass constant
  v.real = GSL_CONST_MKSA_MASS_ELECTRON;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_e"       , v); // The electron mass
  v.real = GSL_CONST_MKSA_MASS_PROTON;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_p"       , v); // The proton mass
  v.real = GSL_CONST_MKSA_MASS_NEUTRON;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_n"       , v); // The neutron mass
  v.real = GSL_CONST_MKSA_MASS_MUON;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_muon"    , v); // The muon mass
  v.real = GSL_CONST_MKSA_SOLAR_MASS;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Msun"      , v); // The solar mass
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_RYDBERG;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = -1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Ry"        , v); // The Rydberg constant
  ppl_units_zero(&v);
  v.real = GSL_CONST_NUM_FINE_STRUCTURE;
  v.dimensionless = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_alpha"     , v); // The fine structure constant
  ppl_units_zero(&v);
  v.real = 6.955e8;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Rsun"      , v); // The solar radius
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_BOHR_MAGNETON;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_CURRENT] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_b"      , v); // The Bohr magneton
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_MOLAR_GAS;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1; v.exponent[UNIT_MOLE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_R"         , v); // The gas constant
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_BOLTZMANN;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_kB"        , v); // The Boltzmann constant
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2;
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_STEFAN_BOLTZMANN_CONSTANT;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] =-3; v.exponent[UNIT_TEMPERATURE] =-4;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_sigma"     , v); // The Stefan-Boltzmann constant
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_GRAVITATIONAL_CONSTANT;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 3; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_MASS] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_G"         , v); // The gravitational constant
  ppl_units_zero(&v);
  v.real = GSL_CONST_MKSA_GRAV_ACCEL;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1; v.exponent[UNIT_TIME] =-2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_g"         , v); // The standard acceleration due to gravity on Earth

  // Copy function descriptors for core mathematical functions into function namespace
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "abs"            , (void *)&fd_abs         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acos"           , (void *)&fd_acos        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acosh"          , (void *)&fd_acosh       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acot"           , (void *)&fd_acot        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acoth"          , (void *)&fd_acoth       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acsc"           , (void *)&fd_acsc        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acsch"          , (void *)&fd_acsch       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "airy_ai"        , (void *)&fd_airy_ai     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "airy_ai_diff"   , (void *)&fd_airy_ai_diff, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "airy_bi"        , (void *)&fd_airy_bi     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "airy_bi_diff"   , (void *)&fd_airy_bi_diff, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "arg"            , (void *)&fd_arg         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "asec"           , (void *)&fd_asec        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "asech"          , (void *)&fd_asech       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "asin"           , (void *)&fd_asin        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "asinh"          , (void *)&fd_asinh       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_age"     , (void *)&fd_ast_Lcdm_age     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_angscale", (void *)&fd_ast_Lcdm_angscale, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_DA"      , (void *)&fd_ast_Lcdm_DA      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_DL"      , (void *)&fd_ast_Lcdm_DL      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_DM"      , (void *)&fd_ast_Lcdm_DM      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_t"       , (void *)&fd_ast_Lcdm_t       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_Lcdm_z"       , (void *)&fd_ast_Lcdm_z       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_moonphase"    , (void *)&fd_ast_moonphase    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ast_sidereal_time", (void *)&fd_ast_sidereal_time, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "atan"           , (void *)&fd_atan        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "atanh"          , (void *)&fd_atanh       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "atan2"          , (void *)&fd_atan2       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besseli"        , (void *)&fd_besseli     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselI"        , (void *)&fd_besselI     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselj"        , (void *)&fd_besselj     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselJ"        , (void *)&fd_besselJ     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselk"        , (void *)&fd_besselk     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselK"        , (void *)&fd_besselK     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "bessely"        , (void *)&fd_bessely     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselY"        , (void *)&fd_besselY     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "beta"           , (void *)&fd_beta        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "binomialPDF"    , (void *)&fd_binomialPDF , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "binomialCDF"    , (void *)&fd_binomialCDF , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "Bv"             , (void *)&fd_Bv          , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "Bvmax"          , (void *)&fd_Bvmax       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ceil"           , (void *)&fd_ceil        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "chisqPDF"       , (void *)&fd_chisqPDF    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "chisqCDF"       , (void *)&fd_chisqCDF    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "chisqCDFi"      , (void *)&fd_chisqCDFi   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "conjugate"      , (void *)&fd_conjugate   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cos"            , (void *)&fd_cos         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cosh"           , (void *)&fd_cosh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cot"            , (void *)&fd_cot         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "coth"           , (void *)&fd_coth        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "csc"            , (void *)&fd_csc         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "csch"           , (void *)&fd_csch        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "degrees"        , (void *)&fd_degrees     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ellipticintK"   , (void *)&fd_ellK        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ellipticintE"   , (void *)&fd_ellE        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ellipticintP"   , (void *)&fd_ellP        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "erf"            , (void *)&fd_erf         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "erfc"           , (void *)&fd_erfc        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "exp"            , (void *)&fd_exp         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "expm1"          , (void *)&fd_expm1       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "expint"         , (void *)&fd_expint      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "floor"          , (void *)&fd_floor       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "gamma"          , (void *)&fd_gamma       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "gaussianPDF"    , (void *)&fd_gaussianPDF , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "gaussianCDF"    , (void *)&fd_gaussianCDF , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "gaussianCDFi"   , (void *)&fd_gaussianCDFi, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "heaviside"      , (void *)&fd_heaviside   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "hypot"          , (void *)&fd_hypot       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "Im"             , (void *)&fd_imag        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ldexp"          , (void *)&fd_ldexp       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "legendreP"      , (void *)&fd_legendreP   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "legendreQ"      , (void *)&fd_legendreQ   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "log"            , (void *)&fd_log         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "log10"          , (void *)&fd_log10       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "logn"           , (void *)&fd_logn        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ln"             , (void *)&fd_ln          , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "lognormalPDF"   , (void *)&fd_lognormalPDF, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "lognormalCDF"   , (void *)&fd_lognormalCDF, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "lognormalCDFi"  , (void *)&fd_lognormalCDFi,sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "max"            , (void *)&fd_max         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "min"            , (void *)&fd_min         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "mod"            , (void *)&fd_mod         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ordinal"        , (void *)&fd_ordinal     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "poissonPDF"     , (void *)&fd_poissonPDF  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "poissonCDF"     , (void *)&fd_poissonCDF  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "pow"            , (void *)&fd_pow         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "radians"        , (void *)&fd_radians     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random"         , (void *)&fd_random      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random_binomial", (void *)&fd_randombin   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random_chisq"   , (void *)&fd_randomcs    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random_gaussian", (void *)&fd_randomg     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random_lognormal",(void *)&fd_randomln    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random_poisson" , (void *)&fd_randomp     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "Re"             , (void *)&fd_real        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "root"           , (void *)&fd_root        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sec"            , (void *)&fd_sec         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sech"           , (void *)&fd_sech        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sin"            , (void *)&fd_sin         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sinh"           , (void *)&fd_sinh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sqrt"           , (void *)&fd_sqrt        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tan"            , (void *)&fd_tan         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tanh"           , (void *)&fd_tanh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "texify"         , (void *)&fd_texify      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_juliandate", (void *)&fd_time_juliandate, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_now"       , (void *)&fd_time_now       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_year"      , (void *)&fd_time_year      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_monthnum"  , (void *)&fd_time_monthnum  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_monthname" , (void *)&fd_time_monthname , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_daymonth"  , (void *)&fd_time_daymonth  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_dayweeknum", (void *)&fd_time_dayweeknum, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_dayweekname",(void *)&fd_time_dayweekname,sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_hour"      , (void *)&fd_time_hour      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_min"       , (void *)&fd_time_min       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_sec"       , (void *)&fd_time_sec       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_string"    , (void *)&fd_time_string    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_diff"      , (void *)&fd_time_diff      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "time_diff_string",(void *)&fd_time_diff_string,sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tophat"         , (void *)&fd_tophat      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "unit"           , (void *)&fd_unit        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "zeta"           , (void *)&fd_zeta        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "int_d?"         , (void *)&fd_int         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "diff_d?"        , (void *)&fd_diff        , sizeof(FunctionDescriptor), DATATYPE_VOID);

  return;
 }

void ppl_units_init()
 {
  int i=0,j=0;

  ppl_unit_database = (unit *)lt_malloc(UNITS_MAX*sizeof(unit));
  if (ppl_unit_database == NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory error whilst trying to malloc units database."); exit(1); }

  // Set up database of known units
  for (i=0;i<UNITS_MAX;i++)
   {
    ppl_unit_database[i].nameAs     = NULL;
    ppl_unit_database[i].nameAp     = NULL;
    ppl_unit_database[i].nameLs     = NULL;
    ppl_unit_database[i].nameLp     = NULL;
    ppl_unit_database[i].nameFs     = NULL;
    ppl_unit_database[i].nameFp     = NULL;
    ppl_unit_database[i].alt1       = NULL;
    ppl_unit_database[i].alt2       = NULL;
    ppl_unit_database[i].alt3       = NULL;
    ppl_unit_database[i].alt4       = NULL;
    ppl_unit_database[i].quantity   = NULL;
    ppl_unit_database[i].comment    = NULL;
    ppl_unit_database[i].multiplier = 1.0;
    ppl_unit_database[i].TempType   = 0;
    ppl_unit_database[i].offset     = 0.0;
    ppl_unit_database[i].UserSel    = 0;
    ppl_unit_database[i].si         = ppl_unit_database[i].cgs       = ppl_unit_database[i].imperial  = ppl_unit_database[i].us = ppl_unit_database[i].planck =
    ppl_unit_database[i].ancient    = ppl_unit_database[i].UserSel   = ppl_unit_database[i].modified  = 0;
    ppl_unit_database[i].MaxPrefix  = ppl_unit_database[i].MinPrefix = 0;
    ppl_unit_database[i].UserSelPrefix = 0;
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[i].exponent[j] = 0;
   }

  // Set up default list of units
  ppl_unit_database[ppl_unit_pos].nameAs     = "percent"; // Percent
  ppl_unit_database[ppl_unit_pos].nameAp     = "percent";
  ppl_unit_database[ppl_unit_pos].nameLs     = "\\%";
  ppl_unit_database[ppl_unit_pos].nameLp     = "\\%";
  ppl_unit_database[ppl_unit_pos].nameFs     = "percent";
  ppl_unit_database[ppl_unit_pos].nameFp     = "percent";
  ppl_unit_database[ppl_unit_pos].quantity   = "dimensionlessness";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.01;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ppm"; // Parts per million
  ppl_unit_database[ppl_unit_pos].nameAp     = "ppm";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "parts_per_million";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parts_per_million";
  ppl_unit_database[ppl_unit_pos].quantity   = "dimensionlessness";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-6;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ppb"; // Parts per billion
  ppl_unit_database[ppl_unit_pos].nameAp     = "ppb";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "parts_per_billion";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parts_per_billion";
  ppl_unit_database[ppl_unit_pos].quantity   = "dimensionlessness";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-9;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "m"; // Metre
  ppl_unit_database[ppl_unit_pos].nameAp     = "m";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "metre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cm"; // Centimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "cm";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "centimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.01;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "dm"; // Decimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "dm";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "decimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "decimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ang"; // Angstrom
  ppl_unit_database[ppl_unit_pos].nameAp     = "ang";
  ppl_unit_database[ppl_unit_pos].nameLs     = "\\A";
  ppl_unit_database[ppl_unit_pos].nameLp     = "\\A";
  ppl_unit_database[ppl_unit_pos].nameFs     = "angstrom";
  ppl_unit_database[ppl_unit_pos].nameFp     = "angstroms";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-10;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "in"; // Inch
  ppl_unit_database[ppl_unit_pos].nameAp     = "in";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_INCH;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ft"; // Foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "ft";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_FOOT;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "yd"; // Yard
  ppl_unit_database[ppl_unit_pos].nameAp     = "yd";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "yard";
  ppl_unit_database[ppl_unit_pos].nameFp     = "yards";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_YARD;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mi"; // Mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "mi";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_MILE;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;
  
  ppl_unit_database[ppl_unit_pos].nameAs     = "furlong"; // Furlong
  ppl_unit_database[ppl_unit_pos].nameAp     = "furlongs";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "furlong";
  ppl_unit_database[ppl_unit_pos].nameFp     = "furlongs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 201.168;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "chain"; // Chain
  ppl_unit_database[ppl_unit_pos].nameAp     = "chains";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "chain";
  ppl_unit_database[ppl_unit_pos].nameFp     = "chains";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 20.1168;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "link"; // Link
  ppl_unit_database[ppl_unit_pos].nameAp     = "links";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "link";
  ppl_unit_database[ppl_unit_pos].nameFp     = "links";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.201168;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cable"; // Cable
  ppl_unit_database[ppl_unit_pos].nameAp     = "cables";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "cable";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cables";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 185.3184;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "fathom"; // Fathom
  ppl_unit_database[ppl_unit_pos].nameAp     = "fathoms";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "fathom";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fathoms";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_FATHOM;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "roman_mile"; // Roman Mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "roman_miles";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "roman_mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "roman_miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1479.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "roman_league"; // Roman League
  ppl_unit_database[ppl_unit_pos].nameAp     = "roman_leagues";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "roman_league";
  ppl_unit_database[ppl_unit_pos].nameFp     = "roman_leagues";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.5 * 1479.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "nautical_mile"; // Nautical mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "nautical_miles";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "nautical_mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "nautical_miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_NAUTICAL_MILE;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pt"; // Point (typesetting unit)
  ppl_unit_database[ppl_unit_pos].nameAp     = "pt";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "point";
  ppl_unit_database[ppl_unit_pos].nameFp     = "points";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_INCH / 72.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pica"; // Pica (typesetting unit)
  ppl_unit_database[ppl_unit_pos].nameAp     = "picas";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "pica";
  ppl_unit_database[ppl_unit_pos].nameFp     = "picas";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_INCH / 6.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubit"; // Cubit
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubits";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubits";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.4572;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "rod"; // Rod
  ppl_unit_database[ppl_unit_pos].nameAp     = "rods";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "rod";
  ppl_unit_database[ppl_unit_pos].nameFp     = "rods";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.02920;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "perch"; // Perch
  ppl_unit_database[ppl_unit_pos].nameAp     = "perches";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "perch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "perches";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.02920;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pole"; // Pole
  ppl_unit_database[ppl_unit_pos].nameAp     = "poles";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "pole";
  ppl_unit_database[ppl_unit_pos].nameFp     = "poles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.02920;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "AU"; // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameAp     = "AU";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "astronomical_unit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "astronomical_units";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_ASTRONOMICAL_UNIT;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lyr"; // Lightyear
  ppl_unit_database[ppl_unit_pos].nameAp     = "lyr";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "light_year";
  ppl_unit_database[ppl_unit_pos].nameFp     = "light_years";
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_LIGHT_YEAR;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pc"; // Parsec
  ppl_unit_database[ppl_unit_pos].nameAp     = "pc";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "parsec";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parsecs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_PARSEC;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Rsun"; // Solar radii
  ppl_unit_database[ppl_unit_pos].nameAp     = "Rsun";
  ppl_unit_database[ppl_unit_pos].nameLs     = "R_\\odot";
  ppl_unit_database[ppl_unit_pos].nameLp     = "R_\\odot";
  ppl_unit_database[ppl_unit_pos].nameFs     = "solar_radius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "solar_radii";
  ppl_unit_database[ppl_unit_pos].alt1       = "Rsolar";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 6.955e8;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Rearth"; // Earth radii
  ppl_unit_database[ppl_unit_pos].nameAp     = "Rearth";
  ppl_unit_database[ppl_unit_pos].nameLs     = "R_E";
  ppl_unit_database[ppl_unit_pos].nameLp     = "R_E";
  ppl_unit_database[ppl_unit_pos].nameFs     = "earth_radius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "earth_radii";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 6371000;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Rjupiter"; // Jupiter radii
  ppl_unit_database[ppl_unit_pos].nameAp     = "Rjupiter";
  ppl_unit_database[ppl_unit_pos].nameLs     = "R_J";
  ppl_unit_database[ppl_unit_pos].nameLp     = "R_J";
  ppl_unit_database[ppl_unit_pos].nameFs     = "jupiter_radius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "jupiter_radii";
  ppl_unit_database[ppl_unit_pos].alt1       = "Rjove";
  ppl_unit_database[ppl_unit_pos].alt2       = "Rjovian";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 71492000;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lunar_distance"; // Lunar distances
  ppl_unit_database[ppl_unit_pos].nameAp     = "lunar_distances";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "lunar_distance";
  ppl_unit_database[ppl_unit_pos].nameFp     = "lunar_distances";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 384403000;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "s"; // Second
  ppl_unit_database[ppl_unit_pos].nameAp     = "s";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "second";
  ppl_unit_database[ppl_unit_pos].nameFp     = "seconds";
  ppl_unit_database[ppl_unit_pos].alt1       = "sec";
  ppl_unit_database[ppl_unit_pos].alt2       = "secs";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "min"; // Minute
  ppl_unit_database[ppl_unit_pos].nameAp     = "min";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "minute";
  ppl_unit_database[ppl_unit_pos].nameFp     = "minutes";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 60;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "hr"; // Hour
  ppl_unit_database[ppl_unit_pos].nameAp     = "hr";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "hour";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hours";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 3600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "day"; // Day
  ppl_unit_database[ppl_unit_pos].nameAp     = "days";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "day";
  ppl_unit_database[ppl_unit_pos].nameFp     = "days";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 86400;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "week"; // Week
  ppl_unit_database[ppl_unit_pos].nameAp     = "weeks";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "week";
  ppl_unit_database[ppl_unit_pos].nameFp     = "weeks";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 604800;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "yr"; // Year
  ppl_unit_database[ppl_unit_pos].nameAp     = "yr";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "year";
  ppl_unit_database[ppl_unit_pos].nameFp     = "years";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
  ppl_unit_database[ppl_unit_pos].multiplier = 31557600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sol"; // Sol (Martian Day)
  ppl_unit_database[ppl_unit_pos].nameAp     = "sols";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "sol";
  ppl_unit_database[ppl_unit_pos].nameFp     = "sols";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 88775.244;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kg"; // Kilogram
  ppl_unit_database[ppl_unit_pos].nameAp     = "kg";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "kilogram";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kilograms";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "g"; // Gramme
  ppl_unit_database[ppl_unit_pos].nameAp     = "g";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "gramme";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grammes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "g"; // Gram
  ppl_unit_database[ppl_unit_pos].nameAp     = "g";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "gram";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grams";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "t"; // Metric Tonne
  ppl_unit_database[ppl_unit_pos].nameAp     = "t";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "tonne";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tonnes";
  ppl_unit_database[ppl_unit_pos].alt1       = "metric_tonne";
  ppl_unit_database[ppl_unit_pos].alt2       = "metric_tonnes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "short_ton"; // US Ton
  ppl_unit_database[ppl_unit_pos].nameAp     = "short_tons";
  ppl_unit_database[ppl_unit_pos].nameLs     = "short\\_ton";
  ppl_unit_database[ppl_unit_pos].nameLp     = "short\\_tons";
  ppl_unit_database[ppl_unit_pos].nameFs     = "ton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_TON;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "long_ton"; // UK Ton
  ppl_unit_database[ppl_unit_pos].nameAp     = "long_tons";
  ppl_unit_database[ppl_unit_pos].nameLs     = "long\\_ton";
  ppl_unit_database[ppl_unit_pos].nameLp     = "long\\_tons";
  ppl_unit_database[ppl_unit_pos].nameFs     = "ton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_UK_TON;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oz"; // Ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "oz";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "ounce";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ounces";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_OUNCE_MASS;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oz_troy"; // Troy Ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "oz_troy";
  ppl_unit_database[ppl_unit_pos].nameLs     = "oz\\_troy";
  ppl_unit_database[ppl_unit_pos].nameLp     = "oz\\_troy";
  ppl_unit_database[ppl_unit_pos].nameFs     = "troy_ounce";
  ppl_unit_database[ppl_unit_pos].nameFp     = "troy_ounces";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_TROY_OUNCE;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lb"; // Pound
  ppl_unit_database[ppl_unit_pos].nameAp     = "lbs";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "pound";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pounds";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_POUND_MASS;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "stone"; // Stone
  ppl_unit_database[ppl_unit_pos].nameAp     = "stone";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "stone";
  ppl_unit_database[ppl_unit_pos].nameFp     = "stone";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 6.35029318;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "slug"; // Slug
  ppl_unit_database[ppl_unit_pos].nameAp     = "slugs";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "slug";
  ppl_unit_database[ppl_unit_pos].nameFp     = "slugs";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 14.5939;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cwt_UK"; // UK hundredweight
  ppl_unit_database[ppl_unit_pos].nameAp     = "cwt_UK";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "hundredweight_UK";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hundredweight_UK";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 50.80234544;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cwt_US"; // US hundredweight
  ppl_unit_database[ppl_unit_pos].nameAp     = "cwt_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "hundredweight_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hundredweight_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 45.359237;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "drachm"; // drachm
  ppl_unit_database[ppl_unit_pos].nameAp     = "drachms";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "drachm";
  ppl_unit_database[ppl_unit_pos].nameFp     = "drachms";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.7718451953125e-3;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "grain"; // grain
  ppl_unit_database[ppl_unit_pos].nameAp     = "grains";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "grain";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grains";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 64.79891e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "CD"; // carat
  ppl_unit_database[ppl_unit_pos].nameAp     = "CDs";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "carat";
  ppl_unit_database[ppl_unit_pos].nameFp     = "carats";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 200e-6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "shekel"; // Shekel
  ppl_unit_database[ppl_unit_pos].nameAp     = "shekels";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "shekel";
  ppl_unit_database[ppl_unit_pos].nameFp     = "shekels";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mina"; // Mina
  ppl_unit_database[ppl_unit_pos].nameAp     = "minas";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "mina";
  ppl_unit_database[ppl_unit_pos].nameFp     = "minas";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011 * 60;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "talent"; // Talent
  ppl_unit_database[ppl_unit_pos].nameAp     = "talents";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "talent";
  ppl_unit_database[ppl_unit_pos].nameFp     = "talents";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011 * 360;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Msun"; // Solar mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "Msun";
  ppl_unit_database[ppl_unit_pos].nameLs     = "M_\\odot";
  ppl_unit_database[ppl_unit_pos].nameLp     = "M_\\odot";
  ppl_unit_database[ppl_unit_pos].nameFs     = "solar_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "solar_masses";
  ppl_unit_database[ppl_unit_pos].alt1       = "Msolar";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_SOLAR_MASS;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mearth"; // Earth mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mearth";
  ppl_unit_database[ppl_unit_pos].nameLs     = "M_E";
  ppl_unit_database[ppl_unit_pos].nameLp     = "M_E";
  ppl_unit_database[ppl_unit_pos].nameFs     = "earth_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "earth_masses";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.9742e24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mjupiter"; // Jupiter mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mjupiter";
  ppl_unit_database[ppl_unit_pos].nameLs     = "M_J";
  ppl_unit_database[ppl_unit_pos].nameLp     = "M_J";
  ppl_unit_database[ppl_unit_pos].nameFs     = "jupiter_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "jupiter_masses";
  ppl_unit_database[ppl_unit_pos].alt1       = "Mjove";
  ppl_unit_database[ppl_unit_pos].alt2       = "Mjovian";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.8986e27;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameAp     = "A";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "ampere";
  ppl_unit_database[ppl_unit_pos].nameFp     = "amperes";
  ppl_unit_database[ppl_unit_pos].alt1       = "amp";
  ppl_unit_database[ppl_unit_pos].alt2       = "amps";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].nameAp     = "K";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "kelvin";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kelvin";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].TempType   = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  TempTypeMultiplier[ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].multiplier;
  TempTypeOffset    [ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].offset;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "R"; // Rankin
  ppl_unit_database[ppl_unit_pos].nameAp     = "R";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "rankin";
  ppl_unit_database[ppl_unit_pos].nameFp     = "rankin";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].TempType   = 2;
  ppl_unit_database[ppl_unit_pos].multiplier = 5.0/9.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  TempTypeMultiplier[ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].multiplier;
  TempTypeOffset    [ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].offset;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].nameAp     = "oC";
  ppl_unit_database[ppl_unit_pos].nameLs     = "^\\circ C";
  ppl_unit_database[ppl_unit_pos].nameLp     = "^\\circ C";
  ppl_unit_database[ppl_unit_pos].nameFs     = "degree_celsius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "degrees_celsius";
  ppl_unit_database[ppl_unit_pos].alt1       = "degree_centigrade";
  ppl_unit_database[ppl_unit_pos].alt2       = "degrees_centigrade";
  ppl_unit_database[ppl_unit_pos].alt3       = "centigrade";
  ppl_unit_database[ppl_unit_pos].alt4       = "celsius";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].TempType   = 3;
  ppl_unit_database[ppl_unit_pos].offset     = 273.15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  TempTypeMultiplier[ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].multiplier;
  TempTypeOffset    [ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].offset;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oF"; // oF
  ppl_unit_database[ppl_unit_pos].nameAp     = "oF";
  ppl_unit_database[ppl_unit_pos].nameLs     = "^\\circ F";
  ppl_unit_database[ppl_unit_pos].nameLp     = "^\\circ F";
  ppl_unit_database[ppl_unit_pos].nameFs     = "degree_fahrenheit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "degrees_fahrenheit";
  ppl_unit_database[ppl_unit_pos].alt1       = "fahrenheit";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].TempType   = 4;
  ppl_unit_database[ppl_unit_pos].multiplier = 5.0/9.0;
  ppl_unit_database[ppl_unit_pos].offset     = 459.67 * 5.0/9.0;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  TempTypeMultiplier[ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].multiplier;
  TempTypeOffset    [ppl_unit_database[ppl_unit_pos].TempType] = ppl_unit_database[ppl_unit_pos].offset;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mol"; // mole
  ppl_unit_database[ppl_unit_pos].nameAp     = "mol";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "mole";
  ppl_unit_database[ppl_unit_pos].nameFp     = "moles";
  ppl_unit_database[ppl_unit_pos].quantity   = "moles";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cd"; // candela
  ppl_unit_database[ppl_unit_pos].nameAp     = "cd";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "candela";
  ppl_unit_database[ppl_unit_pos].nameFp     = "candelas";
  ppl_unit_database[ppl_unit_pos].quantity   = "light_intensity";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0/683;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]  =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "candlepower"; // candlepower
  ppl_unit_database[ppl_unit_pos].nameAp     = "candlepower";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "candlepower";
  ppl_unit_database[ppl_unit_pos].nameFp     = "candlepower";
  ppl_unit_database[ppl_unit_pos].quantity   = "light_intensity";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0/683/0.981;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]  =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lm"; // lumen
  ppl_unit_database[ppl_unit_pos].nameAp     = "lm";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "lumen";
  ppl_unit_database[ppl_unit_pos].nameFp     = "lumens";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_LUMEN;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lx"; // lux
  ppl_unit_database[ppl_unit_pos].nameAp     = "lx";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "lux";
  ppl_unit_database[ppl_unit_pos].nameFp     = "luxs";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_LUX;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Jy"; // jansky
  ppl_unit_database[ppl_unit_pos].nameAp     = "Jy";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "jansky";
  ppl_unit_database[ppl_unit_pos].nameFp     = "janskys";
  ppl_unit_database[ppl_unit_pos].quantity   = "flux_density";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-26;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1; // Watt per square metre per Hz (NOT per steradian!)
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "rad"; // radians
  ppl_unit_database[ppl_unit_pos].nameAp     = "rad";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "radian";
  ppl_unit_database[ppl_unit_pos].nameFp     = "radians";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "deg"; // degrees
  ppl_unit_database[ppl_unit_pos].nameAp     = "deg";
  ppl_unit_database[ppl_unit_pos].nameLs     = "^\\circ";
  ppl_unit_database[ppl_unit_pos].nameLp     = "^\\circ";
  ppl_unit_database[ppl_unit_pos].nameFs     = "degree";
  ppl_unit_database[ppl_unit_pos].nameFp     = "degrees";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = M_PI / 180;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "rev"; // revolution
  ppl_unit_database[ppl_unit_pos].nameAp     = "rev";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "revolution";
  ppl_unit_database[ppl_unit_pos].nameFp     = "revolutions";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = 2 * M_PI;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "arcmin"; // arcminute
  ppl_unit_database[ppl_unit_pos].nameAp     = "arcmins";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "arcminute";
  ppl_unit_database[ppl_unit_pos].nameFp     = "arcminutes";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = M_PI / 180 / 60;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "arcsec"; // arcsecond
  ppl_unit_database[ppl_unit_pos].nameAp     = "arcsecs";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "arcsecond";
  ppl_unit_database[ppl_unit_pos].nameFp     = "arcseconds";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = M_PI / 180 / 3600;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bit"; // bit
  ppl_unit_database[ppl_unit_pos].nameAp     = "bits";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "bit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "euro"; // cost
  ppl_unit_database[ppl_unit_pos].nameAp     = "euros";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "euro";
  ppl_unit_database[ppl_unit_pos].nameFp     = "euros";
  ppl_unit_database[ppl_unit_pos].quantity   = "cost";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_COST]=1;
  ppl_unit_pos++;


  // -------------
  // Derived units
  // -------------

  ppl_unit_database[ppl_unit_pos].nameAs     = "dioptre"; // dioptre
  ppl_unit_database[ppl_unit_pos].nameAp     = "dioptres";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "dioptre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "dioptres";
  ppl_unit_database[ppl_unit_pos].quantity   = "lens_power";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mph"; // mile_per_hour
  ppl_unit_database[ppl_unit_pos].nameAp     = "mph";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "mile_per_hour";
  ppl_unit_database[ppl_unit_pos].nameFp     = "miles_per_hour";
  ppl_unit_database[ppl_unit_pos].quantity   = "velocity";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_MILE / 3600;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kn"; // knot
  ppl_unit_database[ppl_unit_pos].nameAp     = "kn";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "knot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "knots";
  ppl_unit_database[ppl_unit_pos].quantity   = "velocity";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_KNOT;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "acre"; // acre
  ppl_unit_database[ppl_unit_pos].nameAp     = "acres";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "acre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "acres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_ACRE;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "are"; // are
  ppl_unit_database[ppl_unit_pos].nameAp     = "ares";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "are";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ares";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 100;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "barn"; // barn
  ppl_unit_database[ppl_unit_pos].nameAp     = "barns";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "barn";
  ppl_unit_database[ppl_unit_pos].nameFp     = "barns";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-28;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "hectare"; // hectare
  ppl_unit_database[ppl_unit_pos].nameAp     = "hectares";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "hectare";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hectares";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e4;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_mi"; // square mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_mi";
  ppl_unit_database[ppl_unit_pos].nameLs     = "sq\\_mi";
  ppl_unit_database[ppl_unit_pos].nameLp     = "sq\\_mi";
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_MILE,2);
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_km"; // square kilometre
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_km";
  ppl_unit_database[ppl_unit_pos].nameLs     = "sq\\_km";
  ppl_unit_database[ppl_unit_pos].nameLp     = "sq\\_km";
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_kilometre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_kilometres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e6;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_m"; // square metre
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_m";
  ppl_unit_database[ppl_unit_pos].nameLs     = "sq\\_m";
  ppl_unit_database[ppl_unit_pos].nameLp     = "sq\\_m";
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_metre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_cm"; // square centimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_cm";
  ppl_unit_database[ppl_unit_pos].nameLs     = "sq\\_cm";
  ppl_unit_database[ppl_unit_pos].nameLp     = "sq\\_cm";
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_centimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-4;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_ft"; // square foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_ft";
  ppl_unit_database[ppl_unit_pos].nameLs     = "sq\\_ft";
  ppl_unit_database[ppl_unit_pos].nameLp     = "sq\\_ft";
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_FOOT,2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_in"; // square inch
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_in";
  ppl_unit_database[ppl_unit_pos].nameLs     = "sq\\_in";
  ppl_unit_database[ppl_unit_pos].nameLp     = "sq\\_in";
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_INCH,2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_m"; // cubic metre
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_m";
  ppl_unit_database[ppl_unit_pos].nameLs     = "cubic\\_m";
  ppl_unit_database[ppl_unit_pos].nameLp     = "cubic\\_m";
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_metre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_cm"; // cubic centimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_cm";
  ppl_unit_database[ppl_unit_pos].nameLs     = "cubic\\_cm";
  ppl_unit_database[ppl_unit_pos].nameLp     = "cubic\\_cm";
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_centimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-6;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_ft"; // cubic foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_ft";
  ppl_unit_database[ppl_unit_pos].nameLs     = "cubic\\_ft";
  ppl_unit_database[ppl_unit_pos].nameLp     = "cubic\\_ft";
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_FOOT,3);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_in"; // cubic inch
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_in";
  ppl_unit_database[ppl_unit_pos].nameLs     = "cubic\\_in";
  ppl_unit_database[ppl_unit_pos].nameLp     = "cubic\\_in";
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_INCH,3);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "l"; // litre
  ppl_unit_database[ppl_unit_pos].nameAp     = "l";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "litre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "litres";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "fl_oz_UK"; // UK fluid ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "fl_oz_UK";
  ppl_unit_database[ppl_unit_pos].nameLs     = "fl\\_oz\\_UK";
  ppl_unit_database[ppl_unit_pos].nameLp     = "fl\\_oz\\_UK";
  ppl_unit_database[ppl_unit_pos].nameFs     = "fluid_ounce_UK";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fluid_ounce_UK";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 28.4130625e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "fl_oz_US"; // US fluid ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "fl_oz_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = "fl\\_oz\\_US";
  ppl_unit_database[ppl_unit_pos].nameLp     = "fl\\_oz\\_US";
  ppl_unit_database[ppl_unit_pos].nameFs     = "fluid_ounce_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fluid_ounce_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_FLUID_OUNCE;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pint_UK"; // UK pint
  ppl_unit_database[ppl_unit_pos].nameAp     = "pints_UK";
  ppl_unit_database[ppl_unit_pos].nameLs     = "pint\\_UK";
  ppl_unit_database[ppl_unit_pos].nameLp     = "pints\\_UK";
  ppl_unit_database[ppl_unit_pos].nameFs     = "pint_UK";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pints_UK";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 568.26125e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pint_US"; // US pint
  ppl_unit_database[ppl_unit_pos].nameAp     = "pints_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = "pint\\_US";
  ppl_unit_database[ppl_unit_pos].nameLp     = "pints\\_US";
  ppl_unit_database[ppl_unit_pos].nameFs     = "pint_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pints_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_PINT;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "quart_UK"; // UK quart
  ppl_unit_database[ppl_unit_pos].nameAp     = "quarts_UK";
  ppl_unit_database[ppl_unit_pos].nameLs     = "quart\\_UK";
  ppl_unit_database[ppl_unit_pos].nameLp     = "quarts\\_UK";
  ppl_unit_database[ppl_unit_pos].nameFs     = "quart_UK";
  ppl_unit_database[ppl_unit_pos].nameFp     = "quarts_UK";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 1136.5225e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "quart_US"; // US quart
  ppl_unit_database[ppl_unit_pos].nameAp     = "quarts_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = "quart\\_US";
  ppl_unit_database[ppl_unit_pos].nameLp     = "quarts\\_US";
  ppl_unit_database[ppl_unit_pos].nameFs     = "quart_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "quarts_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_QUART;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "gallon_UK"; // UK gallon
  ppl_unit_database[ppl_unit_pos].nameAp     = "gallons_UK";
  ppl_unit_database[ppl_unit_pos].nameLs     = "gallon\\_UK";
  ppl_unit_database[ppl_unit_pos].nameLp     = "gallons\\_UK";
  ppl_unit_database[ppl_unit_pos].nameFs     = "gallon_UK";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gallons_UK";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_UK_GALLON;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "gallon_US"; // US gallon
  ppl_unit_database[ppl_unit_pos].nameAp     = "gallons_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = "gallon\\_US";
  ppl_unit_database[ppl_unit_pos].nameLp     = "gallons\\_US";
  ppl_unit_database[ppl_unit_pos].nameFs     = "gallon_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gallons_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_US_GALLON;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bushel_UK"; // UK bushel
  ppl_unit_database[ppl_unit_pos].nameAp     = "bushels_UK";
  ppl_unit_database[ppl_unit_pos].nameLs     = "bushel\\_UK";
  ppl_unit_database[ppl_unit_pos].nameLp     = "bushels\\_UK";
  ppl_unit_database[ppl_unit_pos].nameFs     = "bushel_UK";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bushels_UK";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 36.36872e-3;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bushel_US"; // US bushel
  ppl_unit_database[ppl_unit_pos].nameAp     = "bushels_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = "bushel\\_US";
  ppl_unit_database[ppl_unit_pos].nameLp     = "bushels\\_US";
  ppl_unit_database[ppl_unit_pos].nameFs     = "bushel_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bushels_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 35.23907016688e-3;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cup_US"; // US cup
  ppl_unit_database[ppl_unit_pos].nameAp     = "cups_US";
  ppl_unit_database[ppl_unit_pos].nameLs     = "cup\\_US";
  ppl_unit_database[ppl_unit_pos].nameLp     = "cups\\_US";
  ppl_unit_database[ppl_unit_pos].nameFs     = "cup_US";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cups_US";
  ppl_unit_database[ppl_unit_pos].comment    = "US customary";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_CUP;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bath"; // bath
  ppl_unit_database[ppl_unit_pos].nameAp     = "baths";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "bath";
  ppl_unit_database[ppl_unit_pos].nameFp     = "baths";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 22e-3;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "homer"; // homer
  ppl_unit_database[ppl_unit_pos].nameAp     = "homers";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "homer";
  ppl_unit_database[ppl_unit_pos].nameFp     = "homers";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 220e-3;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "teaspoon"; // teaspoon
  ppl_unit_database[ppl_unit_pos].nameAp     = "teaspoons";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "teaspoon";
  ppl_unit_database[ppl_unit_pos].nameFp     = "teaspoons";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 5e-6; // 5 mL
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "tablespoon"; // tablespoon
  ppl_unit_database[ppl_unit_pos].nameAp     = "tablespoons";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "tablespoon";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tablespoons";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 15e-6; // 15 mL
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "firkin_UK_ale"; // firkin of ale
  ppl_unit_database[ppl_unit_pos].nameAp     = "firkins_UK_ale";
  ppl_unit_database[ppl_unit_pos].nameLs     = "firkin\\_UK\\_ale";
  ppl_unit_database[ppl_unit_pos].nameLp     = "firkins\\_UK\\_ale";
  ppl_unit_database[ppl_unit_pos].nameFs     = "firkin_UK_ale";
  ppl_unit_database[ppl_unit_pos].nameFp     = "firkins_UK_ale";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 40.91481e-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "firkin_UK_wine"; // firkin of wine
  ppl_unit_database[ppl_unit_pos].nameAp     = "firkins_UK_wine";
  ppl_unit_database[ppl_unit_pos].nameLs     = "firkin\\_UK\\_wine";
  ppl_unit_database[ppl_unit_pos].nameLp     = "firkins\\_UK\\_wine";
  ppl_unit_database[ppl_unit_pos].nameFs     = "firkin_wine";
  ppl_unit_database[ppl_unit_pos].nameFp     = "firkins_wine";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 318e-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kilderkin_UK_ale"; // kilderkin of ale
  ppl_unit_database[ppl_unit_pos].nameAp     = "kilderkins_UK_ale";
  ppl_unit_database[ppl_unit_pos].nameLs     = "kilderkin\\_UK\\_ale";
  ppl_unit_database[ppl_unit_pos].nameLp     = "kilderkins\\_UK\\_ale";
  ppl_unit_database[ppl_unit_pos].nameFs     = "kilderkin_UK_ale";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kilderkins_UK_ale";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 81.82962e-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sterad"; // steradians
  ppl_unit_database[ppl_unit_pos].nameAp     = "sterad";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "steradian";
  ppl_unit_database[ppl_unit_pos].nameFp     = "steradians";
  ppl_unit_database[ppl_unit_pos].quantity   = "solidangle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sqdeg"; // square degrees
  ppl_unit_database[ppl_unit_pos].nameAp     = "sqdeg";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_degree";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_degrees";
  ppl_unit_database[ppl_unit_pos].quantity   = "solidangle";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(M_PI/180, 2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Hz"; // hertz
  ppl_unit_database[ppl_unit_pos].nameAp     = "Hz";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "hertz";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hertz";
  ppl_unit_database[ppl_unit_pos].quantity   = "frequency";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Bq"; // becquerel
  ppl_unit_database[ppl_unit_pos].nameAp     = "Bq";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "becquerel";
  ppl_unit_database[ppl_unit_pos].nameFp     = "becquerel";
  ppl_unit_database[ppl_unit_pos].quantity   = "frequency";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "B"; // bytes
  ppl_unit_database[ppl_unit_pos].nameAp     = "B";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "byte";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 8.0;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Kib"; // kibibits
  ppl_unit_database[ppl_unit_pos].nameAp     = "Kib";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "kibibit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kibibits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1024.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "KiB"; // kibibytes
  ppl_unit_database[ppl_unit_pos].nameAp     = "KiB";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "kibibyte";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kibibytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1024.0 * 8.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mib"; // mebibits
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mib";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "mebibit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "mebibits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1024.0 * 1024.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "MiB"; // mebibytes
  ppl_unit_database[ppl_unit_pos].nameAp     = "MiB";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "mebibyte";
  ppl_unit_database[ppl_unit_pos].nameFp     = "mebibytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1024.0 * 1024.0 * 8.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Gib"; // gibibits
  ppl_unit_database[ppl_unit_pos].nameAp     = "Gib";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "gibibit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gibibits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1024.0 * 1024.0 * 1024.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "GiB"; // gibibytes
  ppl_unit_database[ppl_unit_pos].nameAp     = "GiB";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "gibibyte";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gibibytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1024.0 * 1024.0 * 1024.0 * 8.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "N"; // newton
  ppl_unit_database[ppl_unit_pos].nameAp     = "N";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "newton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "newtons";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "dyn"; // dyne
  ppl_unit_database[ppl_unit_pos].nameAp     = "dyn";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "dyne";
  ppl_unit_database[ppl_unit_pos].nameFp     = "dynes";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-5;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lbf"; // pound force
  ppl_unit_database[ppl_unit_pos].nameAp     = "lbf";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "pound_force";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pounds_force";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_POUND_FORCE;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Pa"; // pascal
  ppl_unit_database[ppl_unit_pos].nameAp     = "Pa";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "pascal";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pascals";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Ba"; // barye
  ppl_unit_database[ppl_unit_pos].nameAp     = "Ba";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "barye";
  ppl_unit_database[ppl_unit_pos].nameFp     = "baryes";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.1;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "atm"; // atmosphere
  ppl_unit_database[ppl_unit_pos].nameAp     = "atms";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "atmosphere";
  ppl_unit_database[ppl_unit_pos].nameFp     = "atmospheres";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_STD_ATMOSPHERE;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bar"; // bar
  ppl_unit_database[ppl_unit_pos].nameAp     = "bars";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "bar";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bars";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e5;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "psi"; // psi
  ppl_unit_database[ppl_unit_pos].nameAp     = "psi";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "pound_per_square_inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pounds_per_square_inch";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_PSI;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "inHg"; // inch of mercury
  ppl_unit_database[ppl_unit_pos].nameAp     = "inHg";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "inch_of_mercury";
  ppl_unit_database[ppl_unit_pos].nameFp     = "inches_of_mercury";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_INCH_OF_MERCURY;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "inAq"; // inch of water
  ppl_unit_database[ppl_unit_pos].nameAp     = "inAq";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "inch_of_water";
  ppl_unit_database[ppl_unit_pos].nameFp     = "inches_of_water";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_INCH_OF_WATER;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "J"; // joule
  ppl_unit_database[ppl_unit_pos].nameAp     = "J";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "joule";
  ppl_unit_database[ppl_unit_pos].nameFp     = "joules";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "eV"; // electronvolt
  ppl_unit_database[ppl_unit_pos].nameAp     = "eV";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "electronvolt";
  ppl_unit_database[ppl_unit_pos].nameFp     = "electronvolts";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_ELECTRON_VOLT;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "BeV"; // billion electronvolts
  ppl_unit_database[ppl_unit_pos].nameAp     = "BeV";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "billion_electronvolts";
  ppl_unit_database[ppl_unit_pos].nameFp     = "billion_electronvolts";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e9 * GSL_CONST_MKSA_ELECTRON_VOLT;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "erg"; // erg
  ppl_unit_database[ppl_unit_pos].nameAp     = "erg";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "erg";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ergs";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-7;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cal"; // calorie
  ppl_unit_database[ppl_unit_pos].nameAp     = "cal";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "calorie";
  ppl_unit_database[ppl_unit_pos].nameFp     = "calories";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_CALORIE;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  3;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kWh"; // Kilowatt hour
  ppl_unit_database[ppl_unit_pos].nameAp     = "kWh";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "kilowatt_hour";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kilowatt_hours";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 3.6e6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "BTU"; // British Thermal Unit
  ppl_unit_database[ppl_unit_pos].nameAp     = "BTU";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "British Thermal Unit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "British Thermal Units";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_BTU;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "therm"; // Therm
  ppl_unit_database[ppl_unit_pos].nameAp     = "therms";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "therm";
  ppl_unit_database[ppl_unit_pos].nameFp     = "therms";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_THERM;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "W"; // watt
  ppl_unit_database[ppl_unit_pos].nameAp     = "W";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "watt";
  ppl_unit_database[ppl_unit_pos].nameFp     = "watts";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "horsepower"; // horsepower
  ppl_unit_database[ppl_unit_pos].nameAp     = "horsepower";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "horsepower";
  ppl_unit_database[ppl_unit_pos].nameFp     = "horsepower";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_HORSEPOWER;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Lsun"; // Solar luminosity
  ppl_unit_database[ppl_unit_pos].nameAp     = "Lsun";
  ppl_unit_database[ppl_unit_pos].nameLs     = "L_\\odot";
  ppl_unit_database[ppl_unit_pos].nameLp     = "L_\\odot";
  ppl_unit_database[ppl_unit_pos].nameFs     = "solar_luminosity";
  ppl_unit_database[ppl_unit_pos].nameFp     = "solar_luminosities";
  ppl_unit_database[ppl_unit_pos].alt1       = "Lsolar";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = 3.839e26;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "clo"; // clo
  ppl_unit_database[ppl_unit_pos].nameAp     = "clos";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "clo";
  ppl_unit_database[ppl_unit_pos].nameFp     = "clos";
  ppl_unit_database[ppl_unit_pos].quantity   = "thermal_insulation";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.154;
  ppl_unit_database[ppl_unit_pos].TempType   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]        =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]        = 3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "tog"; // tog
  ppl_unit_database[ppl_unit_pos].nameAp     = "togs";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "tog";
  ppl_unit_database[ppl_unit_pos].nameFp     = "togs";
  ppl_unit_database[ppl_unit_pos].quantity   = "thermal_insulation";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.1;
  ppl_unit_database[ppl_unit_pos].TempType   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]        =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]        = 3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Gy"; // gray
  ppl_unit_database[ppl_unit_pos].nameAp     = "Gy";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "gray";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gray";
  ppl_unit_database[ppl_unit_pos].quantity   = "radiation_dose";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Sv"; // sievert
  ppl_unit_database[ppl_unit_pos].nameAp     = "Sv";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "sievert";
  ppl_unit_database[ppl_unit_pos].nameFp     = "sieverts";
  ppl_unit_database[ppl_unit_pos].quantity   = "radiation_dose";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kat"; // katal
  ppl_unit_database[ppl_unit_pos].nameAp     = "kat";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "katal";
  ppl_unit_database[ppl_unit_pos].nameFp     = "katals";
  ppl_unit_database[ppl_unit_pos].quantity   = "catalytic_activity";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "P"; // poise
  ppl_unit_database[ppl_unit_pos].nameAp     = "P";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "poise";
  ppl_unit_database[ppl_unit_pos].nameFp     = "poises";
  ppl_unit_database[ppl_unit_pos].quantity   = "viscosity";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_POISE;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kayser"; // kayser
  ppl_unit_database[ppl_unit_pos].nameAp     = "kaysers";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "kayser";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kaysers";
  ppl_unit_database[ppl_unit_pos].quantity   = "wavenumber";
  ppl_unit_database[ppl_unit_pos].multiplier = 100;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "C"; // coulomb
  ppl_unit_database[ppl_unit_pos].nameAp     = "C";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "coulomb";
  ppl_unit_database[ppl_unit_pos].nameFp     = "coulombs";
  ppl_unit_database[ppl_unit_pos].quantity   = "charge";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] = 1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "V"; // volt
  ppl_unit_database[ppl_unit_pos].nameAp     = "V";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "volt";
  ppl_unit_database[ppl_unit_pos].nameFp     = "volts";
  ppl_unit_database[ppl_unit_pos].quantity   = "potential";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ohm"; // ohm
  ppl_unit_database[ppl_unit_pos].nameAp     = "ohms";
  ppl_unit_database[ppl_unit_pos].nameLs     = "\\Omega";
  ppl_unit_database[ppl_unit_pos].nameLp     = "\\Omega";
  ppl_unit_database[ppl_unit_pos].nameFs     = "ohm";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ohms";
  ppl_unit_database[ppl_unit_pos].quantity   = "resistance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "S"; // siemens
  ppl_unit_database[ppl_unit_pos].nameAp     = "S";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "siemens";
  ppl_unit_database[ppl_unit_pos].nameFp     = "siemens";
  ppl_unit_database[ppl_unit_pos].quantity   = "conductance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    = 3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] = 2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mho"; // mho
  ppl_unit_database[ppl_unit_pos].nameAp     = "mhos";
  ppl_unit_database[ppl_unit_pos].nameLs     = "\\mho";
  ppl_unit_database[ppl_unit_pos].nameLp     = "\\mho";
  ppl_unit_database[ppl_unit_pos].nameFs     = "mho";
  ppl_unit_database[ppl_unit_pos].nameFp     = "mhos";
  ppl_unit_database[ppl_unit_pos].quantity   = "conductance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    = 3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] = 2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "F"; // farad
  ppl_unit_database[ppl_unit_pos].nameAp     = "F";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "farad";
  ppl_unit_database[ppl_unit_pos].nameFp     = "farad";
  ppl_unit_database[ppl_unit_pos].quantity   = "capacitance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    = 4;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] = 2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "H"; // henry
  ppl_unit_database[ppl_unit_pos].nameAp     = "H";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "henry";
  ppl_unit_database[ppl_unit_pos].nameFp     = "henry";
  ppl_unit_database[ppl_unit_pos].quantity   = "inductance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "T"; // tesla
  ppl_unit_database[ppl_unit_pos].nameAp     = "T";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "tesla";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tesla";
  ppl_unit_database[ppl_unit_pos].quantity   = "magnetic_field";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "G"; // gauss
  ppl_unit_database[ppl_unit_pos].nameAp     = "G";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "gauss";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gauss";
  ppl_unit_database[ppl_unit_pos].quantity   = "magnetic_field";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-4;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Wb"; // weber
  ppl_unit_database[ppl_unit_pos].nameAp     = "Wb";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "weber";
  ppl_unit_database[ppl_unit_pos].nameFp     = "weber";
  ppl_unit_database[ppl_unit_pos].quantity   = "magnetic_flux";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mx"; // maxwell
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mx";
  ppl_unit_database[ppl_unit_pos].nameLs     = ppl_unit_database[ppl_unit_pos].nameAs;
  ppl_unit_database[ppl_unit_pos].nameLp     = ppl_unit_database[ppl_unit_pos].nameAp;
  ppl_unit_database[ppl_unit_pos].nameFs     = "maxwell";
  ppl_unit_database[ppl_unit_pos].nameFp     = "maxwell";
  ppl_unit_database[ppl_unit_pos].quantity   = "magnetic_flux";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-8;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  // Planck Units
  ppl_unit_database[ppl_unit_pos].nameAs     = "L_planck"; // Planck Length
  ppl_unit_database[ppl_unit_pos].nameAp     = "L_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "L_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "L_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_length";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_lengths";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.61625281e-35;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "M_planck"; // Planck Mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "M_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "M_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "M_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_masses";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 2.1764411e-8;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "T_planck"; // Planck Time
  ppl_unit_database[ppl_unit_pos].nameAp     = "T_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "T_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "T_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_time";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_times";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 5.3912427e-44;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Q_planck"; // Planck Charge
  ppl_unit_database[ppl_unit_pos].nameAp     = "Q_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "Q_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "Q_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_charge";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_charges";
  ppl_unit_database[ppl_unit_pos].quantity   = "charge";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.87554587047e-18;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Theta_planck"; // Planck Temperature
  ppl_unit_database[ppl_unit_pos].nameAp     = "Theta_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "\\Theta_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "\\Theta_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_temperature";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_temperature";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.41678571e32;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "p_planck"; // Planck Momentum
  ppl_unit_database[ppl_unit_pos].nameAp     = "p_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "p_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "p_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_momentum";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_momentum";
  ppl_unit_database[ppl_unit_pos].quantity   = "momentum";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 6.5248018674330712229902929;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]  = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "E_planck"; // Planck Energy
  ppl_unit_database[ppl_unit_pos].nameAp     = "E_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "E_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "E_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_energy";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_energy";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1956085069.7617356777191162109375;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]  = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "F_planck"; // Planck Force
  ppl_unit_database[ppl_unit_pos].nameAp     = "F_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "F_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "F_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_force";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_force";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.2102593465942594902618273e+44;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]  = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "P_planck"; // Planck Power
  ppl_unit_database[ppl_unit_pos].nameAp     = "P_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "P_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "P_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_power";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_power";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 3.6282637948422090509290910e+52;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]  = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "I_planck"; // Planck Current
  ppl_unit_database[ppl_unit_pos].nameAp     = "I_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "I_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "I_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_current";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_current";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 3.4788748621352181073707008e+25;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]= 1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "V_planck"; // Planck Voltage
  ppl_unit_database[ppl_unit_pos].nameAp     = "V_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "V_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "V_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_voltage";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_voltage";
  ppl_unit_database[ppl_unit_pos].quantity   = "potential";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0429417379546963190438953e+27;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Z_planck"; // Planck Impedence
  ppl_unit_database[ppl_unit_pos].nameAp     = "Z_planck";
  ppl_unit_database[ppl_unit_pos].nameLs     = "Z_P";
  ppl_unit_database[ppl_unit_pos].nameLp     = "Z_P";
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_impedence";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_impedence";
  ppl_unit_database[ppl_unit_pos].quantity   = "resistance";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 2.9979282937316497736901511e+01;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-2;
  ppl_unit_pos++;

  if (DEBUG) { sprintf(temp_err_string, "%d system default units loaded.", ppl_unit_pos); ppl_log(temp_err_string); }

  return;
 }

