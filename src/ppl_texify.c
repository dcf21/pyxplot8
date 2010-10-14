// ppl_texify.c
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
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

static unsigned char InMathMode;
static unsigned char InTextRm;

#define ENTERMATHMODE "$\\displaystyle "

static char *LatexVarNamesFr[] = {"alpha"  ,"beta"   ,"gamma"  ,"delta"  ,"epsilon","zeta"   ,"eta"    ,"theta"  ,"iota"   ,"kappa"  ,"lambda" ,"mu"     ,"nu"     ,"xi"     ,"pi"     ,"rho"    ,"sigma"  ,"tau"    ,"upsilon","phi"    ,"chi"    ,"psi"    ,"omega"  ,"Gamma"  ,"Delta"  ,"Theta"  ,"Lambda" ,"Xi"     ,"Pi"     ,"Sigma"  ,"Upsilon","Phi"    ,"Psi"    ,"Omega"  ,"aleph"  , NULL};

static char *LatexVarNamesTo[] = {"\\alpha","\\beta","\\gamma","\\delta","\\epsilon","\\zeta","\\eta","\\theta","\\iota","\\kappa","\\lambda","\\mu","\\nu","\\xi","\\pi","\\rho","\\sigma","\\tau","\\upsilon","\\phi","\\chi","\\psi","\\omega","\\Gamma","\\Delta","\\Theta","\\Lambda","\\Xi","\\Pi","\\Sigma","\\Upsilon","\\Phi","\\Psi","\\Omega","\\aleph",NULL};

void texify_MakeGreek(const char *in, char *out)
 {
  int i,ji=0,jg,k,l,m;
  char *outno_=out;

  while ((isalnum(in[ji]))||(in[ji]=='_'))
   {
    k=0;
    for (i=0; LatexVarNamesFr[i]!=NULL; i++)
     {
      for (jg=0; LatexVarNamesFr[i][jg]!='\0'; jg++) if (LatexVarNamesFr[i][jg]!=in[ji+jg]) break;
      if (LatexVarNamesFr[i][jg]!='\0') continue;
      if (!isalnum(in[ji+jg])) // We have a Greek letter
       {
        strcpy(out, LatexVarNamesTo[i]);
        out += strlen(out);
        ji += jg;
        k=1;
        break;
       }
      else if (isdigit(in[ji+jg]))
       {
        l=ji+jg;
        while (isdigit(in[ji+jg])) jg++;
        if (isalnum(in[ji+jg])) continue;
        sprintf(out, "%s_{", LatexVarNamesTo[i]); // We have Greek letter underscore number
        out += strlen(out);
        for (m=l;m<ji+jg;m++) *(out++)=in[m];
        *(out++) = '}';
        ji += jg;
        k=1;
        break;
       }
     }
    if ((k==0)&&(ji>1))
     {
      for (jg=0; isdigit(in[ji+jg]); jg++);
      if ( (jg>0) && (!isalnum(in[ji+jg])) && (in[ji+jg]!='_') )
       {
        out=outno_;
        sprintf(out, "_{"); // We have an underscore number at the end of variable name
        out += strlen(out);
        for (m=ji;m<ji+jg;m++) *(out++)=in[m];
        *(out++) = '}';
        ji += jg;
        k=1;
       }
     }
    if (k==0)
     {
      for (jg=0; (isalnum(in[ji+jg])); jg++) *(out++)=in[ji+jg];
      ji+=jg;
     }
    outno_=out;
    if (in[ji]=='_')
     {
      *(out++)='\\'; *(out++)='_';
      ji++;
     }
   }
  *(out++)='\0';
  return;
 }

void texify_generic(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth, int *BracketLevel)
 {
  int i=0, j, outpos;
  if (RecursionDepth==1) { InMathMode = InTextRm = 0; }
  while ((in[i]!='\0')&&(in[i]<=' ')) i++;
  if (end!=NULL) j=(*end)-i;
  if ((in[i]=='\'')||(in[i]=='\"')) texify_quotedstring(in, &j, out, EvalStrings, status, errtext, RecursionDepth, BracketLevel);
  else                              texify_algebra     (in, &j, out, EvalStrings, status, errtext, RecursionDepth, BracketLevel);
  if (end!=NULL) *end=j+i;
  if (*status>=0) return;
  outpos = strlen(out);
  if (RecursionDepth==1)
   {
    if (InTextRm)   { InTextRm  =0; out[outpos++]='}'; } // Make sure not in TextRm
    if (InMathMode) { InMathMode=0; out[outpos++]='$'; } // Make sure not in math mode
   }
  out[outpos++]='\0';
  return;
 }

void texify_quotedstring(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth, int *BracketLevel)
 {
  int  i=0, j=0, k, l, m, OpenQuotePos=0, DoubleQuoteLevel=0;
  char QuoteType='\0', buffer[LSTR_LENGTH];
  int  CommaPositions[MAX_STR_FORMAT_ITEMS], Nargs;

  *BracketLevel = 0;

  if (EvalStrings)
   {
    ppl_GetQuotedString(in, buffer, 0, end, 0, status, errtext, 1); // end will get written here, and we want nothing more to do with it
    if (*status>=0) return;
    end = NULL;
    in  = buffer;
   }

  while ((in[i]!='\0')&&(in[i]<=' ')) i++;
  if (!EvalStrings)
   {
    if ((in[i]!='\'')&&(in[i]!='\"')) { strcpy(errtext, "Syntax Error: A quoted string should begin with a \' or a \"."); *status=i; return; }
    QuoteType = in[i];
    OpenQuotePos = i;
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
    else if (in[i]=='<' ) { strcpy(out+j, "$<$"); j+=strlen(out+j); }
    else if (in[i]=='>' ) { strcpy(out+j, "$>$"); j+=strlen(out+j); }
    else if (in[i]=='\"') { out[j++]=DoubleQuoteLevel?'\'':'`'; out[j++]=DoubleQuoteLevel?'\'':'`'; DoubleQuoteLevel=!DoubleQuoteLevel; }
    else                  { out[j++]=in[i]; }
   }

  if (in[i]!=QuoteType) { out[j++]='\0'; strcpy(errtext, "Syntax Error: Mismatched quote in string to be texified."); *status=OpenQuotePos; return; }

  if      (in[i]=='\'') { out[j++]='\''; } // Write closing quote to output
  else if (in[i]=='\"') { out[j++]='\''; out[j++]='\''; }

  if (in[i]!='\0') i++;
  while ((in[i]!='\0')&&(in[i]<=' ')) i++;

  // We have a quoted string with no % operator after it
  if (in[i]!='%')
   {
    out[j++]='\0';
    if ((end!=NULL)&&(*end>0)&&(i<*end)) { out[j++]='\0'; *status=i; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string."); return; }
    *status = -1;
    if ((end!=NULL)&&(*end<0)) *end=i;
    return;
   }

  // We have read a format string, but have a % operator acting upon it
  i++; // Skip over % character
  while ((in[i]>'\0') && (in[i]<=' ')) i++; // Fast-forward over preceding spaces
  if (in[i]!='(') { out[j++]='\0'; *status=i; strcpy(errtext, "Syntax Error: An opening bracket was expected after %% substitution operator."); return; }
  StrBracketMatch(in+i, CommaPositions, &Nargs, &k, MAX_STR_FORMAT_ITEMS);
  if (k<0) { out[j++]='\0'; *status=i; strcpy(errtext, "Syntax Error: Mismatched bracket after %% string substitution operator."); return; }
  if (Nargs>=MAX_STR_FORMAT_ITEMS) { out[j++]='\0'; *status=i; strcpy(errtext, "Overflow Error: Too many string substitution arguments."); return; }
  if ((end!=NULL)&&(*end>0)&&(i+k<*end)) { out[j++]='\0'; *status=i+k; strcpy(errtext, "Syntax Error: Unexpected trailing matter after quoted string."); return; }

  // Brackets work best in mathmode
  if (!InMathMode) { InMathMode=1; strcpy(out+j, ENTERMATHMODE); j+=strlen(out+j); } // Make sure we are in math mode
  if ( InTextRm  ) { InTextRm  =0; out[j++]='}'; } // Make sure we are not in textrm
  strcpy(out+j, "\\%\\left("); j+=strlen(out+j);
  for (l=0;l<Nargs;l++)
   {
    if (l!=0)
     {
      if ((InMathMode)&&(!InTextRm)) { InTextRm=1; strcpy(out+j,"\\textrm{"); j+=strlen(out+j); } // Make sure not in math mode
      out[j++]=',';
     }
    m = CommaPositions[l+1] - (CommaPositions[l]+1);
    texify_generic(in+i+CommaPositions[l]+1, &m, out+j, EvalStrings, status, errtext, RecursionDepth+1, BracketLevel);
    if (*status>=0) { (*status)+=i+CommaPositions[l]+1; return; }
    j+=strlen(out+j);
   }
  if (!InMathMode) { InMathMode=1; strcpy(out+j, ENTERMATHMODE); j+=strlen(out+j); } // Make sure we are in math mode
  if ( InTextRm  ) { InTextRm  =0; out[j++]='}'; } // Make sure we are not in textrm
  strcpy(out+j, "\\right)"); j+=strlen(out+j);
  i+=k+1; // i should point to the character after the )
  *BracketLevel=1; // We have just used ()
  if ((end!=NULL)&&(*end>0)&&(i<*end)) { *status=i; strcpy(errtext, "Syntax Error: Unexpected trailing matter after substitution () after quoted string."); return; }
  *status = -1;
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

void texify_algebra(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth, int *BracketLevel)
 {
  int i, j, k, l, m, outpos=0, CalculatedEnd, OpenBracketPos=0, bufpos=0, BracketType, OldBracketLevel=0;
  int prev_start, prev_end, next_start, next_end, prev_bufno, next_bufno;
  int PowLevel = 0;
  unsigned char ci,cj;
  unsigned char go, wasunit, wasunit2;
  unsigned char OpList[OPLIST_LEN];                  // A list of what operations this expression contains
  unsigned char StatusRow       [ALGEBRA_MAXLENGTH]; // Describes the atoms at each position in the expression
  unsigned char StatusRowInitial[ALGEBRA_MAXLENGTH];
  char          dummy[DUMMYVAR_MAXLEN], GreekifiedDummy[DUMMYVAR_MAXLEN+10], *LaTeXModel, DummyLaTeXModel[FNAME_LENGTH], *FunctionName=NULL;
  int           CommaPositions[MAX_STR_FORMAT_ITEMS], Nargs, RequiredNargs;
  int           LastItemStartPos=0;
  unsigned char LastItemContainedSuperscript=0;
  value ResultBuffer[ALGEBRA_MAXITEMS];       // A buffer of temporary numerical results
  DictIterator *DictIter;

  if (RecursionDepth > MAX_RECURSION_DEPTH) { *status=0; strcpy(errtext,"Overflow Error: Maximum recursion depth exceeded."); return; }

  *BracketLevel = 0;

  CalculatedEnd=-1;
  ppl_GetExpression(in, &CalculatedEnd, 1, StatusRow, OpList, status, errtext);
  if (*status >= 0) return;
  if ((end != NULL) && (*end >  0) && (CalculatedEnd < *end)) { *status=CalculatedEnd; strcpy(errtext,"Syntax Error: Unexpected trailing matter after algebraic expression."); return; }
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
    if (j>=0) { *status=i+j; return; }
    i+=k; while ((in[i]>'\0')&&(in[i]<=' ')) i++;
    if (in[i] != ')') { *status=i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after unit(...) expression."); return; }
    while (StatusRow[i]==3) i--; while ((i>0)&&(StatusRow[i]==8)) i--; if (StatusRow[i]!=8) i++;
    for ( ; StatusRow[i]==8; i++) StatusRow[i] = (unsigned char)(bufpos + BUFFER_OFFSET);
    for ( ; StatusRow[i]==3; i++) StatusRow[i] = (unsigned char)(bufpos + BUFFER_OFFSET);
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *status=0; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
   }

  // EVALUATION OF ALL NUMERICAL CONSTANTS
  for (i=0;i<CalculatedEnd;i++) if (StatusRow[i]==6)
   {
    ppl_units_zero(ResultBuffer+bufpos);
    ResultBuffer[bufpos].real = GetFloat(in+i, &j);
    for (k=i; StatusRow[k]==6; k++) StatusRow[k] = (unsigned char)(bufpos + BUFFER_OFFSET);
    j+=i; while ((in[j]>'\0')&&(in[j]<=' ')) j++;
    if (j!=k) { *status=j; strcpy(errtext,"Syntax Error: Unexpected trailing matter after numeric constant."); return; }
    bufpos++; if (bufpos >= ALGEBRA_MAXITEMS) { *status=0; strcpy(errtext,"Internal error: Temporary results buffer overflow."); return; }
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
        ppl_units_pow(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , status , errtext);
        if (*status >= 0) { *status=i; return; }
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
      if      (MATCH_ONE('*')) ppl_units_mult(ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , status , errtext);
      else if (MATCH_ONE('/')) ppl_units_div (ResultBuffer+prev_bufno , ResultBuffer+next_bufno , ResultBuffer+prev_bufno , status , errtext);
      if (*status >= 0) { *status=i; return; }
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
      LastItemContainedSuperscript = 0;
      if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      LastItemStartPos = outpos;
      strcpy(out+outpos, ppl_units_NumericDisplay(ResultBuffer+StatusRow[i]-BUFFER_OFFSET, 0, SW_DISPLAY_L, 0)+1); // Chop off initial $
      for (k=0; out[outpos+k]!='\0'; k++) if ((out[outpos+k]=='^')||((out[outpos+k]=='\\')&&(out[outpos+k+1]==','))) LastItemContainedSuperscript = 1;
      outpos += strlen(out+outpos) - 1; // Chop off final $
      ci = StatusRow[i];
      while (StatusRow[i]==ci) i++; i--;
      if (!((StatusRow[i+1]==7)&&(in[i+1]=='*')&&(in[i+2]=='*'))) while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
     }
    else if (StatusRow[i]==3)
     {
      LastItemContainedSuperscript = 0;
      i++; // Fastforward over (
      if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      strcpy(out+outpos, "\\left??"); outpos += strlen(out+outpos); OpenBracketPos = outpos-2;
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
        texify_generic(in+i, &j, out+outpos, 0, status, errtext, RecursionDepth+1, BracketLevel);
        if (*status>=0) { (*status)+=i; return; }
        outpos += strlen(out+outpos);
        if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
        if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
        i+=j;

        if ((*BracketLevel)>OldBracketLevel) OldBracketLevel = (*BracketLevel); // OldBracketLevel should record the deepest we've been
       }
      strcpy(out+outpos, "\\right??"); outpos += strlen(out+outpos);

      (*BracketLevel) = OldBracketLevel;
      BracketType = (*BracketLevel) % 3;
      if      (BracketType==0) { out[OpenBracketPos+0]='(' ; out[OpenBracketPos+1]=' '; out[outpos-2]=')' ; out[outpos-1]=' '; }
      else if (BracketType==1) { out[OpenBracketPos+0]='[' ; out[OpenBracketPos+1]=' '; out[outpos-2]=']' ; out[outpos-1]=' '; }
      else                     { out[OpenBracketPos+0]='\\'; out[OpenBracketPos+1]='{'; out[outpos-2]='\\'; out[outpos-1]='}'; }

      while (StatusRow[i]==3) i++; i--;
      if (!((StatusRow[i+1]==7)&&(in[i+1]=='*')&&(in[i+2]=='*'))) while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }

      (*BracketLevel)++;
     }
    else if (StatusRow[i]==5) // Texify a unary minus sign
     {
      LastItemContainedSuperscript = 0;
      if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      strcpy(out+outpos, "-");
      outpos += strlen(out+outpos);
      while (StatusRow[i]==5) i++; i--;
     }
    else if (StatusRow[i]==7) // Texify an operator
     {
      if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      out[outpos]='\0';
      if      ((in[i]=='a')&&(in[i+1]=='n')&&(in[i+2]=='d')) strcpy(out+outpos, "\\mathrm{and}");
      else if ((in[i]=='o')&&(in[i+1]=='r')                ) strcpy(out+outpos, "\\mathrm{or}");
      else if ((in[i]=='<')&&(in[i+1]=='<')                ) strcpy(out+outpos, "<< ");
      else if ((in[i]=='>')&&(in[i+1]=='>')                ) strcpy(out+outpos, ">> ");
      else if ((in[i]=='<')&&(in[i+1]=='=')                ) strcpy(out+outpos, "\\leq ");
      else if ((in[i]=='>')&&(in[i+1]=='=')                ) strcpy(out+outpos, "\\geq ");
      else if ((in[i]=='=')&&(in[i+1]=='=')                ) strcpy(out+outpos, "==");
      else if ((in[i]=='<')&&(in[i+1]=='>')                ) strcpy(out+outpos, "\\neq ");
      else if ((in[i]=='!')&&(in[i+1]=='=')                ) strcpy(out+outpos, "\\neq ");
      else if ((in[i]=='*')&&(in[i+1]=='*')                )
       {
        if (LastItemContainedSuperscript) // Need to put brackets around the last rendered item
         {
          for (k=0; (outpos-k)>=LastItemStartPos; k++) out[outpos-k+6] = out[outpos-k];
          sprintf(out+LastItemStartPos, "\\left");
          out[LastItemStartPos+5]='(';
          sprintf(out+outpos+6, "\\right)");
          outpos += strlen(out+outpos);
         }
        strcpy(out+outpos, "^{"); PowLevel++;
       }
      else if ((in[i]=='+')                                ) strcpy(out+outpos, "+");
      else if ((in[i]=='-')                                ) strcpy(out+outpos, "-");
      else if ((in[i]=='*')                                ) strcpy(out+outpos, "\\times ");
      else if ((in[i]=='/')                                ) strcpy(out+outpos, "/");
      else if ((in[i]=='%')                                ) strcpy(out+outpos, "\\% ");
      else if ((in[i]=='&')                                ) strcpy(out+outpos, "\\& ");
      else if ((in[i]=='|')                                ) strcpy(out+outpos, "|");
      else if ((in[i]=='<')                                ) strcpy(out+outpos, "< ");
      else if ((in[i]=='>')                                ) strcpy(out+outpos, "> ");
      else if ((in[i]=='^')                                ) strcpy(out+outpos, "\\^{}");
      outpos += strlen(out+outpos);
      while (StatusRow[i]==7) i++; i--;
      LastItemContainedSuperscript = 0;
     }
    else if (StatusRow[i]==4) // Texify a $
     {
      LastItemContainedSuperscript = 0;
      if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
      strcpy(out+outpos, "\\$");
      outpos += strlen(out+outpos);
      while (StatusRow[i]==4) i++; i--;
     }
    else if (StatusRow[i]==8) // Texify a variable name or function call
     {
      LastItemContainedSuperscript = 0;
      if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
      if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm

      for (j=i ; ((j<CalculatedEnd)&&(StatusRow[j]==8)) ; j++); // FFW to see what next atom is... if () this is a function call

      if ((j<CalculatedEnd) && (StatusRow[j]==3)) // This is a function call
       {
        RequiredNargs = -1;
        LaTeXModel    = NULL;
        for ( DictIter=DictIterateInit(_ppl_UserSpace_Funcs) ; (DictIter!=NULL) ; DictIter=DictIterate(DictIter,NULL,NULL) )
         {
          for (k=0; ((DictIter->key[k]>' ')&&(DictIter->key[k]!='?')&&(DictIter->key[k]==in[i+k])); k++);
          if (DictIter->key[k]=='?') // This function name, e.g. int_dx, ends with a dummy variable name
           {
            for (l=0; ((isalnum(in[i+k+l]) || (in[i+k+l]=='_')) && (l<DUMMYVAR_MAXLEN)); l++) dummy[l]=in[i+k+l];
            if (l==DUMMYVAR_MAXLEN) continue; // Dummy variable name was too long
            dummy[l]='\0'; // Dummy hold the name of the variable being integrated over
            texify_MakeGreek(dummy , GreekifiedDummy); // Make dummy variable have Greek lettering as required
           } else { // Otherwise, we have to match function name exactly
            if ((DictIter->key[k]>' ') || (isalnum(in[i+k])) || (in[i+k]=='_')) continue; // Nope...
           }
          FunctionName  = DictIter->key;
          RequiredNargs = ((FunctionDescriptor *)DictIter->data)->NumberArguments;
          LaTeXModel    = ((FunctionDescriptor *)DictIter->data)->LaTeX;
          break;
         }
        if (LaTeXModel == NULL) // We have no LaTeX template for this function, so we construct one which puts function name in Roman
         {
          LaTeXModel = DummyLaTeXModel;
          k=0;
          strcpy(DummyLaTeXModel+k, "\\mathrm{"); k+=strlen(DummyLaTeXModel+k);
          for (l=i;l<j;l++)
           {
            if      (in[l]<=' ') continue;
            else if (in[l]=='_') DummyLaTeXModel[k++]='\\';
            DummyLaTeXModel[k++]=in[l];
           }
          strcpy(DummyLaTeXModel+k, "}@<@0@>");
         }
        StrBracketMatch(in+j, CommaPositions, &Nargs, &k, MAX_STR_FORMAT_ITEMS);
        if ((RequiredNargs >= 0) && (RequiredNargs != Nargs))
         {
          *status=j;
          sprintf(errtext,"The %s() function takes %d arguments, but %d have been supplied in expression to be texified.", FunctionName, RequiredNargs, Nargs);
          return;
         }

        // Realise LaTeX model
        for (k=0; LaTeXModel[k]!='\0'; k++)
         {
          if      (LaTeXModel[k  ]!='@') out[outpos++]=LaTeXModel[k];
          else if (LaTeXModel[k+1]=='?') { strcpy(out+outpos , GreekifiedDummy); outpos += strlen(out+outpos); }
          else if (LaTeXModel[k+1]=='<') { strcpy(out+outpos , "\\left??"); outpos += strlen(out+outpos); OpenBracketPos = outpos-2; }
          else if (LaTeXModel[k+1]=='>')
           {
            strcpy(out+outpos , "\\right??"); outpos += strlen(out+outpos);
            (*BracketLevel) = OldBracketLevel;
            BracketType = (*BracketLevel) % 3;
            if      (BracketType==0) { out[OpenBracketPos+0]='(' ; out[OpenBracketPos+1]=' '; out[outpos-2]=')' ; out[outpos-1]=' '; }
            else if (BracketType==1) { out[OpenBracketPos+0]='[' ; out[OpenBracketPos+1]=' '; out[outpos-2]=']' ; out[outpos-1]=' '; }
            else                     { out[OpenBracketPos+0]='\\'; out[OpenBracketPos+1]='{'; out[outpos-2]='\\'; out[outpos-1]='}'; }
            (*BracketLevel)++;
           }
          else if ((LaTeXModel[k+1]>='1') && (LaTeXModel[k+1]<='6'))
           {
            l=(int)(LaTeXModel[k+1]-'1');
            m=-1;
            texify_generic(in+CommaPositions[l]+j+1, &m, out+outpos, 0, status, errtext, RecursionDepth+1, BracketLevel);
            if (*status>=0) { (*status)+=CommaPositions[l]+j; return; }
            outpos += strlen(out+outpos);
            if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
            if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
            if ((*BracketLevel)>OldBracketLevel) OldBracketLevel = (*BracketLevel); // OldBracketLevel should record the deepest we've been
           }
          else if (LaTeXModel[k+1]=='0') // @0 means we list all commandline arguments
           {
            for (l=0; l<Nargs; l++)
             {
              if (l!=0) out[outpos++]=',';
              m=-1;
              texify_generic(in+CommaPositions[l]+j+1, &m, out+outpos, 0, status, errtext, RecursionDepth+1, BracketLevel);
              if (*status>=0) { (*status)+=CommaPositions[l]+j; return; }
              outpos += strlen(out+outpos);
              if (!InMathMode) { InMathMode=1; strcpy(out+outpos, ENTERMATHMODE); outpos+=strlen(out+outpos); } // Make sure we are in math mode
              if ( InTextRm  ) { InTextRm  =0; out[outpos++]='}'; } // Make sure we are not in textrm
              if ((*BracketLevel)>OldBracketLevel) OldBracketLevel = (*BracketLevel); // OldBracketLevel should record the deepest we've been
             }
           }
          if (LaTeXModel[k]=='@') k++; // FFW over two-byte code
         }

        i=j;
        while (StatusRow[i]==3) i++;
        i--;
       }
      else // This is a simple variable name
       {
        texify_MakeGreek(in+i , out+outpos);
        outpos += strlen(out+outpos);
        while (StatusRow[i]==8) i++;
        i--;
       }
      if (!((StatusRow[i+1]==7)&&(in[i+1]=='*')&&(in[i+2]=='*'))) while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
     }
   }
  while (PowLevel>0) { out[outpos++]='}'; PowLevel--; }
  out[outpos++]='\0';
  return; // Finished
 }

// NB: The texify function returns a different status code from all other function wrappers. No error is -1. Otherwise status points to position of error.

void wrapper_texify(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  int BracketLevel=0, outlen, offset=0;

  *status=-1;
  ppl_units_zero(output);
  outlen = 5 * (inlen+10); // Guess length of texified output to be no more than five times original length
  if (outlen > LSTR_LENGTH) outlen = LSTR_LENGTH;
  output->string = lt_malloc(outlen);
  if (output->string==NULL) { sprintf(errtext,"Out of memory."); *status=0; return; }
  output->string[0] = '\0';
  inlen--; // Make inlen point to last character
  if (inlen<=0) return; // Empty string gives empty output
  while ((in[0]!='\0')&&(in[0]<=' '))   { in++; offset++; inlen--; } // Strip spaces off front
  while ((inlen>0)&&(in[inlen]<=' '))   {                 inlen--; } // Strip spaces off back
  if (inlen<=0) return; // Empty string gives empty output
  if (inlen>ALGEBRA_MAXLENGTH) { sprintf(errtext, "Supplied expression is longer than maximum of %d characters.", ALGEBRA_MAXLENGTH); *status=0; return; }
  texify_generic(in, &inlen, output->string, 1, status, errtext, 1, &BracketLevel);
  if (*status>=0) (*status)+=offset;
  return;
 }

