// dcftime.h
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

#ifndef _DCFTIME_H
#define _DCFTIME_H 1

#include "ppl_units.h"

void dcftime_juliandate(value *in1, value *in2, value *in3, value *in4, value *in5, value *in6, value *output, int *status, char *errtext);
void dcftime_now(value *output, int *status, char *errtext);
void dcftime_year(value *in, value *output, int *status, char *errtext);
void dcftime_monthnum(value *in, value *output, int *status, char *errtext);
void dcftime_monthname(value *in, value *in2, value *output, int *status, char *errtext);
void dcftime_daymonth(value *in, value *output, int *status, char *errtext);
void dcftime_dayweeknum(value *in, value *output, int *status, char *errtext);
void dcftime_dayweekname(value *in, value *in2, value *output, int *status, char *errtext);
void dcftime_hour(value *in, value *output, int *status, char *errtext);
void dcftime_min(value *in, value *output, int *status, char *errtext);
void dcftime_sec(value *in, value *output, int *status, char *errtext);
void dcftime_string(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext);
void dcftime_diff(value *in1, value *in2, value *output, int *status, char *errtext);
void dcftime_diff_string(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext);
void dcftime_ordinal(value *in, value *output, int *status, char *errtext);

#endif
