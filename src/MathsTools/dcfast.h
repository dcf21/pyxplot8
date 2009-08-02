// dcfast.h
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

#ifndef _DCFAST_H
#define _DCFAST_H 1

#include "ppl_units.h"

void dcfast_Lcdm_age(value *in1, value *in2, value *in3, int *status, char *errtext);
void dcfast_Lcdm_angscale(value *in1, value *in2, value *in3, value *in4, int *status, char *errtext);
void dcfast_Lcdm_DA(value *in1, value *in2, value *in3, value *in4, int *status, char *errtext);
void dcfast_Lcdm_DL(value *in1, value *in2, value *in3, value *in4, int *status, char *errtext);
void dcfast_Lcdm_DM(value *in1, value *in2, value *in3, value *in4, int *status, char *errtext);
void dcfast_Lcdm_t(value *in1, value *in2, value *in3, value *in4, int *status, char *errtext);
void dcfast_Lcdm_z(value *in1, value *in2, value *in3, value *in4, int *status, char *errtext);
void dcfast_moonphase(value *in, value *output, int *status, char *errtext);

#endif

