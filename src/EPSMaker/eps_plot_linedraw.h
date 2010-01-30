// eps_plot_linedraw.h
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

#ifndef _PPL_EPS_PLOT_LINEDRAW_H
#define _PPL_EPS_PLOT_LINEDRAW_H 1

#include "eps_comm.h"

#include "ppl_settings.h"

typedef struct LineDrawHandle {
 EPSComm *x;
 settings_graph *sg;
 settings_axis *xa, *ya, *za;
 int xrn, yrn, zrn;
 unsigned char ThreeDim;
 double origin_x, origin_y, width, height, zdepth;
 unsigned char x0set, x1set;
 double x0, y0, xpo0, ypo0, zpo0;
 double x1  , y1  , z1  ;
 double xpo1, ypo1, zpo1;
 double xap1, yap1, zap1;
 } LineDrawHandle;

LineDrawHandle *LineDraw_Init (EPSComm *x, settings_axis *xa, settings_axis *ya, settings_axis *za, int xrn, int yrn, int zrn, settings_graph *sg, unsigned char ThreeDim, double origin_x, double origin_y, double width, double height, double zdepth);
void LineDraw_Point(LineDrawHandle *ld, double x, double y, double z, double x_offset, double y_offset, double z_offset, double x_perpoffset, double y_perpoffset, double z_perpoffset, int linetype, double linewidth, char *colstr);
void LineDraw_PenUp(LineDrawHandle *ld);

#endif

