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
#include <limits.h>
#include <math.h>

#include <gsl/gsl_cdf.h>
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

void dcfmath_abs(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  output->real = fabs(in->real);
  ppl_units_DimCpy(output, in);
  return;
 }

void dcfmath_acos(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The acos() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = acos(in->real);
  output->dimensionless = 0;
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
    sprintf(errtext, "The asin() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = asin(in->real);
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
    sprintf(errtext, "The atan() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = atan(in->real);
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
    sprintf(errtext, "The atan2() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = atan2(in1->real, in2->real);
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
    sprintf(errtext, "The besseli() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besseli(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_il_scaled((int)in1->real, in2->real);
  return;
 }

void dcfmath_besselI(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselI() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselI(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_In((int)in1->real, in2->real);
  return;
 }

void dcfmath_besselj(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselj() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselj(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_jl((int)in1->real, in2->real);
  return;
 }

void dcfmath_besselJ(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselJ() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselJ(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_Jn((int)in1->real, in2->real);
  return;
 }

void dcfmath_besselk(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselk() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselk(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_kl_scaled((int)in1->real, in2->real);
  return;
 }

void dcfmath_besselK(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselK() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselK(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_Kn((int)in1->real, in2->real);
  return;
 }

void dcfmath_bessely(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The bessely() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The bessely(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_yl((int)in1->real, in2->real);
  return;
 }

void dcfmath_besselY(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The besselY() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The besselY(l,x) function can only evaluate Bessel functions in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_bessel_Yn((int)in1->real, in2->real);
  return;
 }

void dcfmath_beta(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The beta() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_beta(in1->real, in2->real);
  return;
 }

void dcfmath_binomialPDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The binomialPDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ((in1->real < 0) || (in1->real >= INT_MAX))
   {
    *status = 1;
    sprintf(errtext, "The binomialPDF() function's first parameter must be in the range 0 <= k < %d.",INT_MAX);
    return;
   }
  if ((in2->real < 0) || (in2->real >= INT_MAX))
   {
    *status = 1;
    sprintf(errtext, "The binomialPDF() function's 3rd parameter must be in the range 0 <= n < %d.",INT_MAX);
    return;
   }
  output->real = gsl_ran_binomial_pdf((unsigned int)in1->real, in2->real, (unsigned int)in3->real);
  return;
 }

void dcfmath_binomialCDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The binomialCDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ((in1->real < 0) || (in1->real >= INT_MAX))
   {
    *status = 1;
    sprintf(errtext, "The binomialCDF() function's first parameter must be in the range 0 <= k < %d.",INT_MAX);
    return;
   }
  if ((in2->real < 0) || (in2->real >= INT_MAX))
   {
    *status = 1;
    sprintf(errtext, "The binomialCDF() function's 3rd parameter must be in the range 0 <= n < %d.",INT_MAX);
    return;
   }
  output->real = gsl_cdf_binomial_P((unsigned int)in1->real, in2->real, (unsigned int)in3->real);
  return;
 }

void dcfmath_planck_Bv(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in1->exponent[i] != -(i==UNIT_TIME))
     {
      *status = 1;
      sprintf(errtext, "The first argument to the Bv() function must be a frequency. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in1, NULL,NULL,  1, 0));
      return;
     }
  if (!(in2->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in2->exponent[i] != (i==UNIT_TEMPERATURE))
     {
      *status = 1;
      sprintf(errtext, "The second argument to the Bv() function must be a temperature. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in2, NULL,NULL,  1, 0));
      return;
     }
  output->dimensionless = 0;
  output->exponent[UNIT_MASS] =  1;
  output->exponent[UNIT_TIME] = -2;
  output->exponent[UNIT_ANGLE]= -2;
  output->real              =  2 * GSL_CONST_MKSA_PLANCKS_CONSTANT_H / pow(GSL_CONST_MKSA_SPEED_OF_LIGHT, 2) * pow(in1->real,3) / expm1(GSL_CONST_MKSA_PLANCKS_CONSTANT_H * in1->real / GSL_CONST_MKSA_BOLTZMANN / in2->real);
 }

void dcfmath_planck_Bvmax(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   for (i=0; i<UNITS_MAX_BASEUNITS; i++)
    if (in->exponent[i] != (i==UNIT_TEMPERATURE))
     {
      *status = 1;
      sprintf(errtext, "The Bvmax() function can only act upon temperatures. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->dimensionless = 0;
  output->exponent[UNIT_TIME] = -1;
  output->real = 2.821439 * GSL_CONST_MKSA_BOLTZMANN / GSL_CONST_MKSA_PLANCKS_CONSTANT_H * in->real; // Wien displacement law
 }

void dcfmath_ceil(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The ceil() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = ceil(in->real);
  return;
 }

void dcfmath_chisqPDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The chisqPDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_chisq_pdf(in1->real , in2->real);
  return;
 }

void dcfmath_chisqCDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The chisqCDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_chisq_P(in1->real , in2->real);
  return;
 }

void dcfmath_chisqCDFi(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The chisqCDFi() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_chisq_Pinv(in1->real , in2->real);
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
      sprintf(errtext, "The cos() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->real = cos(in->real);
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
      sprintf(errtext, "The cosh() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->real = cosh(in->real);
  return;
 }

void dcfmath_degrees(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (in->dimensionless)
   {
    output->real = degrees(in->real);
    return;
   }
  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   if (in->exponent[i] != (i==UNIT_ANGLE))
    {
     *status = 1;
     sprintf(errtext, "The degrees() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
     return;
    }
  output->real = degrees(in->real);
  return;
 }

void dcfmath_ellK(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The ellipticalintK() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_ellint_Kcomp(in->real , GSL_PREC_DOUBLE);
  return;
 }

void dcfmath_ellE(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The ellipticalintE() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_ellint_Ecomp(in->real , GSL_PREC_DOUBLE);
  return;
 }

void dcfmath_ellP(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The ellipticalintP() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_ellint_Pcomp(in1->real , in2->real , GSL_PREC_DOUBLE);
  return;
 }

void dcfmath_erf(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The erf() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_erf(in->real);
  return;
 }

void dcfmath_erfc(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The erfc() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_erfc(in->real);
  return;
 }

void dcfmath_exp(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The exp() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = exp(in->real);
  return;
 }

void dcfmath_expm1(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The expm1() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = expm1(in->real);
  return;
 }

void dcfmath_expint(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The expint() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real <= INT_MIN) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The expint(n,x) function can only evaluate exponential integrals of order %d < l < %d", INT_MIN, INT_MAX);
    return;
   }
  output->real = gsl_sf_expint_En((int)in1->real, in2->real);
  return;
 }

void dcfmath_floor(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The floor() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = floor(in->real);
  return;
 }

void dcfmath_gamma(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The gamma() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_gamma(in->real);
  return;
 }

void dcfmath_gaussianPDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The gaussianPDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_gaussian_pdf(in1->real, in2->real);
  return;
 }

void dcfmath_gaussianCDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The gaussianCDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_gaussian_P(in1->real, in2->real);
  return;
 }

void dcfmath_gaussianCDFi(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The gaussianCDFi() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_gaussian_Pinv(in1->real, in2->real);
  return;
 }

void dcfmath_heaviside(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The heaviside() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  if (in->real >= 0) output->real = 1.0;
  return;
 }

void dcfmath_hypot(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The hypot() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = hypot(in1->real, in2->real);
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
    sprintf(errtext, "The ldexp() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = ldexp(in1->real, (int)in2->real);
  return;
 }

void dcfmath_legendreP(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The legendreP() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The legendreP(l,x) function can only evaluate legendre polynomials in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_legendre_Pl((int)in1->real, in2->real);
  return;
 }

void dcfmath_legendreQ(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The legendreQ() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( (in1->real < 0.0) || (in1->real >= INT_MAX) )
   {
    *status = 1;
    sprintf(errtext, "The legendreQ(l,x) function can only evaluate legendre polynomials in the range 0 < l < %d", INT_MAX);
    return;
   }
  output->real = gsl_sf_legendre_Ql((int)in1->real, in2->real);
  return;
 }

void dcfmath_log (value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The log() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = log(in->real);
  return;
 }

void dcfmath_log10(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The log10() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = log10(in->real);
  return;
 }

void dcfmath_lognormalPDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The lognormalPDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_lognormal_pdf(in1->real, in2->real, in3->real);
  return;
 }

void dcfmath_lognormalCDF(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The lognormalCDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_lognormal_P(in1->real, in2->real, in3->real);
  return;
 }

void dcfmath_lognormalCDFi(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless && in3->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The lognormalCDFi() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_lognormal_Pinv(in1->real, in2->real, in3->real);
  return;
 }

void dcfmath_max (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The max() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = max(in1->real, in2->real);
  ppl_units_DimCpy(output, in1);
  return;
 }

void dcfmath_min (value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  if ((!(in1->dimensionless && in2->dimensionless)) && (!(ppl_units_DimEqual(in1, in2))))
   {
    *status = 1;
    sprintf(errtext, "The min() function can only act upon inputs with matching dimensions. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = min(in1->real, in2->real);
  ppl_units_DimCpy(output, in1);
  return;
 }

void dcfmath_mod(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The mod() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = fmod(in1->real , in2->real);
  return;
 }

void dcfmath_poissonPDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The poissonPDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_poisson_pdf(in1->real , in2->real);
  return;
 }

void dcfmath_poissonCDF(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The poissonCDF() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_cdf_poisson_P(in1->real , in2->real);
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
    sprintf(errtext, "The pow() function can only operate with dimensionless exponents; supplied exponent has dimensions of <%s>.", ppl_units_GetUnitStr(in2, NULL, NULL,  1, 0));
    return;
   }
  output->real = pow(in1->real, in2->real);
  if ((output->dimensionless = in1->dimensionless) == 0)
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) output->exponent[j] = in1->exponent[j] * in2->real;
  return;
 }

void dcfmath_radians(value *in, value *output, int *status, char *errtext)
 {
  int i;
  *status = 0;
  ppl_units_zero(output);
  if (in->dimensionless)
   {
    output->real = radians(in->real);
    return;
   }
  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   if (in->exponent[i] != (i==UNIT_ANGLE))
    {
     *status = 1;
     sprintf(errtext, "The radians() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
     return;
    }
  output->real = radians(in->real);
  return;
 }

void dcfmath_frandom(value *output, int *status, char *errtext)
 {
  ppl_units_zero(output);
  output->real = frandom();
  return;
 }

static gsl_rng *rndgen = NULL; // Random number generator for next five functions

void dcfmath_frandombin(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, rand()); }
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The randomBinomial() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ((in2->real < 0) || (in2->real >= INT_MAX))
   {
    *status = 1;
    sprintf(errtext, "The randomBinomial() function's 2nd parameter must be in the range 0 <= n < %d.",INT_MAX);
    return;
   }
  output->real = gsl_ran_binomial(rndgen, in1->real, (unsigned int)in2->real);
  return;
 }

void dcfmath_frandomcs(value *in, value *output, int *status, char *errtext)
 {
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, rand()); }
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The randomChiSq() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_chisq(rndgen, in->real);
 }

void dcfmath_frandomg(value *in, value *output, int *status, char *errtext)
 {
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, rand()); }
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The randomGaussian() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_gaussian(rndgen, in->real);
 }

void dcfmath_frandomln(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, rand()); }
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The randomLogNormal() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_lognormal(rndgen, in1->real, in2->real);
  ppl_units_DimCpy(output, in1);
  return;
 }

void dcfmath_frandomp(value *in, value *output, int *status, char *errtext)
 {
  if (rndgen==NULL) { rndgen = gsl_rng_alloc(gsl_rng_default); gsl_rng_set(rndgen, rand()); }
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The randomPoisson() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_ran_poisson(rndgen, in->real);
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
      sprintf(errtext, "The sin() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->real = sin(in->real);
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
      sprintf(errtext, "The sinh() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->real = sinh(in->real);
  return;
 }

void dcfmath_sqrt(value *in, value *output, int *status, char *errtext)
 {
  int j;
  if (in->real < 0)
   {
    *status = 1;
    sprintf(errtext, "Numerical error: attempt to square root a negative number.");
    return;
   }
  output->real = sqrt(in->real);
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
      sprintf(errtext, "The tan() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->real = tan(in->real);
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
      sprintf(errtext, "The tanh() function can only act upon angles. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
      return;
     }
  output->real = tanh(in->real);
  return;
 }

void dcfmath_tophat(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in1->dimensionless && in2->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The tophat() function can only act upon dimensionless inputs. Supplied inputs have dimensions of <%s> and <%s>.", ppl_units_GetUnitStr(in1, NULL, NULL, 0, 0), ppl_units_GetUnitStr(in2, NULL, NULL, 1, 0));
    return;
   }
  if ( fabs(in1->real) <= fabs(in2->real) ) output->real = 1.0;
  return;
 }

void dcfmath_zeta(value *in, value *output, int *status, char *errtext)
 {
  *status = 0;
  ppl_units_zero(output);
  if (!(in->dimensionless))
   {
    *status = 1;
    sprintf(errtext, "The zeta() function can only act upon dimensionless inputs. Supplied input has dimensions of <%s>.", ppl_units_GetUnitStr(in, NULL, NULL, 1, 0));
    return;
   }
  output->real = gsl_sf_zeta(in->real);
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
