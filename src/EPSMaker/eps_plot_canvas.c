// eps_plot_canvas.c
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

// Provides routines for mapping between coordinate positions on axes and
// positions on a postscript page. Outputs xpos and ypos are measured in
// postscript points.

#define _PPL_EPS_PLOT_CANVAS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <gsl/gsl_math.h>

#include "ppl_settings.h"

void eps_plot_GetPosition(double *xpos, double *ypos, unsigned char ThreeDim, double xin, double yin, double zin, settings_axis *xa, settings_axis *ya, settings_axis *za, settings_graph *sg, double origin_x, double origin_y, double width, double height)
 {
  // Assumes 2D flat linear projection
  if ((xin < xa->MinFinal) && (xin < xa->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
  if ((xin > xa->MinFinal) && (xin > xa->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
  if ((yin < ya->MinFinal) && (yin < ya->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
  if ((yin > ya->MinFinal) && (yin > ya->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }

  *xpos = origin_x + width  * (xin - xa->MinFinal) / (xa->MaxFinal - xa->MinFinal);
  *ypos = origin_y + height * (yin - ya->MinFinal) / (ya->MaxFinal - ya->MinFinal);

  return;
 }

