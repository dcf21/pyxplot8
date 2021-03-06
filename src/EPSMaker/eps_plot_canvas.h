// eps_plot_canvas.h
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

// Provides routines for mapping between coordinate positions on axes and
// positions on a postscript page. Outputs xpos and ypos are measured in
// postscript points.

#ifndef _PPL_EPS_PLOT_CANVAS_H
#define _PPL_EPS_PLOT_CANVAS_H 1

#include <stdlib.h>
#include <stdio.h>

#include "ppl_settings.h"

double eps_plot_axis_GetPosition(double xin, settings_axis *xa, int xrn, unsigned char AllowOffBounds);
double eps_plot_axis_InvGetPosition(double xin, settings_axis *xa);
int eps_plot_axis_InRange(settings_axis *xa, double xin);
void eps_plot_ThreeDimProject(double xap, double yap, double zap, settings_graph *sg, double origin_x, double origin_y, double width, double height, double zdepth, double *xpos, double *ypos, double *depth);
void eps_plot_GetPosition(double *xpos, double *ypos, double *depth, double *xap, double *yap, double *zap, double *theta_x, double *theta_y, double *theta_z, unsigned char ThreeDim, double xin, double yin, double zin, settings_axis *xa, settings_axis *ya, settings_axis *za, int xrn, int yrn, int zrn, settings_graph *sg, double origin_x, double origin_y, double width, double height, double zdepth, unsigned char AllowOffBounds);

#endif

