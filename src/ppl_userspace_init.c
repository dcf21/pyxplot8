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
#include <gsl/gsl_sf.h>

#include "MathsTools/dcfmath.h"

#include "ListTools/lt_memory.h"

#include "ppl_error.h"
#include "ppl_units.h"
#include "ppl_userspace.h"

void ppl_UserSpaceInit()
 {
  value v;

  _ppl_UserSpace_Vars  = DictInit();
  _ppl_UserSpace_Funcs = DictInit();

  // Function descriptors for the mathematical functions which are built into PyXPlot
  FunctionDescriptor fd_abs           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_abs         , NULL, NULL, NULL, NULL, NULL, NULL, "abs(x) returns the absolute magnitude of x"};
  FunctionDescriptor fd_acos          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acos        , NULL, NULL, NULL, NULL, NULL, NULL, "acos(x) returns the arccosine of x"};
  FunctionDescriptor fd_asin          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_asin        , NULL, NULL, NULL, NULL, NULL, NULL, "asin(x) returns the arcsine of x"};
  FunctionDescriptor fd_atan          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_atan        , NULL, NULL, NULL, NULL, NULL, NULL, "atan(x) returns the arctangent of x"};
  FunctionDescriptor fd_atan2         = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_atan2       , NULL, NULL, NULL, NULL, NULL, NULL, "atan2(x,y) returns the arctangent of x/y. Unlike atan(y/x), atan2(x,y) takes account of the signs of both x and y to remove the degeneracy between (1,1) and (-1,-1)"};
  FunctionDescriptor fd_besseli       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besseli     , NULL, NULL, NULL, NULL, NULL, NULL, "besseli(l,x) evaluates the lth regular modified spherical Bessel function at x"};
  FunctionDescriptor fd_besselI       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselI     , NULL, NULL, NULL, NULL, NULL, NULL, "besselI(l,x) evaluates the lth regular modified cylindrical Bessel function at x"};
  FunctionDescriptor fd_besselj       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselj     , NULL, NULL, NULL, NULL, NULL, NULL, "besselj(l,x) evaluates the lth regular spherical Bessel function at x"};
  FunctionDescriptor fd_besselJ       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselJ     , NULL, NULL, NULL, NULL, NULL, NULL, "besselJ(l,x) evaluates the lth regular cylindrical Bessel function at x"};
  FunctionDescriptor fd_besselk       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselk     , NULL, NULL, NULL, NULL, NULL, NULL, "besselk(l,x) evaluates the lth irregular modified spherical Bessel function at x"};
  FunctionDescriptor fd_besselK       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselK     , NULL, NULL, NULL, NULL, NULL, NULL, "besselK(l,x) evaluates the lth irregular modified cylindrical Bessel function at x"};
  FunctionDescriptor fd_bessely       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_bessely     , NULL, NULL, NULL, NULL, NULL, NULL, "bessely(l,x) evaluates the lth irregular spherical Bessel function at x"};
  FunctionDescriptor fd_besselY       = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselY     , NULL, NULL, NULL, NULL, NULL, NULL, "besselY(l,x) evaluates the lth irregular cylindrical Bessel function at x"};
  FunctionDescriptor fd_beta          = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_beta        , NULL, NULL, NULL, NULL, NULL, NULL, "beta(a,b) evaluates the beta function B(a,b)"};
  FunctionDescriptor fd_binomialPDF   = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_binomialPDF , NULL, NULL, NULL, NULL, NULL, NULL, "binomialPDF(k,p,n) evaulates the probability of getting k successes out of n trials in a binomial distribution with success probability p"};
  FunctionDescriptor fd_binomialCDF   = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_binomialCDF , NULL, NULL, NULL, NULL, NULL, NULL, "binomialCDF(k,p,n) evaulates the probability of getting fewer than or exactly k successes out of n trials in a binomial distribution with success probability p"};
  FunctionDescriptor fd_Bv            = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_planck_Bv   , NULL, NULL, NULL, NULL, NULL, NULL, "Bv(nu,T) returns the emitted power of a blackbody of temperature T per unit area, per unit solid angle, and per unit frequency"};
  FunctionDescriptor fd_Bvmax         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_planck_Bvmax, NULL, NULL, NULL, NULL, NULL, NULL, "Bvmax(T) returns the frequency of the maximum of the function Bv(nu,T)"};
  FunctionDescriptor fd_ceil          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ceil        , NULL, NULL, NULL, NULL, NULL, NULL, "ceil(x) returns the smallest integer value greater than or equal to x"};
  FunctionDescriptor fd_chisqPDF      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_chisqPDF    , NULL, NULL, NULL, NULL, NULL, NULL, "chisqPDF(x,nu) returns the probability desity at x in a chi-squared distribution with nu degrees of freedom"};
  FunctionDescriptor fd_chisqCDF      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_chisqCDF    , NULL, NULL, NULL, NULL, NULL, NULL, "chisqCDF(x,nu) returns the cumulative probability desity at x in a chi-squared distribution with nu degrees of freedom"};
  FunctionDescriptor fd_chisqCDFi     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_chisqCDFi   , NULL, NULL, NULL, NULL, NULL, NULL, "chisqCDFi(P,nu) returns the point x at which the cumulative probability desity in a chi-squared distribution with nu degrees of freedom is P"};
  FunctionDescriptor fd_cos           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cos         , NULL, NULL, NULL, NULL, NULL, NULL, "cos(x) returns the cosine of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_cosh          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cosh        , NULL, NULL, NULL, NULL, NULL, NULL, "cosh(x) returns the hyperbolic cosine of x"};
  FunctionDescriptor fd_degrees       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_degrees     , NULL, NULL, NULL, NULL, NULL, NULL, "degrees(x) converts angles measured in radians into degrees"};
  FunctionDescriptor fd_ellK          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ellK        , NULL, NULL, NULL, NULL, NULL, NULL, "ellipticintK(k) evaluates the complete elliptic integral K(k)"};
  FunctionDescriptor fd_ellE          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ellE        , NULL, NULL, NULL, NULL, NULL, NULL, "ellipticintE(k) evaluates the complete elliptic integral E(k)"};
  FunctionDescriptor fd_ellP          = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_ellP        , NULL, NULL, NULL, NULL, NULL, NULL, "ellipticintP(k,n) evaluates the complete elliptic integral P(k,n)"};
  FunctionDescriptor fd_erf           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_erf         , NULL, NULL, NULL, NULL, NULL, NULL, "erf(x) evaluates the error function at x"};
  FunctionDescriptor fd_erfc          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_erfc        , NULL, NULL, NULL, NULL, NULL, NULL, "erfc(x) evaluates the complimentary error function at x"};
  FunctionDescriptor fd_exp           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_exp         , NULL, NULL, NULL, NULL, NULL, NULL, "exp(x) returns e to the power of x"};
  FunctionDescriptor fd_expm1         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_expm1       , NULL, NULL, NULL, NULL, NULL, NULL, "expm1(x) accurately evaluates exp(x)-1"};
  FunctionDescriptor fd_expint        = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_expint      , NULL, NULL, NULL, NULL, NULL, NULL, "expint(n,x) evaluates the integral of exp(-xt)/t**n between one and infinity"};
  FunctionDescriptor fd_floor         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_floor       , NULL, NULL, NULL, NULL, NULL, NULL, "floor(x) returns the largest integer value smaller than or equal to x"};
  FunctionDescriptor fd_gamma         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_gamma       , NULL, NULL, NULL, NULL, NULL, NULL, "gamma(x) evaluates the gamma function at x"};
  FunctionDescriptor fd_gaussianPDF   = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_gaussianPDF , NULL, NULL, NULL, NULL, NULL, NULL, "gaussianPDF(x,sigma) evaluates the Gaussian probability density function of standard deviation sigma at x"};
  FunctionDescriptor fd_gaussianCDF   = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_gaussianCDF , NULL, NULL, NULL, NULL, NULL, NULL, "gaussianCDF(x,sigma) evaluates the Gaussian cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_gaussianCDFi  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_gaussianCDFi, NULL, NULL, NULL, NULL, NULL, NULL, "gaussianCDFi(x,sigma) evaluates the inverse Gaussian cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_heaviside     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_heaviside   , NULL, NULL, NULL, NULL, NULL, NULL, "heaviside(x) returns the Heaviside function, defined to be one for x>=0 and zero otherwise"};
  FunctionDescriptor fd_hypot         = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_hypot       , NULL, NULL, NULL, NULL, NULL, NULL, "hypot(x,y) returns the quadrature sum of x and y"};
  FunctionDescriptor fd_ldexp         = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_ldexp       , NULL, NULL, NULL, NULL, NULL, NULL, "ldexp(x,y) returns x times 2 to the power of an integer y"};
  FunctionDescriptor fd_legendreP     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_legendreP   , NULL, NULL, NULL, NULL, NULL, NULL, "legendreP(l,x) evaluates the lth Legendre polynomial at x"};
  FunctionDescriptor fd_legendreQ     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_legendreQ   , NULL, NULL, NULL, NULL, NULL, NULL, "legendreQ(l,x) evaluates the lth Legendre function at x"};
  FunctionDescriptor fd_log           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log         , NULL, NULL, NULL, NULL, NULL, NULL, "log(x) returns the natural logarithm of x"};
  FunctionDescriptor fd_log10         = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log10       , NULL, NULL, NULL, NULL, NULL, NULL, "log10(x) returns the logarithm to base 10 of x"};
  FunctionDescriptor fd_lognormalPDF  = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_lognormalPDF, NULL, NULL, NULL, NULL, NULL, NULL, "lognormalPDF(x,sigma) evaluates the log normal probability density function of standard deviation sigma at x"};
  FunctionDescriptor fd_lognormalCDF  = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_lognormalCDF, NULL, NULL, NULL, NULL, NULL, NULL, "lognormalCDF(x,sigma) evaluates the log normal cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_lognormalCDFi = { PPL_USERSPACE_SYSTEM , 0 , 3 , (void *)&dcfmath_lognormalCDFi,NULL, NULL, NULL, NULL, NULL, NULL, "lognormalCDFi(x,sigma) evaluates the inverse log normal cumulative distribution function of standard deviation sigma at x"};
  FunctionDescriptor fd_max           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_max         , NULL, NULL, NULL, NULL, NULL, NULL, "max(x,y) returns the greater of the two values x and y"};
  FunctionDescriptor fd_min           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_min         , NULL, NULL, NULL, NULL, NULL, NULL, "min(x,y) returns the lesser of the two values x and y"};
  FunctionDescriptor fd_mod           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_mod         , NULL, NULL, NULL, NULL, NULL, NULL, "mod(x,y) returns the remainder of x/y"};
  FunctionDescriptor fd_poissonPDF    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_poissonPDF  , NULL, NULL, NULL, NULL, NULL, NULL, "poissonPDF(x,mu) returns the probability of getting x from a Poisson distribution with mean mu"};
  FunctionDescriptor fd_poissonCDF    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_poissonCDF  , NULL, NULL, NULL, NULL, NULL, NULL, "poissonCDF(x,mu) returns the probability of getting <= x from a Poisson distribution with mean mu"};
  FunctionDescriptor fd_pow           = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_pow         , NULL, NULL, NULL, NULL, NULL, NULL, "pow(x,y) returns x to the power of y"};
  FunctionDescriptor fd_radians       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_radians     , NULL, NULL, NULL, NULL, NULL, NULL, "radians(x) converts angles measured in degrees into radians"};
  FunctionDescriptor fd_random        = { PPL_USERSPACE_SYSTEM , 0 , 0 , (void *)&dcfmath_frandom     , NULL, NULL, NULL, NULL, NULL, NULL, "random(x) returns a random number between 0 and 1"};
  FunctionDescriptor fd_randombin     = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_frandombin  , NULL, NULL, NULL, NULL, NULL, NULL, "randomBinomial(p,n) returns a random sample from a binomial distribution with n independent trials and a success probability p"};
  FunctionDescriptor fd_randomcs      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_frandomcs   , NULL, NULL, NULL, NULL, NULL, NULL, "randomChiSq(mu) returns a random sample from a chi-squared distribution with mu degrees of freedom"};
  FunctionDescriptor fd_randomg       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_frandomg    , NULL, NULL, NULL, NULL, NULL, NULL, "randomGaussian(sigma) returns a random sample from a Gaussian (normal) distribution of standard deviation sigma"};
  FunctionDescriptor fd_randomln      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_frandomln   , NULL, NULL, NULL, NULL, NULL, NULL, "randomLogNormal(zeta,sigma) returns a random sample from the log normal distribution centred on zeta, and of width sigma"};
  FunctionDescriptor fd_randomp       = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_frandomp    , NULL, NULL, NULL, NULL, NULL, NULL, "randomPoisson(n) returns a random integer from a Poisson distribution with mean n"};
  FunctionDescriptor fd_sin           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sin         , NULL, NULL, NULL, NULL, NULL, NULL, "sin(x) returns the sine of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_sinh          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sinh        , NULL, NULL, NULL, NULL, NULL, NULL, "sinh(x) returns the hyperbolic sine of x"};
  FunctionDescriptor fd_sqrt          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sqrt        , NULL, NULL, NULL, NULL, NULL, NULL, "sqrt(x) returns the square root of x"};
  FunctionDescriptor fd_tan           = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_tan         , NULL, NULL, NULL, NULL, NULL, NULL, "tan(x) returns the tangent of x. If x is dimensionless, it is assumed to be measured in radians"};
  FunctionDescriptor fd_tanh          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_tanh        , NULL, NULL, NULL, NULL, NULL, NULL, "tanh(x) returns the hyperbolic tangent of x"};
  FunctionDescriptor fd_tophat        = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_tophat      , NULL, NULL, NULL, NULL, NULL, NULL, "tophat(x,sigma) returns one if |x| <= |sigma|, and zero otherwise"};
  FunctionDescriptor fd_unit          = { PPL_USERSPACE_UNIT   , 0 ,-1 , NULL                         , NULL, NULL, NULL, NULL, NULL, NULL, "unit(...) multiplies a number by a physical unit"};
  FunctionDescriptor fd_zeta          = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_zeta        , NULL, NULL, NULL, NULL, NULL, NULL, "zeta(x) evaluates the Riemann zeta function at x"};
  FunctionDescriptor fd_int           = { PPL_USERSPACE_INT    , 0 , 3 , NULL                         , NULL, NULL, NULL, NULL, NULL, NULL, "int_dx(e,min,max) numerically integrates an expression e wrt x between min and max"};
  FunctionDescriptor fd_diff          = { PPL_USERSPACE_INT    , 0 , 3 , NULL                         , NULL, NULL, NULL, NULL, NULL, NULL, "diff_da(e,x,step) numerically differentiates an expression e wrt a at x, using a step size of step. 'a' can be any variable name with fewer than 16 characters."};

  // Set up default variables
  ppl_units_zero(&v);
  v.number = M_PI;
  DictAppendValue(_ppl_UserSpace_Vars , "pi"            , v); // pi
  v.number = M_E;
  DictAppendValue(_ppl_UserSpace_Vars , "e"             , v); // e
  v.number = GSL_CONST_MKSA_SPEED_OF_LIGHT;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH]=1 ; v.exponent[UNIT_TIME]=-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_c"         , v); // Speed of light
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_VACUUM_PERMEABILITY;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1; v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] = -2; v.exponent[UNIT_CURRENT] = -2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_0"      , v); // The permeability of free space
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_VACUUM_PERMITTIVITY;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] =-3; v.exponent[UNIT_MASS] =-1; v.exponent[UNIT_TIME] =  4; v.exponent[UNIT_CURRENT] =  2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_epsilon_0" , v); // The permittivity of free space
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_ELECTRON_CHARGE;
  v.dimensionless = 0;
  v.exponent[UNIT_CURRENT] = 1; v.exponent[UNIT_TIME] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_q"         , v); // The fundamental charge
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_PLANCKS_CONSTANT_H;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_h"         , v); // The Planck constant
  v.number = GSL_CONST_MKSA_PLANCKS_CONSTANT_HBAR;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_hbar"      , v); // The Planck constant / 2pi
  ppl_units_zero(&v);
  v.number = GSL_CONST_NUM_AVOGADRO;
  v.dimensionless = 0;
  v.exponent[UNIT_MOLE] = -1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_NA"        , v); // The Avogadro constant
  ppl_units_zero(&v);
  v.number = 3.839e26;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-3;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Lsun"      , v); // The solar luminosity
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_UNIFIED_ATOMIC_MASS;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_u"       , v); // The universal mass constant
  v.number = GSL_CONST_MKSA_MASS_PROTON;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_p"       , v); // The proton mass
  v.number = GSL_CONST_MKSA_MASS_NEUTRON;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_n"       , v); // The neutron mass
  v.number = GSL_CONST_MKSA_SOLAR_MASS;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Msun"      , v); // The solar mass
  ppl_units_zero(&v);
  v.number = 6.955e8;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Rsun"      , v); // The solar radius
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_BOHR_MAGNETON;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_CURRENT] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_b"      , v); // The Bohr magneton
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_MOLAR_GAS;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1; v.exponent[UNIT_MOLE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_R"         , v); // The gas constant
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_BOLTZMANN;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_kB"        , v); // The Boltzmann constant
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2;
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_STEFAN_BOLTZMANN_CONSTANT;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] =-3; v.exponent[UNIT_TEMPERATURE] =-4;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_sigma"     , v); // The Stefan-Boltzmann constant
  ppl_units_zero(&v);
  v.number = GSL_CONST_MKSA_GRAVITATIONAL_CONSTANT;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 3; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_MASS] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_G"         , v); // The gravitational constant

  // Copy function descriptors for core mathematical functions into function namespace
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "abs"            , (void *)&fd_abs         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acos"           , (void *)&fd_acos        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "asin"           , (void *)&fd_asin        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "atan"           , (void *)&fd_atan        , sizeof(FunctionDescriptor), DATATYPE_VOID);
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
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cos"            , (void *)&fd_cos         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cosh"           , (void *)&fd_cosh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
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
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ldexp"          , (void *)&fd_ldexp       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "legendreP"      , (void *)&fd_legendreP   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "legendreQ"      , (void *)&fd_legendreQ   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "log"            , (void *)&fd_log         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "log10"          , (void *)&fd_log10       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "lognormalPDF"   , (void *)&fd_lognormalPDF, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "lognormalCDF"   , (void *)&fd_lognormalCDF, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "lognormalCDFi"  , (void *)&fd_lognormalCDFi,sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "max"            , (void *)&fd_max         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "min"            , (void *)&fd_min         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "mod"            , (void *)&fd_mod         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "poissonPDF"     , (void *)&fd_poissonPDF  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "poissonCDF"     , (void *)&fd_poissonCDF  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "pow"            , (void *)&fd_pow         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "radians"        , (void *)&fd_radians     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random"         , (void *)&fd_random      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "randomBinomial" , (void *)&fd_randombin   , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "randomChiSq"    , (void *)&fd_randomcs    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "randomGaussian" , (void *)&fd_randomg     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "randomLogNormal", (void *)&fd_randomln    , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "randomPoisson"  , (void *)&fd_randomp     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sin"            , (void *)&fd_sin         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sinh"           , (void *)&fd_sinh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sqrt"           , (void *)&fd_sqrt        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tan"            , (void *)&fd_tan         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tanh"           , (void *)&fd_tanh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
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

  // Set up database of known units
  for (i=0;i<UNITS_MAX;i++)
   {
    ppl_unit_database[i].nameAs     = NULL;
    ppl_unit_database[i].nameAp     = NULL;
    ppl_unit_database[i].nameFs     = NULL;
    ppl_unit_database[i].nameFp     = NULL;
    ppl_unit_database[i].quantity   = NULL;
    ppl_unit_database[i].comment    = NULL;
    ppl_unit_database[i].multiplier = 1.0;
    ppl_unit_database[i].offset     = 0.0;
    ppl_unit_database[i].UserSel    = 0;
    ppl_unit_database[i].si         = ppl_unit_database[i].imperial  = ppl_unit_database[i].us = ppl_unit_database[i].planck =
    ppl_unit_database[i].ancient    = ppl_unit_database[i].modified  = 0;
    ppl_unit_database[i].MaxPrefix  = ppl_unit_database[i].MinPrefix = 0;
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[i].exponent[j] = 0;
   }

  // Set up default list of units
  ppl_unit_database[ppl_unit_pos].nameAs     = "percent"; // Percent
  ppl_unit_database[ppl_unit_pos].nameAp     = "percent"; // Percent
  ppl_unit_database[ppl_unit_pos].nameFs     = "percent";
  ppl_unit_database[ppl_unit_pos].nameFp     = "percent";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.01;
  ppl_unit_database[ppl_unit_pos].quantity   = "dimensionlessness";
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ppm"; // Parts per million
  ppl_unit_database[ppl_unit_pos].nameAp     = "ppm"; // Parts per million
  ppl_unit_database[ppl_unit_pos].nameFs     = "parts_per_million";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parts_per_million";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-6;
  ppl_unit_database[ppl_unit_pos].quantity   = "dimensionlessness";
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ppb"; // Parts per billion
  ppl_unit_database[ppl_unit_pos].nameAp     = "ppb"; // Parts per billion
  ppl_unit_database[ppl_unit_pos].nameFs     = "parts_per_billion";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parts_per_billion";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-9;
  ppl_unit_database[ppl_unit_pos].quantity   = "dimensionlessness";
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "m";    // Metre
  ppl_unit_database[ppl_unit_pos].nameAp     = "m";    // Metre
  ppl_unit_database[ppl_unit_pos].nameFs     = "metre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cm";    // Centimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "cm";    // Centimetre
  ppl_unit_database[ppl_unit_pos].nameFs     = "centimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ang";  // Angstrom
  ppl_unit_database[ppl_unit_pos].nameAp     = "ang";  // Angstrom
  ppl_unit_database[ppl_unit_pos].nameFs     = "angstrom";
  ppl_unit_database[ppl_unit_pos].nameFp     = "angstroms";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-10;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "in";   // Inch
  ppl_unit_database[ppl_unit_pos].nameAp     = "in";   // Inch
  ppl_unit_database[ppl_unit_pos].nameFs     = "inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_INCH;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ft";   // Foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "ft";   // Foot
  ppl_unit_database[ppl_unit_pos].nameFs     = "foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_FOOT;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "yd";   // Yard
  ppl_unit_database[ppl_unit_pos].nameAp     = "yd";   // Yard
  ppl_unit_database[ppl_unit_pos].nameFs     = "yard";
  ppl_unit_database[ppl_unit_pos].nameFp     = "yards";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_YARD;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mi";   // Mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "mi";   // Mile
  ppl_unit_database[ppl_unit_pos].nameFs     = "mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_MILE;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;
  
  ppl_unit_database[ppl_unit_pos].nameAs     = "furlong"; // Furlong
  ppl_unit_database[ppl_unit_pos].nameAp     = "furlongs";// Furlong
  ppl_unit_database[ppl_unit_pos].nameFs     = "furlong";
  ppl_unit_database[ppl_unit_pos].nameFp     = "furlongs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 201.168;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "chain"; // Chain
  ppl_unit_database[ppl_unit_pos].nameAp     = "chains";// Chain
  ppl_unit_database[ppl_unit_pos].nameFs     = "chain";
  ppl_unit_database[ppl_unit_pos].nameFp     = "chains";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 20.1168;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "link"; // Link
  ppl_unit_database[ppl_unit_pos].nameAp     = "links";// Link
  ppl_unit_database[ppl_unit_pos].nameFs     = "link";
  ppl_unit_database[ppl_unit_pos].nameFp     = "links";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.201168;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cable"; // Cable
  ppl_unit_database[ppl_unit_pos].nameAp     = "cables";// Cable
  ppl_unit_database[ppl_unit_pos].nameFs     = "cable";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cables";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 185.3184;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "fathom"; // Fathom
  ppl_unit_database[ppl_unit_pos].nameAp     = "fathoms";// Fathom
  ppl_unit_database[ppl_unit_pos].nameFs     = "fathom";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fathoms";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_FATHOM;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "league"; // League
  ppl_unit_database[ppl_unit_pos].nameAp     = "leagues";// League
  ppl_unit_database[ppl_unit_pos].nameFs     = "league";
  ppl_unit_database[ppl_unit_pos].nameFp     = "leagues";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 4828.032;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "nautical_mile"; // Nautical mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "nautical_miles";// Nautical mile
  ppl_unit_database[ppl_unit_pos].nameFs     = "nautical_mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "nautical_miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_NAUTICAL_MILE;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubit"; // Cubit
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubits";// Cubit
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubits";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.04572;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "rod"; // Rod
  ppl_unit_database[ppl_unit_pos].nameAp     = "rods";// Rod
  ppl_unit_database[ppl_unit_pos].nameFs     = "rod";
  ppl_unit_database[ppl_unit_pos].nameFp     = "rods";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.02920;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "perch";  // Perch
  ppl_unit_database[ppl_unit_pos].nameAp     = "perches";// Perch
  ppl_unit_database[ppl_unit_pos].nameFs     = "perch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "perches";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.02920;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pole"; // Pole
  ppl_unit_database[ppl_unit_pos].nameAp     = "poles";// Pole
  ppl_unit_database[ppl_unit_pos].nameFs     = "pole";
  ppl_unit_database[ppl_unit_pos].nameFp     = "poles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.02920;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "AU";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameAp     = "AU";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameFs     = "astronomical_unit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "astronomical_units";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_ASTRONOMICAL_UNIT;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lyr";   // Lightyear
  ppl_unit_database[ppl_unit_pos].nameAp     = "lyr";   // Lightyear
  ppl_unit_database[ppl_unit_pos].nameFs     = "light_year";
  ppl_unit_database[ppl_unit_pos].nameFp     = "light_years";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_LIGHT_YEAR;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pc";   // Parsec
  ppl_unit_database[ppl_unit_pos].nameAp     = "pc";   // Parsec
  ppl_unit_database[ppl_unit_pos].nameFs     = "parsec";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parsecs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_PARSEC;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Rsun";   // Solar radii
  ppl_unit_database[ppl_unit_pos].nameAp     = "Rsolar"; // Solar radii
  ppl_unit_database[ppl_unit_pos].nameFs     = "solar_radius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "solar_radii";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 6.955e8;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Rearth"; // Earth radii
  ppl_unit_database[ppl_unit_pos].nameAp     = "Rearth"; // Earth radii
  ppl_unit_database[ppl_unit_pos].nameFs     = "earth_radius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "earth_radii";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 6371000;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Rjove";   // Jupiter radii
  ppl_unit_database[ppl_unit_pos].nameAp     = "Rjovian"; // Jupiter radii
  ppl_unit_database[ppl_unit_pos].nameFs     = "jupiter_radius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "jupiter_radii";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 71492000;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lunar_distance";  // Lunar distances
  ppl_unit_database[ppl_unit_pos].nameAp     = "lunar_distances"; // Lunar distances
  ppl_unit_database[ppl_unit_pos].nameFs     = "lunar_distance";
  ppl_unit_database[ppl_unit_pos].nameFp     = "lunar_distances";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 384403000;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "s";    // Second
  ppl_unit_database[ppl_unit_pos].nameAp     = "s";    // Second
  ppl_unit_database[ppl_unit_pos].nameFs     = "second";
  ppl_unit_database[ppl_unit_pos].nameFp     = "seconds";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "min";  // Minute
  ppl_unit_database[ppl_unit_pos].nameAp     = "min";  // Minute
  ppl_unit_database[ppl_unit_pos].nameFs     = "minute";
  ppl_unit_database[ppl_unit_pos].nameFp     = "minutes";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 60;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "hr";   // Hour
  ppl_unit_database[ppl_unit_pos].nameAp     = "hr";   // Hour
  ppl_unit_database[ppl_unit_pos].nameFs     = "hour";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hours";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 3600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "day";  // Day
  ppl_unit_database[ppl_unit_pos].nameAp     = "days"; // Day
  ppl_unit_database[ppl_unit_pos].nameFs     = "day";
  ppl_unit_database[ppl_unit_pos].nameFp     = "days";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 86400;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "week"; // Week
  ppl_unit_database[ppl_unit_pos].nameAp     = "weeks";// Week
  ppl_unit_database[ppl_unit_pos].nameFs     = "week";
  ppl_unit_database[ppl_unit_pos].nameFp     = "weeks";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 604800;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "yr";   // Year
  ppl_unit_database[ppl_unit_pos].nameAp     = "yr";   // Year
  ppl_unit_database[ppl_unit_pos].nameFs     = "year";
  ppl_unit_database[ppl_unit_pos].nameFp     = "years";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 31557600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kg";   // Kilogram
  ppl_unit_database[ppl_unit_pos].nameAp     = "kg";   // Kilogram
  ppl_unit_database[ppl_unit_pos].nameFs     = "kilogram";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kilograms";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "g";    // Gramme
  ppl_unit_database[ppl_unit_pos].nameAp     = "g";    // Gramme
  ppl_unit_database[ppl_unit_pos].nameFs     = "gramme";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grammes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "g";    // Gram
  ppl_unit_database[ppl_unit_pos].nameAp     = "g";    // Gram
  ppl_unit_database[ppl_unit_pos].nameFs     = "gram";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grams";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].nameAp     = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].nameFs     = "metric_ton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "metric_tons";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].nameAp     = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].nameFs     = "tonne";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tonnes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "short_ton"; // US Ton
  ppl_unit_database[ppl_unit_pos].nameAp     = "short_tons";// US Ton
  ppl_unit_database[ppl_unit_pos].nameFs     = "ton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 907;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "long_ton";  // UK Ton
  ppl_unit_database[ppl_unit_pos].nameAp     = "long_tons"; // UK Ton
  ppl_unit_database[ppl_unit_pos].nameFs     = "ton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1016;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oz"; // Ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "oz"; // Ounce
  ppl_unit_database[ppl_unit_pos].nameFs     = "ounce";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ounces";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_OUNCE_MASS;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lb";  // Pound
  ppl_unit_database[ppl_unit_pos].nameAp     = "lbs"; // Pound
  ppl_unit_database[ppl_unit_pos].nameFs     = "pound";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pounds";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_POUND_MASS;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "stone"; // Stone
  ppl_unit_database[ppl_unit_pos].nameAp     = "stone"; // Stone
  ppl_unit_database[ppl_unit_pos].nameFs     = "stone";
  ppl_unit_database[ppl_unit_pos].nameFp     = "stone";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 6.35029318;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cwt"; // hundredweight
  ppl_unit_database[ppl_unit_pos].nameAp     = "cwt"; // hundredweight
  ppl_unit_database[ppl_unit_pos].nameFs     = "hundredweight";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hundredweight";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 50.80234544;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "drachm";  // drachm
  ppl_unit_database[ppl_unit_pos].nameAp     = "drachms"; // drachm
  ppl_unit_database[ppl_unit_pos].nameFs     = "drachm";
  ppl_unit_database[ppl_unit_pos].nameFp     = "drachms";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.7718451953125e-3;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "grain";  // grain
  ppl_unit_database[ppl_unit_pos].nameAp     = "grains"; // grain
  ppl_unit_database[ppl_unit_pos].nameFs     = "grain";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grains";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 64.79891e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "CD";  // carat
  ppl_unit_database[ppl_unit_pos].nameAp     = "CDs"; // carat
  ppl_unit_database[ppl_unit_pos].nameFs     = "carat";
  ppl_unit_database[ppl_unit_pos].nameFp     = "carats";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 200e-6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "shekel";  // Shekel
  ppl_unit_database[ppl_unit_pos].nameAp     = "shekels"; // Shekel
  ppl_unit_database[ppl_unit_pos].nameFs     = "shekel";
  ppl_unit_database[ppl_unit_pos].nameFp     = "shekels";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mina";  // Mina
  ppl_unit_database[ppl_unit_pos].nameAp     = "minas"; // Mina
  ppl_unit_database[ppl_unit_pos].nameFs     = "mina";
  ppl_unit_database[ppl_unit_pos].nameFp     = "minas";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011 * 60;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "talent";  // Talent
  ppl_unit_database[ppl_unit_pos].nameAp     = "talents"; // Talent
  ppl_unit_database[ppl_unit_pos].nameFs     = "talent";
  ppl_unit_database[ppl_unit_pos].nameFp     = "talents";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011 * 360;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Msun";   // Solar mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "Msolar"; // Solar mass
  ppl_unit_database[ppl_unit_pos].nameFs     = "solar_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "solar_masses";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_SOLAR_MASS;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mearth"; // Earth mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mearth"; // Earth mass
  ppl_unit_database[ppl_unit_pos].nameFs     = "earth_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "earth_masses";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 5.9742e24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mjove";    // Jupiter mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mjupiter"; // Jupiter mass
  ppl_unit_database[ppl_unit_pos].nameFs     = "jupiter_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "jupiter_masses";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.8986e27;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameAp     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameFs     = "ampere";
  ppl_unit_database[ppl_unit_pos].nameFp     = "amperes";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameAp     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameFs     = "amp";
  ppl_unit_database[ppl_unit_pos].nameFp     = "amps";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].nameAp     = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].nameFs     = "kelvin";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kelvin";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].nameAp     = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].nameFs     = "celsius";
  ppl_unit_database[ppl_unit_pos].nameFp     = "celsius";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].offset     = 273.15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].nameAp     = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].nameFs     = "centigrade";
  ppl_unit_database[ppl_unit_pos].nameFp     = "centigrade";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].offset     = 273.15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "oF"; // oF
  ppl_unit_database[ppl_unit_pos].nameAp     = "oF"; // oF
  ppl_unit_database[ppl_unit_pos].nameFs     = "fahrenheit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fahrenheit";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.8;
  ppl_unit_database[ppl_unit_pos].offset     = 459.67;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mol";  // mole
  ppl_unit_database[ppl_unit_pos].nameAp     = "mol";  // mole
  ppl_unit_database[ppl_unit_pos].nameFs     = "mole";
  ppl_unit_database[ppl_unit_pos].nameFp     = "moles";
  ppl_unit_database[ppl_unit_pos].quantity   = "moles";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cd";   // candela
  ppl_unit_database[ppl_unit_pos].nameAp     = "cd";   // candela
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "candlepower";   // candlepower
  ppl_unit_database[ppl_unit_pos].nameAp     = "candlepower";   // candlepower
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "lm";   // lumen
  ppl_unit_database[ppl_unit_pos].nameAp     = "lm";   // lumen
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "lx";   // lux
  ppl_unit_database[ppl_unit_pos].nameAp     = "lx";   // lux
  ppl_unit_database[ppl_unit_pos].nameFs     = "lux";
  ppl_unit_database[ppl_unit_pos].nameFp     = "luxs";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_LUX;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Jy";   // jansky
  ppl_unit_database[ppl_unit_pos].nameAp     = "Jy";   // jansky
  ppl_unit_database[ppl_unit_pos].nameFs     = "jansky";
  ppl_unit_database[ppl_unit_pos].nameFp     = "janskys";
  ppl_unit_database[ppl_unit_pos].quantity   = "flux_density";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-26;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1; // Watt per square metre per Hz (NOT per steradian!)
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "rad";  // radians
  ppl_unit_database[ppl_unit_pos].nameAp     = "rad";  // radians
  ppl_unit_database[ppl_unit_pos].nameFs     = "radian";
  ppl_unit_database[ppl_unit_pos].nameFp     = "radians";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "deg";  // degrees
  ppl_unit_database[ppl_unit_pos].nameAp     = "deg";  // degrees
  ppl_unit_database[ppl_unit_pos].nameFs     = "degree";
  ppl_unit_database[ppl_unit_pos].nameFp     = "degrees";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = M_PI / 180;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "arcmin";  // arcminute
  ppl_unit_database[ppl_unit_pos].nameAp     = "arcmins"; // arcminute
  ppl_unit_database[ppl_unit_pos].nameFs     = "arcminute";
  ppl_unit_database[ppl_unit_pos].nameFp     = "arcminutes";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = M_PI / 180 / 60;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "arcsec";  // arcsecond
  ppl_unit_database[ppl_unit_pos].nameAp     = "arcsecs"; // arcsecond
  ppl_unit_database[ppl_unit_pos].nameFs     = "arcsecond";
  ppl_unit_database[ppl_unit_pos].nameFp     = "arcseconds";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = M_PI / 180 / 3600;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bit";  // bit
  ppl_unit_database[ppl_unit_pos].nameAp     = "bits"; // bit
  ppl_unit_database[ppl_unit_pos].nameFs     = "bit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "euro";  // cost
  ppl_unit_database[ppl_unit_pos].nameAp     = "euros"; // cost
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "knot";  // knot
  ppl_unit_database[ppl_unit_pos].nameAp     = "knots"; // knot
  ppl_unit_database[ppl_unit_pos].nameFs     = "knot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "knots";
  ppl_unit_database[ppl_unit_pos].quantity   = "velocity";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_KNOT;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]= 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]  =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "acre";  // acre
  ppl_unit_database[ppl_unit_pos].nameAp     = "acres"; // acre
  ppl_unit_database[ppl_unit_pos].nameFs     = "acre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "acres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_ACRE;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "are";  // are
  ppl_unit_database[ppl_unit_pos].nameAp     = "ares"; // are
  ppl_unit_database[ppl_unit_pos].nameFs     = "are";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ares";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 100;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "barn";  // barn
  ppl_unit_database[ppl_unit_pos].nameAp     = "barns"; // barn
  ppl_unit_database[ppl_unit_pos].nameFs     = "barn";
  ppl_unit_database[ppl_unit_pos].nameFp     = "barns";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-28;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "hectare";  // hectare
  ppl_unit_database[ppl_unit_pos].nameAp     = "hectares"; // hectare
  ppl_unit_database[ppl_unit_pos].nameFs     = "hectare";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hectares";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e5;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_mi"; // square mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_mi"; // square mile
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_MILE,2);
  ppl_unit_database[ppl_unit_pos].imperial   = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_km"; // square kilometre
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_km"; // square kilometre
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_kilometre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_kilometres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e6;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_m"; // square metre
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_m"; // square metre
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_metre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_cm"; // square centimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_cm"; // square centimetre
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_centimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-4;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_ft"; // square foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_ft"; // square foot
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_FOOT,2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sq_in"; // square inch
  ppl_unit_database[ppl_unit_pos].nameAp     = "sq_in"; // square inch
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "area";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_INCH,2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_m"; // cubic metre
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_m"; // cubic metre
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_metre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_cm"; // cubic centimetre
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_cm"; // cubic centimetre
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_centimetre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-6;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_ft"; // cubic foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_ft"; // cubic foot
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_FOOT,3);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cubic_in"; // cubic inch
  ppl_unit_database[ppl_unit_pos].nameAp     = "cubic_in"; // cubic inch
  ppl_unit_database[ppl_unit_pos].nameFs     = "cubic_inch";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cubic_inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(GSL_CONST_MKSA_INCH,3);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "l"; // litre
  ppl_unit_database[ppl_unit_pos].nameAp     = "l"; // litre
  ppl_unit_database[ppl_unit_pos].nameFs     = "litre";
  ppl_unit_database[ppl_unit_pos].nameFp     = "litres";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "fl_oz_(UK)"; // fluid ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "fl_oz_(UK)"; // fluid ounce
  ppl_unit_database[ppl_unit_pos].nameFs     = "fluid_ounce_(UK)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fluid_ounce_(UK)";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 28.4130625e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "fl_oz_(US)"; // fluid ounce
  ppl_unit_database[ppl_unit_pos].nameAp     = "fl_oz_(US)"; // fluid ounce
  ppl_unit_database[ppl_unit_pos].nameFs     = "fluid_ounce_(US)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "fluid_ounce_(US)";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 29.5735295625e-6;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pint_(UK)";  // pint
  ppl_unit_database[ppl_unit_pos].nameAp     = "pints_(UK)"; // pint
  ppl_unit_database[ppl_unit_pos].nameFs     = "pint_(UK)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pints_(UK)";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 568.26125e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pint_(US)";  // pint
  ppl_unit_database[ppl_unit_pos].nameAp     = "pints_(US)"; // pint
  ppl_unit_database[ppl_unit_pos].nameFs     = "pint_(US)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pints_(US)";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 473.176473e-6;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "quart_(UK)";  // quart
  ppl_unit_database[ppl_unit_pos].nameAp     = "quarts_(UK)"; // quart
  ppl_unit_database[ppl_unit_pos].nameFs     = "quart_(UK)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "quarts_(UK)";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 1136.5225e-6;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "quart_(US)";  // quart
  ppl_unit_database[ppl_unit_pos].nameAp     = "quarts_(US)"; // quart
  ppl_unit_database[ppl_unit_pos].nameFs     = "quart_(US)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "quarts_(US)";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 946.352946e-6;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "gallon_(UK)";  // gallon
  ppl_unit_database[ppl_unit_pos].nameAp     = "gallons_(UK)"; // gallon
  ppl_unit_database[ppl_unit_pos].nameFs     = "gallon_(UK)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gallons_(UK)";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 4.54609e-3;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "gallon_(US)";  // gallon
  ppl_unit_database[ppl_unit_pos].nameAp     = "gallons_(US)"; // gallon
  ppl_unit_database[ppl_unit_pos].nameFs     = "gallon_(US)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gallons_(US)";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 3.785411784e-3;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bushel_(UK)";  // bushel
  ppl_unit_database[ppl_unit_pos].nameAp     = "bushels_(UK)"; // bushel
  ppl_unit_database[ppl_unit_pos].nameFs     = "bushel_(UK)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bushels_(UK)";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 36.36872e-3;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bushel_(US)";  // bushel
  ppl_unit_database[ppl_unit_pos].nameAp     = "bushels_(US)"; // bushel
  ppl_unit_database[ppl_unit_pos].nameFs     = "bushel_(US)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bushels_(US)";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 35.23907016688e-3;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cup_(US)";  // cup
  ppl_unit_database[ppl_unit_pos].nameAp     = "cups_(US)"; // cup
  ppl_unit_database[ppl_unit_pos].nameFs     = "cup_(US)";
  ppl_unit_database[ppl_unit_pos].nameFp     = "cups_(US)";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 236.5882365e-6;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "teaspoon";  // teaspoon
  ppl_unit_database[ppl_unit_pos].nameAp     = "teaspoons"; // teaspoon
  ppl_unit_database[ppl_unit_pos].nameFs     = "teaspoon";
  ppl_unit_database[ppl_unit_pos].nameFp     = "teaspoons";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 4.92892159375e-6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "tablespoon";  // tablespoon
  ppl_unit_database[ppl_unit_pos].nameAp     = "tablespoons"; // tablespoon
  ppl_unit_database[ppl_unit_pos].nameFs     = "tablespoon";
  ppl_unit_database[ppl_unit_pos].nameFp     = "tablespoons";
  ppl_unit_database[ppl_unit_pos].quantity   = "volume";
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -3;
  ppl_unit_database[ppl_unit_pos].multiplier = 4.92892159375e-6 * 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sterad";  // steradians
  ppl_unit_database[ppl_unit_pos].nameAp     = "sterad";  // steradians
  ppl_unit_database[ppl_unit_pos].nameFs     = "steradian";
  ppl_unit_database[ppl_unit_pos].nameFp     = "steradians";
  ppl_unit_database[ppl_unit_pos].quantity   = "solidangle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "sqdeg";  // square degrees
  ppl_unit_database[ppl_unit_pos].nameAp     = "sqdeg";  // square degrees
  ppl_unit_database[ppl_unit_pos].nameFs     = "square_degree";
  ppl_unit_database[ppl_unit_pos].nameFp     = "square_degrees";
  ppl_unit_database[ppl_unit_pos].quantity   = "solidangle";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(M_PI/180, 2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Hz";  // hertz
  ppl_unit_database[ppl_unit_pos].nameAp     = "Hz";  // hertz
  ppl_unit_database[ppl_unit_pos].nameFs     = "hertz";
  ppl_unit_database[ppl_unit_pos].nameFp     = "hertz";
  ppl_unit_database[ppl_unit_pos].quantity   = "frequency";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Bq";  // becquerel
  ppl_unit_database[ppl_unit_pos].nameAp     = "Bq";  // becquerel
  ppl_unit_database[ppl_unit_pos].nameFs     = "becquerel";
  ppl_unit_database[ppl_unit_pos].nameFp     = "becquerel";
  ppl_unit_database[ppl_unit_pos].quantity   = "frequency";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "B";  // bytes
  ppl_unit_database[ppl_unit_pos].nameAp     = "B";  // bytes
  ppl_unit_database[ppl_unit_pos].nameFs     = "byte";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 8.0;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "N";  // newton
  ppl_unit_database[ppl_unit_pos].nameAp     = "N";  // newton
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "dyn";  // dyne
  ppl_unit_database[ppl_unit_pos].nameAp     = "dyn";  // dyne
  ppl_unit_database[ppl_unit_pos].nameFs     = "dyne";
  ppl_unit_database[ppl_unit_pos].nameFp     = "dynes";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-5;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lbf";  // pound force
  ppl_unit_database[ppl_unit_pos].nameAp     = "lbf";  // pound force
  ppl_unit_database[ppl_unit_pos].nameFs     = "pound_force";
  ppl_unit_database[ppl_unit_pos].nameFp     = "pounds_force";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_POUND_FORCE;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Pa";  // pascal
  ppl_unit_database[ppl_unit_pos].nameAp     = "Pa";  // pascal
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "Ba";  // barye
  ppl_unit_database[ppl_unit_pos].nameAp     = "Ba";  // barye
  ppl_unit_database[ppl_unit_pos].nameFs     = "barye";
  ppl_unit_database[ppl_unit_pos].nameFp     = "baryes";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.1;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "atm";  // atmosphere
  ppl_unit_database[ppl_unit_pos].nameAp     = "atms"; // atmosphere
  ppl_unit_database[ppl_unit_pos].nameFs     = "atmosphere";
  ppl_unit_database[ppl_unit_pos].nameFp     = "atmospheres";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = 101325;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  =  24;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bar";  // bar
  ppl_unit_database[ppl_unit_pos].nameAp     = "bars"; // bar
  ppl_unit_database[ppl_unit_pos].nameFs     = "bar";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bars";
  ppl_unit_database[ppl_unit_pos].quantity   = "pressure";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e5;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "J";  // joule
  ppl_unit_database[ppl_unit_pos].nameAp     = "J";  // joule
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "eV";  // electronvolt
  ppl_unit_database[ppl_unit_pos].nameAp     = "eV";  // electronvolt
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "erg";  // erg
  ppl_unit_database[ppl_unit_pos].nameAp     = "erg";  // erg
  ppl_unit_database[ppl_unit_pos].nameFs     = "erg";
  ppl_unit_database[ppl_unit_pos].nameFp     = "ergs";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-7;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cal";  // calorie
  ppl_unit_database[ppl_unit_pos].nameAp     = "cal";  // calorie
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "BTU";  // British Thermal Unit
  ppl_unit_database[ppl_unit_pos].nameAp     = "BTU";  // British Thermal Unit
  ppl_unit_database[ppl_unit_pos].nameFs     = "British Thermal Unit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "British Thermal Units";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_BTU;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "therm";  // Therm
  ppl_unit_database[ppl_unit_pos].nameAp     = "therms"; // Therm
  ppl_unit_database[ppl_unit_pos].nameFs     = "therm";
  ppl_unit_database[ppl_unit_pos].nameFp     = "therms";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_THERM;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "W";  // watt
  ppl_unit_database[ppl_unit_pos].nameAp     = "W";  // watt
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "horsepower";  // horsepower
  ppl_unit_database[ppl_unit_pos].nameAp     = "horsepower";  // horsepower
  ppl_unit_database[ppl_unit_pos].nameFs     = "horsepower";
  ppl_unit_database[ppl_unit_pos].nameFp     = "horsepower";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_HORSEPOWER;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Lsun";    // Solar luminosity
  ppl_unit_database[ppl_unit_pos].nameAp     = "Lsolar";  // Solar luminosity
  ppl_unit_database[ppl_unit_pos].nameFs     = "solar_luminosity";
  ppl_unit_database[ppl_unit_pos].nameFp     = "solar_luminosities";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = 3.839e26;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Gy";  // gray
  ppl_unit_database[ppl_unit_pos].nameAp     = "Gy";  // gray
  ppl_unit_database[ppl_unit_pos].nameFs     = "gray";
  ppl_unit_database[ppl_unit_pos].nameFp     = "gray";
  ppl_unit_database[ppl_unit_pos].quantity   = "radiation_dose";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Sv";  // sievert
  ppl_unit_database[ppl_unit_pos].nameAp     = "Sv";  // sievert
  ppl_unit_database[ppl_unit_pos].nameFs     = "sievert";
  ppl_unit_database[ppl_unit_pos].nameFp     = "sieverts";
  ppl_unit_database[ppl_unit_pos].quantity   = "radiation_dose";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kat";  // katal
  ppl_unit_database[ppl_unit_pos].nameAp     = "kat";  // katal
  ppl_unit_database[ppl_unit_pos].nameFs     = "katal";
  ppl_unit_database[ppl_unit_pos].nameFp     = "katals";
  ppl_unit_database[ppl_unit_pos].quantity   = "catalytic_activity";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "P";  // poise
  ppl_unit_database[ppl_unit_pos].nameAp     = "P";  // poise
  ppl_unit_database[ppl_unit_pos].nameFs     = "poise";
  ppl_unit_database[ppl_unit_pos].nameFp     = "poises";
  ppl_unit_database[ppl_unit_pos].quantity   = "viscosity";
  ppl_unit_database[ppl_unit_pos].multiplier = GSL_CONST_MKSA_POISE;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "kayser";  // kayser
  ppl_unit_database[ppl_unit_pos].nameAp     = "kaysers"; // kayser
  ppl_unit_database[ppl_unit_pos].nameFs     = "kayser";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kaysers";
  ppl_unit_database[ppl_unit_pos].quantity   = "wavenumber";
  ppl_unit_database[ppl_unit_pos].multiplier = 100;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "C";  // coulomb
  ppl_unit_database[ppl_unit_pos].nameAp     = "C";  // coulomb
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "V";  // volt
  ppl_unit_database[ppl_unit_pos].nameAp     = "V";  // volt
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "ohm";  // ohm
  ppl_unit_database[ppl_unit_pos].nameAp     = "ohms"; // ohm
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "S"; // siemens
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "mho";  // mho
  ppl_unit_database[ppl_unit_pos].nameAp     = "mhos"; // mho
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "F";  // farad
  ppl_unit_database[ppl_unit_pos].nameAp     = "F";  // farad
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "H";  // henry
  ppl_unit_database[ppl_unit_pos].nameAp     = "H";  // henry
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "T";  // tesla
  ppl_unit_database[ppl_unit_pos].nameAp     = "T";  // tesla
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "G";  // gauss
  ppl_unit_database[ppl_unit_pos].nameAp     = "G";  // gauss
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "Wb";  // weber
  ppl_unit_database[ppl_unit_pos].nameAp     = "Wb";  // weber
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "Mx";  // maxwell
  ppl_unit_database[ppl_unit_pos].nameAp     = "Mx";  // maxwell
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "L_planck"; // Planck Length
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_length";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_lengths";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.61625281e-35;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "M_planck"; // Planck Mass
  ppl_unit_database[ppl_unit_pos].nameAp     = "M_planck"; // Planck Mass
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_mass";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_masses";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 2.1764411e-8;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "T_planck"; // Planck Time
  ppl_unit_database[ppl_unit_pos].nameAp     = "T_planck"; // Planck Time
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_time";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_times";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 5.3912427e-44;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "Q_planck"; // Planck Charge
  ppl_unit_database[ppl_unit_pos].nameAp     = "Q_planck"; // Planck Charge
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_charge";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_charges";
  ppl_unit_database[ppl_unit_pos].quantity   = "charge";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.87554587047e-18;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "K_planck"; // Planck Temperature
  ppl_unit_database[ppl_unit_pos].nameAp     = "K_planck"; // Planck Temperature
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_temperature";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_temperature";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 1.41678571e32;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "P_planck"; // Planck Momentum
  ppl_unit_database[ppl_unit_pos].nameAp     = "P_planck"; // Planck Momentum
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "E_planck"; // Planck Energy
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "F_planck"; // Planck Force
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "P_planck"; // Planck Power
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "I_planck"; // Planck Current
  ppl_unit_database[ppl_unit_pos].nameFs     = "planck_current";
  ppl_unit_database[ppl_unit_pos].nameFp     = "planck_current";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].planck     = 1;
  ppl_unit_database[ppl_unit_pos].multiplier = 3.4788748621352181073707008e+25;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]= 1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "V_planck"; // Planck Voltage
  ppl_unit_database[ppl_unit_pos].nameAp     = "V_planck"; // Planck Voltage
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
  ppl_unit_database[ppl_unit_pos].nameAp     = "Z_planck"; // Planck Impedence
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

