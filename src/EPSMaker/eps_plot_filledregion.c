// eps_plot_filledregion.c
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

#define _PPL_EPS_PLOT_FILLEDREGION_C 1

#include <stdlib.h>
#include <stdio.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "eps_comm.h"

#include "eps_plot_canvas.h"
#include "eps_plot_filledregion.h"
#include "eps_plot_linedraw.h"
#include "eps_settings.h"

FilledRegionHandle *FilledRegion_Init (EPSComm *x, settings_axis *xa, settings_axis *ya, settings_axis *za, int xrn, int yrn, int zrn, settings_graph *sg, unsigned char ThreeDim, double origin_x, double origin_y, double width, double height, double zdepth)
 {
  FilledRegionHandle *output;
  output = (FilledRegionHandle *)lt_malloc(sizeof(FilledRegionHandle));
  if (output==NULL) return NULL;
  output->x           = x;
  output->xa          = xa;
  output->ya          = ya;
  output->za          = za;
  output->xrn         = xrn;
  output->yrn         = yrn;
  output->zrn         = zrn;
  output->sg          = sg;
  output->ThreeDim    = ThreeDim;
  output->origin_x    = origin_x;
  output->origin_y    = origin_y;
  output->width       = width;
  output->height      = height;
  output->zdepth      = zdepth;
  output->Naxiscrossings = 0;
  output->first       = 1;
  output->points      = ListInit();
  return output;
 }

void FilledRegion_Point(FilledRegionHandle *fr, double x, double y)
 {
  FilledRegionPoint p;
  double xpos, ypos, depth, xap, yap, zap;
  unsigned char f1, f2;
  int NCrossings, i1, i2;
  double cx1,cy1,cz1,cx2,cy2,cz2;

  // Work out where (x,y) in coordinate space lies on the canvas
  eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, NULL, NULL, NULL, fr->ThreeDim, x, y, 0, fr->xa, fr->ya, fr->za, fr->xrn, fr->yrn, fr->zrn, fr->sg, fr->origin_x, fr->origin_y, fr->width, fr->height, fr->zdepth, 1);

  // Add this to a list of points describing the outline of the region which we are going to fill
  p.inside = ((xap>=0)&&(xap<=1)&&(yap>=0)&&(yap<=1)&&(zap>=0)&&(zap<=1));
  p.x      = xpos;
  p.y      = ypos;
  p.xap    = xap;
  p.yap    = yap;

  ListAppendPtrCpy(fr->points, (void *)&p, sizeof(FilledRegionPoint), DATATYPE_VOID);

  // Count the number of times which the path crosses the boundary of the clip region
  if (!fr->first)
   {
    LineDraw_FindCrossingPoints(fr->lastx,fr->lasty,0,fr->lastxap,fr->lastyap,0.5,
                                xpos,ypos,0,xap,yap,0.5,&i1,&i2,&cx1,&cy1,&cz1,&cx2,&cy2,&cz2,&f1,&f2,&NCrossings);
    fr->Naxiscrossings += NCrossings;
   }
  fr->lastx = xpos; fr->lasty = ypos; fr->lastxap = xap; fr->lastyap = yap;
  fr->first = 0;
  return;
 }

int frac_sorter(const void *av, const void *bv)
 {
  const FilledRegionAxisCrossing *a = (const FilledRegionAxisCrossing *)av;
  const FilledRegionAxisCrossing *b = (const FilledRegionAxisCrossing *)bv;
  if (b->AxisFace > a->AxisFace) return -1;
  if (b->AxisFace < a->AxisFace) return  1;
  if (b->AxisFace < FACE_BOTTOM) // Sort top from right to left, and left from top to bottom
   {
    if      (b->axispos > a->axispos) return  1;
    else if (b->axispos < a->axispos) return -1;
   }
  else // Sort bottom from left to right, and right from bottom to top
   {
    if      (b->axispos > a->axispos) return -1;
    else if (b->axispos < a->axispos) return  1;
   }
  return 0;
 }

void FilledRegion_Finish(FilledRegionHandle *fr, int linetype, double linewidth)
 {
  unsigned char f1, f2;
  int i, j, l, NCrossings, i1, i2;
  double lastx, lasty, lastxap, lastyap;
  double cx1,cy1,cz1,cx2,cy2,cz2;
  ListItem *li;
  FilledRegionPoint *p;
  FilledRegionAxisCrossing *CrossPointList;

  // Malloc a structure to hold the positions of all of the points where we cross the boundary of the clip region
  CrossPointList = (FilledRegionAxisCrossing *)lt_malloc(fr->Naxiscrossings * sizeof(FilledRegionAxisCrossing));
  if (CrossPointList == NULL) return;
  l = fr->points->length;
  li = fr->points->first;
  for (i=j=0; i<l; i++)
   {
    p = (FilledRegionPoint *)(li->data);
    LineDraw_FindCrossingPoints(lastx,lasty,0,lastxap,lastyap,0.5,p->x,p->y,0,p->xap,p->yap,0.5,&i1,&i2,&cx1,&cy1,&cz1,&cx2,&cy2,&cz2,&f1,&f2,&NCrossings);
    if (NCrossings>0) // We have crossed boundary
     {
      if (!i1) { CrossPointList[j].x = cx1; CrossPointList[j].y = cy1; CrossPointList[j].AxisFace = f1; }
      else     { CrossPointList[j].x = cx2; CrossPointList[j].y = cy2; CrossPointList[j].AxisFace = f2; }
      if ((!i1)&&(!i2)) { CrossPointList[j].x2 = cx2; CrossPointList[j].y2 = cy2; }
      CrossPointList[j].singleton = (NCrossings==2);
      CrossPointList[j].sense     = (i1) ? OUTGOING : INGOING;
      CrossPointList[j].point     = li;
      j++;
     }
    if (NCrossings==2)
     {
      CrossPointList[j] = CrossPointList[j-1];
      CrossPointList[j].x = cx2; CrossPointList[j].y = cy2;
      CrossPointList[j].x2= cx1; CrossPointList[j].y2= cy1;
      CrossPointList[j].AxisFace = f2;
      j++;
     }
    lastx   = p->x;
    lasty   = p->y;
    lastxap = p->xap;
    lastyap = p->yap;
    li=li->next;
   }

  // Sort list into order around perimeter of clipping area
  qsort((void *)CrossPointList, j, sizeof(FilledRegionAxisCrossing), &frac_sorter);
  return;
 }

