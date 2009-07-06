// asciidouble.c
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>

/* GetFloat(): This gets a float from a string */

double GetFloat(char *str, int *Nchars)
 {
  double accumulator = 0;
  int decimals = 0;
  int past_decimal_point = 0;
  int negative = 0;
  int pos = 0;
  int pos2= 0;

  if (str[pos] == '-') { negative = 1; pos++; }                     /* Deal with negatives */
  if (str[pos] == '+') {               pos++; }                     /* Deal with e.g. 1E+09 */

  while (((str[pos]>='0') && (str[pos]<='9')) || (str[pos] == '.'))
   {
    if (str[pos] == '.')
     {
      past_decimal_point = 1;
     } else {
      accumulator = ((10 * accumulator) + (((int)str[pos])-48));
      if (past_decimal_point == 1) decimals++;
     }
    pos++;
   }

  while (decimals != 0)                                         /* Deals with decimals */
   {
    decimals--;
    accumulator /= 10;
   }

  if (negative == 1) accumulator *= -1;                         /* Deals with negatives */

  if ((str[pos] == 'e') || (str[pos] == 'E')) accumulator *= pow(10.0,GetFloat(str+pos+1 , &pos2)); /* Deals with exponents */

  if (pos2   >     0) pos += (1+pos2); // Add on characters taken up by exponent, including one for the 'e' character.
  if (pos    ==    0) pos = -1; // Alert the user that this was a blank string!
  if (Nchars != NULL) *Nchars = pos;
  return(accumulator);
 }

/* NumericDisplay(): Displays a double in either %f or %e formats */

char *NumericDisplay(double in, int N, int SigFig)
 {
  static char format[16], outputA[128], outputB[128], outputC[128], outputD[128];
  double x, AccLevel;
  char *output;
  int DecimalLevel, DPmax, i, j, k;
  if      (N==0) output = outputA;
  else if (N==1) output = outputB;
  else if (N==2) output = outputC;
  else           output = outputD;
  if ((fabs(in) < 1e10) && (fabs(in) > 1e-3))
   {
    x = fabs(in);
    AccLevel = x*(1.0+pow(10,-SigFig));
    DPmax    = SigFig-log10(x);
    for (DecimalLevel=0; DecimalLevel<DPmax; DecimalLevel++) if ((x - ((floor(x*pow(10,DecimalLevel))/pow(10,DecimalLevel)) - x))<AccLevel) break;
    sprintf(format,"%%.%df",DecimalLevel);
    sprintf(output,format,in);
   }
  else
   {
    if (in==0)
     { sprintf(output,"0"); }
    else
     {
      x  = fabs(in);
      x /= pow(10,(int)log10(x));
      AccLevel = x*(1.0+pow(10,-SigFig));
      for (DecimalLevel=0; DecimalLevel<SigFig; DecimalLevel++) if ((x - ((floor(x*pow(10,DecimalLevel))/pow(10,DecimalLevel)) - x))<AccLevel) break;
      sprintf(format,"%%.%de",DecimalLevel);
      sprintf(output,format,in);
     }
   }
  for (i=0; ((output[i]!='\0')&&(output[i]!='.')); i++); // If we have trailing decimal zeros, get rid of them
  if (output[i]!='.') return output;
  for (j=i+1; isdigit(output[j]); j++);
  if (i==j) return output;
  for (k=j-1; output[k]=='0'; k--);
  if (k==i) k--;
  k++;
  if (k==j) return output;
  strcpy(output+k , output+j);
  return output;
 }

unsigned char DblEqual(double a, double b)
 {
  if ( (fabs(a) < 1e-100) && (fabs(b) < 1e-100) ) return 1;
  if ( (fabs(a-b) < fabs(1e-7*a)) && (fabs(a-b) < fabs(1e-7*b)) ) return 1;
  return 0;
 }

/* file_readline(): This remarkably useful function forwards a file to the next newline */

void file_readline(FILE *file, char *output)
{
 char c = '\x07';
 char *outputscan = output;

 while (((int)c != '\n') && (!feof(file)) && (!ferror(file)))
   if ((fscanf(file,"%c",&c)>=0) && (((int)c)>31)) *(outputscan++) = c;
  *(outputscan++) = '\0';
}

/* GetWord(): This returns the first word (terminated by any whitespace). Maximum <max> characters. */

void GetWord(char *out, char *in, int max)
 {
  int count = 0;
  while ((*in <= ' ') && (*in != '\0')) in++; /* Fastforward over preceeding whitespace */
  while ((*in >  ' ') && (count < (max-1)))
   {
    *(out++) = *(in++);
    count++;
   }
  *out = '\0'; /* Terminate output */
 }

/* NextWord(): Fast forward over word, and return pointer to next word */

char *NextWord(char *in)
 {
  while ((*in <= ' ') && (*in != '\0')) in++; /* Fastforward over preceeding whitespace */
  while  (*in >  ' ')                   in++; /* Fastforward over one word */
  while ((*in <= ' ') && (*in != '\0')) in++; /* Fastforward over whitespace before next word */
  return(in); /* Return pointer to next word */
 }

/* FriendlyTimestring(): Returns pointer to time string */

char *FriendlyTimestring()
 {
  time_t timenow;
  timenow = time(NULL);
  return( ctime(&timenow) );
 }

/* StrStrip(): Strip whitespace from both ends of a string */

char *StrStrip(char *in, char *out)
 {
  char *scan = out;
  while ((*in <= ' ') && (*in > '\0')) in++;
  while (                (*in > '\0')) *(scan++)=*(in++);
  scan--;
  while ((scan>out) && (*scan <= ' ')) scan--;
  *++scan = '\0';
  return out;
 }

/* StrUpper(): Capitalise a string */

char *StrUpper(char *in, char *out)
 {
  char *scan = out;
  while (*in > 0)
   if ((*in >='a') && (*in <='z')) *scan++ = *in++ +'A'-'a';
   else                            *scan++ = *in++;
  *scan = '\0';
  return out;
 }

/* StrLower(): Lowercase a string */

char *StrLower(char *in, char *out)
 {
  char *scan = out;
  while (*in > 0)
   if ((*in >='A') && (*in <='Z')) *scan++ = *in++ +'a'-'A';
   else                            *scan++ = *in++;
  *scan = '\0';
  return out;
 }

/* StrUnderline(): Underline a string */

char *StrUnderline(char *in, char *out)
 {
  char *scan = out;
  while (*in > 0) if (*in++ >= ' ') *scan++='-';
  *scan = '\0';
  return out;
 }

/* StrRemoveCompleteLine(): Removes a single complete line from a text buffer, if there is one */

char  *StrRemoveCompleteLine(char *in, char *out)
 {
  char *scan, *scan2, *scanout;
  scan = scan2 = in;
  scanout      = out;
  while ((*scan != '\0') && (*scan != '\x0D') && (*scan != '\x0A')) scan++; // Find first carriage-return in text buffer
  if (*scan != '\0') while (scan2<scan) *scanout++=*scan2++; // If one is found, copy up to this point into temporary string processing buffer
  *scanout = '\0';
  StrStrip(out, out); // Strip it, and then this is what we will return

  if (*scan != '\0') // If we've taken a line out of the buffer, delete it from buffer
   {
    scan2 = in;
    while ((*scan == '\x0D') || (*scan == '\x0A')) scan++; // Forward over carriage return
    while (*scan != '\0') *scan2++ = *scan++;
    *scan2 = '\0';
   }
  return out;
 }

/* StrSlice(): Take a slice out of a string */

char *StrSlice(char *in, char *out, int start, int end)
 {
  char *scan = out;
  int   pos  = 0;
  while ((pos<start) && (in[pos]!='\0')) pos++;
  while ((pos<end  ) && (in[pos]!='\0')) *(scan++) = in[pos++];
  *scan = '\0';
  return out;
 }

/* StrCommaSeparatedListScan(): Split up a comma-separated list into individual values */

char *StrCommaSeparatedListScan(char **inscan, char *out)
 {
  char *outscan = out;
  while ((**inscan != '\0') && (**inscan != ',')) *(outscan++) = *((*inscan)++);
  if (**inscan == ',') (*inscan)++; // Fastforward over comma character
  *outscan = '\0';
  StrStrip(out,out);
  return out;
 }

/* StrAutocomplete(): Test whether a candidate string matches the beginning of test string, and is a least N characters long */

int StrAutocomplete(char *candidate, char *test, int Nmin)
 {
  int IsAlphanumeric = 1; // Alphanumeric test strings can be terminated by punctuation; others must have spaces after them
  int i,j;

  if ((candidate==NULL) || (test==NULL)) return -1;

  for (i=0; test[i]!='\0'; i++) if ((test[i]>' ') && (isalnum(test[i])==0) && (test[i]!='_')) {IsAlphanumeric=0; break;}
  for (j=0; ((candidate[j]>'\0') && (candidate[j]<=' ')); j++); // Fastforward over whitespace at beginning of candidate

  for (i=0; 1; i++,j++)
   if (test[i]!='\0')
    {
     if (toupper(test[i]) != toupper(candidate[j])) // Candidate string has deviated from test string
      {
       if ( (candidate[j]<=' ') || ((IsAlphanumeric==1) && (isalnum(candidate[j])==0) && (candidate[j]!='_'))) // If this is with a space...
        {
         if (i>=Nmin) return j; // ... it's okay so long as we've already passed enough characters
         return -1;
        }
       else return -1;
      }
    } else { // We've hit the end of the test string
     if ( (candidate[j]<=' ') || ((IsAlphanumeric==1) && (isalnum(candidate[j])==0) && (candidate[j]!='_'))) return j; // If candidate string has a space, it's okay
     else return -1; // Otherwise it has unexpected characters on the end
    }
 }

/* StrWordWrap(): Word wrap a piece of text to a certain width */

void StrWordWrap(char *in, char *out, int width)
 {
  int WhiteSpace =  1;
  int LastSpace  = -1;
  int LineStart  =  0;
  int LineFeeds  =  0; // If we meet > 1 linefeed during a period of whitespace, it marks the beginning of a new paragraph
  int i,j;
  for (i=0,j=0 ; in[i]!='\0' ; i++)
   {
    if ((WhiteSpace==1) && (in[i]<=' ')) // Once we've encountered one space, ignore any further whitespaceness
     {
      if (j==0) j++; // If we open document with a new paragraph, we haven't already put down a space character to overwrite
      if ((in[i]=='\n') && (++LineFeeds==2)) { out[j-1]='\n'; out[j]='\n'; LineStart=j++; LastSpace=-1; } // Two linefeeds in a period of whitespace means a new paragraph
      continue;
     }
    if ((WhiteSpace==0) && (in[i]<=' ')) // First whitespace character after a string of letters
     {
      if (in[i]=='\n') LineFeeds=1;
      out[j]=' '; LastSpace=j++; WhiteSpace=1;
      continue;
     }
    if ((in[i]=='\\') && (in[i+1]=='\\')) {i++; out[j]='\n'; LineStart=j++; LastSpace=-1; WhiteSpace=1; continue;} // Double-backslash implies a hard linebreak.
    if (in[i]=='#') {out[j++]=' '; WhiteSpace=1; continue;} // A hash character implies a hard space character, used to tabulate data
    WhiteSpace=0; LineFeeds=0;
    if (((j-LineStart) > width) && (LastSpace != -1)) { out[LastSpace]='\n'; LineStart=LastSpace; LastSpace=-1; } // If line is too line, insert a linebreak
    if (strncmp(in+i, "\\lab"    , 4)==0) {i+=3; out[j++]='<'; continue;} // Macros for left-angle-brackets, etc.
    if (strncmp(in+i, "\\rab"    , 4)==0) {i+=3; out[j++]='>'; continue;}
    if (strncmp(in+i, "\\VERSION", 8)==0) {i+=7; strcpy(out+j,VERSION); j+=strlen(out+j); continue;}
    if (strncmp(in+i, "\\DATE"   , 5)==0) {i+=4; strcpy(out+j,DATE   ); j+=strlen(out+j); continue;}
    out[j++] = in[i];
   }
  out[j]='\0';
  return;
 }

/* StrBacketMatch(): Find a closing bracket to match an opening bracket, and optionally return a list of all comma positions */
/*                   'in' should point to the opening bracket character for which we are looking for the closing partner */

void StrBracketMatch(char *in, int *CommaPositions, int *Nargs, int *ClosingBracketPos, int MaxCommaPoses)
 {
  int  BracketLevel = 0;
  int  inpos        = 0;
  int  commapos     = 0;
  char QuoteType    = '\0';

  for ( ; in[inpos] != '\0'; inpos++)
   {
    if (QuoteType != '\0') // Do not pay attention to brackets inside quoted strings
     {
      if ((in[inpos]==QuoteType) && (in[inpos-1]!='\\')) QuoteType='\0';
      continue;
     }
    else if ((in[inpos]=='\'') || (in[inpos]=='\"')) QuoteType     = in[inpos]; // Entering a quoted string
    else if  (in[inpos]=='(')  // Entering a nested level of brackets
     {
      BracketLevel += 1;
      if (BracketLevel == 1)
       if ((CommaPositions != NULL) && ((MaxCommaPoses < 0) || (commapos < MaxCommaPoses))) *(CommaPositions+(commapos++)) = inpos; // Put ( on comma list
     }
    else if  (in[inpos]==')')  // Leaving a nested level of brackets
     {
      BracketLevel -= 1;
      if (BracketLevel == 0) break;
     }
    else if ((in[inpos]==',') && (BracketLevel==1)) // Found a new comma-separated item
     {
      if ((CommaPositions != NULL) && ((MaxCommaPoses < 0) || (commapos < MaxCommaPoses))) *(CommaPositions+(commapos++)) = inpos; // Put , on comma list
     }
   }

  if (in[inpos] == '\0')
   {
    if (Nargs             != NULL) *Nargs             = -1;
    if (ClosingBracketPos != NULL) *ClosingBracketPos = -1;
    return;
   } else {
    if ((CommaPositions != NULL) && ((MaxCommaPoses < 0) || (commapos < MaxCommaPoses))) *(CommaPositions+(commapos++)) = inpos; // Put ) on comma list
    if (Nargs             != NULL) *Nargs             = commapos-1; // There are N+1 arguments between N commas, but we've also counted ( and ).
    if (ClosingBracketPos != NULL) *ClosingBracketPos = inpos;
    return;
   }
 }

/* StrCmpNoCase(): A case-insensitive version of the standard strcmp() function */

int StrCmpNoCase(char *a, char *b)
 {
  char aU, bU;
  while (1)
   {
    if ((*a == '\0')&&(*b == '\0')) return 0;
    if (*a == *b) {a++; b++; continue;}
    if ((*a>='a')&&(*a<='z')) aU=*a-'a'+'A'; else aU=*a;
    if ((*b>='a')&&(*b<='z')) bU=*b-'a'+'A'; else bU=*b;
    if (aU==bU) {a++; b++; continue;}
    if (aU< bU) return -1;
    return 1;
   }
 }

/* StrEscapify(): Inserts escape characters into strings before quote characters */

char *StrEscapify(char *in, char *out)
 {
  char *scanin  = in;
  char *scanout = out;
  *scanout++ = '\"';
  while (*scanin != '\0')
   {
    if ((*scanin=='\'')||(*scanin=='\"')||(*scanin=='\\')) *(scanout++) = '\\';
    *(scanout++) = *(scanin++);
   }
  *scanout++ = '\"';
  *scanout++ = '\0';
  return out;
 }

