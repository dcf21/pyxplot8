// dcftime.h
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

#ifndef _DCFTIME_H
#define _DCFTIME_H 1

#include "ppl_units.h"

void dcftime_juliandate(value *in1, value *in2, value *in3, value *in4, value *in5, value *in6, value *output, int *status, char *errtext);
void dcftime_now(value *output, int *status, char *errtext);
void dcftime_year(value *in, value *output, int *status, char *errtext);
void dcftime_month(value *in, value *output, int *status, char *errtext);
void dcftime_day(value *in, value *output, int *status, char *errtext);
void dcftime_hour(value *in, value *output, int *status, char *errtext);
void dcftime_min(value *in, value *output, int *status, char *errtext);
void dcftime_sec(value *in, value *output, int *status, char *errtext);
void dcftime_moonphase(value *in, value *output, int *status, char *errtext);
void dcftimediff_years(value *in1, value *in2, value *output, int *status, char *errtext);
void dcftimediff_days(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcftimediff_hours(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcftimediff_minutes(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);
void dcftimediff_seconds(value *in1, value *in2, value *in3, value *output, int *status, char *errtext);

#endif
