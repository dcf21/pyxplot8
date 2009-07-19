// ppl_texify.h
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

#ifndef _PPL_TEXIFY_H
#define _PPL_TEXIFY_H 1

#include "ppl_units.h"

int  texify_MakeGreek   (const char *in, char *out);

void texify_generic     (char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth, int *BracketLevel);
void texify_quotedstring(char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth, int *BracketLevel);
void texify_algebra     (char *in, int *end, char *out, int EvalStrings, int *status, char *errtext, int RecursionDepth, int *BracketLevel);

void wrapper_texify     (char *in, int inlen, value *output, int *status, char *errtext);

#endif
