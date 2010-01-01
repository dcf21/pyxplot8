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
#include "ppl_setting_types.h"

double eps_plot_axis_GetPosition(double xin, settings_axis *xa)
 {
  if (xa->log!=SW_BOOL_TRUE) return (xin - xa->MinFinal) / (xa->MaxFinal - xa->MinFinal); // Either linear...
  else                       return log(xin / xa->MinFinal) / log(xa->MaxFinal / xa->MinFinal); // ... or logarithmic
 }

void eps_plot_GetPosition(double *xpos, double *ypos, double *depth, unsigned char ThreeDim, double xin, double yin, double zin, settings_axis *xa, settings_axis *ya, settings_axis *za, settings_graph *sg, double origin_x, double origin_y, double width, double height, unsigned char AllowOffBounds)
 {
  // 3D plots
  if (ThreeDim)
   {
    double x,y,z,x2,y2,z2,x3,y3,z3;

    if (!AllowOffBounds)
     {
      if ((xin < xa->MinFinal) && (xin < xa->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
      if ((xin > xa->MinFinal) && (xin > xa->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
      if ((yin < ya->MinFinal) && (yin < ya->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
      if ((yin > ya->MinFinal) && (yin > ya->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
      if ((zin < za->MinFinal) && (zin < za->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
      if ((zin > za->MinFinal) && (zin > za->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
     }

    x = width  * eps_plot_axis_GetPosition(xin, xa);
    y = height * eps_plot_axis_GetPosition(yin, ya);
    z = width  * eps_plot_axis_GetPosition(zin, za);

    x2 = x*cos(sg->XYview.real) + y*sin(sg->XYview.real);
    y2 =-x*sin(sg->XYview.real) + y*cos(sg->XYview.real);
    z2 = z;

    x3 = x2;
    y3 = y2*cos(sg->XYview.real) + z2*sin(sg->XYview.real);
    z3 =-y2*sin(sg->XYview.real) + z2*cos(sg->XYview.real);

    *xpos  = origin_x + x3;
    *ypos  = origin_y + y3;
    *depth = z3;
    return;
   }

  // 2D gnomonic projections
  if (sg->projection == SW_PROJ_GNOM)
   {
    *xpos  = origin_x;
    *ypos  = origin_y;
    *depth = 0.0;
    return;
   }

  // We assume 2D flat projection
  if (!AllowOffBounds)
   {
    if ((xin < xa->MinFinal) && (xin < xa->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
    if ((xin > xa->MinFinal) && (xin > xa->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
    if ((yin < ya->MinFinal) && (yin < ya->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
    if ((yin > ya->MinFinal) && (yin > ya->MaxFinal)) { *xpos = *ypos = GSL_NAN; return; }
   }

  *xpos = origin_x + width  * eps_plot_axis_GetPosition(xin, xa);
  *ypos = origin_y + height * eps_plot_axis_GetPosition(yin, ya);
  *depth = 0.0;
  return;
 }

