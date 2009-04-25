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

#define _PPL_UNITS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_units.h"

#include "ListTools/lt_memory.h"

unit *ppl_unit_database;

static int ppl_unit_pos = 0;

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
    ppl_unit_database[i].si         = ppl_unit_database[i].imperial  = ppl_unit_database[i].us = ppl_unit_database[i].ancient = 0;
    ppl_unit_database[i].MaxPrefix  = ppl_unit_database[i].MinPrefix = 0;
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[i].exponent[j] = 0;
   }

  ppl_unit_database[ppl_unit_pos].name1      = "m";    // Metre
  ppl_unit_database[ppl_unit_pos].name2      = "metre";
  ppl_unit_database[ppl_unit_pos].name3      = "metres";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cm";    // Centimetre
  ppl_unit_database[ppl_unit_pos].name2      = "centimetre";
  ppl_unit_database[ppl_unit_pos].name3      = "centimetres";
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "ang";  // Angstrom
  ppl_unit_database[ppl_unit_pos].name2      = "angstrom";
  ppl_unit_database[ppl_unit_pos].name3      = "angstroms";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-10;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "in";   // Inch
  ppl_unit_database[ppl_unit_pos].name2      = "inch";
  ppl_unit_database[ppl_unit_pos].name3      = "inches";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.0254;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "ft";   // Foot
  ppl_unit_database[ppl_unit_pos].name2      = "foot";
  ppl_unit_database[ppl_unit_pos].name3      = "feet";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.0254;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "yd";   // Yard
  ppl_unit_database[ppl_unit_pos].name2      = "yard";
  ppl_unit_database[ppl_unit_pos].name3      = "yards";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.9144;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "mi";   // Mile
  ppl_unit_database[ppl_unit_pos].name2      = "mile";
  ppl_unit_database[ppl_unit_pos].name3      = "miles";
  ppl_unit_database[ppl_unit_pos].multiplier = 1610;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;
  
  ppl_unit_database[ppl_unit_pos].name1      = "furlongs";// Furlong
  ppl_unit_database[ppl_unit_pos].name2      = "furlong";
  ppl_unit_database[ppl_unit_pos].name3      = "furlongs";
  ppl_unit_database[ppl_unit_pos].multiplier = 201.17;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "chains";// Chain
  ppl_unit_database[ppl_unit_pos].name2      = "chain";
  ppl_unit_database[ppl_unit_pos].name3      = "chains";
  ppl_unit_database[ppl_unit_pos].multiplier = 20.12;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "nautical miles";// Nautical mile
  ppl_unit_database[ppl_unit_pos].name2      = "nautical mile";
  ppl_unit_database[ppl_unit_pos].name3      = "nautical miles";
  ppl_unit_database[ppl_unit_pos].multiplier = 1850;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cubits";// Cubit
  ppl_unit_database[ppl_unit_pos].name2      = "cubit";
  ppl_unit_database[ppl_unit_pos].name3      = "cubits";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.04572;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "au";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].name2      = "astronomical unit";
  ppl_unit_database[ppl_unit_pos].name3      = "astronomical units";
  ppl_unit_database[ppl_unit_pos].multiplier = 149598e6;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "pc";   // Parsec
  ppl_unit_database[ppl_unit_pos].name2      = "parsec";
  ppl_unit_database[ppl_unit_pos].name3      = "parsecs";
  ppl_unit_database[ppl_unit_pos].multiplier = 3.08568025e16;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 9;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "s";    // Second
  ppl_unit_database[ppl_unit_pos].name2      = "second";
  ppl_unit_database[ppl_unit_pos].name3      = "seconds";
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
  ppl_unit_database[ppl_unit_pos].multiplier = 60;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "hr";   // Hour
  ppl_unit_database[ppl_unit_pos].name2      = "hour";
  ppl_unit_database[ppl_unit_pos].name3      = "hours";
  ppl_unit_database[ppl_unit_pos].multiplier = 3600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "dy";   // Day
  ppl_unit_database[ppl_unit_pos].name2      = "day";
  ppl_unit_database[ppl_unit_pos].name3      = "days";
  ppl_unit_database[ppl_unit_pos].multiplier = 86400;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "wk";   // Week
  ppl_unit_database[ppl_unit_pos].name2      = "week";
  ppl_unit_database[ppl_unit_pos].name3      = "weeks";
  ppl_unit_database[ppl_unit_pos].multiplier = 604800;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "yr";   // Year
  ppl_unit_database[ppl_unit_pos].name2      = "year";
  ppl_unit_database[ppl_unit_pos].name3      = "years";
  ppl_unit_database[ppl_unit_pos].multiplier = 31557600;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "kg";   // Kilogram
  ppl_unit_database[ppl_unit_pos].name2      = "kilogram";
  ppl_unit_database[ppl_unit_pos].name3      = "kilograms";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "g";    // Gram
  ppl_unit_database[ppl_unit_pos].name2      = "gram";
  ppl_unit_database[ppl_unit_pos].name3      = "grams";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "g";    // Gramme
  ppl_unit_database[ppl_unit_pos].name2      = "gramme";
  ppl_unit_database[ppl_unit_pos].name3      = "grammes";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e-3;
  ppl_unit_database[ppl_unit_pos].MinPrefix  = -24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].name2      = "metric ton";
  ppl_unit_database[ppl_unit_pos].name3      = "metric tons";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].MaxPrefix  = 24;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "t";    // Metric Tonne
  ppl_unit_database[ppl_unit_pos].name2      = "tonne";
  ppl_unit_database[ppl_unit_pos].name3      = "tonnes";
  ppl_unit_database[ppl_unit_pos].multiplier = 1e3;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "short ton"; // US Ton
  ppl_unit_database[ppl_unit_pos].name2      = "ton";
  ppl_unit_database[ppl_unit_pos].name3      = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "US imperial";
  ppl_unit_database[ppl_unit_pos].multiplier = 907;
  ppl_unit_database[ppl_unit_pos].us         = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "long ton"; // UK Ton
  ppl_unit_database[ppl_unit_pos].name2      = "ton";
  ppl_unit_database[ppl_unit_pos].name3      = "tons";
  ppl_unit_database[ppl_unit_pos].comment    = "UK imperial";
  ppl_unit_database[ppl_unit_pos].multiplier = 1016;
  ppl_unit_database[ppl_unit_pos].imperial   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "shekels"; // Shekel
  ppl_unit_database[ppl_unit_pos].name2      = "shekel";
  ppl_unit_database[ppl_unit_pos].name3      = "shekels";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.011;
  ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].name2      = "ampere";
  ppl_unit_database[ppl_unit_pos].name3      = "amperes";
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].name2      = "kelvin";
  ppl_unit_database[ppl_unit_pos].si         = 1;
  ppl_unit_database[ppl_unit_pos].cgs        = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "oC"; // oC
  ppl_unit_database[ppl_unit_pos].name2      = "centigrade";
  ppl_unit_database[ppl_unit_pos].name3      = "celsius";
  ppl_unit_database[ppl_unit_pos].offset     = 273.15;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "oF"; // oF
  ppl_unit_database[ppl_unit_pos].name3      = "fahrenheit";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.8;
  ppl_unit_database[ppl_unit_pos].offset     = 459.67;
 ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TEMPERATURE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "mol";  // mole
  ppl_unit_database[ppl_unit_pos].name2      = "mole";
  ppl_unit_database[ppl_unit_pos].name3      = "moles";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cd";   // candela
  ppl_unit_database[ppl_unit_pos].name2      = "candela";
  ppl_unit_database[ppl_unit_pos].name3      = "candelas";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CANDELA]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "candlepower";   // candlepower
  ppl_unit_database[ppl_unit_pos].multiplier = 0.981;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CANDELA]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "rad";  // radians
  ppl_unit_database[ppl_unit_pos].name2      = "radian";
  ppl_unit_database[ppl_unit_pos].name3      = "radians";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "deg";  // degrees
  ppl_unit_database[ppl_unit_pos].name2      = "degree";
  ppl_unit_database[ppl_unit_pos].name3      = "degrees";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "bit";  // bit
  ppl_unit_database[ppl_unit_pos].name3      = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "euro";  // cost
  ppl_unit_database[ppl_unit_pos].name3      = "euros";
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
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "sqdeg";  // square degrees
  ppl_unit_database[ppl_unit_pos].name2      = "square degree";
  ppl_unit_database[ppl_unit_pos].name3      = "square degrees";
  ppl_unit_database[ppl_unit_pos].multiplier = pow(M_PI/180, 2);
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "N";  // newton
  ppl_unit_database[ppl_unit_pos].name2      = "newton";
  ppl_unit_database[ppl_unit_pos].name3      = "newtons";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "J";  // joule
  ppl_unit_database[ppl_unit_pos].name2      = "joule";
  ppl_unit_database[ppl_unit_pos].name3      = "joules";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "cal";  // calorie
  ppl_unit_database[ppl_unit_pos].name2      = "calorie";
  ppl_unit_database[ppl_unit_pos].name3      = "calories";
  ppl_unit_database[ppl_unit_pos].multiplier = 4200;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-2;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "W";  // watt
  ppl_unit_database[ppl_unit_pos].name2      = "watt";
  ppl_unit_database[ppl_unit_pos].name3      = "watts";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].name1      = "horsepower";  // horsepower
  ppl_unit_database[ppl_unit_pos].multiplier = 745.699872;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]   = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH] = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]   =-3;
  ppl_unit_pos++;

  return;
 }
