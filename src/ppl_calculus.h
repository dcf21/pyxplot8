// ppl_calculus.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
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

#ifndef _PPL_CALCULUS_H
#define _PPL_CALCULUS_H 1

#include "ppl_units.h"

void Integrate    (char *expr, char *dummy, value *min  , value *max , value *out, int *errpos, char *errtext, int RecursionDepth);
void Differentiate(char *expr, char *dummy, value *point, value *step, value *out, int *errpos, char *errtext, int RecursionDepth);

#endif

