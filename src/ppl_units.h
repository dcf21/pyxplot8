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
#define UNIT_ANGLE       6
#define UNIT_BIT         7
#define UNIT_COST        8
#define UNIT_FIRSTUSER   9

typedef struct unit
 {
  char         *nameAs, *nameAp, *nameLs, *nameLp, *nameFs, *nameFp, *comment, *quantity;
  double        multiplier, offset;
  unsigned char si, cgs, imperial, us, planck, ancient, UserSel, modified;
  int           MaxPrefix;
  int           MinPrefix;
  int           UserSelPrefix;
  double        exponent[UNITS_MAX_BASEUNITS];
 } unit;

typedef struct value
 {
  double        real, imag;
  unsigned char dimensionless, FlagComplex, modified;
  char         *string;
  double        exponent[UNITS_MAX_BASEUNITS];
 } value;

#ifndef _PPL_UNITS_C
extern char *SIprefixes_full  [];
extern char *SIprefixes_abbrev[];
extern char *SIprefixes_latex [];

extern unit  *ppl_unit_database;
extern int    ppl_unit_pos;
extern int    ppl_baseunit_pos;
#endif

value *ppl_units_zero          (value *in);
char  *ppl_units_NumericDisplay(value *in, int N, int typeable);

unsigned char __inline__ ppl_units_DblEqual    (double a, double b);
unsigned char __inline__ ppl_units_DblApprox   (double a, double b, double err);
void          __inline__ ppl_units_DimCpy      (value *o, const value *i);
void          __inline__ ppl_units_DimInverse  (value *o, const value *i);
int           __inline__ ppl_units_DimEqual    (const value *a, const value *b);
int           __inline__ ppl_units_DimEqual2   (const value *a, const unit  *b);
int           __inline__ ppl_units_UnitDimEqual(const unit  *a, const unit  *b);


char  *ppl_units_GetUnitStr    (const value *in, double *NumberOutReal, double *NumberOutImag, int N, int typeable);
void   ppl_units_StringEvaluate(char *in, value *out, int *end, int *errpos, char *errtext);

void   ppl_units_pow (const value *a, const value *b, value *o, int *status, char *errtext);
void   ppl_units_mult(const value *a, const value *b, value *o, int *status, char *errtext);
void   ppl_units_div (const value *a, const value *b, value *o, int *status, char *errtext);
void   ppl_units_add (const value *a, const value *b, value *o, int *status, char *errtext);
void   ppl_units_sub (const value *a, const value *b, value *o, int *status, char *errtext);
void   ppl_units_mod (const value *a, const value *b, value *o, int *status, char *errtext);

void   ppl_units_init();

#endif

