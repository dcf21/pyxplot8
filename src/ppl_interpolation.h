// ppl_interpolation.h
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

#ifndef _PPL_INTERPOLATION_H
#define _PPL_INTERPOLATION_H 1

#include "ListTools/lt_dict.h"

#include "ppl_units.h"
#include "ppl_userspace.h"

#define INTERP_AKIMA    23001
#define INTERP_LINEAR   23002
#define INTERP_LOGLIN   23003
#define INTERP_POLYN    23004
#define INTERP_SPLINE   23005
#define INTERP_STEPWISE 23006
#define INTERP_2D       23007
#define INTERP_BMPR     23008
#define INTERP_BMPG     23009
#define INTERP_BMPB     23010

int  directive_interpolate(Dict *command, int mode);
void ppl_spline_evaluate(char *FuncName, SplineDescriptor *desc, value *in, value *out, int *status, char *errout);
void ppl_interp2d_evaluate(const char *FuncName, SplineDescriptor *desc, const value *in1, const value *in2, const unsigned char bmp, value *out, int *status, char *errout);

#endif

