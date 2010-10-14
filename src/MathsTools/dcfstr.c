// dcfstr.c
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

#define _DCFSTR_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#include "ListTools/lt_memory.h"

#include "StringTools/str_constants.h"

#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "dcfstr.h"

#include "dcfmath_macros.h"

#define GET_INPUT_STRING \
  static char *InString=NULL; \
  int i=0, j; \
 \
  *status=-1; \
  ppl_units_zero(output); \
  inlen--; /* Make inlen point to last character */ \
  while ((in[i]!='\0')&&(in[i]<=' ')) i++; /* Strip spaces off front */ \
 \
  /* Fetch input string */ \
  j=-1; \
  if (InString==NULL) InString = lt_malloc_incontext(LSTR_LENGTH,0); \
  if (InString==NULL) { *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; } \
  ppl_GetQuotedString(in+i, InString, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1); \
  if (*status >= 0) { (*status) += i; return; } \
  i+=j; \
  while ((in[i]>'\0')&&(in[i]<=' ')) i++;

#define GET_INPUT_NUMBER(X) \
  if (in[i] != ',') \
   { \
    if (in[i] ==')') { *status = i; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; } \
    else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; } \
   } \
  i++; j=-1; \
  ppl_EvaluateAlgebra(in+i, &tempval, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1); \
  if (*status >= 0) { (*status) += i; return; } \
  if (tempval.FlagComplex) { *status=i; strcpy(errtext,"This input should be real, but is complex."); return; } \
  if (!tempval.dimensionless) { *status=i; sprintf(errtext,"This input should be dimensionless, but has units of <%s>.", ppl_units_GetUnitStr(&tempval,NULL,NULL,0,1,0)); return; } \
  if (tempval.real < 0) { tempval.real += strlen(InString); } \
  if (tempval.real < 0) { tempval.real = 0; } \
  if (tempval.real > INT_MAX) { *status=i; strcpy(errtext,"This input is too large."); return; } \
  i+=j; \
  while ((in[i]>'\0')&&(in[i]<=' ')) i++; \
  X = (int)tempval.real;

#define END_INPUT_STRING \
  /* Check for closing bracket */ \
  if (in[i] != ')') \
   { \
    *status = i; \
    if (in[i] == ',') { strcpy(errtext,"Syntax Error: Too many arguments supplied to function."); } \
    else              { strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); } \
    return; \
   }

void dcfstr_strcmp (char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  static char *InString2 = NULL;
  GET_INPUT_STRING;
  if (in[i] != ',')
   {
    if (in[i] ==')') { *status = i; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; }
    else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; }
   }
  i++;
  while ((in[i]!='\0')&&(in[i]<=' ')) i++; /* Strip spaces off front */
  /* Fetch second input string */
  j=-1;
  if (InString2==NULL) InString2 = lt_malloc_incontext(LSTR_LENGTH, 0);
  if (InString2==NULL) { output->real=GSL_NAN; *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; }
  ppl_GetQuotedString(in+i, InString2, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1);
  if (*status >= 0) { (*status) += i; return; }
  i+=j;
  while ((in[i]>'\0')&&(in[i]<=' ')) i++;
  END_INPUT_STRING;
  output->real = strcmp(InString, InString2);
  if (output->real > 0) output->real =  1;
  if (output->real < 0) output->real = -1;
  return;
 }

void dcfstr_strlen  (char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  GET_INPUT_STRING;
  END_INPUT_STRING;
  output->real = strlen(InString);
  return;
 }

void dcfstr_strlower(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  char *outstr;
  GET_INPUT_STRING;
  END_INPUT_STRING;
  for (i=0;InString[i]!='\0';i++) InString[i]=tolower(InString[i]);
  outstr = (char *)lt_malloc(i+1);
  if (outstr==NULL) { *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; }
  strcpy(outstr, InString);
  output->string = outstr;
  return;
 }

void dcfstr_strrange(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  value tempval;
  int   start,end;
  char *outstr;
  GET_INPUT_STRING;
  GET_INPUT_NUMBER(start);
  GET_INPUT_NUMBER(end);
  END_INPUT_STRING;
  if (start >= strlen(InString)) { output->string=""; return; }
  if ((start>0)&&(end==0)) { end=strlen(InString); }
  for (i=0,j=start; (InString[j]!='\0')&&(j<end);i++,j++) InString[i]=InString[j];
  InString[i]='\0';
  outstr = (char *)lt_malloc(i+1);
  if (outstr==NULL) { *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; }
  strcpy(outstr, InString);
  output->string = outstr;
  return;
 }

void dcfstr_strupper(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  char *outstr;
  GET_INPUT_STRING;
  END_INPUT_STRING;
  for (i=0;InString[i]!='\0';i++) InString[i]=toupper(InString[i]);
  outstr = (char *)lt_malloc(i+1);
  if (outstr==NULL) { *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; }
  strcpy(outstr, InString);
  output->string = outstr;
  return;
 }

#define MAX_CONDITIONAL_DEPTH 32
#define COND_INIT {NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL}
int NestedConditionalDepth=0;

void dcfstr_conditionalS(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  static char *bs[MAX_CONDITIONAL_DEPTH] = COND_INIT;
  static char *cs[MAX_CONDITIONAL_DEPTH] = COND_INIT;
  char *outstr;
  int i=0, j, fail;
  value tempval,a,b,c;


#define CONDITIONAL_GET_INPUT_STRING(START, InString) \
  fail=0; \
  if (!START) \
   { \
    if (in[i] != ',') \
     { \
      if (in[i] ==')') { *status = i; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; } \
      else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; } \
     } \
    i++; \
   } \
  *status=-1; \
  ppl_units_zero(output); \
  inlen--; /* Make inlen point to last character */ \
  while ((in[i]!='\0')&&(in[i]<=' ')) i++; /* Strip spaces off front */ \
 \
  /* Fetch input string */ \
  j=-1; \
  if (InString[NestedConditionalDepth]==NULL) InString[NestedConditionalDepth] = lt_malloc_incontext(LSTR_LENGTH,0); \
  if (InString[NestedConditionalDepth]==NULL) { *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; } \
  NestedConditionalDepth++; \
  ppl_GetQuotedString(in+i, InString[NestedConditionalDepth-1], 0, &j, DollarAllowed, status, errtext, RecursionDepth+1); \
  NestedConditionalDepth--; \
  if (*status >= 0) { (*status) += i; fail=1; } \
  else \
   { \
    i+=j; \
    while ((in[i]>'\0')&&(in[i]<=' ')) i++; \
   }

#define CONDITIONAL_GET_INPUT_VALUE(START) \
  fail=0; \
  if (!START) \
   { \
    if (in[i] != ',') \
     { \
      if (in[i] ==')') { *status = i; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; } \
      else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; } \
     } \
    i++; \
   } \
  j=-1; \
  ppl_EvaluateAlgebra(in+i, &tempval, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1); \
  if (*status >= 0) { (*status) += i; fail=1; } \
  else \
   { \
    i+=j; \
    while ((in[i]>'\0')&&(in[i]<=' ')) i++; \
   }

  if (NestedConditionalDepth >= MAX_CONDITIONAL_DEPTH) { *status=0; sprintf(errtext, "Numerical Error: Recursion Depth Exceeded."); return; }
  CONDITIONAL_GET_INPUT_VALUE(1);
  if (fail) return;
  a = tempval;

  CONDITIONAL_GET_INPUT_STRING(0,bs);
  if (fail) return;
  ppl_units_zero(&b);
  b.string = bs[NestedConditionalDepth];

  CONDITIONAL_GET_INPUT_STRING(0,cs);
  if (fail) return;
  ppl_units_zero(&c);
  c.string = cs[NestedConditionalDepth];

  END_INPUT_STRING;

  if (hypot(a.real,a.imag)>1e-250) *output=b;
  else                             *output=c;

  outstr = lt_malloc(strlen(output->string)+1);
  if (outstr==NULL) { *status=0; sprintf(errtext, "Memory Error: Out of Memory."); return; }
  strcpy(outstr, output->string);
  output->string = outstr;
  return;
 }


void dcfstr_conditionalN(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  int i=0, j, fail;
  value tempval,a,b,c;

  CONDITIONAL_GET_INPUT_VALUE(1);
  if (fail) return;
  a = tempval;

  CONDITIONAL_GET_INPUT_VALUE(0);
  if (fail) return;
  b = tempval;

  CONDITIONAL_GET_INPUT_VALUE(0);
  if (fail) return;
  c = tempval;

  END_INPUT_STRING;

  if (hypot(a.real,a.imag)>1e-250) *output=b;
  else                             *output=c;

  return;
 }

