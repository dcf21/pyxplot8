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
#include <math.h>
#include <string.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_threedimbuff.h"
#include "eps_settings.h"

// Internal state variables
unsigned char  ThreeDimBuffer_ACTIVE        = 0;
static long    ThreeDimBuffer_LineSegmentID = 0;
static List   *ThreeDimBuffer_buffer        = NULL;

static int     linetype_old  ; static int linetype_old_SET  ;
static double  linewidth_old ; static int linewidth_old_SET ;
static double  pointsize_old ; static int pointsize_old_SET ;
static char   *colstr_old;

void ThreeDimBuffer_Reset()
 {
  ThreeDimBuffer_ACTIVE        = 0;
  ThreeDimBuffer_LineSegmentID = 0;
  ThreeDimBuffer_buffer        = NULL;
  linetype_old_SET = linewidth_old_SET = pointsize_old_SET = 0;
  colstr_old = NULL;
  return;
 }

int ThreeDimBuffer_Activate(EPSComm *x)
 {
  ThreeDimBuffer_Deactivate(x);
  ThreeDimBuffer_ACTIVE        = 1;
  ThreeDimBuffer_LineSegmentID = 0;
  ThreeDimBuffer_buffer        = ListInit();
  linetype_old_SET = linewidth_old_SET = pointsize_old_SET = 0;
  colstr_old = NULL;
  return (ThreeDimBuffer_buffer != NULL);
 }

struct DepthArrayEntry { double depth; void *item; };

static int ThreeDimBuffer_qsort_slave(const void *x, const void *y)
 {
  const struct DepthArrayEntry *x1 = (const struct DepthArrayEntry *)x;
  const struct DepthArrayEntry *y1 = (const struct DepthArrayEntry *)y;

  if      (x1->depth > y1->depth) return  1.0;
  else if (x1->depth < y1->depth) return -1.0;
  else                            return  0.0;
 }

int ThreeDimBuffer_Deactivate(EPSComm *x)
 {
  int   Nitems, i, j;
  struct DepthArrayEntry *DepthArray;
  ListIterator *ListIter;

  if (!ThreeDimBuffer_ACTIVE) return 0;
  Nitems = ListLen(ThreeDimBuffer_buffer);
  ThreeDimBuffer_ACTIVE = 0;
  if (Nitems > 0)
   {
    // Display all items in 3D display buffer, from back to front
    DepthArray = (struct DepthArrayEntry *)lt_malloc(Nitems * sizeof(struct DepthArrayEntry));
    i=0;
    ListIter = ListIterateInit(ThreeDimBuffer_buffer);
    while (ListIter != NULL)
     {
      DepthArray[i].item  = ListIter->data;
      DepthArray[i].depth = ((ThreeDimBufferItem *)(ListIter->data))->depth;
      i++;
      ListIter = ListIterate(ListIter, NULL);
     }
    qsort((void *)DepthArray, i, sizeof(struct DepthArrayEntry), ThreeDimBuffer_qsort_slave);
    for (j=0; j<i; j++)
     {
     }
   }
  ThreeDimBuffer_Reset();
  return 0;
 }

int ThreeDimBuffer_writeps(EPSComm *x, double z, int linetype, double linewidth, double pointsize, char *colstr, char *psfrag)
 {
  ThreeDimBufferItem *item;

  if (!ThreeDimBuffer_ACTIVE)
   {
    if ((!pointsize_old_SET) || (pointsize_old != pointsize)) { pointsize_old_SET=1; pointsize_old=pointsize; fprintf(x->epsbuffer, "/ps { %f } def\n", pointsize*3); } // Scale up all pointsizes by 3
    if ((colstr_old == NULL) || (strcmp(colstr_old, colstr)!=0)) { colstr_old = colstr; fprintf(x->epsbuffer, "%s\n", colstr); }
    eps_core_SetLinewidth(x, linewidth * EPS_DEFAULT_LINEWIDTH, linetype);
    fprintf(x->epsbuffer, "%s\n", psfrag);
   }
  else
   {
    item = (ThreeDimBufferItem *)lt_malloc(sizeof(ThreeDimBufferItem));
    if (item == NULL) return 1;
    item->FlagLineSegment = item->FirstLineSegment = 0;
    item->linetype        = linetype;
    item->linewidth       = linewidth;
    item->pointsize       = pointsize;
    item->colstr          = colstr;
    item->psfrag          = psfrag;
    item->depth           = z;
    ListAppendPtr(ThreeDimBuffer_buffer, (void *)item, 0, 0, DATATYPE_VOID);
   }
  return 0;
 }

int ThreeDimBuffer_linesegment(EPSComm *x, double z, int linetype, double linewidth, char *colstr, double x0, double y0, double x1, double y1, double x2, double y2, unsigned char FirstSegment)
 {
  ThreeDimBufferItem *item;

  if (!ThreeDimBuffer_ACTIVE)
   {
    if (FirstSegment && (ThreeDimBuffer_LineSegmentID > 0)) ThreeDimBuffer_linepenup(x); // If first segment of new line, finish old line
    if ((!linetype_old_SET) || (linetype_old != linetype) || (!linewidth_old_SET) || (linewidth_old != linewidth) || (colstr_old == NULL) || (strcmp(colstr_old, colstr)!=0))
     {
      ThreeDimBuffer_linepenup(x);
      linetype_old_SET = linewidth_old_SET = 1;
      linetype_old     = linetype;
      linewidth_old    = linewidth;
      eps_core_SetLinewidth(x, linewidth * EPS_DEFAULT_LINEWIDTH, linetype);
      if ((colstr_old == NULL) || (strcmp(colstr_old, colstr)!=0)) { colstr_old = colstr; fprintf(x->epsbuffer, "%s\n", colstr); }
      if ((!FirstSegment) && ((x0!=x1) || (y0!=y1)))
       {
        double theta = atan2(x0-x1,y0-y1);
        x0 = x1 + 2 * sin(theta) * linewidth * EPS_DEFAULT_LINEWIDTH; // Start line 2 linewidths along in the direction of x0 to get correct linecap
        y0 = y1 + 2 * cos(theta) * linewidth * EPS_DEFAULT_LINEWIDTH;
       }
      if (FirstSegment) fprintf(x->epsbuffer, "%.2f %.2f moveto\n%.2f %.2f lineto\n", x1, y1, x2, y2);
      else              fprintf(x->epsbuffer, "%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n", x0, y0, x1, y1, x2, y2);
     }
    else
     {
      if (FirstSegment) fprintf(x->epsbuffer, "%.2f %.2f moveto\n%.2f %.2f lineto\n", x1, y1, x2, y2);
      else              fprintf(x->epsbuffer, "%.2f %.2f lineto\n", x2, y2);
     }
    ThreeDimBuffer_LineSegmentID = 1;
   }
  else
   {
    item = (ThreeDimBufferItem *)lt_malloc(sizeof(ThreeDimBufferItem));
    if (item == NULL) return 1;
    item->FlagLineSegment  = 1;
    item->FirstLineSegment = FirstSegment;
    item->linetype        = linetype;
    item->linewidth       = linewidth;
    item->colstr          = colstr;
    item->depth           = z;
    item->x0              = x0;
    item->y0              = y0;
    item->x1              = x1;
    item->y1              = y1;
    item->x2              = x2;
    item->y2              = y2;
    ListAppendPtr(ThreeDimBuffer_buffer, (void *)item, 0, 0, DATATYPE_VOID);
   }
  return 0;
 }

int ThreeDimBuffer_linepenup(EPSComm *x)
 {
  if (!ThreeDimBuffer_ACTIVE)
   {
    if (ThreeDimBuffer_LineSegmentID > 0) fprintf(x->epsbuffer, "stroke\n");
   }
  return 0;
 }
