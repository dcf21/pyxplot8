// eps_eps.h
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

#ifndef _PPL_EPS_EPS_H
#define _PPL_EPS_EPS_H 1

#include "eps_comm.h"

void eps_eps_RenderEPS(EPSComm *x);
void eps_eps_ExtractBBox(FILE *in, double *bl, double *bb, double *br, double *bt, unsigned char *GotBox);

#endif

