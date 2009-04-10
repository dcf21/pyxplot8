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

/* file_readline(): This remarkably useful function forwards a file to the next newline */

void file_readline(FILE *file, char *output)
{
 char c = '\x07';
 char *outputscan = output;

 while (((int)c != 10) && (!feof(file)) && (!ferror(file)))
  {
   fscanf(file,"%c",&c);
   if (((int)c)>31) *(outputscan++) = c;
  }
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
  char *scan;
  int   pos;
  scan = out;
  pos  = 0;
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
    if ((in[i]=='\\') && (in[i+1]=='\\')) {i++; out[j]='\n'; LastSpace=j++; WhiteSpace=1; continue;} // Double-backslash implies a hard linebreak.
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

