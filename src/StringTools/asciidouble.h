// asciidouble.h
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

#ifndef _ASCIIDOUBLE_H
#define _ASCIIDOUBLE_H 1

#include <stdio.h>

double GetFloat                 (char *str, int *Nchars);
char  *NumericDisplay           (double in, int N);
unsigned char DblEqual          (double a, double b);
void   file_readline            (FILE *file, char *output);
void   GetWord                  (char *out, char *in, int max);
char  *NextWord                 (char *in);
char  *FriendlyTimestring       ();
char  *StrStrip                 (char *in, char *out);
char  *StrUpper                 (char *in, char *out);
char  *StrLower                 (char *in, char *out);
char  *StrUnderline             (char *in, char *out);
char  *StrRemoveCompleteLine    (char *in, char *out);
char  *StrSlice                 (char *in, char *out, int start, int end);
char  *StrCommaSeparatedListScan(char **inscan, char *out);
int    StrAutocomplete          (char *candidate, char *test, int Nmin);
void   StrWordWrap              (char *in, char *out, int width);
void   StrBracketMatch          (char *in, int *CommaPositions, int *Nargs, int *ClosingBracketPos, int MaxCommaPoses);
int    StrCmpNoCase             (char *a, char *b);
char  *StrEscapify              (char *in, char *out);
#endif

