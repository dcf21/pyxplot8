// ppl_units.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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
  char         *nameAs, *nameAp, *nameLs, *nameLp, *nameFs, *nameFp, *alt1, *alt2, *alt3, *alt4, *comment, *quantity;
  double        multiplier, offset;
  unsigned char si, cgs, imperial, us, planck, ancient, UserSel, NotToBeCompounded, modified, TempType;
  int           MaxPrefix;
  int           MinPrefix;
  int           UserSelPrefix;
  double        exponent[UNITS_MAX_BASEUNITS];
 } unit;

typedef struct value
 {
  double        real, imag;
  unsigned char dimensionless, FlagComplex, modified, TempType;
  char         *string;
  double        exponent[UNITS_MAX_BASEUNITS];
 } value;

typedef struct PreferredUnit
 {
  int    NUnits;
  int    UnitID[UNITS_MAX_BASEUNITS];
  int    prefix[UNITS_MAX_BASEUNITS];
  double exponent[UNITS_MAX_BASEUNITS];
  value  value;
  unsigned char modified;
 } PreferredUnit;

#endif
