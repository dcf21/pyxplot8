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

#include "ppl_error.h"

#include "eps_comm.h"
#include "eps_core.h"
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
  output->EverInside  = 0;
  output->points      = ListInit();
  return output;
 }

void FilledRegion_Point(FilledRegionHandle *fr, double x, double y)
 {
  FilledRegionPoint p;
  double xpos, ypos, depth, xap, yap, zap, ap1, ap2;
  unsigned char f1, f2;
  int NCrossings, i1, i2;
  double cx1,cy1,cz1,cx2,cy2,cz2;

  // Work out where (x,y) in coordinate space lies on the canvas
  eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, NULL, NULL, NULL, fr->ThreeDim, x, y, 0, fr->xa, fr->ya, fr->za, fr->xrn, fr->yrn, fr->zrn, fr->sg, fr->origin_x, fr->origin_y, fr->width, fr->height, fr->zdepth, 1);

  // Add this to a list of points describing the outline of the region which we are going to fill
  p.inside = ((xap>=0)&&(xap<=1)&&(yap>=0)&&(yap<=1)&&(zap>=0)&&(zap<=1));
  p.x      = xpos;
  p.y      = ypos;
  p.xa     = x;
  p.ya     = y;
  p.xap    = xap;
  p.yap    = yap;

  ListAppendPtrCpy(fr->points, (void *)&p, sizeof(FilledRegionPoint), DATATYPE_VOID);

  // Count the number of times which the path crosses the boundary of the clip region
  if (!fr->first)
   {
    LineDraw_FindCrossingPoints(fr->lastx,fr->lasty,0,fr->lastxap,fr->lastyap,0.5,
                                xpos,ypos,0,xap,yap,0.5,&i1,&i2,&cx1,&cy1,&cz1,&cx2,&cy2,&cz2,&f1,&ap1,&f2,&ap2,&NCrossings);
    fr->Naxiscrossings += NCrossings;
    fr->EverInside = (fr->EverInside) || (NCrossings>0) || i1 || i2;
   }
  fr->lastx = xpos; fr->lasty = ypos; fr->lastxap = xap; fr->lastyap = yap;
  fr->first = 0;
  return;
 }

static int frac_sorter(const void *av, const void *bv)
 {
  const FilledRegionAxisCrossing *a = (const FilledRegionAxisCrossing *)av;
  const FilledRegionAxisCrossing *b = (const FilledRegionAxisCrossing *)bv;
  if (b->AxisFace > a->AxisFace) return -1;
  if (b->AxisFace < a->AxisFace) return  1;
  if (b->AxisFace < FACE_BOTTOM) // Sort top from right to left, and left from top to bottom
   {
    if      (b->AxisPos > a->AxisPos) return  1;
    else if (b->AxisPos < a->AxisPos) return -1;
   }
  else // Sort bottom from left to right, and right from bottom to top
   {
    if      (b->AxisPos > a->AxisPos) return -1;
    else if (b->AxisPos < a->AxisPos) return  1;
   }
  return 0;
 }

static unsigned char TestPointInside(FilledRegionHandle *fr, double X, double Y, int dir_x, int dir_y)
 {
  unsigned char inside;
  double x_intersect, y_intersect;
  int i, n, Npoints;
  ListItem *li1, *li2;
  FilledRegionPoint *p1, *p2;

  // Move fractionally off test point
  if      (dir_x==-1) X -= fabs(1e-15*X);
  else if (dir_x== 1) X += fabs(1e-15*X);
  else if (dir_y==-1) Y -= fabs(1e-15*X);
  else if (dir_y== 1) Y += fabs(1e-15*X);

  Npoints = fr->points->length;
  li2     = fr->points->first;
  p2      = (FilledRegionPoint *)(li2->data);

  // Loop around perimeter of region which we are going to fill
  for (i=n=0; i<Npoints; i++)
   {
    li1 = li2;
    p1  = p2;
    li2 = li1->next;
    if (li2==NULL) li2=fr->points->first;
    p2  = (FilledRegionPoint *)(li2->data); // p1 -> p2 is a line segment of perimeter

    // Count how many segments cross lines travelling out up/left/down/right from (X,Y)
    if (dir_x==0)
     {
      y_intersect = (p2->y - p1->y)/(p2->x - p1->x)*(X - p1->x) + p1->y; // y point where line p1->p2 intersects x=X
      if ((gsl_finite(y_intersect)) && (y_intersect>=p1->y) && (y_intersect<p2->y) && (((dir_y>0)&&(y_intersect>Y)) || ((dir_y<0)&&(y_intersect<Y)))) n++;
     }
    else
     {
      x_intersect = (p2->x - p1->x)/(p2->y - p1->y)*(Y - p1->y) + p1->x; // x point where line p1->p2 intersects y=Y
      if ((gsl_finite(x_intersect)) && (x_intersect>=p1->x) && (x_intersect<p2->x) && (((dir_x>0)&&(x_intersect>X)) || ((dir_x<0)&&(x_intersect<X)))) n++;
     }
   }
  inside = (n%2); // If were an even number of segments, we are outside. If there were an odd number, we are inside.
  return inside;
 }

#define PS_POINT(X,Y) \
   if (first_point) { fprintf(fr->x->epsbuffer, "newpath %.2f %.2f moveto\n", X, Y); first_point=0; } \
   else             { fprintf(fr->x->epsbuffer, "%.2f %.2f lineto\n", X, Y); }

static void OutputPath(FilledRegionHandle *fr, FilledRegionAxisCrossing *CrossPointList, char *EndText)
 {
  unsigned char face, sense, inside_ahead, FillSide, first_point=1, fail=0;
  int i, j, k, l, nac, dir_x, dir_y;
  ListItem *li, *lil;
  FilledRegionPoint *p;
  nac = fr->Naxiscrossings;

  // Clear used flags
  for (i=0; i<nac; i++) CrossPointList[i].used=0;

  // Loop over multiple detached segments of filled region which may be caused by clipping it into pieces
  while (!fail)
   {
    // Find an axis crossing point which is unique
    l=-1;
    for (i=0; i<nac; i++)
     {
      if (CrossPointList[i].used) continue;
      l=i; // Fallback option; an unused crossing point
      j=(i+1)%nac; // Next axis crossing
      k=(i+nac-1)%nac; // Prev axis crossing
      if ((CrossPointList[i].x == CrossPointList[k].x) && (CrossPointList[i].y == CrossPointList[k].y)) continue;
      if ((CrossPointList[i].x == CrossPointList[j].x) && (CrossPointList[i].y == CrossPointList[j].y)) continue;
      break;
     }
    if (l==-1) break;
    i=l;

    // Work out which face first axis crossing is on, and which direction we're moving going to next item in crossing point list
    face = CrossPointList[i].AxisFace;
    if      (face==FACE_TOP   ) { dir_x=-1; dir_y= 0; }
    else if (face==FACE_LEFT  ) { dir_x= 0; dir_y=-1; }
    else if (face==FACE_BOTTOM) { dir_x= 1; dir_y= 0; }
    else                        { dir_x= 0; dir_y= 1; }
    inside_ahead = TestPointInside(fr, CrossPointList[i].x, CrossPointList[i].y, dir_x, dir_y);

    // Decide whether we are proceeding clockwise or anticlockwise around the region we're going to fill
    FillSide = !inside_ahead;

    while (1)
     {
      CrossPointList[i].used = 1;
      if (CrossPointList[i].singleton) // Move along a linesegment which streaks across canvas in one step
       {
        PS_POINT(CrossPointList[i].x , CrossPointList[i].y ); // Cut point where line enters canvas
        PS_POINT(CrossPointList[i].x2, CrossPointList[i].y2); // Cut point where line leaves canvas
        i = CrossPointList[i].twin;
        CrossPointList[i].used = 1;
       }
      else // Follow a path across the canvas, step by step
       {
        PS_POINT(CrossPointList[i].x, CrossPointList[i].y); // Cut point where line enters canvas
        li = CrossPointList[i].point;
        sense = CrossPointList[i].sense;
        if (sense == OUTGOING) { li=li->prev; if (li==NULL) li=fr->points->last; }
        p  = (FilledRegionPoint *)(li->data);
        while (p->inside)
         {
          PS_POINT(p->x, p->y);
          lil=li;
          if (sense == OUTGOING) { li=li->prev; if (li==NULL) li=fr->points->last; } // Moving against flow in which line was originally drawn
          else                   { li=li->next; if (li==NULL) li=fr->points->first; } // Moving with flow in which line was originally drawn
          p=(FilledRegionPoint *)(li->data);
         }
        for (i=0; i<nac; i++) if ((!CrossPointList[i].used)&&((CrossPointList[i].point==lil)||(CrossPointList[i].point==li))) break;
        if (!((!CrossPointList[i].used)&&((CrossPointList[i].point==lil)||(CrossPointList[i].point==li)))) { ppl_error(ERR_INTERNAL, "Failure within FilledRegion"); fail=1; break; }
        PS_POINT(CrossPointList[i].x, CrossPointList[i].y); // Cut point where line leaves canvas
        CrossPointList[i].used = 1;
       }

      // Skirt along edge of clip region to find next entry point
      for (k=i; ((CrossPointList[i].x==CrossPointList[k].x)&&(CrossPointList[i].y==CrossPointList[k].y)); k=(k+1)%nac) if (!CrossPointList[k].used) { i=k; FillSide = !FillSide; continue; }
      for (k=i; ((CrossPointList[i].x==CrossPointList[k].x)&&(CrossPointList[i].y==CrossPointList[k].y)); k=(k+nac-1)%nac) if (!CrossPointList[k].used) { i=k; FillSide = !FillSide; continue; }
      if (FillSide) j=(i+1)%nac; // Move anticlockwise around path
      else          j=(i+nac-1)%nac; // Move clockwise around path
      i=j;
      if (CrossPointList[i].used) break;
      FillSide = !FillSide;
     }
    fprintf(fr->x->epsbuffer, "%s\n", EndText);
    first_point=1;
   }
  return;
 }

void FilledRegion_Finish(FilledRegionHandle *fr, int linetype, double linewidth, unsigned char StrokeOutline)
 {
  unsigned char f1, f2, first_point=1;
  int i, j, l, NCrossings, i1, i2;
  double lastx, lasty, lastxap, lastyap;
  double cx1,cy1,cz1,cx2,cy2,cz2,ap1,ap2;
  ListItem *li;
  FilledRegionPoint *p;
  FilledRegionAxisCrossing *CrossPointList;

  l = fr->points->length;
  li = fr->points->first;
  if (l < 2) return; // No points on outline to stroke
  if (!fr->EverInside) return; // Path never ventures within clip region

  // Path never touches the edges of the clip region
  if (fr->Naxiscrossings < 1)
   {
    for (i=0; i<l; i++)
     {
      p = (FilledRegionPoint *)(li->data);
      PS_POINT(p->x, p->y);
      li=li->next;
     }
    fprintf(fr->x->epsbuffer, "closepath fill\n");
    eps_core_SwitchFrom_FillColour(fr->x);
    if (!StrokeOutline) return;
    eps_core_SetLinewidth(fr->x, linewidth, linetype, 0.0);
    li = fr->points->first;
    first_point=1;
    for (i=0; i<l; i++)
     {
      p = (FilledRegionPoint *)(li->data);
      PS_POINT(p->x, p->y);
      li=li->next;
     }
    fprintf(fr->x->epsbuffer, "closepath stroke\n");
    return;
   }

  // Close path to get potential final axis crossing
  p = (FilledRegionPoint *)(li->data);
  FilledRegion_Point(fr, p->xa, p->ya); // Add first point to end of point list
  l = fr->points->length;
  lastx   = p->x;
  lasty   = p->y;
  lastxap = p->xap;
  lastyap = p->yap;
  li=li->next;

  // Malloc a structure to hold the positions of all of the points where we cross the boundary of the clip region
  CrossPointList = (FilledRegionAxisCrossing *)lt_malloc(fr->Naxiscrossings * sizeof(FilledRegionAxisCrossing));
  if (CrossPointList == NULL) return;
  for (i=1,j=0; i<l; i++)
   {
    p = (FilledRegionPoint *)(li->data);
    LineDraw_FindCrossingPoints(lastx,lasty,0,lastxap,lastyap,0.5,p->x,p->y,0,p->xap,p->yap,0.5,&i1,&i2,&cx1,&cy1,&cz1,&cx2,&cy2,&cz2,&f1,&ap1,&f2,&ap2,&NCrossings);
    if (NCrossings>0) // We have crossed boundary
     {
      if (!i1) { CrossPointList[j].x = cx1; CrossPointList[j].y = cy1; CrossPointList[j].AxisFace = f1; CrossPointList[j].AxisPos = (f1<FACE_BOTTOM)?(1-ap1):ap1; }
      else     { CrossPointList[j].x = cx2; CrossPointList[j].y = cy2; CrossPointList[j].AxisFace = f2; CrossPointList[j].AxisPos = (f2<FACE_BOTTOM)?(1-ap2):ap2; }
      if ((!i1)&&(!i2)) { CrossPointList[j].x2 = cx2; CrossPointList[j].y2 = cy2; }
      CrossPointList[j].singleton = (NCrossings==2);
      CrossPointList[j].twin      = j+1;
      CrossPointList[j].sense     = (i1) ? OUTGOING : INGOING;
      CrossPointList[j].point     = li;
      CrossPointList[j].used      = 0;
      j++;
     }
    if (NCrossings==2)
     {
      CrossPointList[j] = CrossPointList[j-1];
      CrossPointList[j].x = cx2; CrossPointList[j].y = cy2;
      CrossPointList[j].x2= cx1; CrossPointList[j].y2= cy1;
      CrossPointList[j].AxisFace = f2;
      CrossPointList[j].AxisPos  = (f2<FACE_BOTTOM)?(1-ap2):ap2;
      CrossPointList[j].twin = j-1;
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

  // Output path
  OutputPath(fr, CrossPointList, "closepath fill\n");
  eps_core_SwitchFrom_FillColour(fr->x);
  if (!StrokeOutline) return;
  eps_core_SetLinewidth(fr->x, linewidth, linetype, 0.0);
  li = fr->points->first;
  OutputPath(fr, CrossPointList, "closepath stroke\n");
  return;
 }
