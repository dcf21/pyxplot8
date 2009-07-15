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

double GetFloat                 (const char *str, int *Nchars);
int    ValidFloat               (const char *str, int *end);
char  *NumericDisplay           (double in, int N, int SigFig);
unsigned char DblEqual          (double a, double b);
void   file_readline            (FILE *file, char *output);
void   GetWord                  (char *out, const char *in, int max);
char  *NextWord                 (char *in);
char  *FriendlyTimestring       ();
char  *StrStrip                 (const char *in, char *out);
char  *StrUpper                 (const char *in, char *out);
char  *StrLower                 (const char *in, char *out);
char  *StrUnderline             (const char *in, char *out);
char  *StrRemoveCompleteLine    (char *in, char *out);
char  *StrSlice                 (const char *in, char *out, int start, int end);
char  *StrCommaSeparatedListScan(char **inscan, char *out);
int    StrAutocomplete          (const char *candidate, char *test, int Nmin);
void   StrWordWrap              (const char *in, char *out, int width);
void   StrBracketMatch          (const char *in, int *CommaPositions, int *Nargs, int *ClosingBracketPos, int MaxCommaPoses);
int    StrCmpNoCase             (const char *a, const char *b);
char  *StrEscapify              (const char *in, char *out);
int    StrWildcardTest          (const char *test, const char *wildcard);
#endif

