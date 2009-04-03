// asciidouble.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-8 Dominic Ford <coders@pyxplot.org.uk>
//               2008   Ross Church
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

#include "ppl_list.h"
#include "ppl_constants.h"

char temp_strproc_buffer[LSTR_LENGTH];


/* GETFLOAT(): This gets a float from a string */

double GetFloat(char *str)
 {
  double accumulator = 0;
  int decimals = 0;
  int past_decimal_point = 0;
  int negative = 0;

  if (*str == '-') { negative = 1; str++; }                     /* Deal with negatives */
  if (*str == '+') {               str++; }                     /* Deal with e.g. 1E+09 */

  while (((((int)*str)>47) && (((int)*str)<59)) || (*str == '.'))
   {
    if (*str == '.')
     {
      past_decimal_point = 1;
     } else {
      accumulator = ((10 * accumulator) + (((int)*str)-48));
      if (past_decimal_point == 1) decimals++;
     }
    str++;
   }

  while (decimals != 0)                                         /* Deals with decimals */
   {
    decimals--;
    accumulator /= 10;
   }

  if (negative == 1) accumulator *= -1;                         /* Deals with negatives */

  if ((*str == 'e') || (*str == 'E')) accumulator *= pow(10.0,GetFloat(str+1));       /* Deals with exponents */

  return(accumulator);
 }

/* FILE_READLINE(): This remarkably useful function forwards a file to the next newline */
/*                  Why is this not in stdio? */

void file_readline(FILE *file, char *output)
{
 char c = 'X';
 char *outputscan = output;

 while (((int)c != 10) && (!feof(file)))
  {
   fscanf(file,"%c",&c);
   if (((int)c)>31) *(outputscan++) = c;
  }
  *(outputscan++) = '\0';
}

/* GETWORD(): This returns the first word (terminated by any whitespace). Maximum <max> characters. */

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

/* NEXTWORD(): Fast forward over word, and return pointer to next word */

char *NextWord(char *in)
 {
  while ((*in <= ' ') && (*in != '\0')) in++; /* Fastforward over preceeding whitespace */
  while  (*in >  ' ')                   in++; /* Fastforward over one word */
  while ((*in <= ' ') && (*in != '\0')) in++; /* Fastforward over whitespace before next word */
  return(in); /* Return pointer to next word */
 }

/* FRIENDLY_TIMESTRING(): Returns pointer to time string */

char *FriendlyTimestring()
 {
  time_t timenow;
  timenow = time(NULL);
  return( ctime(&timenow) );
 }

/* STRSTRIP(): Strip whitespace from both ends of a string */

char *StrStrip(char *in)
 {
  char *out = temp_strproc_buffer;
  while ((*in <= ' ') && (*in > '\0')) in++;
  while (                (*in > '\0')) *(out++)=*(in++);
  out--;
  while ((out!=temp_strproc_buffer) && (*out <= ' ')) out--;
  *++out = '\0';
  return temp_strproc_buffer;
 }

/* STRUPPER(): Capitalise a string */

char *StrUpper(char *in)
 {
  char *out = temp_strproc_buffer;
  while (*in > 0)
   if ((*in >='a') && (*in <='z')) *out++ = *in++ +'A'-'a';
   else                            *out++ = *in++;
  *out = '\0';
  return temp_strproc_buffer;
 }

/* STRLOWER(): Lowercase a string */

char *StrLower(char *in)
 {
  char *out = temp_strproc_buffer;
  while (*in > 0)
   if ((*in >='A') && (*in <='Z')) *out++ = *in++ +'a'-'A';
   else                            *out++ = *in++;
  *out = '\0';
  return temp_strproc_buffer;
 }

/* STRUNDERLINE(): Underline a string */

char *StrUnderline(char *in)
 {
  char *out = temp_strproc_buffer;
  while (*in > 0) if (*in++ >= ' ') *out++='-';
  *out = '\0';
  return temp_strproc_buffer;
 }

/* STRREMOVECOMPLETELINE(): Removes a single complete line from a text buffer, if there is one */

char  *StrRemoveCompleteLine(char *in)
 {
  char *scan, *scan2, *out;
  scan = scan2 = in; out = temp_strproc_buffer;
  while ((*scan != '\0') && (*scan != '\x0D') && (*scan != '\x0A')) scan++; // Find first carriage-return in text buffer
  if (*scan != '\0') while (scan2<scan) *out++=*scan2++; // If one is found, copy up to this point into temporary string processing buffer
  *out = '\0';
  out = StrStrip(temp_strproc_buffer); // Strip it, and then this is what we will return

  if (*scan != '\0') // If we've taken a line out of the buffer, delete it from buffer
   {
    scan2 = in;
    while ((*scan == '\x0D') || (*scan == '\x0A')) scan++; // Forward over carriage return
    while (*scan != '\0') *scan2 = *scan;
    *scan2 = '\0';
   }
  return out;
 }

/* STRSLICE(): Take a slice out of a string */

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

/* STRSPLIT(): Split up a string into bits separated by whitespace */

List *StrSplit(char *in)
 {
  int pos, start, end;
  char *word;
  char text_buffer[LSTR_LENGTH];
  List *out;
  out  = ListInit();
  pos  = 0;
  while (in[pos] != '\0')
   {
    // Scan along to find the next word
    while ((in[pos] <= ' ') && (in[pos] > '\0')) pos++;
    start = pos;

    // Scan along to find the end of this word
    while ((in[pos] >  ' ') && (in[pos] > '\0')) pos++;
    end = pos;

    if (end>start)
     {
      word = StrSlice(in, text_buffer, start, end);
      ListAppendString(out, word);
     }
   }
  return out;
 }

