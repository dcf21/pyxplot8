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

#include <gsl/gsl_complex.h>
#include <gsl/gsl_complex_math.h>
#include <gsl/gsl_math.h>

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"

#include "StringTools/asciidouble.h"

unit *ppl_unit_database;
int   ppl_unit_pos = 0;
int   ppl_baseunit_pos = UNIT_FIRSTUSER; 

char *SIprefixes_full  [] = {"yocto","zepto","atto","femto","pico","nano","micro","milli","","kilo","mega","giga","tera","peta","exa","zetta","yotta"};
char *SIprefixes_abbrev[] = {"y","z","a","f","p","n","u","m","","k","M","G","T","P","E","Z","Y"};
char *SIprefixes_latex [] = {"y","z","a","f","p","n","\\mu ","m","","k","M","G","T","P","E","Z","Y"};

// Make a value structure equal zero, and be dimensionless. Used for preconfiguring values.
value *ppl_units_zero(value *in)
 {
  int i;
  in->real = in->imag = 0.0;
  in->dimensionless = 1;
  in->modified = in->FlagComplex = 0;
  in->string = NULL;
  for (i=0; i<UNITS_MAX_BASEUNITS; i++) in->exponent[i]=0;
  return in;
 }

// Display a value with units
char *ppl_units_NumericDisplay(value *in, int N, int typeable)
 {
  static char outputA[LSTR_LENGTH], outputB[LSTR_LENGTH];
  double NumberOutReal, NumberOutImag, OoM;
  char *output, *unitstr;
  int i=0;
  if (N==0) output = outputA;
  else      output = outputB;

  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && (in->FlagComplex!=0)) return NumericDisplay(GSL_NAN, N, settings_term_current.SignificantFigures, (typeable==SW_DISPLAY_L));

  if (typeable==0) typeable = settings_term_current.NumDisplay;
  unitstr = ppl_units_GetUnitStr(in, &NumberOutReal, &NumberOutImag, N, typeable);

  if (((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) && (in->FlagComplex!=0)) || (!gsl_finite(NumberOutReal)) || (!gsl_finite(NumberOutImag)))
   {
    if (typeable == SW_DISPLAY_L) output[i++] = '$';
    strcpy(output+i, NumericDisplay(GSL_NAN, N, settings_term_current.SignificantFigures, (typeable==SW_DISPLAY_L)));
    i+=strlen(output+i);
   }
  else
   {
    OoM = hypot(NumberOutReal , NumberOutImag) * pow(10 , -settings_term_current.SignificantFigures);

    if (typeable == SW_DISPLAY_L) output[i++] = '$';
    if ((fabs(NumberOutReal) >= OoM) && (fabs(NumberOutImag) > OoM)) output[i++] = '('; // open brackets on complex number
    if (fabs(NumberOutReal) >= OoM) { strcpy(output+i, NumericDisplay(NumberOutReal, N, settings_term_current.SignificantFigures, (typeable==SW_DISPLAY_L))); i+=strlen(output+i); }
    if ((fabs(NumberOutReal) >= OoM) && (fabs(NumberOutImag) > OoM) && (NumberOutImag > 0)) output[i++] = '+';
    if (fabs(NumberOutImag) > OoM)
     {
      if (fabs(NumberOutImag-1)>=OoM)
       {
        strcpy(output+i, NumericDisplay(NumberOutImag, N, settings_term_current.SignificantFigures, (typeable==SW_DISPLAY_L)));
        i+=strlen(output+i);
       }
      if (typeable != SW_DISPLAY_T) output[i++] = 'i';
      else                          { strcpy(output+i, "*sqrt(-1)"); i+=strlen(output+i); }
     }
    if ((fabs(NumberOutReal) >= OoM) && (fabs(NumberOutImag) > OoM)) output[i++] = ')'; // close brackets on complex number
   }

  if (unitstr[0]!='\0')
   {
    if      (typeable == SW_DISPLAY_N) output[i++] = ' ';
    else if (typeable == SW_DISPLAY_L) { output[i++] = '\\'; output[i++] = ','; }
    sprintf(output+i, "%s", unitstr);
    i+=strlen(output+i); // Add unit string as required
   }

  if (typeable == SW_DISPLAY_L) output[i++] = '$';
  output[i++] = '\0'; // null terminate string
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

void __inline__ ppl_units_DimCpy(value *o, const value *i)
 {
  int j;
  o->dimensionless = i->dimensionless;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) o->exponent[j] = i->exponent[j];
  return;
 }

void __inline__ ppl_units_DimInverse(value *o, const value *i)
 {
  int j;
  o->dimensionless = i->dimensionless;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) o->exponent[j] = -i->exponent[j];
  return;
 }

int __inline__ ppl_units_DimEqual(const value *a, const value *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

int __inline__ ppl_units_DimEqual2(const value *a, const unit *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

int __inline__ ppl_units_UnitDimEqual(const unit *a, const unit *b)
 {
  int j;
  for (j=0; j<UNITS_MAX_BASEUNITS; j++) if (ppl_units_DblEqual(a->exponent[j] , b->exponent[j]) == 0) return 0;
  return 1;
 }

// -------------------------------------------------------------------
// Routines for printing units
// -------------------------------------------------------------------

#define UNIT_INSCHEME(X)  (  ((X).si       && (settings_term_current.UnitScheme == SW_UNITSCH_SI  )) \
                          || ((X).cgs      && (settings_term_current.UnitScheme == SW_UNITSCH_CGS )) \
                          || ((X).imperial && (settings_term_current.UnitScheme == SW_UNITSCH_IMP )) \
                          || ((X).us       && (settings_term_current.UnitScheme == SW_UNITSCH_US  )) \
                          || ((X).planck   && (settings_term_current.UnitScheme == SW_UNITSCH_PLK )) \
                          || ((X).ancient  && (settings_term_current.UnitScheme == SW_UNITSCH_ANC ))  )


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
      if ((score == BestScore) && (ppl_units_DblEqual(fabs(power), 1.0) && (!ppl_units_DblEqual(fabs(*pow), 1.0))) )
        { *best = ppl_unit_database+i; *pow = power; continue; }
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
  double  NewValueReal, NewValueImag, PrefixBestVal, NewMagnitude, OldMagnitude;
  int     PrefixBestPos, BestPrefix;

  // Apply unit multipliers to the value we're going to display
  for (i=0; i<Nunits; i++)
   {
    in->real /= pow(UnitList[i]->multiplier , UnitPow[i]);
    in->imag /= pow(UnitList[i]->multiplier , UnitPow[i]);
    UnitPref[i]=0;
   }

  // Search for alternative dimensionally-equivalent units which give a smaller value
  for (i=0; i<Nunits; i++)
   for (j=0; j<ppl_unit_pos; j++)
    if (ppl_units_UnitDimEqual(UnitList[i] , ppl_unit_database + j))
     {
      OldMagnitude = hypot(in->real , in->imag);
      NewValueReal = in->real * pow(UnitList[i]->multiplier / ppl_unit_database[j].multiplier , UnitPow[i]);
      NewValueImag = in->imag * pow(UnitList[i]->multiplier / ppl_unit_database[j].multiplier , UnitPow[i]);
      NewMagnitude = hypot(NewValueReal , NewValueImag);

      // A user-preferred unit always beats a non-user preferred unit
      if ( ( (ppl_unit_database[j].UserSel)) && (!(UnitList[i]->UserSel)) )
       { UnitList[i] = ppl_unit_database+j; in->real = NewValueReal; in->imag = NewValueImag; continue; }
      if ( (!(ppl_unit_database[j].UserSel)) && ( (UnitList[i]->UserSel)) )
       continue;

      // A unit in the current scheme always beats one which is not
      if (( UNIT_INSCHEME(ppl_unit_database[j])) && (!UNIT_INSCHEME(*(UnitList[i]))))
        { UnitList[i] = ppl_unit_database+j; in->real = NewValueReal; in->imag = NewValueImag; continue; }
      if ((!UNIT_INSCHEME(ppl_unit_database[j])) && ( UNIT_INSCHEME(*(UnitList[i]))))
        continue;

      // Otherwise, a unit with a smaller display value wins
      if ((NewMagnitude < OldMagnitude) && (NewMagnitude >= 1))
        { UnitList[i] = ppl_unit_database+j; in->real = NewValueReal; in->imag = NewValueImag; }
     }

  // Apply unit multiplier which arise from user-preferred SI prefixes, for example, millimetres
  for (i=0; i<Nunits; i++)
   if (UnitList[i]->UserSel)
    {
     in->real /= pow(10,(UnitList[i]->UserSelPrefix-8)*3);
     in->imag /= pow(10,(UnitList[i]->UserSelPrefix-8)*3);
     UnitPref[i] = UnitList[i]->UserSelPrefix-8;
    }

  // Search for an SI prefix we can use to reduce the size of this number
  if (settings_term_current.UnitDisplayPrefix == SW_ONOFF_ON)
   {
    OldMagnitude = hypot(in->real , in->imag);
    PrefixBestPos = -1;
    PrefixBestVal = OldMagnitude;
    for (i=0; i<Nunits; i++) if (ppl_units_DblEqual(UnitPow[i] , 1))
     if (UnitList[i]->UserSel == 0)
      for (j=UnitList[i]->MinPrefix; j<=UnitList[i]->MaxPrefix; j+=3)
       {
        NewMagnitude = OldMagnitude / pow(10,j);
        if ( (NewMagnitude >= 1) && ((NewMagnitude < PrefixBestVal) || (PrefixBestVal<1)) )
         { PrefixBestPos = i; BestPrefix = j; PrefixBestVal = NewMagnitude; }
       }
    if (PrefixBestPos>=0)
     {
      in->real /= pow(10,BestPrefix);
      in->imag /= pow(10,BestPrefix);
      UnitPref[PrefixBestPos] = BestPrefix/3;
     }
   }
  return;
 }

// Main entry point for printing units
char *ppl_units_GetUnitStr(const value *in, double *NumberOutReal, double *NumberOutImag, int N, int typeable)
 {
  static char outputA[LSTR_LENGTH], outputB[LSTR_LENGTH], outputC[LSTR_LENGTH];
  char  *output,*temp;
  value  residual;
  unit         *UnitList[UNITS_MAX_BASEUNITS];
  double        UnitPow [UNITS_MAX_BASEUNITS];
  int           UnitPref[UNITS_MAX_BASEUNITS];
  unsigned char UnitDisp[UNITS_MAX_BASEUNITS];
  double        ExpMax;
  int           pos=0, OutputPos=0;
  int           i, j, k, l, found, first;

  if (typeable==0) typeable = settings_term_current.NumDisplay;

  if      (N==0) output = outputA;
  else if (N==1) output = outputB;
  else           output = outputC;

  if (in->dimensionless != 0)
   {
    output[0]='\0';
    if (NumberOutReal != NULL) *NumberOutReal = in->real;
    if (NumberOutImag != NULL) *NumberOutImag = in->imag;
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

  // Go through list of units and fix prefixes / unit choice to minimise displayed number
  ppl_units_PrefixFix(&residual, UnitList, UnitPow, UnitPref, pos);

  // Display units one by one
  first = 1;
  while (1)
   {
    found = 0;
    for (i=0; i<pos; i++) if ((UnitDisp[i]==0) && ((found==0) || (UnitPow[i]>ExpMax))) { ExpMax=UnitPow[i]; found=1; j=i; }
    if (found==0) break;
    if ((typeable==SW_DISPLAY_T) && first) { strcpy(output+OutputPos, "*unit("); OutputPos+=strlen(output+OutputPos); }
    if (!first) // Print * or /
     {
      if ((UnitPow[j] > 0) || (ppl_units_DblEqual(UnitPow[j],0))) { if (typeable==SW_DISPLAY_L) { output[OutputPos++] = '\\'; output[OutputPos++] = ','; }  else output[OutputPos++] = '*'; }
      else                                                        { output[OutputPos++] = '/'; }
     }
    if (typeable==SW_DISPLAY_L) { strcpy(output+OutputPos, "\\mathrm{"); OutputPos+=strlen(output+OutputPos); }
    if (UnitPref[j] != 0) // Print SI prefix
     {
      if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON)
       {
        if (typeable!=SW_DISPLAY_L) strcpy(output+OutputPos, SIprefixes_abbrev[UnitPref[j]+8]);
        else                        strcpy(output+OutputPos, SIprefixes_latex [UnitPref[j]+8]);
       }
      else
       { strcpy(output+OutputPos, SIprefixes_full  [UnitPref[j]+8]); }
      OutputPos+=strlen(output+OutputPos);
     }
    if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON)
     {
      if (typeable!=SW_DISPLAY_L)
       {
        if (UnitPow[j] >= 0) strcpy(output+OutputPos, UnitList[j]->nameAp); // Use abbreviated name for unit
        else                 strcpy(output+OutputPos, UnitList[j]->nameAs);
       }
      else
       {
        if (UnitPow[j] >= 0) strcpy(output+OutputPos, UnitList[j]->nameLp); // Use abbreviated LaTeX name for unit
        else                 strcpy(output+OutputPos, UnitList[j]->nameLs);
       }
     } else {
      if (UnitPow[j] >= 0) temp = UnitList[j]->nameFp; // Use full name for unit...
      else                 temp = UnitList[j]->nameFs;

      if (typeable!=SW_DISPLAY_L)
       { strcpy(output+OutputPos, temp); } // ... either as it comes
      else
       {
        for (k=l=0;temp[k]!='\0';k++)
         {
          if (temp[k]=='_') output[OutputPos+(l++)]='\\'; // ... or with escaped underscores for LaTeX
          output[OutputPos+(l++)]=temp[k];
         }
        output[OutputPos+(l++)]='\0';
       }
     }
    OutputPos+=strlen(output+OutputPos);
    if (typeable==SW_DISPLAY_L) { output[OutputPos++]='}'; }
    if ( (first && (!ppl_units_DblEqual(UnitPow[j],1))) || ((!first) && (!ppl_units_DblEqual(fabs(UnitPow[j]),1))) ) // Print power
     {
      if (typeable==SW_DISPLAY_L) { output[OutputPos++]='^'; output[OutputPos++]='{'; }
      else                        { output[OutputPos++]='*'; output[OutputPos++]='*'; }
      if (first) sprintf(output+OutputPos, "%s", NumericDisplay(     UnitPow[j] , N, settings_term_current.SignificantFigures, (typeable==SW_DISPLAY_L)));
      else       sprintf(output+OutputPos, "%s", NumericDisplay(fabs(UnitPow[j]), N, settings_term_current.SignificantFigures, (typeable==SW_DISPLAY_L)));
      OutputPos+=strlen(output+OutputPos);
      if (typeable==SW_DISPLAY_L) { output[OutputPos++]='}'; }
     }
    UnitDisp[j] = 1;
    first = 0;
   }

  // Clean up and return
  if (typeable==SW_DISPLAY_T) output[OutputPos++] = ')';
  output[OutputPos] = '\0';
  if (NumberOutReal != NULL) *NumberOutReal = residual.real;
  if (NumberOutImag != NULL) *NumberOutImag = residual.imag;
  return output;
 }

// ------------------------------------------------
// Function to evaluate strings of the form "m/s"
// ------------------------------------------------

void ppl_units_StringEvaluate(char *in, value *out, int *end, int *errpos, char *errtext)
 {
  int i=0,j=0,k,l,m,p;
  double power=1.0, powerneg=1.0, multiplier;
  ppl_units_zero(out);

  while ((in[i]<=' ')&&(in[i]!='\0')) i++;
  out->real = GetFloat(in+i , &j); // Unit strings can have numbers out the front
  if (j<0) j=0;
  i+=j;
  if (j==0)
   { out->real = 1.0; }
  else 
   {
    while ((in[i]<=' ')&&(in[i]!='\0')) i++;
    if      (in[i]=='*')   i++;
    else if (in[i]=='/') { i++; powerneg=-1.0; }
   }

  while (powerneg!=0.0)
   {
    p=0;
    while ((in[i]<=' ')&&(in[i]!='\0')) i++;
    for (j=0; j<ppl_unit_pos; j++)
     {
      multiplier = 1.0;
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameAp[k]!='\0') && (ppl_unit_database[j].nameAp[k]==in[i+k])); k++);
                  if ((ppl_unit_database[j].nameAp[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameAs[k]!='\0') && (ppl_unit_database[j].nameAs[k]==in[i+k])); k++);
                  if ((ppl_unit_database[j].nameAs[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameFp[k]!='\0') && (toupper(ppl_unit_database[j].nameFp[k])==toupper(in[i+k]))); k++);
                  if ((ppl_unit_database[j].nameFp[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0) { for (k=0; ((ppl_unit_database[j].nameFs[k]!='\0') && (toupper(ppl_unit_database[j].nameFs[k])==toupper(in[i+k]))); k++);
                  if ((ppl_unit_database[j].nameFs[k]=='\0') && (!(isalnum(in[i+k]) || (in[i+k]=='_')))) p=1; }
      if (p==0)
       {
        for (l=ppl_unit_database[j].MinPrefix/3+8; l<=ppl_unit_database[j].MaxPrefix/3+8; l++)
         {
          if (l==8) continue;
          for (k=0; ((SIprefixes_full[l][k]!='\0') && (toupper(SIprefixes_full[l][k])==toupper(in[i+k]))); k++);
          if (SIprefixes_full[l][k]=='\0')
           {
            for (m=0; ((ppl_unit_database[j].nameFp[m]!='\0') && (toupper(ppl_unit_database[j].nameFp[m])==toupper(in[i+k+m]))); m++);
            if ((ppl_unit_database[j].nameFp[m]=='\0') && (!(isalnum(in[i+k+m]) || (in[i+k+m]=='_')))) { p=1; k+=m; multiplier=pow(10,(l-8)*3); break; }
            for (m=0; ((ppl_unit_database[j].nameFs[m]!='\0') && (toupper(ppl_unit_database[j].nameFs[m])==toupper(in[i+k+m]))); m++);
            if ((ppl_unit_database[j].nameFs[m]=='\0') && (!(isalnum(in[i+k+m]) || (in[i+k+m]=='_')))) { p=1; k+=m; multiplier=pow(10,(l-8)*3); break; }
           }
          for (k=0; ((SIprefixes_abbrev[l][k]!='\0') && (SIprefixes_abbrev[l][k]==in[i+k])); k++);
          if (SIprefixes_abbrev[l][k]=='\0')
           {
            for (m=0; ((ppl_unit_database[j].nameAp[m]!='\0') && (ppl_unit_database[j].nameAp[m]==in[i+k+m])); m++);
            if ((ppl_unit_database[j].nameAp[m]=='\0') && (!(isalnum(in[i+k+m]) || (in[i+k+m]=='_')))) { p=1; k+=m; multiplier=pow(10,(l-8)*3); break; }
            for (m=0; ((ppl_unit_database[j].nameAs[m]!='\0') && (ppl_unit_database[j].nameAs[m]==in[i+k+m])); m++);
            if ((ppl_unit_database[j].nameAs[m]=='\0') && (!(isalnum(in[i+k+m]) || (in[i+k+m]=='_')))) { p=1; k+=m; multiplier=pow(10,(l-8)*3); break; }
           }
         }
       }
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
      out->real *= multiplier * pow( ppl_unit_database[j].multiplier , power*powerneg );
      power = 1.0;
      if      (in[i]=='*') { powerneg= 1.0; i++; }
      else if (in[i]=='/') { powerneg=-1.0; i++; }
      else                 { powerneg= 0.0;      }
      break;
     }
    if (p==0)
     {
      if ((in[i]==')') || (in[i]=='\0'))  { powerneg=0.0; }
      else                                { *errpos=i; strcpy(errtext, "No such unit."); return; }
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

void __inline__ ppl_units_pow (const value *a, const value *b, value *o, int *status, char *errtext)
 {
  int i;
  double exponent=0;
  gsl_complex ac, bc;
  unsigned char DimLess=1;

  if (b->dimensionless == 0)
   {
    if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { ppl_units_zero(o); o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
    else { sprintf(errtext, "Exponent should be dimensionless, but instead has dimensions of <%s>.", ppl_units_GetUnitStr(b, NULL, NULL, 0, 0)); *status = 1; return; }
   }

  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) || ((!a->FlagComplex) && (!b->FlagComplex))) // Real pow()
   {
    if (a->FlagComplex || b->FlagComplex) { o->real = GSL_NAN; }
    else                                  { exponent = b->real; o->real = pow(a->real, exponent); }
    o->imag = 0.0;
    o->FlagComplex=0;
   }
  else // Complex pow()
   {
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { ppl_units_zero(o); o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
    else if (a->dimensionless == 0)
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { ppl_units_zero(o); o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
      else { sprintf(errtext, "Raising quantities with physical units to complex powers produces quantities with complex physical dimensions, which is forbidden. The operand in question has dimensions of <%s>.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0)); *status = 1; return; }
     }
    else
     {
      if (!b->FlagComplex)
       {
        GSL_SET_COMPLEX(&ac, a->real, a->imag);
        ac = gsl_complex_pow_real(ac, b->real);
       }
      else
       {
        GSL_SET_COMPLEX(&ac, a->real, a->imag);
        GSL_SET_COMPLEX(&bc, b->real, b->imag);
        ac = gsl_complex_pow(ac, bc);
       }
      o->real = GSL_REAL(ac);
      o->imag = GSL_IMAG(ac);
      o->FlagComplex = !ppl_units_DblEqual(o->imag, 0);
     }
   }

  if (a->dimensionless != 0) { if ((o != a) && (o != b)) ppl_units_DimCpy(o,a); return; }

  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   {
    o->exponent[i] = a->exponent[i] * exponent;
    if (ppl_units_DblEqual(o->exponent[i], 0) == 0) DimLess=0;
    if (fabs(o->exponent[i]) > 20000 )
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { ppl_units_zero(o); o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
      else { sprintf(errtext, "Overflow of physical dimensions of argument."); *status = 1; return; }
     }
   }
  o->dimensionless = DimLess;
  return;
 }

void __inline__ ppl_units_mult(const value *a, const value *b, value *o, int *status, char *errtext)
 {
  int i;
  double tmp;
  unsigned char DimLess=1;

  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) || ((!a->FlagComplex) && (!b->FlagComplex))) // Real multiplication
   {
    if (a->FlagComplex || b->FlagComplex) { o->real = GSL_NAN; }
    else                                  { o->real = a->real * b->real; }
    o->imag = 0.0;
    o->FlagComplex=0;
   }
  else // Complex multiplication
   {
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; }
    else
     {
      tmp            = (a->real * b->real - a->imag * b->imag);
      o->imag        = (a->imag * b->real + a->real * b->imag);
      o->real        = tmp;
      o->FlagComplex = !ppl_units_DblEqual(o->imag, 0);
     }
   }

  if ((a->dimensionless != 0) && (b->dimensionless != 0)) { if ((o != a) && (o != b)) ppl_units_DimCpy(o,a); return; }

  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   {
    o->exponent[i] = a->exponent[i] + b->exponent[i];
    if (ppl_units_DblEqual(o->exponent[i], 0) == 0) DimLess=0;
    if (fabs(o->exponent[i]) > 20000 )
     { 
      if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { ppl_units_zero(o); o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
      else { sprintf(errtext, "Overflow of physical dimensions of argument."); *status = 1; return; }
     }
   }
  o->dimensionless = DimLess;
  return;
 }

void __inline__ ppl_units_div (const value *a, const value *b, value *o, int *status, char *errtext)
 {
  int i;
  double mag, tmp;
  unsigned char DimLess=1;

  if ((settings_term_current.ComplexNumbers == SW_ONOFF_OFF) || ((!a->FlagComplex) && (!b->FlagComplex))) // Real division
   {
    if (a->FlagComplex || b->FlagComplex) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; }
    else if (fabs(b->real) < 1e-200)
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; }
      else                                                      { sprintf(errtext, "Division by zero error."); *status = 1; return; }
     }
    else
     {
      if (a->FlagComplex || b->FlagComplex) { o->real = GSL_NAN; }
      else                                  { o->real = a->real / b->real; }
      o->imag = 0.0;
      o->FlagComplex=0;
     }
   }
  else // Complex division
   {
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; }
    else if ((mag = pow(b->real,2)+pow(b->imag,2)) < 1e-200)
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; }
      else                                                      { sprintf(errtext, "Division by zero error."); *status = 1; return; }
     }
    else
     {
      tmp            = (a->real * b->real + a->imag * b->imag) / mag;
      o->imag        = (a->imag * b->real - a->real * b->imag) / mag;
      o->real        = tmp;
      o->FlagComplex = !ppl_units_DblEqual(o->imag, 0);
     }
   }

  if ((a->dimensionless != 0) && (b->dimensionless != 0)) { if ((o != a) && (o != b)) ppl_units_DimCpy(o,a); return; }

  for (i=0; i<UNITS_MAX_BASEUNITS; i++)
   {
    o->exponent[i] = a->exponent[i] - b->exponent[i];
    if (ppl_units_DblEqual(o->exponent[i], 0) == 0) DimLess=0;
    if (fabs(o->exponent[i]) > 20000 )
     {
      if (settings_term_current.ExplicitErrors == SW_ONOFF_OFF) { ppl_units_zero(o); o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
      else { sprintf(errtext, "Overflow of physical dimensions of argument."); *status = 1; return; }
     }
   }
  o->dimensionless = DimLess;
  return;
 }

void __inline__ ppl_units_add (const value *a, const value *b, value *o, int *status, char *errtext)
 {
  o->real = a->real + b->real;
  if ((o != a) && (o != b)) { ppl_units_DimCpy(o,a); o->imag = 0.0; o->FlagComplex=0; }
  if ((a->dimensionless == 0) || (b->dimensionless == 0))
   {
    if (ppl_units_DimEqual(a, b) == 0)
     {
      if (a->dimensionless)
       { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand is dimensionless, while right operand has units of <%s>.", ppl_units_GetUnitStr(b, NULL, NULL, 1, 0) ); }
      else if (b->dimensionless)
       { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand has units of <%s>, while right operand is dimensionless.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0) ); }
      else
       { sprintf(errtext, "Attempt to add quantities with conflicting dimensions: left operand has units of <%s>, while right operand has units of <%s>.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, NULL, 1, 0) ); }
      *status = 1; return;
     }
   }

  if (a->FlagComplex || b->FlagComplex)
   {
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
    o->imag = a->imag + b->imag;
    o->FlagComplex = !ppl_units_DblEqual(o->imag, 0);
   }
  return;
 }

void __inline__ ppl_units_sub (const value *a, const value *b, value *o, int *status, char *errtext)
 {
  o->real = a->real - b->real;
  if ((o != a) && (o != b)) { ppl_units_DimCpy(o,a); o->imag = 0.0; o->FlagComplex=0; }
  if ((a->dimensionless == 0) || (b->dimensionless == 0))
   {
    if (ppl_units_DimEqual(a, b) == 0)
     {
      if (a->dimensionless)
       { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand is dimensionless, while right operand has units of <%s>.", ppl_units_GetUnitStr(b, NULL, NULL, 1, 0) ); }
      else if (b->dimensionless)
       { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand has units of <%s>, while right operand is dimensionless.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0) ); }
      else
       { sprintf(errtext, "Attempt to subtract quantities with conflicting dimensions: left operand has units of <%s>, while right operand has units of <%s>.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, NULL, 1, 0) ); }
      *status = 1; return;
     }
   }

  if (a->FlagComplex || b->FlagComplex)
   {
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
    o->imag = a->imag - b->imag;
    o->FlagComplex = !ppl_units_DblEqual(o->imag, 0);
   }
  return;
 }

void __inline__ ppl_units_mod (const value *a, const value *b, value *o, int *status, char *errtext)
 {
  o->real = a->real - floor(a->real / b->real) * b->real;
  if ((o != a) && (o != b)) { ppl_units_DimCpy(o,a); o->imag = 0.0; o->FlagComplex=0; }
  if ((a->dimensionless == 0) || (b->dimensionless == 0))
   {
    if (ppl_units_DimEqual(a, b) == 0)
     {
      if (a->dimensionless)
       { sprintf(errtext, "Attempt to apply mod operator to quantities with conflicting dimensions: left operand is dimensionless, while right operand has units of <%s>.", ppl_units_GetUnitStr(b, NULL, NULL, 1, 0) ); }
      else if (b->dimensionless)
       { sprintf(errtext, "Attempt to apply mod operator to quantities with conflicting dimensions: left operand has units of <%s>, while right operand is dimensionless.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0) ); }
      else
       { sprintf(errtext, "Attempt to apply mod operator to quantities with conflicting dimensions: left operand has units of <%s>, while right operand has units of <%s>.", ppl_units_GetUnitStr(a, NULL, NULL, 0, 0), ppl_units_GetUnitStr(b, NULL, NULL, 1, 0) ); }
      *status = 1; return;
     }
   }
  if (a->FlagComplex || b->FlagComplex)
   {
    if (settings_term_current.ComplexNumbers == SW_ONOFF_OFF) { o->real = GSL_NAN; o->imag = 0; o->FlagComplex=0; return; }
    sprintf(errtext, "Mod operator can only be applied to real operands; complex operands supplied.");
    *status = 1; return;
   }
  return;
 }

