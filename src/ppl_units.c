// ppl_units.c
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

// NB: This source file is not included in the PyXPlot Makefile, but is
// included as a part of ppl_userspace.c. This allows some functions to be
// compiled inline for speed.

#define _PPL_UNITS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_units.h"

#include "StringTools/asciidouble.h"

#include "ListTools/lt_memory.h"

unit *ppl_unit_database;
int   ppl_unit_pos = 0;

value *ppl_units_zero(value *in)
 {
  int i;
  in->number = 0.0;
  in->dimensionless = 1;
  for (i=0; i<UNITS_MAX_BASEUNITS; i++) in->exponent[i]=0;
  return in;
 }

char *ppl_units_NumericDisplay(value *in, int N, int typeable)
 {
  static char outputA[LSTR_LENGTH], outputB[LSTR_LENGTH];
  double NumberOut;
  char *output, *unitstr;
  if (N==0) output = outputA;
  else      output = outputB;

  unitstr = ppl_units_GetUnitStr(in, &NumberOut, N, typeable);
  if (unitstr[0]=='\0') return NumericDisplay(NumberOut, N);
  else if (typeable==0) sprintf(output, "%s %s", NumericDisplay(NumberOut, N), unitstr);
  else                  sprintf(output, "%s%s" , NumericDisplay(NumberOut, N), unitstr);

  return output;
 }

char *ppl_units_GetUnitStr(value *in, double *NumberOut, int N, int typeable)
 {
  static char outputA[LSTR_LENGTH], outputB[LSTR_LENGTH];
  char *output;
  if (N==0) output = outputA;
  else      output = outputB;

  if (in->dimensionless != 0)
   {
    output[0]='\0';
    if (NumberOut != NULL) *NumberOut = in->number;
    return output;
   }

  // In selecting display units to use, criteria are:
  // 1. A unit which matches as many as possible of the dimensions of X
  // 2. A unit which is user-prefered
  // 2. A unit which is in the current units scheme (e.g. SI)
  // 3. A unit which is smaller than X, but larger than the present best unit

  // If there is a unit raised to power of one which we can append a prefix to, do so

  // Display units in order of decreasing exponent
  if (NumberOut != NULL) *NumberOut = in->number;
  strcpy(output, "foo");
  return output;
 }

value *ppl_units_StringEvaluate(char *in)
 {
  return NULL;
 }

// ARITHMETIC OPERATIONS ON VALUES

unsigned char __inline__ ppl_units_DblEqual(double a, double b)
 {
  if ( (fabs(a) < 1e-100) && (fabs(b) < 1e-100) ) return 1;
  if ( (fabs(a-b) < fabs(1e-7*a)) && (fabs(a-b) < fabs(1e-7*b)) ) return 1;
  return 0;
 }

int __inline__ ppl_units_DimEqual(value *a, value *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

void __inline__ ppl_units_pow (value *a, value *b, value *o, int *status, char *errtext)
 {
  int i;
  double exponent;
  unsigned char DimLess=1;

  if (b->dimensionless == 0)
   {
    sprintf(errtext, "Exponent should be dimensionless, but instead has dimensions of %s.", ppl_units_GetUnitStr(b, NULL, 0, 0));
    *status = 1;
    return;
   }
  exponent = b->number;
  o->number = pow( a->number , exponent );
  if (a->dimensionless != 0) return;

  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   {
    o->exponent[i] = a->exponent[i] * exponent;
    if (ppl_units_DblEqual(o->exponent[i], 0) == 0) DimLess=0;
    if (fabs(o->exponent[i]) > 20000 ) { sprintf(errtext, "Overflow of physical dimensions of argument."); *status = 1; return; }
   }
  o->dimensionless = DimLess;
  return;
 }

void __inline__ ppl_units_mult(value *a, value *b, value *o, int *status, char *errtext)
 {
  int i;
  unsigned char DimLess=1;

  o->number = a->number * b->number;
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   {
    o->exponent[i] = a->exponent[i] + b->exponent[i];
    if (ppl_units_DblEqual(o->exponent[i], 0) == 0) DimLess=0;
    if (fabs(o->exponent[i]) > 20000 ) { sprintf(errtext, "Overflow of physical dimensions of argument."); *status = 1; return; }
   }
  o->dimensionless = DimLess;
  return;
 }

void __inline__ ppl_units_div (value *a, value *b, value *o, int *status, char *errtext)
 {
  int i;
  unsigned char DimLess=1;

  if (fabs(b->number) < 1e-200) { sprintf(errtext, "Division by zero error."); *status = 1; return; }
  o->number = a->number / b->number;
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   {
    o->exponent[i] = a->exponent[i] - b->exponent[i];
    if (ppl_units_DblEqual(o->exponent[i], 0) == 0) DimLess=0;
    if (fabs(o->exponent[i]) > 20000 ) { sprintf(errtext, "Overflow of physical dimensions of argument."); *status = 1; return; }
   }
  o->dimensionless = DimLess;
  return;
 }

void __inline__ ppl_units_add (value *a, value *b, value *o, int *status, char *errtext)
 {
  o->number = a->number + b->number;
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  if (ppl_units_DimEqual(a, b) == 0)
   { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand has units of '%s', while right operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, 1, 0) ); *status = 1; return; }
  return;
 }

void __inline__ ppl_units_sub (value *a, value *b, value *o, int *status, char *errtext)
 {
  o->number = a->number - b->number;
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  if (ppl_units_DimEqual(a, b) == 0)
   { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand has units of '%s', while right operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, 1, 0) ); *status = 1; return; }
  return;
 }

void __inline__ ppl_units_mod (value *a, value *b, value *o, int *status, char *errtext)
 {
  o->number = a->number - floor(a->number / b->number) * b->number;
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  sprintf(errtext, "Mod operator can only be applied to dimensionless operands; left operand has units of '%s', while right operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, 1, 0) );
  *status = 1;
  return;
 }

// ----------------------------------------------------------------------
// SET UP DEFAULT DATABASE OF KNOWN UNITS

void ppl_units_init()
 {
  int i=0,j=0;

  ppl_unit_database = (unit *)lt_malloc(UNITS_MAX*sizeof(unit));

  for (i=0;i<UNITS_MAX;i++)
   {
    ppl_unit_database[i].name1      = NULL;
    ppl_unit_database[i].name2      = NULL;
    ppl_unit_database[i].multiplier = 1.0;
    ppl_unit_database[i].offset     = 0.0;
    ppl_unit_database[i].UserSel    = 0;
    ppl_unit_database[i].si         = ppl_unit_database[i].imperial  = ppl_unit_database[i].us = ppl_unit_database[i].ancient = 0;
    ppl_unit_database[i].MaxPrefix  = ppl_unit_database[i].MinPrefix = 0;
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[i].exponent[j] = 0;
   }

  ppl_unit_database[ppl_unit_pos].name1      = "m";    // Metre
  ppl_unit_database[ppl_unit_pos].name2      = "metre";
  ppl_unit_database[ppl_unit_pos].name3      = "metres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cm";    // Centimetre
  ppl_unit_database[ppl_unit_pos].name2      = "centimetre";
  ppl_unit_database[ppl_unit_pos].name3      = "centimetres";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "ang";  // Angstrom
  ppl_unit_database[ppl_unit_pos].name2      = "angstrom";
  ppl_unit_database[ppl_unit_pos].name3      = "angstroms";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-10;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "in";   // Inch
  ppl_unit_database[ppl_unit_pos].name2      = "inch";
  ppl_unit_database[ppl_unit_pos].name3      = "inches";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.0254;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "ft";   // Foot
  ppl_unit_database[ppl_unit_pos].name2      = "foot";
  ppl_unit_database[ppl_unit_pos].name3      = "feet";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.0254;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "yd";   // Yard
  ppl_unit_database[ppl_unit_pos].name2      = "yard";
  ppl_unit_database[ppl_unit_pos].name3      = "yards";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.9144;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "mi";   // Mile
  ppl_unit_database[ppl_unit_pos].name2      = "mile";
  ppl_unit_database[ppl_unit_pos].name3      = "miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1610;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;
  
  ppl_unit_database[ppl_unit_pos].name1      = "furlongs";// Furlong
  ppl_unit_database[ppl_unit_pos].name2      = "furlong";
  ppl_unit_database[ppl_unit_pos].name3      = "furlongs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 201.17;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "chains";// Chain
  ppl_unit_database[ppl_unit_pos].name2      = "chain";
  ppl_unit_database[ppl_unit_pos].name3      = "chains";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 20.12;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "nautical miles";// Nautical mile
  ppl_unit_database[ppl_unit_pos].name2      = "nautical mile";
  ppl_unit_database[ppl_unit_pos].name3      = "nautical miles";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 1850;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cubits";// Cubit
  ppl_unit_database[ppl_unit_pos].name2      = "cubit";
  ppl_unit_database[ppl_unit_pos].name3      = "cubits";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.04572;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "au";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].name2      = "astronomical unit";
  ppl_unit_database[ppl_unit_pos].name3      = "astronomical units";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 149598e6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "pc";   // Parsec
  ppl_unit_database[ppl_unit_pos].name2      = "parsec";
  ppl_unit_database[ppl_unit_pos].name3      = "parsecs";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 3.08568025e16;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "s";    // Second
  ppl_unit_database[ppl_unit_pos].name2      = "second";
  ppl_unit_database[ppl_unit_pos].name3      = "seconds";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "min";  // Minute
  ppl_unit_database[ppl_unit_pos].name2      = "minute";
  ppl_unit_database[ppl_unit_pos].name3      = "minutes";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 60;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "hr";   // Hour
  ppl_unit_database[ppl_unit_pos].name2      = "hour";
  ppl_unit_database[ppl_unit_pos].name3      = "hours";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 3600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "dy";   // Day
  ppl_unit_database[ppl_unit_pos].name2      = "day";
  ppl_unit_database[ppl_unit_pos].name3      = "days";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 86400;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "wk";   // Week
  ppl_unit_database[ppl_unit_pos].name2      = "week";
  ppl_unit_database[ppl_unit_pos].name3      = "weeks";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 604800;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "yr";   // Year
  ppl_unit_database[ppl_unit_pos].name2      = "year";
  ppl_unit_database[ppl_unit_pos].name3      = "years";
  ppl_unit_database[ppl_unit_pos].quantity   = "time";
  ppl_unit_database[ppl_unit_pos].multiplier = 31557600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "kg";   // Kilogram
  ppl_unit_database[ppl_unit_pos].name2      = "kilogram";
  ppl_unit_database[ppl_unit_pos].name3      = "kilograms";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "g";    // Gram
  ppl_unit_database[ppl_unit_pos].name2      = "gram";
  ppl_unit_database[ppl_unit_pos].name3      = "grams";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "g";    // Gramme
  ppl_unit_database[ppl_unit_pos].name2      = "gramme";
  ppl_unit_database[ppl_unit_pos].name3      = "grammes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].name2      = "metric ton";
  ppl_unit_database[ppl_unit_pos].name3      = "metric tons";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].name2      = "tonne";
  ppl_unit_database[ppl_unit_pos].name3      = "tonnes";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "short ton"; // US Ton
  ppl_unit_database[ppl_unit_pos].name2      = "ton";
  ppl_unit_database[ppl_unit_pos].name3      = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 907;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "long ton"; // UK Ton
  ppl_unit_database[ppl_unit_pos].name2      = "ton";
  ppl_unit_database[ppl_unit_pos].name3      = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 1016;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "shekels"; // Shekel
  ppl_unit_database[ppl_unit_pos].name2      = "shekel";
  ppl_unit_database[ppl_unit_pos].name3      = "shekels";
  ppl_unit_database[ppl_unit_pos].quantity   = "mass";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].name2      = "ampere";
  ppl_unit_database[ppl_unit_pos].name3      = "amperes";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].name2      = "amp";
  ppl_unit_database[ppl_unit_pos].name3      = "amps";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].name2      = "kelvin";
  ppl_unit_database[ppl_unit_pos].name3      = "kelvin";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].name2      = "celsius";
  ppl_unit_database[ppl_unit_pos].name3      = "celsius";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].offset     = 273.15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].name2      = "centigrade";
  ppl_unit_database[ppl_unit_pos].name3      = "centigrade";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].offset     = 273.15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "oF"; // oF
  ppl_unit_database[ppl_unit_pos].name2      = "fahrenheit";
  ppl_unit_database[ppl_unit_pos].name3      = "fahrenheit";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.8;
  ppl_unit_database[ppl_unit_pos].offset     = 459.67;
 ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "mol";  // mole
  ppl_unit_database[ppl_unit_pos].name2      = "mole";
  ppl_unit_database[ppl_unit_pos].name3      = "moles";
  ppl_unit_database[ppl_unit_pos].quantity   = "moles";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cd";   // candela
  ppl_unit_database[ppl_unit_pos].name2      = "candela";
  ppl_unit_database[ppl_unit_pos].name3      = "candelas";
  ppl_unit_database[ppl_unit_pos].quantity   = "intensity";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CANDELA]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "candlepower";   // candlepower
  ppl_unit_database[ppl_unit_pos].name2      = "candlepower";
  ppl_unit_database[ppl_unit_pos].name3      = "candlepower";
  ppl_unit_database[ppl_unit_pos].quantity   = "intensity";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.981;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CANDELA]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "rad";  // radians
  ppl_unit_database[ppl_unit_pos].name2      = "radian";
  ppl_unit_database[ppl_unit_pos].name3      = "radians";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "deg";  // degrees
  ppl_unit_database[ppl_unit_pos].name2      = "degree";
  ppl_unit_database[ppl_unit_pos].name3      = "degrees";
  ppl_unit_database[ppl_unit_pos].quantity   = "angle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "bit";  // bit
  ppl_unit_database[ppl_unit_pos].name2      = "bit";
  ppl_unit_database[ppl_unit_pos].name3      = "bits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "euro";  // cost
  ppl_unit_database[ppl_unit_pos].name2      = "euro";
  ppl_unit_database[ppl_unit_pos].name3      = "euros";
  ppl_unit_database[ppl_unit_pos].quantity   = "cost";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_COST]=1;
  ppl_unit_pos++;


  // -------------
  // Derived units
  // -------------

  ppl_unit_database[ppl_unit_pos].name1      = "sterad";  // steradians
  ppl_unit_database[ppl_unit_pos].name2      = "steradian";
  ppl_unit_database[ppl_unit_pos].name3      = "steradians";
  ppl_unit_database[ppl_unit_pos].quantity   = "solidangle";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "sqdeg";  // square degrees
  ppl_unit_database[ppl_unit_pos].name2      = "square degree";
  ppl_unit_database[ppl_unit_pos].name3      = "square degrees";
  ppl_unit_database[ppl_unit_pos].quantity   = "solidangle";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(M_PI/180, 2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "Hz";  // hertz
  ppl_unit_database[ppl_unit_pos].name2      = "hertz";
  ppl_unit_database[ppl_unit_pos].name3      = "hertz";
  ppl_unit_database[ppl_unit_pos].quantity   = "frequency";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "b";  // bytes
  ppl_unit_database[ppl_unit_pos].name2      = "byte";
  ppl_unit_database[ppl_unit_pos].name3      = "bytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "N";  // newton
  ppl_unit_database[ppl_unit_pos].name2      = "newton";
  ppl_unit_database[ppl_unit_pos].name3      = "newtons";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "J";  // joule
  ppl_unit_database[ppl_unit_pos].name2      = "joule";
  ppl_unit_database[ppl_unit_pos].name3      = "joules";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "erg";  // erg
  ppl_unit_database[ppl_unit_pos].name2      = "erg";
  ppl_unit_database[ppl_unit_pos].name3      = "ergs";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-7;
  ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cal";  // calorie
  ppl_unit_database[ppl_unit_pos].name2      = "calorie";
  ppl_unit_database[ppl_unit_pos].name3      = "calories";
  ppl_unit_database[ppl_unit_pos].quantity   = "energy";
  ppl_unit_database[ppl_unit_pos].multiplier = 4200;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "W";  // watt
  ppl_unit_database[ppl_unit_pos].name2      = "watt";
  ppl_unit_database[ppl_unit_pos].name3      = "watts";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "horsepower";  // horsepower
  ppl_unit_database[ppl_unit_pos].name2      = "horsepower";
  ppl_unit_database[ppl_unit_pos].name3      = "horsepower";
  ppl_unit_database[ppl_unit_pos].quantity   = "power";
  ppl_unit_database[ppl_unit_pos].multiplier = 745.699872;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "C";  // coulomb
  ppl_unit_database[ppl_unit_pos].name2      = "coulomb";
  ppl_unit_database[ppl_unit_pos].name3      = "coulombs";
  ppl_unit_database[ppl_unit_pos].quantity   = "charge";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] = 1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "F";  // farad
  ppl_unit_database[ppl_unit_pos].name2      = "farad";
  ppl_unit_database[ppl_unit_pos].name3      = "farad";
  ppl_unit_database[ppl_unit_pos].quantity   = "capacitance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    =-1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    = 4;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] = 2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "H";  // henry
  ppl_unit_database[ppl_unit_pos].name2      = "henry";
  ppl_unit_database[ppl_unit_pos].name3      = "henry";
  ppl_unit_database[ppl_unit_pos].quantity   = "inductance";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "T";  // tesla
  ppl_unit_database[ppl_unit_pos].name2      = "tesla";
  ppl_unit_database[ppl_unit_pos].name3      = "tesla";
  ppl_unit_database[ppl_unit_pos].quantity   = "magnetic field";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "Wb";  // weber
  ppl_unit_database[ppl_unit_pos].name2      = "weber";
  ppl_unit_database[ppl_unit_pos].name3      = "weber";
  ppl_unit_database[ppl_unit_pos].quantity   = "magnetic flux";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  return;
 }

