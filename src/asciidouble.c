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

char *friendly_timestring()
 {
  time_t timenow;
  timenow = time(NULL);
  return( ctime(&timenow) );
 }

