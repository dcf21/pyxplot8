// lt_StringProc.c
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

// List-based string processing functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StringTools/asciidouble.h"

#include "lt_list.h"

/* StrSplit(): Split up a string into bits separated by whitespace */

List *StrSplit(char *in)
 {
  int pos, start, end;
  char *word;
  char *text_buffer;
  List *out;
  out  = ListInit();
  pos  = 0;
  text_buffer = (char *)malloc((strlen(in)+8)*sizeof(char));
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
  free(text_buffer);
  return out;
 }

