// ppl_histogram.c
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

#define _PPL_HISTOGRAM_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wordexp.h>
#include <glob.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_histogram.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Main entry point for the implementation of the histogram command
int directive_histogram(Dict *command)
 {
  return 0;
 }

