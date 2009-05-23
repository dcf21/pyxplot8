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

#define _PPL_USERSPACE 1

// Include <ppl_units.c> as a part of this source file to allow inline
// compilation of arithmetic functions.
#include "ppl_units.c"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"

#include "ppl_constants.h"
#include "ppl_userspace.h"

// -------------------------------------------------------------------
// Data structures used for storing the user's variables and functions
// -------------------------------------------------------------------

Dict *_ppl_UserSpace_Vars;
Dict *_ppl_UserSpace_Funcs;

void ppl_UserSpace_SetVarStr(char *name, char *inval)
 {
  DictAppendString(_ppl_UserSpace_Vars , name , inval);
  return;
 }

void ppl_UserSpace_SetVarNumeric(char *name, value *inval)
 {
  DictAppendValue(_ppl_UserSpace_Vars , name , *inval);
  return;
 }

void ppl_UserSpace_UnsetVar(char *name)
 {
  DictRemoveKey(_ppl_UserSpace_Vars , name);
  return;
 }

void ppl_UserSpace_SetFunc(char *name, char *inval)
 {
  return;
 }

void ppl_UserSpace_UnsetFunc(char *name)
 {
  if (strcmp(name,"unit")==0)     { ppl_error("Cannot undefine the built-in function unit()."); return; }
  if (strncmp(name,"int_d",5)==0) { ppl_error("Cannot undefine a built-in integration function."); return; }
  DictRemoveKey(_ppl_UserSpace_Funcs , name);
  return;
 }

// ---------------------------------------------------------------------------------
// ppl_GetQuotedString(): Extract a string from an input of the form "...." % (....)
//
// in: pointer to string from which quoted string is to be extracted
// out: string output is written here
// start: start scanning at in+start
// end: If NULL, does nothing. If *end>0, we must use all characters up to this point. If *end<0, last character of string is written here
// Local?Vars: Local variables which take precedence over global variables. 2 takes precedence over 1
// errpos: Set >0 if an error is found at (in+errpos).
// errtext: Error messages are written here
// ---------------------------------------------------------------------------------

void ppl_GetQuotedString(char *in, char *out, int start, int *end, int *errpos, char *errtext, int RecursionDepth)
 {
  char QuoteType;
  char FormatString[LSTR_LENGTH]; // Used to store the string found between quotes, before applying the substitution operator
  char FormatArg   [LSTR_LENGTH]; // Used to store string arguments in the %(...) part of a quoted string
  char FormatToken [SSTR_LENGTH];
  int  pos      = start;
  int  outpos   = 0;
  int  DataType = -1;
  char *VarData = NULL;
  int  pos2, CommaPositions[MAX_STR_FORMAT_ITEMS], NFormatItems;
  int  i,j,k,l,ArgCount,RequiredArgs;
  char AllowedFormats[] = "cdieEfgGosSxX%"; // These tokens are allowed after a % format specifier

  int    arg1i, arg2i; // Integers for passing to sprintf
  value  arg1v, arg2v; // Doubles (well, actually values with units) for passing to sprintf
  struct {double d; int i; char *s; unsigned int u; value v; } argf; // We pass this lump of memory to sprintf

  if (RecursionDepth > MAX_RECURSION_DEPTH) { *errpos=start; strcpy(errtext,"Overflow Error: Maximum recursion depth exceeded"); return; }

  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces

  if ((in[pos]!='\'') && (in[pos]!='\"')) // If quoted string doesn't start with quote, it must be a string variable name
   {
    while ((in[pos]>' ') && ((end==NULL)||(*end<0)||(pos<*end))) FormatString[outpos++]=in[pos++]; // Fetch a word
    FormatString[outpos] = '\0';
    while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over trailing spaces
    if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after variable name"); return; } // Have we used up as many characters as we were told we had to?
    DictLookup(_ppl_UserSpace_Vars, FormatString, &DataType, (void **)&VarData); // Global variables
    if (VarData == NULL)             { *errpos = start; strcpy(errtext, "No such variable"); return; }
    if (DataType != DATATYPE_STRING) { *errpos = start; strcpy(errtext, "Type Error: This is a numeric variable where a string is expected"); return; }
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
  if (in[pos]!=QuoteType) { *errpos = start; strcpy(errtext, "Syntax Error: Mismatched quote"); return; }
  while ((in[++pos]>'\0') && (in[pos]<=' ')); // Fast-forward over trailing spaces

  // We have a quoted string with no % operator after it
  if (in[pos]!='%')
   {
    if ((end!=NULL)&&(*end>0)&&(pos<*end)) { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string"); return; } // Have we used up as many characters as we were told we had to?
    *errpos = -1;
    if ((end!=NULL)&&(*end<0)) *end=pos;
    strcpy(out, FormatString);
    return;
   }

  // We have read a format string, but have a % operator acting upon it
  pos++; // Skip over % character
  while ((in[pos]>'\0') && (in[pos]<=' ')) pos++; // Fast-forward over preceding spaces
  if (in[pos]!='(') { *errpos = pos; strcpy(errtext, "Syntax Error: An opening bracket was expected here"); return; }
  StrBracketMatch(in+pos, CommaPositions, &NFormatItems, &pos2, MAX_STR_FORMAT_ITEMS);
  if (pos2<0) { *errpos = pos; strcpy(errtext, "Syntax Error: Mismatched bracket"); return; }
  if (NFormatItems>=MAX_STR_FORMAT_ITEMS) { *errpos = pos; strcpy(errtext, "Overflow Error: Too many string formatting arguments"); return; }
  if ((end!=NULL)&&(*end>0)&&(pos+pos2<*end))
       { *errpos = pos; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string"); return; } // Have we used up as many characters as we were told we had to?

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
        ppl_EvaluateAlgebra(in+pos , &arg1v, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        if (arg1v.dimensionless == 0) { *errpos = pos; sprintf(errtext, "This argument should be dimensionless, but has dimensions of %s.", ppl_units_GetUnitStr(&arg1v, NULL, 0, 0) ); return; }
        arg1i = (int)(arg1v.number);
       }
      if (RequiredArgs>2)
       {
        ppl_EvaluateAlgebra(in+pos , &arg2v, CommaPositions[ArgCount+1]+1, &CommaPositions[ArgCount+2], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        if (arg2v.dimensionless == 0) { *errpos = pos; sprintf(errtext, "This argument should be dimensionless, but has dimensions of %s.", ppl_units_GetUnitStr(&arg2v, NULL, 0, 0) ); return; }
        arg2i = (int)(arg2v.number);
       }

      ArgCount += (RequiredArgs-1); // Fastforward ArgCount to point at the number or string argument that we're actually going to substitute
      if ((AllowedFormats[l]=='c') || (AllowedFormats[l]=='s') || (AllowedFormats[l]=='S'))
       {
        ppl_GetQuotedString(in+pos , FormatArg , CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        argf.s = FormatArg;
        if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.s); // Print a string variable
        if (RequiredArgs==2) sprintf(out+j, FormatToken, arg1i, argf.s);
        if (RequiredArgs==3) sprintf(out+j, FormatToken, arg1i, arg2i, argf.s);
       }
      else
       {
        ppl_EvaluateAlgebra(in+pos , &argf.v, CommaPositions[ArgCount+0]+1, &CommaPositions[ArgCount+1], errpos, errtext, RecursionDepth+1);
        if (*errpos>=0) { *errpos += pos; return; }
        argf.s = ppl_units_GetUnitStr(&argf.v, &argf.d, 0, 0);
        if      ((AllowedFormats[l]=='d') || (AllowedFormats[l]=='i') || (AllowedFormats[l]=='o') || (AllowedFormats[l]=='x') || (AllowedFormats[l]=='X'))
         {
          argf.i = (int)argf.d; // sprintf will expect to be passed an int
          if (RequiredArgs==1) sprintf(out+j, FormatToken, argf.i); // Print an integer variable
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
        if (argf.v.dimensionless != 0)
         {
          j += strlen(out+j);
          strcpy(out+j, argf.s); // Print dimensions of this value
         }
       }
      ArgCount++;
     }
    j += strlen(out+j);
    i = k;
   }
  out[j]  = '\0';
  *errpos = -1;
  if ((end!=NULL)&&(*end<0)) *end=pos+pos2+1;
  return;
 }

// ---------------------------------------------
// Functions for analysing algebraic expressions
// ---------------------------------------------

// ppl_EvaluateAlgebra(): Evaluates an algebraic expression to return a double. All fields as above.

// Order of precedence:
//  1 f(x)
//  2 ()
//    [evaluate numerics and variables]
//  3 minus signs
//  4 **
//  5 *  /  %
//  6 +  -
//  7 <<  >>
//  8 <  <=  >=  >
//  9 ==  !=  <>
// 10 &
// 11 ^
// 12 |
// 13 and
// 14 or

// StatusRow values 20-255 correspond to ResultBuffer values 0-235
#define BUFFER_OFFSET 20

#define MATCH_ONE(A)       (in[i]==A)
#define MATCH_TWO(A,B)    ((in[i]==A)&&(in[i+1]==B))
#define MATCH_THR(A,B,C)  ((in[i]==A)&&(in[i+1]==B)&&(in[i+2]==C))

#define FETCHPREV(VARA,VARB,VARC)  { for (j=p-1,cj=StatusRow[p-1]; ((StatusRow[j]==cj)&&(j>0)); j--); if (cj<BUFFER_OFFSET) {*errpos=i; strcpy(errtext, "Internal Error: Trying to do arithmetic before numeric conversion"); return;}; VARA=j; VARB=(int)(cj-BUFFER_OFFSET); VARC=p-1; }
#define FETCHNEXT(VARA,VARB,VARC)  { for (j=p,ci=StatusRow[p]; StatusRow[j]==ci; j++); if ((cj=StatusRow[j])<BUFFER_OFFSET) {*errpos=i; strcpy(errtext, "Internal Error: Trying to do arithmetic before numeric conversion"); return;}; for (k=j; StatusRow[k]==cj; k++); VARA=j; VARB=(int)(cj-BUFFER_OFFSET); VARC=k; }
#define SETSTATUS(BEG,END,VAL)     { ci = (unsigned char)(VAL+BUFFER_OFFSET); for (j=BEG;j<END;j++) StatusRow[j]=ci; }

void ppl_EvaluateAlgebra(char *in, value *out, int start, int *end, int *errpos, char *errtext, int RecursionDepth)
 {
  unsigned char OpList[OPLIST_LEN];           // A list of what operations this expression contains
  unsigned char StatusRow[ALGEBRA_MAXLENGTH]; // Describes the atoms at each position in the expression
  value ResultBuffer[ALGEBRA_MAXITEMS];       // A buffer of temporary numerical results
  int len, CalculatedEnd;
  int i,p,j,k;
  int prev_start, prev_end, next_start, next_end, prev_bufno, next_bufno;
  unsigned char ci,cj;
  char ck;
  int bufpos = 0;
  int     DataType = -1; 
  value  *VarData  = NULL;
  double  TempDbl;

  if (RecursionDepth > MAX_RECURSION_DEPTH) { *errpos=start; strcpy(errtext,"Overflow Error: Maximum recursion depth exceeded"); return; }

  *errpos = -1;
  ppl_GetExpression(in+start, &len, 0, StatusRow, OpList, errpos, errtext);
  if (*errpos > 0) { (*errpos) += start; return; }
  CalculatedEnd = start + len;
  if ((end != NULL) && (*end >  0) && (CalculatedEnd < *end)) { *errpos=CalculatedEnd; strcpy(errtext,"Syntax Error: Unexpected trailing matter after algebraic expression"); return; }
  if ((end != NULL) && (*end <= 0)) *end = CalculatedEnd;

  // PHASE  1: EVALUATION OF FUNCTIONS
  if (OpList[1]!=0) for (i=0;i<len-1;i++) if ((StatusRow[i]==8)&&(StatusRow[i+1]==3))
   {
   }
  // PHASE  2: EVALUATION OF BRACKETED EXPRESSIONS
  if (OpList[2]!=0) for (i=0;i<len;i++) if (StatusRow[i]==3)
   {
    j=-1;
    ppl_EvaluateAlgebra(in+start+i, ResultBuffer+bufpos, 1, &j, errpos, errtext, RecursionDepth+1);
    if (*errpos > 0) { (*errpos) += start+i; return; }
    j+=i; while ((in[start+j]>'\0')&&(in[start+j]<=' ')) j++;
    if (in[start+j]!=')') { *errpos=start+j; strcpy(errtext,"Syntax Error: Unexpected trailing matter within brackets"); return; }
    for (k=i; StatusRow[k]==3; k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    bufpos++;
   }
  // PHASE 2b: EVALUATION OF ALL NUMERICS AND VARIABLES
  for (i=0;i<len;i++) if (StatusRow[i]==6)
   {
    ppl_units_zero(ResultBuffer+bufpos);
    ResultBuffer[bufpos].number = GetFloat(in+start+i, &j);
    for (k=i; StatusRow[k]==6; k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    j+=i; while ((in[start+j]>'\0')&&(in[start+j]<=' ')) j++;
    if (j!=k) { *errpos=start+i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after numeric constant"); return; }
    bufpos++;
   }
  for (i=0;i<len;i++) if ((StatusRow[i]==8) || (StatusRow[i]==4))
   {
    for (j=i;((StatusRow[j]==8)||(StatusRow[i]==4));j++);
    while ((j>i) && (in[start+j-1]<=' ')) j--;
    ck = in[start+j] ; in[start+j]='\0'; // This will not work if string constant is passed to us!!
    DictLookup(_ppl_UserSpace_Vars, in+start+i, &DataType, (void **)&VarData);
    in[start+j] = ck;
    if (VarData == NULL) { *errpos = start+i; strcpy(errtext, "No such variable"); return; } 
    in[start+j] = ck;
    if (DataType != DATATYPE_VALUE) { *errpos = start+i; strcpy(errtext, "Type Error: This is a string variable where numeric value is expected"); return; }
    ResultBuffer[bufpos] = *VarData;
    for (k=i; ((StatusRow[k]==8)||(StatusRow[k]==4)); k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    bufpos++;
   }
  // PHASE  3: EVALUATION OF MINUS SIGNS
  if (OpList[3]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==5)
   {
    FETCHNEXT(next_start, next_bufno, next_end);
    ResultBuffer[next_bufno].number *= -1;
    SETSTATUS(p, next_start, next_bufno);
    i = start + next_start;
    i--; p=i-start;
   }
  // PHASE  4: EVALUATION OF **
  if (OpList[4]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_TWO('*','*'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      ppl_units_pow(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      if (*errpos >= 0) { *errpos=i; return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE  5: EVALUATION OF *  /  %
  if (OpList[5]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_ONE('*'))||(MATCH_ONE('/'))||(MATCH_ONE('%')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if      (MATCH_ONE('*')) ppl_units_mult(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      else if (MATCH_ONE('/')) ppl_units_div (ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      else if (MATCH_ONE('%')) ppl_units_mod (ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      if (*errpos >= 0) { *errpos=i; return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE  6: EVALUATION OF +  -
  if (OpList[6]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_ONE('+'))||(MATCH_ONE('-')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if      (MATCH_ONE('+')) ppl_units_add(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      else if (MATCH_ONE('-')) ppl_units_sub(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , errpos , errtext);
      if (*errpos >= 0) { *errpos=i; return; }
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start; 
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE  7: EVALUATION OF << >>
  if (OpList[7]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_TWO('<','<'))||(MATCH_TWO('>','>')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s and the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 0, 0) ); return; }
       }
      if      (MATCH_ONE('<')) ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number << (int)ResultBuffer[next_bufno].number);
      else if (MATCH_ONE('>')) ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number >> (int)ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE  8: EVALUATION OF < <= >= >
  if (OpList[8]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_ONE('<'))||(MATCH_ONE('>')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if (ppl_units_DimEqual(ResultBuffer+prev_bufno , ResultBuffer+next_bufno) == 0)
       {
        *errpos=i;
        if      (ResultBuffer[prev_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity which is dimensionless with one with dimensions of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,1,0)); }
        else if (ResultBuffer[next_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of %s with one which is dimensionless.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,0,0)); }
        else
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of %s with one with dimensions of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,0,0), ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,1,0)); }
        return;
       }
      TempDbl = ResultBuffer[prev_bufno].number;
      ppl_units_zero(ResultBuffer+prev_bufno);
      if      (MATCH_TWO('<','=')) ResultBuffer[prev_bufno].number = (double)(TempDbl <= ResultBuffer[next_bufno].number);
      else if (MATCH_TWO('>','=')) ResultBuffer[prev_bufno].number = (double)(TempDbl >= ResultBuffer[next_bufno].number);
      else if (MATCH_ONE('<')    ) ResultBuffer[prev_bufno].number = (double)(TempDbl <  ResultBuffer[next_bufno].number);
      else if (MATCH_ONE('>')    ) ResultBuffer[prev_bufno].number = (double)(TempDbl <  ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE  9: EVALUATION OF == !=  <>
  if (OpList[9]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if ((MATCH_TWO('=','='))||(MATCH_TWO('!','='))||(MATCH_TWO('<','>')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if (ppl_units_DimEqual(ResultBuffer+prev_bufno , ResultBuffer+next_bufno) == 0)
      if (ppl_units_DimEqual(ResultBuffer+prev_bufno , ResultBuffer+next_bufno) == 0)
       { 
        *errpos=i; 
        if      (ResultBuffer[prev_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity which is dimensionless with one with dimensions of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,1,0)); }
        else if (ResultBuffer[next_bufno].dimensionless)
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of %s with one which is dimensionless.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,0,0)); }
        else
         { sprintf(errtext, "Attempt to compare a quantity with dimensions of %s with one with dimensions of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno,NULL,0,0), ppl_units_GetUnitStr(ResultBuffer+next_bufno,NULL,1,0)); }
        return;
       }
      TempDbl = ResultBuffer[prev_bufno].number;
      ppl_units_zero(ResultBuffer+prev_bufno);
      if      (MATCH_TWO('=','=')) ResultBuffer[prev_bufno].number = (double)(TempDbl == ResultBuffer[next_bufno].number);
      else if (MATCH_TWO('!','=')) ResultBuffer[prev_bufno].number = (double)(TempDbl != ResultBuffer[next_bufno].number);
      else if (MATCH_TWO('<','>')) ResultBuffer[prev_bufno].number = (double)(TempDbl != ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE 10: EVALUATION OF &
  if (OpList[10]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_ONE('&'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s and the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 0, 0) ); return; }
       }
      ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number & (int)ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE 11: EVALUATION OF ^
  if (OpList[11]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_ONE('^'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s and the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 0, 0) ); return; }
       }
      ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number ^ (int)ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE 12: EVALUATION OF |
  if (OpList[12]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_ONE('|'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s and the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the left operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Binary operators can only be applied to dimensionless operands; here, the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 0, 0) ); return; }
       }
      ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number | (int)ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE 13: EVALUATION OF and
  if (OpList[13]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_THR('a','n','d'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of %s and the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 0, 0) ); return; }
       }
      ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number && (int)ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE 14: EVALUATION OF or
  if (OpList[14]!=0) for (i=start,p=0;i<CalculatedEnd;i++,p++) if (StatusRow[p]==7)
   {
    if (MATCH_TWO('o','r'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      FETCHNEXT(next_start, next_bufno, next_end);
      if ( (ResultBuffer[prev_bufno].dimensionless == 0) || (ResultBuffer[next_bufno].dimensionless == 0) )
       {
        if ( (ResultBuffer[prev_bufno].dimensionless == 0) && (ResultBuffer[next_bufno].dimensionless == 0) )
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of %s and the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0), ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 1, 0)); return; }
        else if (ResultBuffer[prev_bufno].dimensionless == 0)
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the left operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+prev_bufno, NULL, 0, 0) ); return; }
        else
         { *errpos=i; sprintf(errtext, "Logical operators can only be applied to dimensionless operands; here, the right operand has units of %s.", ppl_units_GetUnitStr(ResultBuffer+next_bufno, NULL, 0, 0) ); return; }
       }
      ResultBuffer[prev_bufno].number = (double)((int)ResultBuffer[prev_bufno].number || (int)ResultBuffer[next_bufno].number);
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = start + next_start;
     } else {
      while (StatusRow[i]==7) i++;
     }
    i--; p=i-start;
   }
  // PHASE 15: RETURN RESULT TO USER
  for (i=0;i<len;i++) if (StatusRow[i] >= BUFFER_OFFSET) { *out = ResultBuffer[ StatusRow[i] - BUFFER_OFFSET ]; return; }
  // for (i=0;i<len;i++) printf("%c",(StatusRow[i]+'A')); printf("\n"); // DEBUG LINE
  *errpos = start;
  strcpy(errtext,"Internal Error: ppl_EvaluateAlgebra failed to evaluate this expression"); // Oops
  return;
 }

// ppl_GetExpression(): Extracts a syntactically complete algebraic expression.

// end -- This is an output, and returns the end of the algebraic expression found
// dollar_allowed -- Indicates whether a dollar-sign is an allowable character in variable names
// status -- If non-NULL, the atom found at each character position is written here

// Go through string, spotting atoms from the following list:

// 1 S -- the beginning of the string
// 2 E -- the end of the expression
// 3 B -- a bracketed () series of characters
// 4 D -- a dollar sign -- only allowed in using expressions in the plot command as special variable name
// 5 M -- a minus sign before a numeric value, variable name, or ()
// 6 N -- a numerical value, e.g. 1.2e-34
// 7 O -- an operator, + - * ** / % << >> & | ^ < > <= >= == != <>
// 8 V -- a variable name

// 1 S can be followed by  BDMN V not by E    O
// 2 E
// 3 B can be followed by E    O  not by  BDMN V
// 4 D can be followed by  B  N V not by E DM O
// 5 M can be followed by  BD N V not by E  M O
// 6 N can be followed by E    O  not by  BDMN V
// 7 O can be followed by  BDMN V not by E    O
// 8 V can be followed by EB   O  not by   DMN V

#define MATCH_TRACK_ONE(A,OPN)      ((in[scanpos]==A)                                                                    && (OpList[OPN]=1,1))
#define MATCH_TRACK_TWO(A,B,OPN)    ((in[scanpos]==A)&&(in[scanpos+1]==B)                                                && (OpList[OPN]=1,1))
#define MATCH_TRACK_THR(A,B,C,OPN)  ((in[scanpos]==A)&&(in[scanpos+1]==B)&&(in[scanpos+2]==C)                            && (OpList[OPN]=1,1))
#define MATCH_TRACK_THRS(A,B,C,OPN) ((in[scanpos]==A)&&(in[scanpos+1]==B)&&(in[scanpos+2]==C)&&(!isalpha(in[scanpos+3])) && (OpList[OPN]=1,1))

#define PGE_OVERFLOW { *errpos = scanpos; strcpy(errtext, "Overflow Error: Algebraic expression too long"); *end = -1; return; }

#define NEWSTATE(L) state=trial; for (i=0;i<L;i++) { status[scanpos++]=(unsigned char)(state-'0') ; if (scanpos>=ALGEBRA_MAXLENGTH) PGE_OVERFLOW; }
#define SAMESTATE                                  { status[scanpos++]=(unsigned char)(state-'0') ; if (scanpos>=ALGEBRA_MAXLENGTH) PGE_OVERFLOW; }

void ppl_GetExpression(char *in, int *end, int DollarAllowed, unsigned char *status, unsigned char *OpList, int *errpos, char *errtext)
 {
  static char *AllowedNext[] = {"34568","","72","368","34568","72","34568","372"};
  static unsigned char DummyOpList[OPLIST_LEN];
  char state='1', oldstate;
  char trial;
  int scanpos=0, trialpos, i, j; // scanpos scans through in.

  if (OpList==NULL) OpList = DummyOpList; // OpList is a table of which operations we've found, for speeding up evaluation. If this output is not requested, we put it in a dummy.
  for (i=0;i<OPLIST_LEN;i++) OpList[i]=0;

  while (state != '2')
   {
    oldstate = state;
    while ((in[scanpos]!='\0') && (in[scanpos]<=' ')) { SAMESTATE; } // Sop up whitespace

    for (trialpos=0; ((trial=AllowedNext[(int)(state-'1')][trialpos])!='\0'); trialpos++)
     {
      if      (trial=='2') // E
       { NEWSTATE(0); }
      else if ((trial=='3') && MATCH_TRACK_ONE('(', 0)) // B
       {
        if (state=='8') OpList[1]=1; else OpList[2]=1; // We either have a bracketed expression, or a function call
        StrBracketMatch(in+scanpos, NULL, NULL, &j, 0);
        if (j>0) { j++; NEWSTATE(j); }
        else     { *errpos = scanpos; strcpy(errtext, "Syntax Error: Mismatched bracket"); *end = -1; return; }
       }
      else if ((trial=='4') && (DollarAllowed != 0) && MATCH_TRACK_ONE('$', 0)) // D
       { NEWSTATE(1); }
      else if (trial=='5') // M
       {
        if      (MATCH_TRACK_ONE ('-', 3))         { NEWSTATE(1); }
        else if (MATCH_TRACK_THRS('n','o','t', 3)) { NEWSTATE(3); }
       }
      else if (trial=='6') // N
       {
        j=scanpos; i=0;
        if ((in[j]=='+')||(in[j]=='-')) j++;
        while (isdigit(in[j])) { j++; i=1; }
        if (in[j]=='.') { j++; while (isdigit(in[j])) { j++; i=1; } }
        if (i==0) continue;
        if ((in[j]=='e')||(in[j]=='E'))
         {
          j++;
          if ((in[j]=='+')||(in[j]=='-')) j++;
          while (isdigit(in[j])) j++;
         }
        j-=scanpos; NEWSTATE(j);
       }
      else if (trial=='7') // O
       {
        if (MATCH_TRACK_THRS('a','n','d',13))
         { NEWSTATE(3); }
        else if (MATCH_TRACK_TWO('*','*', 4)||MATCH_TRACK_TWO('<','<', 7)||MATCH_TRACK_TWO('>','>', 7)||MATCH_TRACK_TWO('<','=', 8)||MATCH_TRACK_TWO('>','=', 8)||
                 MATCH_TRACK_TWO('=','=', 9)||MATCH_TRACK_TWO('!','=', 9)||MATCH_TRACK_TWO('o','r',14)||MATCH_TRACK_TWO('<','>', 9))
         { NEWSTATE(2); }
        else if (MATCH_TRACK_ONE('+', 6)||MATCH_TRACK_ONE('-', 6)||MATCH_TRACK_ONE('*', 5)||MATCH_TRACK_ONE('/', 5)||MATCH_TRACK_ONE('%', 5)||MATCH_TRACK_ONE('&',10)||
                 MATCH_TRACK_ONE('|',12)||MATCH_TRACK_ONE('^',11)||MATCH_TRACK_ONE('<', 8)||MATCH_TRACK_ONE('>', 8))
         { NEWSTATE(1); }
       }
      else if ((trial=='8') && (isalpha(in[scanpos]))) // V
       {
        NEWSTATE(1);
        while ((isalnum(in[scanpos])) || (in[scanpos]=='_')) { SAMESTATE; }
       }
      if (state != oldstate) break;
     }
    if (state == oldstate) break; // We've got stuck
   }
  if (state == '2') // We reached state 2(E)... end of expression
   {
    *errpos = -1; *errtext='\0';
    status[scanpos]=0;
    *end = scanpos;
    //for (i=0;i<scanpos;i++) printf("%c",in[i]); printf("\n"); // DEBUG LINES
    //for (i=0;i<scanpos;i++) printf("%c",(status[i]+'0')); printf("\n");
    //for (i=0;i<OPLIST_LEN;i++) printf("%c",(OpList[i]+'0')); printf("\n");
    return;
   }
  *errpos = scanpos; // Error; we didn't reach state 2(E)
  *end    = -1;
  // Now we need to construct an error string
  strcpy(errtext,"Syntax Error: At this point, was expecting "); i=strlen(errtext); j=0;
  for (trialpos=0; ((trial=AllowedNext[(int)(state-'1')][trialpos])!='\0'); trialpos++ )
   {
    if ((trial=='4') && (DollarAllowed == 0)) continue;
    if (j!=0) {strcpy(errtext+i," or "); i+=strlen(errtext+i);} else j=1;
    if      (trial=='3') { strcpy(errtext+i,"a bracketed expression"); i+=strlen(errtext+i); }
    else if (trial=='4') { strcpy(errtext+i,"a dollar"); i+=strlen(errtext+i); }
    else if (trial=='5') { strcpy(errtext+i,"a minus sign"); i+=strlen(errtext+i); }
    else if (trial=='6') { strcpy(errtext+i,"a numeric value"); i+=strlen(errtext+i); }
    else if (trial=='7') { strcpy(errtext+i,"an operator"); i+=strlen(errtext+i); }
    else if (trial=='8') { strcpy(errtext+i,"a variable or function name"); i+=strlen(errtext+i); }
   }
  strcpy(errtext+i,"."); i+=strlen(errtext+i);
  return;
 }

