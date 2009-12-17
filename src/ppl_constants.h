// ppl_constants.h
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

#ifndef _PPL_CONSTANTS_H
#define _PPL_CONSTANTS_H 1

#define MULTIPLOT_MAXINDEX 32768

#define PALETTE_LENGTH 512

#define MAX_PLOTSTYLES 128 // The maximum number of plot styles (e.g. plot sin(x) with style 23) which are be defined. Similar to 'with linestyle 23' in gnuplot

#define MAX_AXES 128

#define MAX_STR_FORMAT_ITEMS 256 // The maximum number of arguments passed to the %(....) string substitution operator

#define ALGEBRA_MAXLENGTH 4096 // The maximum length of an algebraic expression
#define ALGEBRA_MAXITEMS   128 // The number of calculated quantities we can store
#define MAX_RECURSION_DEPTH 50 // The maximum recursion depth

#define DUMMYVAR_MAXLEN 16 // Maximum number of characters in a dummy variable name for integration / differentiation

#define EQNSOLVE_MAXDIMS 16 // Maximum number of via variables when equation solving / fitting

#define OPLIST_LEN 15

#endif

