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

  // Set up default variables
  ppl_units_zero(&v);
  v.number = M_PI;
  DictAppendValue(_ppl_UserSpace_Vars , "pi"            , PPL_USERSPACE_NUMERIC , v);
  v.number = M_E;
  DictAppendValue(_ppl_UserSpace_Vars , "e"             , PPL_USERSPACE_NUMERIC , v);
  v.number = 299792458.0;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH]=1 ; v.exponent[UNIT_TIME]=-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_c"         , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 4e-7*M_PI;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 1; v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] = -2; v.exponent[UNIT_CURRENT] = -2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_0"      , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 8.8541878176e-12;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] =-3; v.exponent[UNIT_MASS] =-1; v.exponent[UNIT_TIME] =  4; v.exponent[UNIT_CURRENT] =  2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_epsilon_0" , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 1.60217646e-19;
  v.dimensionless = 0;
  v.exponent[UNIT_CURRENT] = 1; v.exponent[UNIT_TIME] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_q"         , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 6.626068e-34;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_h"         , PPL_USERSPACE_NUMERIC , v);
  v.number = 1.0546e-34;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_hbar"      , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 6.0221415e23;
  v.dimensionless = 0;
  v.exponent[UNIT_MOLE] = -1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_NA"        , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 1.66053886e-27;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_u"       , PPL_USERSPACE_NUMERIC , v);
  v.number = 1.67262158e-27;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_p"       , PPL_USERSPACE_NUMERIC , v);
  v.number = 1.67492729e-27;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_m_n"       , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 9.27400949e-24;
  v.dimensionless = 0;
  v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_CURRENT] = 1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_mu_b"      , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 8.314472;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1; v.exponent[UNIT_MOLE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_R"         , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 1.3806503e-23;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_TEMPERATURE] =-1;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_kB"        , PPL_USERSPACE_NUMERIC , v);
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 2; v.exponent[UNIT_TIME] =-2;
  ppl_units_zero(&v);
  v.number = 5.6704e-8;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_TIME] =-3; v.exponent[UNIT_TEMPERATURE] =-4;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_sigma"     , PPL_USERSPACE_NUMERIC , v);
  ppl_units_zero(&v);
  v.number = 6.67300e-11;
  v.dimensionless = 0;
  v.exponent[UNIT_MASS] = 1; v.exponent[UNIT_LENGTH] = 3; v.exponent[UNIT_TIME] =-2; v.exponent[UNIT_MASS] =-2;
  DictAppendValue(_ppl_UserSpace_Vars , "phy_G"         , PPL_USERSPACE_NUMERIC , v);

  // Set up default mathematical functions
  DictAppendPtr  (_ppl_UserSpace_Funcs, "ceil"   , PPL_USERSPACE_SYSTEM+1, (void *)&ceil        ,0,0, DATATYPE_VOID); // 1 indicates that function takes (double)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "erf"    , PPL_USERSPACE_SYSTEM+1, (void *)&gsl_sf_erf  ,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "exp"    , PPL_USERSPACE_SYSTEM+1, (void *)&exp         ,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "floor"  , PPL_USERSPACE_SYSTEM+1, (void *)&floor       ,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "fmod"   , PPL_USERSPACE_SYSTEM+2, (void *)&fmod        ,0,0, DATATYPE_VOID); // 2 indicates that function takes (double, double)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "frexp"  , PPL_USERSPACE_SYSTEM+3, (void *)&frexp       ,0,0, DATATYPE_VOID); // 3 indicates that these functions take (double, int)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "gamma"  , PPL_USERSPACE_SYSTEM+1, (void *)&gsl_sf_gamma,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "ldexp"  , PPL_USERSPACE_SYSTEM+3, (void *)&ldexp       ,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "log"    , PPL_USERSPACE_SYSTEM+1, (void *)&log         ,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "log10"  , PPL_USERSPACE_SYSTEM+1, (void *)&log10       ,0,0, DATATYPE_VOID);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "random" , PPL_USERSPACE_SYSTEM+0, (void *)&frandom     ,0,0, DATATYPE_VOID); // 0 indicates that this function takes no arguments

  // These functions need replacing with unit-friendly versions
  DictAppendPtr  (_ppl_UserSpace_Funcs, "acos"   , PPL_USERSPACE_SYSTEM+1, (void *)&acos        ,0,0, DATATYPE_VOID); // // 1 indicates that function takes (double)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "asin"   , PPL_USERSPACE_SYSTEM+1, (void *)&asin        ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "atan"   , PPL_USERSPACE_SYSTEM+1, (void *)&atan        ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "atan2"  , PPL_USERSPACE_SYSTEM+2, (void *)&atan2       ,0,0, DATATYPE_VOID); // // 2 indicates that function takes (double, double)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "cos"    , PPL_USERSPACE_SYSTEM+1, (void *)&cos         ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "cosh"   , PPL_USERSPACE_SYSTEM+1, (void *)&cosh        ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "degrees", PPL_USERSPACE_SYSTEM+1, (void *)&degrees     ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "fabs"   , PPL_USERSPACE_SYSTEM+1, (void *)&fabs        ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "hypot"  , PPL_USERSPACE_SYSTEM+2, (void *)&hypot       ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "max"    , PPL_USERSPACE_SYSTEM+2, (void *)&max         ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "min"    , PPL_USERSPACE_SYSTEM+2, (void *)&min         ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "pow"    , PPL_USERSPACE_SYSTEM+2, (void *)&pow         ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "radians", PPL_USERSPACE_SYSTEM+1, (void *)&radians     ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "sin"    , PPL_USERSPACE_SYSTEM+1, (void *)&sin         ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "sinh"   , PPL_USERSPACE_SYSTEM+1, (void *)&sinh        ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "sqrt"   , PPL_USERSPACE_SYSTEM+1, (void *)&sqrt        ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "tan"    , PPL_USERSPACE_SYSTEM+1, (void *)&tan         ,0,0, DATATYPE_VOID); //
  DictAppendPtr  (_ppl_UserSpace_Funcs, "tanh"   , PPL_USERSPACE_SYSTEM+1, (void *)&tanh        ,0,0, DATATYPE_VOID); //

  return;
 }

void ppl_units_init()
 {
  int i=0,j=0;

  ppl_unit_database = (unit *)lt_malloc(UNITS_MAX*sizeof(unit));

  for (i=0;i<UNITS_MAX;i++)
   {
    ppl_unit_database[i].nameAs     = NULL;
    ppl_unit_database[i].nameAp     = NULL;
    ppl_unit_database[i].nameFs     = NULL;
    ppl_unit_database[i].nameFp     = NULL;
    ppl_unit_database[i].multiplier = 1.0;
    ppl_unit_database[i].offset     = 0.0;
    ppl_unit_database[i].UserSel    = 0;
    ppl_unit_database[i].si         = ppl_unit_database[i].imperial  = ppl_unit_database[i].us = ppl_unit_database[i].ancient = 0;
    ppl_unit_database[i].MaxPrefix  = ppl_unit_database[i].MinPrefix = 0;
    for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[i].exponent[j] = 0;
   }

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

  ppl_unit_database[ppl_unit_pos].nameAs     = "au";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameAp     = "au";   // Astronomical unit
  ppl_unit_database[ppl_unit_pos].nameFs     = "astronomical_unit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "astronomical_units";
  ppl_unit_database[ppl_unit_pos].quantity   = "length";
  ppl_unit_database[ppl_unit_pos].multiplier = 149598e6;
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameAp     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameFs     = "ampere";
  ppl_unit_database[ppl_unit_pos].nameFp     = "amperes";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameAp     = "A"; // Ampere
  ppl_unit_database[ppl_unit_pos].nameFs     = "amp";
  ppl_unit_database[ppl_unit_pos].nameFp     = "amps";
  ppl_unit_database[ppl_unit_pos].quantity   = "current";
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].nameAp     = "K"; // Kelvin
  ppl_unit_database[ppl_unit_pos].nameFs     = "kelvin";
  ppl_unit_database[ppl_unit_pos].nameFp     = "kelvin";
  ppl_unit_database[ppl_unit_pos].quantity   = "temperature";
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MOLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "cd";   // candela
  ppl_unit_database[ppl_unit_pos].nameAp     = "cd";   // candela
  ppl_unit_database[ppl_unit_pos].nameFs     = "candela";
  ppl_unit_database[ppl_unit_pos].nameFp     = "candelas";
  ppl_unit_database[ppl_unit_pos].quantity   = "light_intensity";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CANDELA]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "candlepower";   // candlepower
  ppl_unit_database[ppl_unit_pos].nameAp     = "candlepower";   // candlepower
  ppl_unit_database[ppl_unit_pos].nameFs     = "candlepower";
  ppl_unit_database[ppl_unit_pos].nameFp     = "candlepower";
  ppl_unit_database[ppl_unit_pos].quantity   = "light_intensity";
  ppl_unit_database[ppl_unit_pos].multiplier = 0.981;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CANDELA]=1;
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
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "bit";  // bit
  ppl_unit_database[ppl_unit_pos].nameAp     = "bits"; // bit
  ppl_unit_database[ppl_unit_pos].nameFs     = "bit";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bits";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "euro";  // cost
  ppl_unit_database[ppl_unit_pos].nameAp     = "euros"; // cost
  ppl_unit_database[ppl_unit_pos].nameFs     = "euro";
  ppl_unit_database[ppl_unit_pos].nameFp     = "euros";
  ppl_unit_database[ppl_unit_pos].quantity   = "cost";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_ANGLE]=-1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "b";  // bytes
  ppl_unit_database[ppl_unit_pos].nameAp     = "b";  // bytes
  ppl_unit_database[ppl_unit_pos].nameFs     = "byte";
  ppl_unit_database[ppl_unit_pos].nameFp     = "bytes";
  ppl_unit_database[ppl_unit_pos].quantity   = "bits";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_BIT]=1;
  ppl_unit_pos++;

  ppl_unit_database[ppl_unit_pos].nameAs     = "N";  // newton
  ppl_unit_database[ppl_unit_pos].nameAp     = "N";  // newton
  ppl_unit_database[ppl_unit_pos].nameFs     = "newton";
  ppl_unit_database[ppl_unit_pos].nameFp     = "newtons";
  ppl_unit_database[ppl_unit_pos].quantity   = "force";
  ppl_unit_database[ppl_unit_pos].multiplier = 1;
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
  ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
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

  ppl_unit_database[ppl_unit_pos].nameAs     = "C";  // coulomb
  ppl_unit_database[ppl_unit_pos].nameAp     = "C";  // coulomb
  ppl_unit_database[ppl_unit_pos].nameFs     = "coulomb";
  ppl_unit_database[ppl_unit_pos].nameFp     = "coulombs";
  ppl_unit_database[ppl_unit_pos].quantity   = "charge";
  ppl_unit_database[ppl_unit_pos].multiplier = 1.0;
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
  ppl_unit_database[ppl_unit_pos].si = ppl_unit_database[ppl_unit_pos].cgs = ppl_unit_database[ppl_unit_pos].imperial = ppl_unit_database[ppl_unit_pos].us = ppl_unit_database[ppl_unit_pos].ancient = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_LENGTH]  = 2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_MASS]    = 1;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_TIME]    =-2;
  ppl_unit_database[ppl_unit_pos].exponent[UNIT_CURRENT] =-1;
  ppl_unit_pos++;

  return;
 }

