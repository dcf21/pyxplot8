// eps_settings.c
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

// This file contains various numerical constants which are used by the eps
// generation routines

#define _PPL_EPS_SETTINGS_C 1

#include <math.h>

#include <gsl/gsl_const_mksa.h>

#include "eps_settings.h"

// Constant to convert between millimetres and 72nds of an inch
double M_TO_PS = 1.0 / (GSL_CONST_MKSA_INCH / 72.0);

double EPS_DEFAULT_LINEWIDTH = 1.0 * EPS_BASE_DEFAULT_LINEWIDTH;
double EPS_ARROW_ANGLE       = 1.0 * EPS_BASE_ARROW_ANGLE;
double EPS_ARROW_CONSTRICT   = 1.0 * EPS_BASE_ARROW_CONSTRICT;
double EPS_ARROW_HEADSIZE    = 1.0 * EPS_BASE_ARROW_HEADSIZE;

