// ppl_units_fns.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
//
// $Id: ppl_units.h 337 2009-08-19 23:06:39Z dcf21 $
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

#ifndef _PPL_UNITS_FNS_H
#define _PPL_UNITS_FNS_H 1

#include "ListTools/lt_list.h"

#include "ppl_units.h"

#ifndef _PPL_UNITS_C
extern char *SIprefixes_full  [];
extern char *SIprefixes_abbrev[];
extern char *SIprefixes_latex [];

extern double TempTypeMultiplier[];
extern double TempTypeOffset    [];

extern unit  *ppl_unit_database;
extern int    ppl_unit_pos;
extern int    ppl_baseunit_pos;
extern List  *ppl_unit_PreferredUnits;
extern List  *ppl_unit_PreferredUnits_default;
#endif

value *ppl_units_zero          (value *in);
char  *ppl_units_NumericDisplay(value *in, int N, int typeable, int NSigFigs);

char  *ppl_units_GetUnitStr    (const value *in, double *NumberOutReal, double *NumberOutImag, int N, int DivAllowed, int typeable);
int __inline__ UnitNameCmp     (const char *in, const char *unit, const unsigned char CaseSensitive);
void   ppl_units_StringEvaluate(char *in, value *out, int *end, int *errpos, char *errtext);

void   MakePreferredUnit(PreferredUnit **output, char *instr, int OutputContext, int *errpos, char *errtext);

void   ppl_units_init();

#ifndef DISABLE_INLINE
#define MAKE_INLINE static __inline__
#include "ppl_units_inline.c"
#else
unsigned char ppl_units_DblEqual    (double a, double b);
unsigned char ppl_units_DblApprox   (double a, double b, double err);
void          ppl_units_DimCpy      (value *o, const value *i);
void          ppl_units_DimInverse  (value *o, const value *i);
int           ppl_units_DimEqual    (const value *a, const value *b);
int           ppl_units_DimEqual2   (const value *a, const unit  *b);
int           ppl_units_UnitDimEqual(const unit  *a, const unit  *b);
unsigned char TempTypeMatch         (unsigned char a, unsigned char b);
void          ppl_units_pow         (const value *a, const value *b, value *o, int *status, char *errtext);
void          ppl_units_mult        (const value *a, const value *b, value *o, int *status, char *errtext);
void          ppl_units_div         (const value *a, const value *b, value *o, int *status, char *errtext);
void          ppl_units_add         (const value *a, const value *b, value *o, int *status, char *errtext);
void          ppl_units_sub         (const value *a, const value *b, value *o, int *status, char *errtext);
void          ppl_units_mod         (const value *a, const value *b, value *o, int *status, char *errtext);
#endif

#endif

