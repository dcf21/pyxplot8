// ppl_texify.c
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
#include <ctype.h>
#include <string.h>

#include "StringTools/str_constants.h"
#include "StringTools/asciidouble.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_memory.h"

#include "ppl_constants.h"
#include "ppl_setting_types.h"
#include "ppl_texify.h"
#include "ppl_units.h"
#include "ppl_userspace.h"

static unsigned char InMathMode;
static unsigned char InTextRm;

static Dict *LatexVarNames = NULL;

void texify_init()
 {
  if (LatexVarNames != NULL) return;
  LatexVarNames = DictInit();
  DictAppendString(LatexVarNames, "alpha"  , "\\alpha");
  DictAppendString(LatexVarNames, "beta"   , "\\beta");
  DictAppendString(LatexVarNames, "gamma"  , "\\gamma");
  DictAppendString(LatexVarNames, "delta"  , "\\delta");
  DictAppendString(LatexVarNames, "epsilon", "\\epsilon");
  DictAppendString(LatexVarNames, "zeta"   , "\\zeta");
  DictAppendString(LatexVarNames, "eta"    , "\\eta");
  DictAppendString(LatexVarNames, "theta"  , "\\theta");
  DictAppendString(LatexVarNames, "iota"   , "\\iota");
  DictAppendString(LatexVarNames, "kappa"  , "\\kappa");
  DictAppendString(LatexVarNames, "lambda" , "\\lambda");
  DictAppendString(LatexVarNames, "mu"     , "\\mu");
  DictAppendString(LatexVarNames, "nu"     , "\\nu");
  DictAppendString(LatexVarNames, "xi"     , "\\xi");
  DictAppendString(LatexVarNames, "pi"     , "\\pi");
  DictAppendString(LatexVarNames, "rho"    , "\\rho");
  DictAppendString(LatexVarNames, "sigma"  , "\\sigma");
  DictAppendString(LatexVarNames, "tau"    , "\\tau");
  DictAppendString(LatexVarNames, "upsilon", "\\upsilon");
  DictAppendString(LatexVarNames, "phi"    , "\\phi");
  DictAppendString(LatexVarNames, "chi"    , "\\chi");
  DictAppendString(LatexVarNames, "psi"    , "\\psi");
  DictAppendString(LatexVarNames, "omega"  , "\\omega");
  DictAppendString(LatexVarNames, "Gamma"  , "\\Gamma");
  DictAppendString(LatexVarNames, "Delta"  , "\\Delta");
  DictAppendString(LatexVarNames, "Theta"  , "\\Theta");
  DictAppendString(LatexVarNames, "Lambda" , "\\Lambda");
  DictAppendString(LatexVarNames, "Xi"     , "\\Xi");
  DictAppendString(LatexVarNames, "Pi"     , "\\Pi");
  DictAppendString(LatexVarNames, "Sigma"  , "\\Sigma");
  DictAppendString(LatexVarNames, "Upsilon", "\\Upsilon");
  DictAppendString(LatexVarNames, "Phi"    , "\\Phi");
  DictAppendString(LatexVarNames, "Psi"    , "\\Psi");
  DictAppendString(LatexVarNames, "Omega"  , "\\Omega");
  DictAppendString(LatexVarNames, "aleph"  , "\\aleph");
  return;
 }

void texify_generic(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth)
 {
  int i=0, j, outpos;
  if (RecursionDepth==1) { InMathMode = InTextRm = 0; }
  while ((in[i]!='\0')&&(in[i]<=' ')) i++;
  if (end!=NULL) j=(*end)-i;
  if ((in[i]=='\'')||(in[i]=='\"')) texify_quotedstring(in, &j, out, EvalStrings, status, errtext, RecursionDepth);
  else                              texify_algebra     (in, &j, out, EvalStrings, status, errtext, RecursionDepth);
  if (end!=NULL) *end=j+i;
  outpos = strlen(out);
  if (RecursionDepth==1)
   {
    if (InTextRm)   { InTextRm  =0; out[outpos++]='}'; } // Make sure not in TextRm
    if (InMathMode) { InMathMode=0; out[outpos++]='$'; } // Make sure not in math mode
   }
  out[outpos++]='\0';
 }

void texify_quotedstring(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth)
 {
  int  i=0, j=0, k, l, m;
  char QuoteType='\0', buffer[LSTR_LENGTH];
  int  CommaPositions[MAX_STR_FORMAT_ITEMS], Nargs;

  if (EvalStrings)
   {
    *status=-1;
    ppl_GetQuotedString(in, buffer, 0, end, status, errtext, 1); // end will get written here, and we want nothing more to do with it
    if (*status>=0) return;
    *status=0;
    end = NULL;
    in  = buffer;
   }

  while ((in[i]!='\0')&&(in[i]<=' ')) i++;
  if (!EvalStrings)
   {
    if ((in[i]!='\'')&&(in[i]!='\"')) { strcpy(errtext, "A quoted string should begin with a \' or a \"."); *status=1; return; }
    QuoteType = in[i];
   }

  if ((InMathMode)&&(!InTextRm)) { InTextRm=1; strcpy(out+j,"\\textrm{"); j+=strlen(out+j); } // Make sure not in math mode

  for (; ((in[i]!='\0') && ((i==0)||(in[i]!=QuoteType)||(in[i-1]=='\\')) && ((end==NULL)||(*end<0)||(i<*end))); i++)
   {
    if      (in[i]=='\\') { strcpy(out+j, "$\\backslash$"); j+=strlen(out+j); }
    else if (in[i]=='_' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='&' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='%' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='$' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='{' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='}' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='#' ) { out[j++]='\\'; out[j++]=in[i]; }
    else if (in[i]=='^' ) { strcpy(out+j, "\\^{}"); j+=strlen(out+j); }
    else if (in[i]=='~' ) { strcpy(out+j, "$\\sim$"); j+=strlen(out+j); }
    else if (in[i]=='<' ) { strcpy(out+j, "$\\lt$"); j+=strlen(out+j); }
    else if (in[i]=='>' ) { strcpy(out+j, "$\\gt$"); j+=strlen(out+j); }
    else if (in[i]=='\"') { out[j++]='`'; out[j++]='`'; }
    else                  { out[j++]=in[i]; }
   }

  if (in[i]!=QuoteType) { out[j++]='\0'; strcpy(errtext, "Mismatched quote in string to be texified."); *status=1; return; }

  if      (in[i]=='\'') { out[j++]='\''; } // Write closing quote to output
  else if (in[i]=='\"') { out[j++]='\''; out[j++]='\''; }

  if (in[i]!='\0') i++;
  while ((in[i]!='\0')&&(in[i]<=' ')) i++;

  // We have a quoted string with no % operator after it
  if (in[i]!='%')
   {
    out[j++]='\0';
    if ((end!=NULL)&&(*end>0)&&(i<*end)) { out[j++]='\0'; *status=1; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string."); return; }
    *status = 0;
    if ((end!=NULL)&&(*end<0)) *end=i;
    return;
   }

  // We have read a format string, but have a % operator acting upon it
  i++; // Skip over % character
  while ((in[i]>'\0') && (in[i]<=' ')) i++; // Fast-forward over preceding spaces
  if (in[i]!='(') { out[j++]='\0'; *status=1; strcpy(errtext, "Syntax Error: An opening bracket was expected after %% substitution operator."); return; }
  StrBracketMatch(in+i, CommaPositions, &Nargs, &k, MAX_STR_FORMAT_ITEMS);
  if (k<0) { out[j++]='\0'; *status=1; strcpy(errtext, "Syntax Error: Mismatched bracket"); return; }
  if (Nargs>=MAX_STR_FORMAT_ITEMS) { out[j++]='\0'; *status=1; strcpy(errtext, "Overflow Error: Too many string substitution arguments"); return; }
  if ((end!=NULL)&&(*end>0)&&(i+k<*end)) { out[j++]='\0'; *status=1; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string"); return; }

  strcpy(out+j, "\\%("); j+=strlen(out+j);
  for (l=0;l<Nargs;l++)
   {
    if (l!=0) out[j++]=',';
    m = CommaPositions[l+1] - (CommaPositions[l]+1);
    texify_generic(in+i+CommaPositions[l]+1, &m, out+j, EvalStrings, status, errtext, RecursionDepth+1);
    if (*status) return;
    j+=strlen(out+j);
    if ((InMathMode)&&(!InTextRm)) { InTextRm=1; strcpy(out+j,"\\textrm{"); j+=strlen(out+j); } // Make sure not in math mode
   }
  strcpy(out+j, ")"); j+=strlen(out+j);
  i+=k+1; // i should point to the character after the )
  if ((end!=NULL)&&(*end>0)&&(i<*end)) { *status=1; strcpy(errtext, "Syntax Error: Unexpected trailing matter after substitution () after quoted string."); return; }
  *status = 0;
  if ((end!=NULL)&&(*end<0)) *end=i;
  return;
 }

// StatusRow values 20-255 correspond to ResultBuffer values 0-235
#define BUFFER_OFFSET 20

#define MATCH_ONE(A)       (in[i]==A)
#define MATCH_TWO(A,B)    ((in[i]==A)&&(in[i+1]==B))
#define MATCH_THR(A,B,C)  ((in[i]==A)&&(in[i+1]==B)&&(in[i+2]==C))

#define FETCHPREV(VARA,VARB,VARC)  { wasunit=(StatusRowInitial[i-1]==3); for (j=i-1,cj=StatusRow[i-1]; ((StatusRow[j]==cj)&&(j>0)); j--); if (cj<BUFFER_OFFSET) { go=0; wasunit=0; } else { go=1; VARA=j; VARB=(int)(cj-BUFFER_OFFSET); VARC=i-1; } }
#define FETCHNEXT(VARA,VARB,VARC)  { for (j=i,ci=StatusRow[i]; StatusRow[j]==ci; j++); if ((cj=StatusRow[j])<BUFFER_OFFSET) { go=0; wasunit=0; } else { go=1; wasunit=(StatusRowInitial[j]==8); for (k=j; StatusRow[k]==cj; k++); VARA=j; VARB=(int)(cj-BUFFER_OFFSET); VARC=k; } }
#define SETSTATUS(BEG,END,VAL)     { ci = (unsigned char)(VAL+BUFFER_OFFSET); for (j=BEG;j<END;j++) StatusRow[j]=ci; }
#define RESETSTATUS(BEG,END)       { for (j=BEG;j<END;j++) StatusRow[j]=StatusRowInitial[j]; }

void texify_algebra(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth)
 {
  int i, j, k, l, m, outpos=0, CalculatedEnd, bufpos=0;
  int prev_start, prev_end, next_start, next_end, prev_bufno, next_bufno;
  int PowLevel = 0;
  unsigned char ci,cj;
  unsigned char go, wasunit, wasunit2;
  unsigned char OpList[OPLIST_LEN];                  // A list of what operations this expression contains
  unsigned char StatusRow       [ALGEBRA_MAXLENGTH]; // Describes the atoms at each position in the expression
  unsigned char StatusRowInitial[ALGEBRA_MAXLENGTH];
  char dummy[DUMMYVAR_MAXLEN];
  value ResultBuffer[ALGEBRA_MAXITEMS];       // A buffer of temporary numerical results
  DictIterator *DictIter;

  if (RecursionDepth > MAX_RECURSION_DEPTH) { *status=1; strcpy(errtext,"Overflow Error: Maximum recursion depth exceeded"); return; }

  *status=-1;
  CalculatedEnd=-1;
  ppl_GetExpression(in, &CalculatedEnd, 0, StatusRow, OpList, status, errtext);
  if (*status >= 0) { *status=1; return; }
  *status=0;
  if ((end != NULL) && (*end >  0) && (CalculatedEnd < *end)) { *status=1; strcpy(errtext,"Syntax Error: Unexpected trailing matter after algebraic expression"); return; }
  if ((end != NULL) && (*end <= 0)) *end = CalculatedEnd;

  for (i=0;i<ALGEBRA_MAXLENGTH;i++) StatusRowInitial[i] = StatusRow[i];

  // EVALUATION OF UNIT() FUNCTION
  if (OpList[1]!=0) for (i=0;i<CalculatedEnd-1;i++) if ((StatusRow[i]==8)&&(StatusRow[i+1]==3))
   {
    for (j=i;((j>0)&&(StatusRow[j-1]==8));j--); // Rewind to beginning of function name
    for (k=0; (("unit"[k]==in[j+k])); k++); // See if string we have matches the name of the unit() function
    if ((k!=4) || (isalnum(in[j+k])) || (in[j+k]=='_')) continue; // Nope...
    i+=2;
    j=-1;
    ppl_units_StringEvaluate(in+i, ResultBuffer+bufpos, &k, &j, errtext);
    if (j>=0) { *status=1; return; }
    i+=k; while ((in[i]>'\0')&&(in[i]<=' ')) i++;
    if (in[i] != ')') { *status=1; strcpy(errtext,"Syntax Error: Unexpected trailing matter after unit(...) expression."); return; }
    while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++;
    for ( ; StatusRow[i]==8; i++) StatusRow[i] = (unsigned char)(bufpos + BUFFER_OFFSET);
    for ( ; StatusRow[i]==3; i++) StatusRow[i] = (unsigned char)(bufpos + BUFFER_OFFSET);
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *status=1; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
   }

  // EVALUATION OF ALL NUMERICAL CONSTANTS
  for (i=0;i<CalculatedEnd;i++) if (StatusRow[i]==6)
   {
    ppl_units_zero(ResultBuffer+bufpos);
    ResultBuffer[bufpos].real = GetFloat(in+i, &j);
    for (k=i; StatusRow[k]==6; k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    j+=i; while ((in[j]>'\0')&&(in[j]<=' ')) j++;
    if (j!=k) { *status=1; strcpy(errtext,"Syntax Error: Unexpected trailing matter after numeric constant."); return; }
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *status=1; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
   }

  // EVALUATION OF MINUS SIGNS
  if (OpList[3]!=0) for (i=0;i<CalculatedEnd;i++) if (StatusRow[i]==5)
   {
    FETCHNEXT(next_start, next_bufno, next_end);
    if (!go) continue;
    ResultBuffer[next_bufno].real *= -1;
    ResultBuffer[next_bufno].imag *= -1;
    SETSTATUS(i, next_start, next_bufno);
    i = next_start;
    i--;
   }

  // EVALUATION OF ** (if left operand was a unit() call)
  if (OpList[4]!=0) for (i=0;i<CalculatedEnd;i++) if (StatusRow[i]==7)
   {
    if (MATCH_TWO('*','*'))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      if ((!go) || (!wasunit)) { while (StatusRow[i]==7) i++; continue; }
      FETCHNEXT(next_start, next_bufno, next_end);
      if (!go) { RESETSTATUS(prev_start, i); while (StatusRow[i]==7) i++; }
      else
       {
        *status=-1;
        ppl_units_pow(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , status , errtext);
        if (*status >= 0) { *status=1; return; }
        *status=0;
        SETSTATUS(prev_end, next_end, prev_bufno);
        i = next_start - 1;
       }
     } else {
      while (StatusRow[i]==7) i++;
     }
   }

  // EVALUATION OF *  /  (if either operand was a unit() call)
  if (OpList[5]!=0) for (i=0;i<CalculatedEnd;i++) if (StatusRow[i]==7)
   {
    if ((MATCH_ONE('*'))||(MATCH_ONE('/')))
     {
      FETCHPREV(prev_start, prev_bufno, prev_end);
      if (!go) { while (StatusRow[i]==7) i++; continue; }
      wasunit2 = wasunit;
      FETCHNEXT(next_start, next_bufno, next_end);
      if (!go) { while (StatusRow[i]==7) i++; continue; }
      if ((!wasunit)&&(!wasunit2)) { while (StatusRow[i]==7) i++; continue; }
      *status=-1;
      if      (MATCH_ONE('*')) ppl_units_mult(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , status , errtext);
      else if (MATCH_ONE('/')) ppl_units_div (ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , status , errtext);
      if (*status >= 0) { *status=1; return; }
      *status=0;
      SETSTATUS(prev_end, next_end, prev_bufno);
      i = next_start - 1;
     } else {
      while (StatusRow[i]==7) i++;
     }
   }

  // Now start displaying things
  for (i=0;i<CalculatedEnd;i++)
   {
    if (StatusRow[i]>=BUFFER_OFFSET) // A numeric constant
     {
      if (!InMathMode) { InMathMode=1; out[outpos++]='$'; } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      strcpy(out+outpos, ppl_units_NumericDisplay(ResultBuffer+StatusRow[i]-BUFFER_OFFSET, 0, SW_DISPLAY_L)+1); // Chop off initial $
      outpos += strlen(out+outpos) - 1; // Chop off final $
      ci = StatusRow[i];
      while (StatusRow[i]==ci) i++; i--;
      while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
     }
    else if (StatusRow[i]==3)
     {
      i++; // Fastforward over (
      strcpy(out+outpos, "\\left( "); outpos += strlen(out+outpos);
      while (1)
       {
        while (1)
         {
          while ((in[i]!='\0')&&(in[i]<=' ')) i++;
          if (in[i]!=',') break;
          out[outpos++]=',';
          i++;
         }
        if ((in[i]=='\0')||(in[i]==')')) break;
        j=-1;
        texify_generic(in+i, &j, out+outpos, 0, status, errtext, RecursionDepth+1);
        if (*status) return;
        outpos += strlen(out+outpos);
        if (!InMathMode) { InMathMode=1; out[outpos++]='$'; } // Make sure we are in math mode
        if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
        i+=j;
       }
      strcpy(out+outpos, "\\right) ");
      outpos += strlen(out+outpos);
      while (StatusRow[i]==3) i++; i--;
      while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
     }
    else if (StatusRow[i]==7) // Texify an operator
     {
      if (!InMathMode) { InMathMode=1; out[outpos++]='$'; } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      out[outpos]='\0';
      if      ((in[i]=='a')&&(in[i+1]=='n')&&(in[i+2]=='d')) strcpy(out+outpos, "\\mathrm{and}");
      else if ((in[i]=='o')&&(in[i+1]=='r')                ) strcpy(out+outpos, "\\mathrm{or}");
      else if ((in[i]=='<')&&(in[i+1]=='<')                ) strcpy(out+outpos, "\\lt\\lt ");
      else if ((in[i]=='>')&&(in[i+1]=='>')                ) strcpy(out+outpos, "\\gt\\gt ");
      else if ((in[i]=='<')&&(in[i+1]=='=')                ) strcpy(out+outpos, "\\leq ");
      else if ((in[i]=='>')&&(in[i+1]=='=')                ) strcpy(out+outpos, "\\geq ");
      else if ((in[i]=='=')&&(in[i+1]=='=')                ) strcpy(out+outpos, "==");
      else if ((in[i]=='<')&&(in[i+1]=='>')                ) strcpy(out+outpos, "\\neq ");
      else if ((in[i]=='!')&&(in[i+1]=='=')                ) strcpy(out+outpos, "\\neq ");
      else if ((in[i]=='*')&&(in[i+1]=='*')                ) { strcpy(out+outpos, "^{"); PowLevel++; }
      else if ((in[i]=='+')                                ) strcpy(out+outpos, "+");
      else if ((in[i]=='-')                                ) strcpy(out+outpos, "-");
      else if ((in[i]=='*')                                ) strcpy(out+outpos, "\\times ");
      else if ((in[i]=='/')                                ) strcpy(out+outpos, "/");
      else if ((in[i]=='%')                                ) strcpy(out+outpos, "\\% ");
      else if ((in[i]=='&')                                ) strcpy(out+outpos, "\\& ");
      else if ((in[i]=='|')                                ) strcpy(out+outpos, "|");
      else if ((in[i]=='<')                                ) strcpy(out+outpos, "\\lt ");
      else if ((in[i]=='>')                                ) strcpy(out+outpos, "\\gt ");
      else if ((in[i]=='^')                                ) strcpy(out+outpos, "\\^{}");
      outpos += strlen(out+outpos);
      while (StatusRow[i]==7) i++; i--;
     }
    else if (StatusRow[i]==4) // Texify a $
     {
      if (!InMathMode) { InMathMode=1; out[outpos++]='$'; } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      strcpy(out+outpos, "\\$");
      outpos += strlen(out+outpos);
      while (StatusRow[i]==4) i++; i--;
     }
    else if (StatusRow[i]==8) // Texify a variable name
     {
      if (!InMathMode) { InMathMode=1; out[outpos++]='$'; } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      k=0;
      for ( DictIter=DictIterateInit(LatexVarNames) ; (DictIter!=NULL) ; DictIter=DictIterate(DictIter,NULL,NULL) )
       {
        for (j=0; DictIter->key[j]!='\0'; j++) if (DictIter->key[j]!=in[i+j]) break;
        if (DictIter->key[j]!='\0') continue;
        if ((!isalnum(in[i+j]))&&(in[i+j]!='_')) { strcpy(out+outpos, (char *)DictIter->data); outpos += strlen(out+outpos); k=1; break; } // We have a Greek letter
        if (in[i+j]=='_') j++;
        if (isdigit(in[i+j]))
         {
          l=j;
          while (isdigit(in[i+j])) j++;
          if ((isalnum(in[i+j]))||(in[i+j]=='_')) continue;
          sprintf(out+outpos, "%s_{", (char *)DictIter->data); // We have Greek letter underscore number
          outpos += strlen(out+outpos);
          for (m=l;m<j;m++) out[outpos++]=in[i+m];
          out[outpos++]='}';
          k=1;
          break;
         }
       }
      if (k==0) // Variable name was not a known Greek character
       {
        for ( ; StatusRow[i]==8; i++)
         {
          if (in[i]<' ') continue;
          if (in[i]=='_') out[outpos++]='\\';
          out[outpos++]=in[i];
         }
       }
      while (StatusRow[i]==8) i++;
      i--;
      while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
     }
   }
  while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
  out[outpos++]='\0';
  return; // Finished
 }

void wrapper_texify(char *in, int inlen, value *output, int *status, char *errtext)
 {
  *status=0;
  ppl_units_zero(output);
  inlen--; // Make inlen point to last character
  while ((in[0]!='\0')&&(in[0]<=' '))   { in++; inlen--; } // Strip spaces off front
  while ((inlen>0)&&(in[inlen]<=' '))   {       inlen--; } // Strip spaces off back
  if ((inlen<0)||(inlen>ALGEBRA_MAXLENGTH)) { sprintf(errtext, "Supplied expression is longer than maximum of %d characters.", ALGEBRA_MAXLENGTH); *status=1; return; }
  output->string = lt_malloc(ALGEBRA_MAXLENGTH);
  output->string[0] = '\0';
  texify_generic(in, &inlen, output->string, 1, status, errtext, 1);
 }

