// asciidouble.h
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

#ifndef _PPL_ASCIIDOUBLE
#define _PPL_ASCIIDOUBLE 1

#include <stdio.h>

double GetFloat        (char *str);
void   file_readline   (FILE *file, char *output);
void   GetWord         (char *out, char *in, int max);
char  *NextWord        (char *in);
char  *FriendlyTimestring();
char  *StrStrip        (char *in);
char  *StrUpper        (char *in);
char  *StrLower        (char *in);
char  *StrUnderline    (char *in);
char  *StrRemoveCompleteLine(char *in);

#endif
