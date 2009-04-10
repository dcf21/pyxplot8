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

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "MathsTools/dcfmath.h"

#include "ListTools/lt_dict.h"

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
  DictAppendPtr  (_ppl_UserSpace_Funcs, "exp"    , PPL_USERSPACE_SYSTEM+1, (void *)&exp    );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "fabs"   , PPL_USERSPACE_SYSTEM+1, (void *)&fabs   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "floor"  , PPL_USERSPACE_SYSTEM+1, (void *)&floor  );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "fmod"   , PPL_USERSPACE_SYSTEM+2, (void *)&fmod   );
  DictAppendPtr  (_ppl_UserSpace_Funcs, "frexp"  , PPL_USERSPACE_SYSTEM+3, (void *)&frexp  ); // 3 indicates that these functions take (double, int)
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

void ppl_GetQuotedString(char *in, char *out, int start, int *end, Dict *NamespaceVars, Dict *NamespaceFuncs, int *errpos, char *errtext)
 {
  char QuoteType;
  char FormatString[LSTR_LENGTH];
  int  pos      = start;
  int  outpos   = 0;
  int  UserData = -1;
  int  DataType = -1;
  char *VarData = NULL;
  int  pos2, CommaPositions[MAX_STR_FORMAT_ITEMS], NFormatItems;

  if (NamespaceVars  == NULL) NamespaceVars  = _ppl_UserSpace_Vars;
  if (NamespaceFuncs == NULL) NamespaceFuncs = _ppl_UserSpace_Funcs;

  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces

  if ((in[pos]!='\'') && (in[pos]!='\"')) // If quoted string doesn't start with quote, it must be a string variable name
   {
    while ((in[pos]>' ') && ((end==NULL)||(*end<0)||(pos<*end))) FormatString[outpos++]=in[pos++]; // Fetch a word
    FormatString[outpos] = '\0';
    while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over trailing spaces
    if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Unexpected trailing matter after variable name"); return; } // Have we used up as many characters as we were told we had to?
    DictLookup(NamespaceVars, FormatString, &UserData, &DataType, (void **)&VarData);
    if (VarData  ==                 NULL) { *errpos = start; strcpy(errtext, "No such variable"); return; }
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
  *errpos = -1;
  if ((end!=NULL)&&(*end<0)) *end=pos+pos2;
  strcpy(out, FormatString);
  return;
 }

void ppl_EvaluateAlgebra(char *in, double *out, int start, int *end, int *errpos, char *errtext)
 {
  *out = 2.0;
  return;
 }

