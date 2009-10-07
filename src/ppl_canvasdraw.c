// ppl_canvasdraw.c
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

#define _PPL_CANVASDRAW_C 1

#include <stdlib.h>
#include <stdio.h>

#include "ppl_canvasitems.h"

// Table of the functions we call for each phase of the canvas drawing process for different object types

void canvas_draw(unsigned char *unsuccessful_ops)
 {
  int i;
  for (i=0;i<MULTIPLOT_MAXINDEX; i++) unsuccessful_ops[i]=0; // By default, all operations are successful

  // for () if type==plot   ReadAccessibleData
  // for () if type==plot   DecideAxisRanges
  // for () if type==plot   LinkedAxesPropagate
  // for () if type==plot   SampleFunctions
  // for ()                 YieldUpText
  // cd tempdir
  // Call LaTeX
  // Open temporary text buffer with tmpfile()
  // for ()                 RenderEPS( &bb_tl, ..., tmpfile); MergeBoundingBoxes
  // Open EPS output file
  // Write EPS header
  // Copy tmpfile --> output
  // Close tmpfile
  // Write EPS footer
  // Close EPS file
  // Convert EPS output as required
  // cd cwd

  return;
 }

