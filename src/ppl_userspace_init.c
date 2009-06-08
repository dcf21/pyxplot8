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

#include <gsl/gsl_sf.h>

#include "MathsTools/dcfmath.h"

#include "ListTools/lt_memory.h"

#include "ppl_units.h"
#include "ppl_userspace.h"

void ppl_UserSpaceInit()
 {
  value v;

  _ppl_UserSpace_Vars  = DictInit();
  _ppl_UserSpace_Funcs = DictInit();

  // Function descriptors for the mathematical functions which are built into PyXPlot
  FunctionDescriptor fd_abs      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_fabs        , "abs(x) returns the absolute magnitude of x"};
  FunctionDescriptor fd_acos     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_acos        , "acos(x) returns the arccosine of x"};
  FunctionDescriptor fd_asin     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_asin        , "asin(x) returns the arcsine of x"};
  FunctionDescriptor fd_atan     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_atan        , "atan(x) returns the arctangent of x"};
  FunctionDescriptor fd_atan2    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_atan2       , "atan2(x,y) returns the arctangent of x/y"};
  FunctionDescriptor fd_besseli  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besseli     , "besseli(l,x) evaluates the lth regular modified spherical Bessel function at x"};
  FunctionDescriptor fd_besselI  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselI     , "besselI(l,x) evaluates the lth regular modified cylindrical Bessel function at x"};
  FunctionDescriptor fd_besselj  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselj     , "besselj(l,x) evaluates the lth regular spherical Bessel function at x"};
  FunctionDescriptor fd_besselJ  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselJ     , "besselJ(l,x) evaluates the lth regular cylindrical Bessel function at x"};
  FunctionDescriptor fd_besselk  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselk     , "besselk(l,x) evaluates the lth irregular modified spherical Bessel function at x"};
  FunctionDescriptor fd_besselK  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselK     , "besselK(l,x) evaluates the lth irregular modified cylindrical Bessel function at x"};
  FunctionDescriptor fd_bessely  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_bessely     , "bessely(l,x) evaluates the lth irregular spherical Bessel function at x"};
  FunctionDescriptor fd_besselY  = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_besselY     , "besselY(l,x) evaluates the lth irregular cylindrical Bessel function at x"};
  FunctionDescriptor fd_ceil     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_ceil        , "ceil(x) returns the smallest integer value greater than or equal to x"};
  FunctionDescriptor fd_cos      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cos         , "cos(x) returns the cosine of x (measured in radians)"};
  FunctionDescriptor fd_cosh     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_cosh        , "cosh(x) returns the hyperbolic cosine of x"};
  FunctionDescriptor fd_degrees  = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_degrees     , "degrees(x) converts angles measured in radians into degrees"};
  FunctionDescriptor fd_erf      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_erf         , "erf(x) evaluates the error function at x"};
  FunctionDescriptor fd_exp      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_exp         , "exp(x) returns e to the power of x"};
  FunctionDescriptor fd_floor    = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_floor       , "floor(x) returns the largest integer value smaller than or equal to x"};
  FunctionDescriptor fd_gamma    = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_gamma       , "gamma(x) evaluates the gamma function at x"};
  FunctionDescriptor fd_hypot    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_hypot       , "hypot(x,y) returns the quadrature sum of x and y"};
  FunctionDescriptor fd_ldexp    = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_ldexp       , "ldexp(x,y) returns x times 2 to the power of an integer y"};
  FunctionDescriptor fd_legendreP= { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_legendreP   , "legendreP(l,x) evaluates the lth Legendre polynomial at x"};
  FunctionDescriptor fd_legendreQ= { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_legendreQ   , "legendreQ(l,x) evaluates the lth Legendre function at x"};
  FunctionDescriptor fd_log      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log         , "log(x) returns the natural logarithm of x"};
  FunctionDescriptor fd_log10    = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_log10       , "log10(x) returns the logarithm to base 10 of x"};
  FunctionDescriptor fd_max      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_max         , "max(x,y) returns the greater of the two values x and y"};
  FunctionDescriptor fd_min      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_min         , "min(x,y) returns the lesser of the two values x and y"};
  FunctionDescriptor fd_mod      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_fmod        , "mod(x,y) returns the remainder of x/y"};
  FunctionDescriptor fd_pow      = { PPL_USERSPACE_SYSTEM , 0 , 2 , (void *)&dcfmath_pow         , "pow(x,y) returns x to the power of y"};
  FunctionDescriptor fd_radians  = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_radians     , "radians(x) converts angles measured in degrees into radians"};
  FunctionDescriptor fd_random   = { PPL_USERSPACE_SYSTEM , 0 , 0 , (void *)&dcfmath_frandom     , "random(x) returns a random number between 0 and 1"};
  FunctionDescriptor fd_randomg  = { PPL_USERSPACE_SYSTEM , 0 , 0 , (void *)&dcfmath_frandomg    , "random_gaussian(x) returns a random sample from the Gaussian (normal) distribution"};
  FunctionDescriptor fd_sin      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sin         , "sin(x) returns the sine of x (measured in radians)"};
  FunctionDescriptor fd_sinh     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sinh        , "sinh(x) returns the hyperbolic sine of x"};
  FunctionDescriptor fd_sqrt     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_sqrt        , "sqrt(x) returns the square root of x"};
  FunctionDescriptor fd_tan      = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_tan         , "tan(x) returns the tangent of x (measured in radians)"};
  FunctionDescriptor fd_tanh     = { PPL_USERSPACE_SYSTEM , 0 , 1 , (void *)&dcfmath_tanh        , "tanh(x) returns the hyperbolic tangent of x"};
  FunctionDescriptor fd_unit     = { PPL_USERSPACE_UNIT   , 0 ,-1 , NULL                         , "unit(...) multiplies a number by a physical unit"};
  FunctionDescriptor fd_int      = { PPL_USERSPACE_INT    , 0 ,-1 , NULL                         , "int_dx(e,min,max) numerically integrates an expression e wrt x between min and max"};

  // Set up default variables
  ppl_units_zero(&v);
  v.number = M_PI;
  DictAppendValue(_ppl_UserSpace_Vars , "pi"            , v); // pi
  v.number = M_E;
  DictAppendValue(_ppl_UserSpace_Vars , "e"             , v); // e
  v.number = 299792458.0;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH]=1 ; v.exponent[UNIT_TIME]=-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_c"         , v); // Speed of light
  ppl_units_zero(&v);
  v.number = 4e-7*M_PI;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1; v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] = -2; v.exponent[UNIT_CURRENT] = -2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_0"      , v); // The permeability of free space
  ppl_units_zero(&v);
  v.number = 8.8541878176e-12;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] =-3; v.exponent[UNIT_MASS] =-1; v.exponent[UNIT_TIME] =  4; v.exponent[UNIT_CURRENT] =  2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_epsilon_0" , v); // The permittivity of free space
  ppl_units_zero(&v);
  v.number = 1.60217646e-19;
  v.dimensionless = 0;
  v.exponent[UNIT_CURRENT] = 1; v.exponent[UNIT_TIME] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_q"         , v); // The fundamental charge
  ppl_units_zero(&v);
  v.number = 6.626068e-34;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_h"         , v); // The Planck constant
  v.number = 1.0546e-34;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_hbar"      , v); // The Planck constant / 2pi
  ppl_units_zero(&v);
  v.number = 6.0221415e23;
  v.dimensionless = 0;
  v.exponent[UNIT_MOLE] = -1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_NA"        , v); // The Avogadro constant
  ppl_units_zero(&v);
  v.number = 3.839e26;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-3;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Lsun"      , v); // The solar luminosity
  ppl_units_zero(&v);
  v.number = 1.66053886e-27;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_u"       , v); // The universal mass constant
  v.number = 1.67262158e-27;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_p"       , v); // The proton mass
  v.number = 1.67492729e-27;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_n"       , v); // The neutron mass
  v.number = 1.98892e30;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_Msun"      , v); // The solar mass
  ppl_units_zero(&v);
  v.number = 9.27400949e-24;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_CURRENT] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_b"      , v); // The Bohr magneton
  ppl_units_zero(&v);
  v.number = 8.314472;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1; v.exponent[UNIT_MOLE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_R"         , v); // The gas constant
  ppl_units_zero(&v);
  v.number = 1.3806503e-23;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_kB"        , v); // The Boltzmann constant
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2;
  ppl_units_zero(&v);
  v.number = 5.6704e-8;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] =-3; v.exponent[UNIT_TEMPERATURE] =-4;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_sigma"     , v); // The Stefan-Boltzmann constant
  ppl_units_zero(&v);
  v.number = 6.67300e-11;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 3; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_MASS] =-2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_G"         , v); // The gravitational constant

  // Copy function descriptors for core mathematical functions into function namespace
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "abs"    , (void *)&fd_abs         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "acos"   , (void *)&fd_acos        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "asin"   , (void *)&fd_asin        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "atan"   , (void *)&fd_atan        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "atan2"  , (void *)&fd_atan2       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besseli", (void *)&fd_besseli     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselI", (void *)&fd_besselI     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselj", (void *)&fd_besselj     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselJ", (void *)&fd_besselJ     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselk", (void *)&fd_besselk     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselK", (void *)&fd_besselK     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "bessely", (void *)&fd_bessely     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "besselY", (void *)&fd_besselY     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ceil"   , (void *)&fd_ceil        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cos"    , (void *)&fd_cos         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "cosh"   , (void *)&fd_cosh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "degrees", (void *)&fd_degrees     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "erf"    , (void *)&fd_erf         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "exp"    , (void *)&fd_exp         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "floor"  , (void *)&fd_floor       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "gamma"  , (void *)&fd_gamma       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "hypot"  , (void *)&fd_hypot       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "ldexp"  , (void *)&fd_ldexp       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "legendreP",(void *)&fd_legendreP  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "legendreQ",(void *)&fd_legendreQ  , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "log"    , (void *)&fd_log         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "log10"  , (void *)&fd_log10       , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "max"    , (void *)&fd_max         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "min"    , (void *)&fd_min         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "mod"    , (void *)&fd_mod         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "pow"    , (void *)&fd_pow         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "radians", (void *)&fd_radians     , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random" , (void *)&fd_random      , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "random_gaussian", (void *)&fd_randomg, sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sin"    , (void *)&fd_sin         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sinh"   , (void *)&fd_sinh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "sqrt"   , (void *)&fd_sqrt        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tan"    , (void *)&fd_tan         , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "tanh"   , (void *)&fd_tanh        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "unit"   , (void *)&fd_unit        , sizeof(FunctionDescriptor), DATATYPE_VOID);
  DictAppendPtrCpy  (_ppl_UserSpace_Funcs, "int_d?" , (void *)&fd_int         , sizeof(FunctionDescriptor), DATATYPE_VOID);

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
    ppl_unit_database[i].multiplier = 1.0;
    ppl_unit_database[i].offset     = 0.0;
    ppl_unit_database[i].UserSel    = 0;
    ppl_unit_database[i].si         = ppl_unit_database[i].imperial  = ppl_unit_database[i].us = ppl_unit_database[i].ancient = ppl_unit_database[i].modified = 0;
    ppl_unit_database[i].MaxPrefix  = ppl_unit_database[i].MinPrefix = 0;
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[i].exponent[j] = 0;
   }

  // Set up default list of units
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
  ppl_unit_database[ppl_unit_pos].multiplier = 0.0254;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "ft";   // Foot
  ppl_unit_database[ppl_unit_pos].nameAp     = "ft";   // Foot
  ppl_unit_database[ppl_unit_pos].nameFs     = "foot";
  ppl_unit_database[ppl_unit_pos].nameFp     = "feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.0254;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "yd";   // Yard
  ppl_unit_database[ppl_unit_pos].nameAp     = "yd";   // Yard
  ppl_unit_database[ppl_unit_pos].nameFs     = "yard";
  ppl_unit_database[ppl_unit_pos].nameFp     = "yards";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.9144;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "mi";   // Mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "mi";   // Mile
  ppl_unit_database[ppl_unit_pos].nameFs     = "mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1610;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;
  
  ppl_unit_database[ppl_unit_pos].nameAs     = "furlong"; // Furlong
  ppl_unit_database[ppl_unit_pos].nameAp     = "furlongs";// Furlong
  ppl_unit_database[ppl_unit_pos].nameFs     = "furlong";
  ppl_unit_database[ppl_unit_pos].nameFp     = "furlongs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 201.17;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "chain"; // Chain
  ppl_unit_database[ppl_unit_pos].nameAp     = "chains";// Chain
  ppl_unit_database[ppl_unit_pos].nameFs     = "chain";
  ppl_unit_database[ppl_unit_pos].nameFp     = "chains";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 20.12;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "nautical_mile"; // Nautical mile
  ppl_unit_database[ppl_unit_pos].nameAp     = "nautical_miles";// Nautical mile
  ppl_unit_database[ppl_unit_pos].nameFs     = "nautical_mile";
  ppl_unit_database[ppl_unit_pos].nameFp     = "nautical_miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1850;
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "AU";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameAp     = "AU";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameFs     = "astronomical_unit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "astronomical_units";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 149598e6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "lyr";   // Lightyear
  ppl_unit_database[ppl_unit_pos].nameAp     = "lyr";   // Lightyear
  ppl_unit_database[ppl_unit_pos].nameFs     = "light_year";
  ppl_unit_database[ppl_unit_pos].nameFp     = "light_years";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 9.4605284e15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "pc";   // Parsec
  ppl_unit_database[ppl_unit_pos].nameAp     = "pc";   // Parsec
  ppl_unit_database[ppl_unit_pos].nameFs     = "parsec";
  ppl_unit_database[ppl_unit_pos].nameFp     = "parsecs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 3.08568025e16;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "g";    // Gramme
  ppl_unit_database[ppl_unit_pos].nameAp     = "g";    // Gramme
  ppl_unit_database[ppl_unit_pos].nameFs     = "gramme";
  ppl_unit_database[ppl_unit_pos].nameFp     = "grammes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 1.98892e30;
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0/683;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0/683;
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
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 4200;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 1055.05585;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 745.699872;
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  return;
 }

