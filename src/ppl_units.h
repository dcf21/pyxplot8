// ppl_units.h
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

#ifndef _PPL_UNITS_H
#define _PPL_UNITS_H 1

#define UNITS_MAX_BASEUNITS   24
#define UNITS_MAX           1024

#define UNIT_LENGTH      0
#define UNIT_TIME        1
#define UNIT_MASS        2
#define UNIT_CURRENT     3
#define UNIT_TEMPERATURE 4
#define UNIT_MOLE        5
#define UNIT_CANDELA     6
#define UNIT_ANGLE       7
#define UNIT_BIT         8
#define UNIT_COST        9

typedef struct unit
 {
  char         *nameAs, *nameAp, *nameFs, *nameFp, *comment, *quantity;
  double        multiplier, offset;
  unsigned char si, cgs, imperial, us, ancient, UserSel;
  int           MaxPrefix;
  int           MinPrefix;
  int           UserSelPrefix;
  double        exponent[UNITS_MAX_BASEUNITS];
 } unit;

typedef struct value
 {
  double        number;
  unsigned char dimensionless;
  double        exponent[UNITS_MAX_BASEUNITS];
 } value;

#ifndef _PPL_UNITS_C
extern unit  *ppl_unit_database;
extern int    ppl_unit_pos;
#endif

value *ppl_units_zero          (value *in);
char  *ppl_units_NumericDisplay(value *in, int N, int typeable);
char  *ppl_units_GetUnitStr    (value *in, double *NumberOut, int N, int typeable);
value *ppl_units_StringEvaluate(char *in);
void   ppl_units_pow (value *a, value *b, value *o, int *status, char *errtext);
void   ppl_units_mult(value *a, value *b, value *o, int *status, char *errtext);
void   ppl_units_div (value *a, value *b, value *o, int *status, char *errtext);
void   ppl_units_add (value *a, value *b, value *o, int *status, char *errtext);
void   ppl_units_sub (value *a, value *b, value *o, int *status, char *errtext);
void   ppl_units_mod (value *a, value *b, value *o, int *status, char *errtext);
int    ppl_units_DimEqual(value *a, value *b);
void   ppl_units_init();

#endif

