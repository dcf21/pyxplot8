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
#include "eps_plot_threedimbuff.h"

void LineDraw_FindCrossingPoints(LineDrawHandle *ld, double x1, double y1, double x2, double y2, int *Inside1, int *Inside2, double *cx1, double *cy1, double *cx2, double *cy2, int *NCrossings)
 {
  double yleft, yright, xtop, xbottom;

  *Inside1 = (((x1 >= ld->clip_left) && (x1 <= ld->clip_right)) && ((y1 >= ld->clip_bottom) && (y1 <= ld->clip_top)));
  *Inside2 = (((x2 >= ld->clip_left) && (x2 <= ld->clip_right)) && ((y2 >= ld->clip_bottom) && (y2 <= ld->clip_top)));
  *cx1 = x1; *cy1 = y1; // If either point is inside canvas, set clip-region crossings to be the points themselves
  *cx2 = x2; *cy2 = y2;
  *NCrossings = 0;
  if ((*Inside1) && (*Inside2)) return; // If both points are inside canvas, don't need to find clip-region crossings

  // Check for clip-region crossings on left/right sides
  if (x2 != x1)
   {
    yleft   = y1 + (y2-y1)/(x2-x1) * (ld->clip_left   - x1); // left side
    if (((yleft  <=y1)&&(yleft  >=y2)) || ((yleft  >=y1)&&(yleft  <=y2)))
     {
      if      ((!*Inside1) && (( *Inside2) || (x1<x2))) { *cx1 = ld->clip_left  ; *cy1 = yleft  ; (*NCrossings)++; }
      else if ((!*Inside2) && (( *Inside1) || (x2<x1))) { *cx2 = ld->clip_left  ; *cy2 = yleft  ; (*NCrossings)++; }
     }
    yright  = y1 + (y2-y1)/(x2-x1) * (ld->clip_right  - x1); // right side
    if (((yright <=y1)&&(yright >=y2)) || ((yright >=y1)&&(yright <=y2)))
     {
      if      ((!*Inside1) && (( *Inside2) || (x1>x2))) { *cx1 = ld->clip_right ; *cy1 = yright ; (*NCrossings)++; }
      else if ((!*Inside2) && (( *Inside1) || (x2>x1))) { *cx2 = ld->clip_right ; *cy2 = yright ; (*NCrossings)++; }
     }
   }

  // Check for clip-region crossings on top/bottom sides
  if (y2 != y1) 
   {
    xbottom = x1 + (x2-x1)/(y2-y1) * (ld->clip_bottom - y1); // bottom side
    if (((xbottom<=x1)&&(xbottom>=x2)) || ((xbottom>=x1)&&(xbottom<=x2)))
     {
      if      ((!*Inside1) && (( *Inside2) || (y1<y2))) { *cy1 = ld->clip_bottom; *cx1 = xbottom; (*NCrossings)++; }
      else if ((!*Inside2) && (( *Inside1) || (y2<y1))) { *cy2 = ld->clip_bottom; *cx2 = xbottom; (*NCrossings)++; }
     }
    xtop    = x1 + (x2-x1)/(y2-y1) * (ld->clip_top    - y1); // top side
    if (((xtop   <=x1)&&(xtop   >=x2)) || ((xtop   >=x1)&&(xtop   <=x2)))
     {
      if      ((!*Inside1) && (( *Inside2) || (y1>y2))) { *cy1 = ld->clip_top   ; *cx1 = xtop   ; (*NCrossings)++; }
      else if ((!*Inside2) && (( *Inside1) || (y2>y1))) { *cy2 = ld->clip_top   ; *cx2 = xtop   ; (*NCrossings)++; }
     }
   }
  return;
 }

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
  output->x0set       = 0;
  output->x1set       = 0;
  return output;
 }

void LineDraw_Point(LineDrawHandle *ld, double x, double y, double z, int linetype, double linewidth, char *colstr)
 {
  int Inside1, Inside2, NCrossings;
  double cx1, cy1, cx2, cy2;

  if (!ld->x1set) { ld->x1set = 1; ld->x1=x; ld->y1=y; return; }

  LineDraw_FindCrossingPoints(ld, ld->x1, ld->y1, x, y, &Inside1, &Inside2, &cx1, &cy1, &cx2, &cy2, &NCrossings);

  if      ((!Inside1) && (!Inside2)) // Neither point on line segment is inside clip-region
   {
    if (NCrossings>=2) // Check that we haven't crossed clip region during the course of line segment
      ThreeDimBuffer_linesegment(ld->x, z, linetype, linewidth, colstr, cx1, cy1, cx1, cy1, cx2, cy2, 1, 0, 0.0);
    ld->x0set=0;
    ld->x1=x; ld->y1=y;
   }
  else if ((!Inside1) && ( Inside2)) // We have just entered clip region; previous point was outside
   {
    ThreeDimBuffer_linesegment(ld->x, z, linetype, linewidth, colstr, cx1, cy1, cx1, cy1, cx2, cy2, 1, 0, 0.0);
    if ((!ld->x0set)||(ld->x1!=x)||(ld->y1!=y)) ld->x0=ld->x1; ld->y0=ld->y1;
    ld->x0set=1;
    ld->x1=x; ld->y1=y;
   }
  else if (( Inside1) && (!Inside2)) // We have just left clip region; previous point was inside
   {
    if (ld->x0set) ThreeDimBuffer_linesegment(ld->x, z, linetype, linewidth, colstr, ld->x0, ld->y0, cx1, cy1, cx2, cy2, 0, 0, 0.0);
    else           ThreeDimBuffer_linesegment(ld->x, z, linetype, linewidth, colstr, cx1   , cy1   , cx1, cy1, cx2, cy2, 1, 0, 0.0);
    ld->x0set=0;
    ld->x1=x; ld->y1=y;
   }
  else // if (( Inside1) && ( Inside2)) // We are within the clip region
   {
    if (ld->x0set) ThreeDimBuffer_linesegment(ld->x, z, linetype, linewidth, colstr, ld->x0, ld->y0, cx1, cy1, cx2, cy2, 0, 0, 0.0);
    else           ThreeDimBuffer_linesegment(ld->x, z, linetype, linewidth, colstr, cx1   , cy1   , cx1, cy1, cx2, cy2, 1, 0, 0.0);
    if ((!ld->x0set)||(ld->x1!=x)||(ld->y1!=y)) ld->x0=ld->x1; ld->y0=ld->y1;
    ld->x0set=1;
    ld->x1=x; ld->y1=y;
   }
  return;
 }

void LineDraw_PenUp(LineDrawHandle *ld)
 {
  ThreeDimBuffer_linepenup(ld->x);
  ld->x0set=0;
  ld->x1set=0;
  return;
 }

