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

double eps_plot_axis_GetPosition(double xin, settings_axis *xa, int xrn)
 {
  int imin, imax, i;
  if (xa->AxisLinearInterpolation != NULL) // Axis is linearly interpolated
   {
    imin = xa->AxisTurnings[xrn  ];
    imax = xa->AxisTurnings[xrn+1];
    for (i=imin; i<imax; i++)
     {
      if (   ((xa->AxisLinearInterpolation[i] < xin) && (xa->AxisLinearInterpolation[i+1] >= xin))
          || ((xa->AxisLinearInterpolation[i] > xin) && (xa->AxisLinearInterpolation[i+1] <= xin)) )
       return (i + (xin-xa->AxisLinearInterpolation[i])/(xa->AxisLinearInterpolation[i+1]-xa->AxisLinearInterpolation[i])) / (AXISLINEARINTERPOLATION_NPOINTS-1);
     }
   }
  if (xa->log!=SW_BOOL_TRUE) return (xin - xa->MinFinal) / (xa->MaxFinal - xa->MinFinal); // Either linear...
  else                       return log(xin / xa->MinFinal) / log(xa->MaxFinal / xa->MinFinal); // ... or logarithmic
 }

double eps_plot_axis_InvGetPosition(double xin, settings_axis *xa)
 {
  if (xa->AxisLinearInterpolation != NULL) // Axis is linearly interpolated
   {
    int    i = floor(xin * (AXISLINEARINTERPOLATION_NPOINTS-1));
    double x = xin * (AXISLINEARINTERPOLATION_NPOINTS-1) - i;
    if (i>=AXISLINEARINTERPOLATION_NPOINTS-1) return xa->AxisLinearInterpolation[AXISLINEARINTERPOLATION_NPOINTS-1];
    return xa->AxisLinearInterpolation[i]*(1-x) + xa->AxisLinearInterpolation[i+1]*x;
   }
  if (xa->log!=SW_BOOL_TRUE) return xa->MinFinal + xin * (xa->MaxFinal - xa->MinFinal); // Either linear...
  else                       return xa->MinFinal * pow(xa->MaxFinal / xa->MinFinal , xin); // ... or logarithmic
 }

void eps_plot_GetPosition(double *xpos, double *ypos, double *depth, unsigned char ThreeDim, double xin, double yin, double zin, settings_axis *xa, settings_axis *ya, settings_axis *za, int xrn, int yrn, int zrn, settings_graph *sg, double origin_x, double origin_y, double width, double height, unsigned char AllowOffBounds)
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

    x = width  * (eps_plot_axis_GetPosition(xin, xa, xrn) - 0.5);
    y = height * (eps_plot_axis_GetPosition(yin, ya, yrn) - 0.5);
    z = width  * (eps_plot_axis_GetPosition(zin, za, zrn) - 0.5);

    x2 = x*cos(sg->XYview.real) + y*sin(sg->XYview.real);
    y2 =-x*sin(sg->XYview.real) + y*cos(sg->XYview.real);
    z2 = z;

    x3 = x2;
    y3 = y2*cos(sg->YZview.real) + z2*sin(sg->YZview.real);
    z3 =-y2*sin(sg->YZview.real) + z2*cos(sg->YZview.real);

    *xpos  = origin_x + x3 + width/2.0;
    *ypos  = origin_y + y3 + height/2.0;
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

  *xpos = origin_x + width  * eps_plot_axis_GetPosition(xin, xa, xrn);
  *ypos = origin_y + height * eps_plot_axis_GetPosition(yin, ya, yrn);
  *depth = 0.0;
  return;
 }

