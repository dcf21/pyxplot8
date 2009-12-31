// eps_plot_linedraw.c
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

#define _PPL_EPS_PLOT_LINEDRAW_C 1

#include <stdlib.h>
#include <stdio.h>

#include "ListTools/lt_memory.h"

#include "eps_comm.h"
#include "eps_plot_linedraw.h"

LineDrawHandle *LineDraw_Init (EPSComm *x, double clip_left, double clip_bottom, double clip_right, double clip_top)
 {
  LineDrawHandle *output;
  output = (LineDrawHandle *)lt_malloc(sizeof(LineDrawHandle));
  if (output==NULL) return NULL;
  output->x           = x;
  output->clip_left   = clip_left;
  output->clip_bottom = clip_bottom;
  output->clip_right  = clip_right;
  output->clip_top    = clip_top;
  output->x1set       = 0;
  output->x2set       = 0;
  return output;
 }

void LineDraw_Point(LineDrawHandle *ld, double x, double y, double z, int linetype, double linewidth, char *colstr)
 {
  if (!ld->x1set) { ld->x1set = 1; ld->lastx1=x; ld->lasty1=y; return; }
  return;
 }

void LineDraw_PenUp(LineDrawHandle *ld)
 {
  return;
 }

