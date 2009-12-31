// eps_plot_threedimbuff.c
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

#define _PPL_EPS_PLOT_THREEDIMBUFF_C 1

#include <stdlib.h>
#include <stdio.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "eps_plot_threedimbuff.h"

unsigned char ThreeDimBuffer_ACTIVE        = 0;
static long   ThreeDimBuffer_LineSegmentID = 0;
static List  *ThreeDimBuffer_buffer        = NULL;

void ThreeDimBuffer_Reset()
 {
  ThreeDimBuffer_ACTIVE        = 0;
  ThreeDimBuffer_LineSegmentID = 0;
  ThreeDimBuffer_buffer        = NULL;
  return;
 }

int ThreeDimBuffer_Activate()
 {
  ThreeDimBuffer_Deactivate();
  ThreeDimBuffer_ACTIVE        = 1;
  ThreeDimBuffer_LineSegmentID = 0;
  ThreeDimBuffer_buffer        = ListInit();
  return (ThreeDimBuffer_buffer != NULL);
 }

int ThreeDimBuffer_Deactivate()
 {
  if (!ThreeDimBuffer_ACTIVE) return 0;
  ThreeDimBuffer_Reset();
  return 0;
 }

int ThreeDimBuffer_writeps(double z, int linetype, double linewidth, double pointsize, char *colstr, char *psfrag)
 {
  return 0;
 }

int ThreeDimBuffer_linesegment(double z, int linetype, double linewidth, char *colstr, double x0, double y0, double x1, double y1, double x2, double y2)
 {
  return 0;
 }

