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
#include <ctype.h>
#include <math.h>
#include <string.h>

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"

#include "StringTools/asciidouble.h"

unit *ppl_unit_database;
int   ppl_unit_pos = 0;

char *SIprefixes_full  [] = {"yocto","zepto","atto","femto","pico","nano","micro","milli","","kilo","mega","giga","tera","peta","exa","zetta","yotta"};
char *SIprefixes_abbrev[] = {"y","z","a","f","p","n","u","m","","k","M","G","T","P","E","Z","Y"};

// Make a value structure equal zero, and be dimensionless. Used for preconfiguring values.
value *ppl_units_zero(value *in)
 {
  int i;
  in->number = 0.0;
  in->dimensionless = 1;
  for (i=0; i<UNITS_MAX_BASEUNITS; i++) in->exponent[i]=0;
  return in;
 }

// Display a value with units
char *ppl_units_NumericDisplay(value *in, int N, int typeable)
 {
  static char outputA[LSTR_LENGTH], outputB[LSTR_LENGTH];
  double NumberOut;
  char *output, *unitstr;
  if (N==0) output = outputA;
  else      output = outputB;

  if (settings_term_default.UnitDisplayTypeable == SW_ONOFF_ON) typeable = 1;
  unitstr = ppl_units_GetUnitStr(in, &NumberOut, N, typeable);
  if (unitstr[0]=='\0') return NumericDisplay(NumberOut, N);
  else if (typeable==0) sprintf(output, "%s %s", NumericDisplay(NumberOut, N), unitstr);
  else                  sprintf(output, "%s%s" , NumericDisplay(NumberOut, N), unitstr);

  return output;
 }

// -------------------------------------------------------------------
// Functions for comparing the units of quantities
// -------------------------------------------------------------------

// Useful function for checking whether two doubles are roughly equal to one another
unsigned char __inline__ ppl_units_DblEqual(double a, double b)
 {
  if ( (fabs(a) < 1e-100) && (fabs(b) < 1e-100) ) return 1;
  if ( (fabs(a-b) > fabs(1e-7*a)) || (fabs(a-b) > fabs(1e-7*b)) ) return 0;
  return 1;
 }

void __inline__ ppl_units_DimCpy(value *o, value *i)
 {
  int j;
  o->dimensionless = i->dimensionless;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) o->exponent[j] = i->exponent[j];
  return;
 }

int __inline__ ppl_units_DimEqual(value *a, value *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

int __inline__ ppl_units_DimEqual2(value *a, unit *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

int __inline__ ppl_units_UnitDimEqual(unit *a, unit *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

// -------------------------------------------------------------------
// Routines for printing units
// -------------------------------------------------------------------

#define UNIT_INSCHEME(X)  (  ((X).si       && (settings_term_default.UnitScheme == SW_UNITSCH_SI  )) \
                          || ((X).cgs      && (settings_term_default.UnitScheme == SW_UNITSCH_CGS )) \
                          || ((X).imperial && (settings_term_default.UnitScheme == SW_UNITSCH_IMP )) \
                          || ((X).us       && (settings_term_default.UnitScheme == SW_UNITSCH_US  )) \
                          || ((X).ancient  && (settings_term_default.UnitScheme == SW_UNITSCH_ANC ))  )


void ppl_units_FindOptimalNextUnit(value *in, unit **best, double *pow)
 {
  int i,j,k,score,found=0,BestScore;
  double power;

  for (i=0; i<ppl_unit_pos; i++)
   {
    for (j=0; j<UNITS_MAX_BASEUNITS; j++)
     {
      if ( (ppl_unit_database[i].exponent[j] == 0) || (in->exponent[j]==0) ) continue;
      power = in->exponent[j] / ppl_unit_database[i].exponent[j];
      score = 0;
      for (k=0; k<UNITS_MAX_BASEUNITS; k++) if (ppl_units_DblEqual(in->exponent[k] , power*ppl_unit_database[i].exponent[k])) score++;

      if (found == 0) // This is first possible unit we've found, and we have nothing to compare it to.
       {
        *best     = ppl_unit_database+i;
        *pow      = power;
        BestScore = score;
        found     = 1;
       }

      // A unit in the current scheme always beats one not in current scheme
      if (( UNIT_INSCHEME(ppl_unit_database[i])) && (!UNIT_INSCHEME(**best)))
        { *best = ppl_unit_database+i; *pow = power; BestScore = score; continue; }
      if ((!UNIT_INSCHEME(ppl_unit_database[i])) && ( UNIT_INSCHEME(**best)))
        continue;

      // A unit which matches more dimensions wins
      if (score > BestScore)
        { *best = ppl_unit_database+i; *pow = power; BestScore = score; continue; }
      if (score < BestScore)
        continue;
     }
   }
  if (found==0) {*pow = 0; *best = NULL; return;}
  for (j=0; j<UNITS_MAX_BASEUNITS; j++)
   {
    in->exponent[j] -= (*pow) * (*best)->exponent[j];
   }
  return;
 }

void ppl_units_PrefixFix(value *in, unit **UnitList, double *UnitPow, int *UnitPref, int Nunits)
 {
  int     i,j;
  double  NewValue, PrefixBestVal;
  int     PrefixBestPos, BestPrefix;

  // Apply unit multipliers to the value we're going to display
  for (i=0; i<Nunits; i++) { in->number /= pow(UnitList[i]->multiplier , UnitPow[i]); UnitPref[i]=0; }

  // Search for alternative dimensionally-equivalent units which give a smaller value
  for (i=0; i<Nunits; i++)
   for (j=0; j<UNITS_MAX_BASEUNITS; j++)
    if (ppl_units_UnitDimEqual(UnitList[i] , ppl_unit_database + j))
     {
      NewValue = in->number * pow(UnitList[i]->multiplier / ppl_unit_database[j].multiplier , UnitPow[i]);

      // A user-preferred unit always beats a non-user preferred unit
      if ( ( (ppl_unit_database[j].UserSel)) && (!(UnitList[i]->UserSel)) )
       { UnitList[i] = ppl_unit_database+j; in->number = NewValue; continue; }
      if ( (!(ppl_unit_database[j].UserSel)) && ( (UnitList[i]->UserSel)) )
       continue;

      // A unit in the current scheme always beats one which is not
      if (( UNIT_INSCHEME(ppl_unit_database[j])) && (!UNIT_INSCHEME(*(UnitList[i]))))
        { UnitList[i] = ppl_unit_database+j; in->number = NewValue; continue; }
      if ((!UNIT_INSCHEME(ppl_unit_database[j])) && ( UNIT_INSCHEME(*(UnitList[i]))))
        continue;

      // Otherwise, a unit with a smaller display value wins
      if ((NewValue < (in->number)) && (NewValue >= 1))
        { UnitList[i] = ppl_unit_database+j; in->number = NewValue; }
     }

  // Search for an SI prefix we can use to reduce the size of this number
  if (settings_term_current.UnitDisplayPrefix == SW_ONOFF_ON)
   {
    PrefixBestPos = -1;
    PrefixBestVal = in->number;
    for (i=0; i<Nunits; i++) if (ppl_units_DblEqual(UnitPow[i] , 1))
     {
      for (j=UnitList[i]->MinPrefix; j<=UnitList[i]->MaxPrefix; j+=3)
       {
        NewValue = in->number / pow(10,j);
        if ( (NewValue >= 1) && ((NewValue < PrefixBestVal) || (PrefixBestVal<1)) )
         { PrefixBestPos = i; BestPrefix = j; PrefixBestVal = NewValue; }
       }
     }
    if (PrefixBestPos>=0)
     {
      in->number = PrefixBestVal;
      UnitPref[PrefixBestPos] = BestPrefix/3;
     }
   }
  return;
 }

// Main entry point for printing units
char *ppl_units_GetUnitStr(value *in, double *NumberOut, int N, int typeable)
 {
  static char outputA[LSTR_LENGTH], outputB[LSTR_LENGTH];
  char  *output;
  value  residual;
  unit         *UnitList[UNITS_MAX_BASEUNITS];
  double        UnitPow [UNITS_MAX_BASEUNITS];
  int           UnitPref[UNITS_MAX_BASEUNITS];
  unsigned char UnitDisp[UNITS_MAX_BASEUNITS];
  double        ExpMax;
  int           pos=0, OutputPos=0;
  int           i, j, found, first;

  if (settings_term_default.UnitDisplayTypeable == SW_ONOFF_ON) typeable = 1;

  if (N==0) output = outputA;
  else      output = outputB;

  if (in->dimensionless != 0)
   {
    output[0]='\0';
    if (NumberOut != NULL) *NumberOut = in->number;
    return output;
   }

  residual = *in;

  // Find a list of units which multiply together to match dimensions of quantity to display
  while (1)
   {
    if (pos>=UNITS_MAX_BASEUNITS) { ppl_error("Internal overflow whilst trying to display a unit."); break; }
    ppl_units_FindOptimalNextUnit(&residual, UnitList + pos, UnitPow + pos);
    UnitDisp[pos] = 0;
    if (ppl_units_DblEqual(UnitPow[pos],0)!=0) break;
    pos++;
   }

  // If user-preferred units in list have preferred prefixes (e.g. mm), apply exponents now
  for (i=0; i<pos; i++) if (UnitList[i]->UserSel != 0) residual.number /= pow(10,UnitList[i]->UserSelPrefix);

  // Go through list of units and fix prefixes / unit choice to minimise displayed number
  ppl_units_PrefixFix(&residual, UnitList, UnitPow, UnitPref, pos);

  // Display units one by one
  first = 1;
  while (1)
   {
    found = 0;
    for (i=0; i<pos; i++) if ((UnitDisp[i]==0) && ((found==0) || (UnitPow[i]>ExpMax))) { ExpMax=UnitPow[i]; found=1; j=i; }
    if (found==0) break;
    if (typeable && first) { strcpy(output+OutputPos, "*unit("); OutputPos+=strlen(output+OutputPos); }
    if (!first) // Print * or /
     {
      if ((UnitPow[j] > 0) || (ppl_units_DblEqual(UnitPow[j],0))) output[OutputPos++] = '*';
      else                                                        output[OutputPos++] = '/';
     }
    if (UnitPref[j] != 0) // Print SI prefix
     {
      if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON) strcpy(output+OutputPos, SIprefixes_abbrev[UnitPref[j]+8]);
      else                                                        strcpy(output+OutputPos, SIprefixes_full  [UnitPref[j]+8]);
      OutputPos+=strlen(output+OutputPos);
     }
    if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON)
     {
      if (UnitPow[j] >= 0) strcpy(output+OutputPos, UnitList[j]->nameAp);
      else                 strcpy(output+OutputPos, UnitList[j]->nameAs);
     } else {
      if (UnitPow[j] >= 0) strcpy(output+OutputPos, UnitList[j]->nameFp);
      else                 strcpy(output+OutputPos, UnitList[j]->nameFs);
     }
    OutputPos+=strlen(output+OutputPos);
    if ( (first && (!ppl_units_DblEqual(UnitPow[j],1))) || ((!first) && (!ppl_units_DblEqual(fabs(UnitPow[j]),1))) ) // Print power
     {
      output[OutputPos++]='*'; output[OutputPos++]='*';
      if (first) sprintf(output+OutputPos, "%s", NumericDisplay(     UnitPow[j] , N));
      else       sprintf(output+OutputPos, "%s", NumericDisplay(fabs(UnitPow[j]), N));
      OutputPos+=strlen(output+OutputPos);
     }
    UnitDisp[j] = 1;
    first = 0;
   }

  // Clean up and return
  if (typeable) output[OutputPos++] = ')';
  output[OutputPos] = '\0';
  if (NumberOut != NULL) *NumberOut = residual.number;
  return output;
 }

// ------------------------------------------------
// Function to evaluate strings of the form "m/s"
// ------------------------------------------------

void ppl_units_StringEvaluate(char *in, value *out, int *end, int *errpos, char *errtext)
 {
  int i=0,j,k,p;
  double power=1.0, powerneg=1.0;
  ppl_units_zero(out);
  out->number = 1;
  while (powerneg!=0.0)
   {
    p=0;
    while ((in[i]<=' ')&&(in[i]!='\0')) i++;
    for (j=0; j<ppl_unit_pos; j++)
     {
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameAp[k]!='\0') && (ppl_unit_database[j].nameAp[k]==in[i+k])); k++);
                  if ((ppl_unit_database[j].nameAp[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameAs[k]!='\0') && (ppl_unit_database[j].nameAs[k]==in[i+k])); k++);
                  if ((ppl_unit_database[j].nameAs[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameFp[k]!='\0') && (ppl_unit_database[j].nameFp[k]==in[i+k])); k++);
                  if ((ppl_unit_database[j].nameFp[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameFs[k]!='\0') && (ppl_unit_database[j].nameFs[k]==in[i+k])); k++);
                  if ((ppl_unit_database[j].nameFs[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0)   continue;
      i+=k;
      while ((in[i]<=' ')&&(in[i]!='\0')) i++;
      if (((in[i]=='^') && (i++,1)) || (((in[i]=='*') && (in[i+1]=='*')) && (i+=2,1)))
       {
        power = GetFloat(in+i,&k);
        if (k<=0) { *errpos=i; strcpy(errtext, "Syntax error: Was expecting a numerical constant here."); return; }
        i+=k;
        while ((in[i]<=' ')&&(in[i]!='\0')) i++;
       }
      for (k=0; k<UNITS_MAX_BASEUNITS; k++) out->exponent[k] += ppl_unit_database[j].exponent[k] * power * powerneg;
      out->number *= pow( ppl_unit_database[j].multiplier , power*powerneg );
      power = 1.0;
      if      (in[i]=='*') { powerneg= 1.0; i++; }
      else if (in[i]=='/') { powerneg=-1.0; i++; }
      else                 { powerneg= 0.0;      }
      break;
     }
    if (p==0)
     {
      if (in[i]==')') { powerneg=0.0; }
      else            { *errpos=i; strcpy(errtext, "No such unit."); return; }
     }
   }
  j=1;
  for (k=0; k<UNITS_MAX_BASEUNITS; k++) if (ppl_units_DblEqual(out->exponent[k], 0) == 0) j=0;
  out->dimensionless = j;
  if (end != NULL) *end=i;
  return;
 }

// -------------------------------
// ARITHMETIC OPERATIONS ON VALUES
// -------------------------------

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
  if (a->dimensionless != 0) { if ((o != a) && (o != b)) ppl_units_DimCpy(o,a); return; }

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
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) { if ((o != a) && (o != b)) ppl_units_DimCpy(o,a); return; }

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
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) { if ((o != a) && (o != b)) ppl_units_DimCpy(o,a); return; }

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
  if ((o != a) && (o != b)) ppl_units_DimCpy(o,a);
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  if (ppl_units_DimEqual(a, b) == 0)
   {
    if (a->dimensionless)
     { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand is dimensionless, while right operand has units of '%s'.", ppl_units_GetUnitStr(b, NULL, 1, 0) ); }
    else if (b->dimensionless)
     { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand has units of '%s', while right operand is dimensionless.", ppl_units_GetUnitStr(a, NULL, 0, 0) ); }
    else
     { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand has units of '%s', while right operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, 1, 0) ); }
    *status = 1; return;
   }
  return;
 }

void __inline__ ppl_units_sub (value *a, value *b, value *o, int *status, char *errtext)
 {
  o->number = a->number - b->number;
  if ((o != a) && (o != b)) ppl_units_DimCpy(o,a);
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  if (ppl_units_DimEqual(a, b) == 0)
   {
    if (a->dimensionless)
     { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand is dimensionless, while right operand has units of '%s'.", ppl_units_GetUnitStr(b, NULL, 1, 0) ); }
    else if (b->dimensionless)
     { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand has units of '%s', while right operand is dimensionless.", ppl_units_GetUnitStr(a, NULL, 0, 0) ); }
    else
     { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand has units of '%s', while right operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, 1, 0) ); }
    *status = 1; return;
   }
  return;
 }

void __inline__ ppl_units_mod (value *a, value *b, value *o, int *status, char *errtext)
 {
  o->number = a->number - floor(a->number / b->number) * b->number;
  if ((o != a) && (o != b)) ppl_units_DimCpy(o,a);
  if ((a->dimensionless != 0) && (b->dimensionless != 0)) return;

  if (a->dimensionless)
   { sprintf(errtext, "Mod operator can only be applied to dimensionless operands; right operand has units of '%s'.", ppl_units_GetUnitStr(b, NULL, 1, 0) ); }
  else if (b->dimensionless)
   { sprintf(errtext, "Mod operator can only be applied to dimensionless operands; left operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0) ); }
  else
   { sprintf(errtext, "Mod operator can only be applied to dimensionless operands; left operand has units of '%s', while right operand has units of '%s'.", ppl_units_GetUnitStr(a, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, 1, 0) ); }
  *status = 1;
  return;
 }

