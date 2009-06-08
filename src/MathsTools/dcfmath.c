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

#include <gsl/gsl_math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_legendre.h>

#include "ppl_units.h"

#include "dcfmath.h"

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
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The acos() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = acos(in->number);
  output->exponent[UNIT_ANGLE] = 1;
  return;
 }

void dcfmath_asin(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The asin() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = asin(in->number);
  output->dimensionless = 0;
  output->exponent[UNIT_ANGLE] = 1;
  return;
 }

void dcfmath_atan(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The atan() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = atan(in->number);
  output->dimensionless = 0;
  output->exponent[UNIT_ANGLE] = 1;
  return;
 }

void dcfmath_atan2(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The atan2() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  output->number = atan2(in1->number, in2->number);
  output->dimensionless = 0;
  output->exponent[UNIT_ANGLE] = 1;
  return;
 }

void dcfmath_besseli(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besseli() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besseli(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_il_scaled((int)in1->number, in2->number);
  return;
 }

void dcfmath_besselI(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselI() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselI(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_In((int)in1->number, in2->number);
  return;
 }

void dcfmath_besselj(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselj() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselj(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_jl((int)in1->number, in2->number);
  return;
 }

void dcfmath_besselJ(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselJ() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselJ(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_Jn((int)in1->number, in2->number);
  return;
 }

void dcfmath_besselk(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselk() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselk(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_kl_scaled((int)in1->number, in2->number);
  return;
 }

void dcfmath_besselK(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselK() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselK(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_Kn((int)in1->number, in2->number);
  return;
 }

void dcfmath_bessely(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The bessely() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The bessely(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_yl((int)in1->number, in2->number);
  return;
 }

void dcfmath_besselY(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselY() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselY(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_bessel_Yn((int)in1->number, in2->number);
  return;
 }

void dcfmath_ceil(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The ceil() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = ceil(in->number);
  return;
 }

void dcfmath_cos (value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_ANGLE))
     {
      *status = 1;
      sprintf(errtext, "The cos() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
      return;
     }
  output->number = cos(in->number);
  return;
 }

void dcfmath_cosh(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_ANGLE))
     {
      *status = 1;
      sprintf(errtext, "The cosh() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
      return;
     }
  output->number = cosh(in->number);
  return;
 }

void dcfmath_degrees(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (in->dimensionless)
   {
    output->number = degrees(in->number);
    return;
   }
  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   if (in->exponent[i] != (i==UNIT_ANGLE))
    {
     *status = 1;
     sprintf(errtext, "The degrees() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
     return;
    }
  output->number = degrees(in->number);
  return;
 }

void dcfmath_erf(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The gsl_sf_erf() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = gsl_sf_erf(in->number);
  return;
 }

void dcfmath_exp (value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The exp() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = exp(in->number);
  return;
 }

void dcfmath_fabs(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The fabs() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = fabs(in->number);
  return;
 }

void dcfmath_floor(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The floor() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = floor(in->number);
  return;
 }

void dcfmath_fmod(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
 }

void dcfmath_gamma(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The gsl_sf_gamma() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = gsl_sf_gamma(in->number);
  return;
 }

void dcfmath_hypot(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The hypot() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  output->number = hypot(in1->number, in2->number);
  ppl_units_DimCpy(output, in1);
  return;
 }

void dcfmath_ldexp(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The ldexp() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  output->number = ldexp(in1->number, (int)in2->number);
  return;
 }

void dcfmath_legendreP(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The legendreP() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The legendreP(l,x) function can only evaluate legendre polynomials in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_legendre_Pl((int)in1->number, in2->number);
  return;
 }

void dcfmath_legendreQ(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The legendreQ() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  if ( (in1->number < 0.0) || (in1->number >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The legendreQ(l,x) function can only evaluate legendre polynomials in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->number = gsl_sf_legendre_Ql((int)in1->number, in2->number);
  return;
 }

void dcfmath_log (value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The log() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = log(in->number);
  return;
 }

void dcfmath_log10(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The log10() function can only act upon dimensionless inputs. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
    return;
   }
  output->number = log10(in->number);
  return;
 }

void dcfmath_max (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The max() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  output->number = max(in1->number, in2->number);
  ppl_units_DimCpy(output, in1);
  return;
 }

void dcfmath_min (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The min() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  output->number = min(in1->number, in2->number);
  ppl_units_DimCpy(output, in1);
  return;
 }

void dcfmath_pow (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  int j;
  *status = 0;
  ppl_units_zero(output);
  if (!(in2->dimensionless)) 
   {
    *status = 1;
    sprintf(errtext, "The pow() function can only operate with dimensionless exponents; supplied exponent has dimensions of <%s>.", ppl_units_GetUnitStr(in2, NULL, 1, 0));
    return;
   }
  output->number = pow(in1->number, in2->number);
  if ((output->dimensionless = in1->dimensionless) == 0)
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) output->exponent[j] = in1->exponent[j] * in2->number;
  return;
 }

void dcfmath_radians(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (in->dimensionless)
   {
    output->number = radians(in->number);
    return;
   }
  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   if (in->exponent[i] != (i==UNIT_ANGLE))
    {
     *status = 1;
     sprintf(errtext, "The radians() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
     return;
    }
  output->number = radians(in->number);
  return;
 }

void dcfmath_frandom(value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
  output->number = frandom();
  return;
 }

void dcfmath_frandomg(value *output, int *status, char *errtext)
 {
  static gsl_rng *rndgen = NULL;
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, rand()); }
  ppl_units_zero(output);
  output->number = gsl_ran_gaussian(rndgen, 1.0);
 }

void dcfmath_sin (value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_ANGLE))
     {
      *status = 1;
      sprintf(errtext, "The sin() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
      return;
     }
  output->number = sin(in->number);
  return;
 }

void dcfmath_sinh(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_ANGLE))
     {
      *status = 1;
      sprintf(errtext, "The sinh() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
      return;
     }
  output->number = sinh(in->number);
  return;
 }

void dcfmath_sqrt(value *in, value *output, int *status, char *errtext)
 {
  int j;
  if (in->number < 0)
   {
    *status = 1;
    sprintf(errtext, "Numerical error: attempt to square root a negative number.");
    return;
   }
  output->number = sqrt(in->number);
  output->dimensionless = in->dimensionless;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) output->exponent[j] = in->exponent[j] / 2;
  return;
 }

void dcfmath_tan (value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_ANGLE))
     {
      *status = 1;
      sprintf(errtext, "The tan() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
      return;
     }
  output->number = tan(in->number);
  return;
 }

void dcfmath_tanh(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_ANGLE))
     {
      *status = 1;
      sprintf(errtext, "The tanh() function can only act upon angles. Supplied input has dimensions of %s.", ppl_units_GetUnitStr(in, NULL, 1, 0));
      return;
     }
  output->number = tanh(in->number);
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

double frandom()
 {
  return (double)rand() / RAND_MAX;
 }
