// eps_settings.h
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

#ifndef _PPL_EPS_SETTINGS_H
#define _PPL_EPS_SETTINGS_H 1

// Baseline values of settings used by the eps generation routines
#define EPS_BASE_DEFAULT_LINEWIDTH ( 0.566929       ) /* 0.2mm in TeX points */
#define EPS_BASE_ARROW_ANGLE       ( 45.0 *M_PI/180 )
#define EPS_BASE_ARROW_CONSTRICT   ( 0.2            )
#define EPS_BASE_ARROW_HEADSIZE    ( 6.0            )

// Copies of the values actually used by the eps generation routines, which may have been scaled relative to their baselines
#ifndef _PPL_EPS_SETTINGS_C
extern double M_TO_PS;
extern double EPS_DEFAULT_LINEWIDTH;
extern double EPS_ARROW_ANGLE;
extern double EPS_ARROW_CONSTRICT;
extern double EPS_ARROW_HEADSIZE;
#endif

#endif

