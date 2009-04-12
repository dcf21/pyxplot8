// ppl_userspace.c
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
#include <math.h>

#include <gsl/gsl_sf.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "MathsTools/dcfmath.h"

#include "ListTools/lt_dict.h"

#include "ppl_userspace.h"

#define PPL_USERSPACE_NUMERIC 32000
#define PPL_USERSPACE_STRING  32001

#define PPL_USERSPACE_USERDEF 32100
#define PPL_USERSPACE_SYSTEM  32200
#define PPL_USERSPACE_SPLINE  32300

Dict *_ppl_UserSpace_Vars;
Dict *_ppl_UserSpace_Funcs;

void ppl_UserSpaceInit()
 {
  _ppl_UserSpace_Vars  = DictInit();
  _ppl_UserSpace_Funcs = DictInit();

  DictAppendFloat(_ppl_UserSpace_Vars , "pi"     , PPL_USERSPACE_NUMERIC , M_PI);
  DictAppendFloat(_ppl_UserSpace_Vars , "e"      , PPL_USERSPACE_NUMERIC , M_E );

  DictAppendPtr  (_ppl_UserSpace_Funcs, "acos"   , PPL_USERSPACE_SYSTEM+1, (void *)&acos   ); // 1 indicates that function takes (double)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "asin"   , PPL_USERSPACE_SYSTEM+1, (void *)&asin   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "atan"   , PPL_USERSPACE_SYSTEM+1, (void *)&atan   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "atan2"  , PPL_USERSPACE_SYSTEM+2, (void *)&atan2  ); // 2 indicates that function takes (double, double)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "ceil"   , PPL_USERSPACE_SYSTEM+1, (void *)&ceil   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "cos"    , PPL_USERSPACE_SYSTEM+1, (void *)&cos    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "cosh"   , PPL_USERSPACE_SYSTEM+1, (void *)&cosh   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "degrees", PPL_USERSPACE_SYSTEM+1, (void *)&degrees);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "erf"    , PPL_USERSPACE_SYSTEM+1, (void *)&gsl_sf_erf);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "exp"    , PPL_USERSPACE_SYSTEM+1, (void *)&exp    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "fabs"   , PPL_USERSPACE_SYSTEM+1, (void *)&fabs   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "floor"  , PPL_USERSPACE_SYSTEM+1, (void *)&floor  );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "fmod"   , PPL_USERSPACE_SYSTEM+2, (void *)&fmod   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "frexp"  , PPL_USERSPACE_SYSTEM+3, (void *)&frexp  ); // 3 indicates that these functions take (double, int)
  DictAppendPtr  (_ppl_UserSpace_Funcs, "gamma"  , PPL_USERSPACE_SYSTEM+1, (void *)&gsl_sf_gamma);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "hypot"  , PPL_USERSPACE_SYSTEM+2, (void *)&hypot  );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "ldexp"  , PPL_USERSPACE_SYSTEM+3, (void *)&ldexp  );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "log"    , PPL_USERSPACE_SYSTEM+1, (void *)&log    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "log10"  , PPL_USERSPACE_SYSTEM+1, (void *)&log10  );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "max"    , PPL_USERSPACE_SYSTEM+2, (void *)&max    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "min"    , PPL_USERSPACE_SYSTEM+2, (void *)&min    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "pow"    , PPL_USERSPACE_SYSTEM+2, (void *)&pow    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "radians", PPL_USERSPACE_SYSTEM+1, (void *)&radians);
  DictAppendPtr  (_ppl_UserSpace_Funcs, "random" , PPL_USERSPACE_SYSTEM+0, (void *)&frandom); // 0 indicates that this function takes no arguments
  DictAppendPtr  (_ppl_UserSpace_Funcs, "sin"    , PPL_USERSPACE_SYSTEM+1, (void *)&sin    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "sinh"   , PPL_USERSPACE_SYSTEM+1, (void *)&sinh   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "sqrt"   , PPL_USERSPACE_SYSTEM+1, (void *)&sqrt   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "tan"    , PPL_USERSPACE_SYSTEM+1, (void *)&tan    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "tanh"   , PPL_USERSPACE_SYSTEM+1, (void *)&tanh   );

  return;
 }

void ppl_UserSpace_SetVarStr(char *name, char *value)
 {
  return;
 }

void ppl_UserSpace_SetVarNumeric(char *name, double value)
 {
  return;
 }

void ppl_UserSpace_SetFunc(char *name, char *value)
 {
  return;
 }

#define MAX_STR_FORMAT_ITEMS 256

void ppl_GetQuotedString(char *in, char *out, int start, int *end, Dict *Local1Vars, Dict *Local2Vars, int *errpos, char *errtext)
 {
  Dict *GlobalVars, *GlobalFuncs;
  char QuoteType;
  char FormatString[LSTR_LENGTH]; // Used to store the string found between quotes, before applying the substitution operator
  char FormatArg   [LSTR_LENGTH]; // Used to store string arguments in the %(...) part of a quoted string
  char FormatToken [SSTR_LENGTH];
  int  pos      = start;
  int  outpos   = 0;
  int  UserData = -1;
  int  DataType = -1;
  char *VarData = NULL;
  int  pos2, CommaPositions[MAX_STR_FORMAT_ITEMS], NFormatItems;
  int  i,j,k,l,ArgCount,RequiredArgs;
  char AllowedFormats[] = "cdieEfgGosSxX%"; // These tokens are allowed after a % format specifier

  int    arg1i, arg2i; // Integers for passing to sprintf
  double arg1d, arg2d; // Doubles for passing to sprintf
  union {double d; int i; char *s; unsigned int u;} argf; // We pass this lump of memory to sprintf

  GlobalVars  = _ppl_UserSpace_Vars;
  GlobalFuncs = _ppl_UserSpace_Funcs;

  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces

  if ((in[pos]!='\'') && (in[pos]!='\"')) // If quoted string doesn't start with quote, it must be a string variable name
   {
    while ((in[pos]>' ') && ((end==NULL)||(*end<0)||(pos<*end))) FormatString[outpos++]=in[pos++]; // Fetch a word
    FormatString[outpos] = '\0';
    while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over trailing spaces
    if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Unexpected trailing matter after variable name"); return; } // Have we used up as many characters as we were told we had to?
    DictLookup(Local2Vars, FormatString, &UserData, &DataType, (void **)&VarData);
     if (VarData == NULL)
      {
       DictLookup(Local1Vars, FormatString, &UserData, &DataType, (void **)&VarData); // Try looking up variable in Local2, Local1 and Global, in that sequence
       if (VarData == NULL) 
        {
         DictLookup(GlobalVars, FormatString, &UserData, &DataType, (void **)&VarData);
         if (VarData == NULL)
          { *errpos = start; strcpy(errtext, "No such variable"); return; }
        }
      }
    if (UserData != PPL_USERSPACE_STRING) { *errpos = start; strcpy(errtext, "Type error: this is a numeric variable where a string is expected"); return; }
    *errpos = -1;
    if ((end!=NULL)&&(*end<0)) *end=pos;
    strcpy(out, VarData);
    return;
   }

  // We have a quoted string
  QuoteType = in[pos++];
  for (; ((in[pos]!='\0') && (in[pos]!=QuoteType) && ((end==NULL)||(*end<0)||(pos<*end))); pos++)
   {
    if ((in[pos]=='\\') && (in[pos+1]=='\\')) { FormatString[outpos++]='\\'; pos++; continue; }
    if ((in[pos]=='\\') && (in[pos+1]=='\'')) { FormatString[outpos++]='\''; pos++; continue; }
    if ((in[pos]=='\\') && (in[pos+1]=='\"')) { FormatString[outpos++]='\"'; pos++; continue; }
    FormatString[outpos++] = in[pos];
   }
  FormatString[outpos] = '\0';
  if (in[pos]!=QuoteType) { *errpos = start; strcpy(errtext, "Mismatched quote"); return; }
  while ((in[++pos]>'\0') && (in[pos]<=' ')); // Fast-forward over trailing spaces

  // We have a quoted string with no % operator after it
  if (in[pos]!='%')
   {
    if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Unexpected trailing matter after quoted string"); return; } // Have we used up as many characters as we were told we had to?
    *errpos = -1;
    if ((end!=NULL)&&(*end<0)) *end=pos;
    strcpy(out, FormatString);
    return;
   }

  // We have read a format string, but have a % operator acting upon it
  pos++; // Skip over % character
  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces
  if (in[pos]!='(') { *errpos = pos; strcpy(errtext, "Was expecting an opening bracket"); return; }
  StrBracketMatch(in+pos, CommaPositions, &NFormatItems, &pos2, MAX_STR_FORMAT_ITEMS);
  if (pos2<0) { *errpos = pos; strcpy(errtext, "Mismatched bracket"); return; }
  if (NFormatItems>=MAX_STR_FORMAT_ITEMS) { *errpos = pos; strcpy(errtext, "Too many string formatting arguments"); return; }
  if ((end!=NULL)&&(*end>0)&&(pos+pos2<*end))
       { *errpos = pos; strcpy(errtext, "Unexpected trailing matter after quoted string"); return; } // Have we used up as many characters as we were told we had to?

  // We have now read in a format string, and a bracketed list of format arguments
  // See <http://www.cplusplus.com/reference/clibrary/cstdio/sprintf/>
  for (i=j=ArgCount=0; FormatString[i]!='\0'; i++)
   {
    if (FormatString[i]!='%') { out[j++] = FormatString[i]; continue; }
    k=i+1; // k looks ahead to see experimentally if syntax is right
    RequiredArgs =1; // Normal %f like tokens require 1 argument
    if ((FormatString[k]=='-')||(FormatString[k]=='-')||(FormatString[k]==' ')||(FormatString[k]=='#')) k++; // optional flag can be <+- #>
    if (FormatString[k]=='*') { k++; RequiredArgs++; }
    else while ((FormatString[k]>='0') && (FormatString[k]<='9')) k++; // length can be * or some digits
    if (FormatString[k]=='.') // precision starts with a . and is followed by * or by digits
     {
      k++;
      if (FormatString[k]=='*') { k++; RequiredArgs++; }
      else while ((FormatString[k]>='0') && (FormatString[k]<='9')) k++; // length can be * or some digits
     }
    // We do not allow user to specify optional length flag, which could potentially be <hlL>

    for (l=0; AllowedFormats[l]!='\0'; l++) if (FormatString[k]==AllowedFormats[l]) break;
    if ((AllowedFormats[l]=='\0') || ((AllowedFormats[l]=='%')&&(FormatString[k-1]!='%')) || (RequiredArgs > NFormatItems-ArgCount))
     { out[j++] = FormatString[i]; continue; } // Have not got correct syntax for a format specifier

    if (AllowedFormats[l]=='%')
     {
      strcpy(out+j, "%"); // %% just produces a % character
     } else {
      StrSlice(FormatString, FormatToken, i, k+1);
      if (RequiredArgs>1) // Length and/or precision was specified with a *, so we need to take an integer from the list of arguments
       {
        ppl_EvaluateAlgebra(in+pos , &arg1d, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], Local1Vars, Local2Vars, errpos, errtext);
        if (*errpos>=0) { *errpos += pos; return; }
        arg1i = (int)arg1d;
       }
      if (RequiredArgs>2)
       {
        ppl_EvaluateAlgebra(in+pos , &arg2d, CommaPositions[ArgCount+1]+1, &CommaPositions[ArgCount+2], Local1Vars, Local2Vars, errpos, errtext);
        if (*errpos>=0) { *errpos += pos; return; }
        arg2i = (int)arg2d;
       }

      ArgCount += (RequiredArgs-1); // Fastforward ArgCount to point at the number or string argument that we're actually going to substitute
      if ((AllowedFormats[l]=='c') || (AllowedFormats[l]=='s') || (AllowedFormats[l]=='S'))
       {
        ppl_GetQuotedString(in+pos , FormatArg , CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], Local1Vars, Local2Vars, errpos, errtext);
        if (*errpos>=0) { *errpos += pos; return; }
        argf.s = FormatArg;
        if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.s);
        if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.s);
        if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.s);
       }
      else
       {
        ppl_EvaluateAlgebra(in+pos , &argf.d, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], Local1Vars, Local2Vars, errpos, errtext);
        if (*errpos>=0) { *errpos += pos; return; }
        if      ((AllowedFormats[l]=='d') || (AllowedFormats[l]=='i') || (AllowedFormats[l]=='o') || (AllowedFormats[l]=='x') || (AllowedFormats[l]=='X'))
         {
          argf.i = (int)argf.d; // sprintf will expect to be passed an int
          if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.i);
          if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.i);
          if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.i);
         }
        else if ((AllowedFormats[l]=='x') || (AllowedFormats[l]=='X'))
         {
          argf.u = (unsigned int)argf.d; // sprintf will expect to be passed an unsigned int
          if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.u);
          if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.u);
          if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.u);
         }
        else // otherwise, sprintf will expect a double
         {
          if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.d);
          if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.d);
          if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.d);
         }
       }
      ArgCount++;
     }
    j += strlen(out+j);
    i = k;
   }
  out[j]  = '\0';
  *errpos = -1;
  if ((end!=NULL)&&(*end<0)) *end=pos+pos2;
  return;
 }

void ppl_EvaluateAlgebra(char *in, double *out, int start, int *end, Dict *Local1Vars, Dict *Local2Vars, int *errpos, char *errtext)
 {
  *out = 61.0;
  return;
 }

