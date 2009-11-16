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

#include "EPSMaker/eps_comm.h"
#include "EPSMaker/eps_arrow.h"
#include "EPSMaker/eps_eps.h"
#include "EPSMaker/eps_image.h"
#include "EPSMaker/eps_plot.h"
#include "EPSMaker/eps_text.h"

#include "ppl_canvasitems.h"
#include "ppl_canvasdraw.h"

// Table of the functions we call for each phase of the canvas drawing process for different object types

static void(*ArrowHandlers[])(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , NULL                , eps_arrow_RenderEPS, NULL};
static void(*EPSHandlers[]  )(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , NULL                , eps_eps_RenderEPS  , NULL};
static void(*ImageHandlers[])(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , NULL                , eps_image_RenderEPS, NULL};
static void(*PlotHandlers[] )(EPSComm *) = {eps_plot_ReadAccessibleData, eps_plot_DecideAxisRanges, eps_plot_LinkedAxesPropagate, eps_plot_SampleFunctions, eps_plot_YieldUpText, eps_plot_RenderEPS , NULL};
static void(*TextHandlers[] )(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , eps_text_YieldUpText, eps_text_RenderEPS , NULL};
static void(*AfterHandlers[])(EPSComm *) = {NULL                       , NULL                     , NULL                        , NULL                    , canvas_CallLaTeX    , canvas_EPSWrite    , NULL};

// Main entry point for rendering a canvas to graphical output

void canvas_draw(unsigned char *unsuccessful_ops)
 {
  int i, j, status=0;
  EPSComm comm;
  canvas_item *item;
  void(*ArrowHandler)(EPSComm *);
  void(*EPSHandler  )(EPSComm *);
  void(*ImageHandler)(EPSComm *);
  void(*PlotHandler )(EPSComm *);
  void(*TextHandler )(EPSComm *);
  void(*AfterHandler)(EPSComm *);

  // By default, we record all operations as having been successful
  for (i=0;i<MULTIPLOT_MAXINDEX; i++) unsuccessful_ops[i]=0;

  comm.itemlist = canvas_items;
  comm.status = &status;

  // Rendering of EPS occurs in a series of phases which we now loop over
  for (j=0; ; j++)
   {
    ArrowHandler = ArrowHandlers[j]; // Each object type has a handler for each phase of postscript generation
    EPSHandler   = EPSHandlers  [j];
    ImageHandler = ImageHandlers[j];
    PlotHandler  = PlotHandlers [j];
    TextHandler  = TextHandlers [j];
    AfterHandler = AfterHandlers[j];
    if ((ArrowHandler==NULL)&&(EPSHandler==NULL)&&(ImageHandler==NULL)&&(PlotHandler==NULL)&&(TextHandler==NULL)&&(AfterHandler==NULL)) break;

    // Loop over all of the items on the canvas
    for (item=comm.itemlist->first; item!=NULL; item=item->next)
     {
      if (unsuccessful_ops[item->id]) continue; // ... except those which have already failed
      comm.current = item;
      if      ((item->type == CANVAS_ARROW) && (ArrowHandler != NULL)) (*ArrowHandler)(&comm); // Call the relevant handler for each one
      else if ((item->type == CANVAS_EPS  ) && (EPSHandler   != NULL)) (*EPSHandler  )(&comm);
      else if ((item->type == CANVAS_IMAGE) && (ImageHandler != NULL)) (*ImageHandler)(&comm);
      else if ((item->type == CANVAS_PLOT ) && (PlotHandler  != NULL)) (*PlotHandler )(&comm);
      else if ((item->type == CANVAS_TEXT ) && (TextHandler  != NULL)) (*TextHandler )(&comm);
      if (status) { unsuccessful_ops[item->id] = 1; } // If something went wrong... flag it up and give up on this object
      status = 0;
     }
    if (AfterHandler != NULL) (*AfterHandler)(&comm); // At the end of each phase, a canvas-wide handler may be called
   }

  // Now convert eps output to bitmaped graphics if requested

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

void canvas_CallLaTeX(EPSComm *x)
 {
  return;
 }

void canvas_EPSWrite(EPSComm *x)
 {
  return;
 }

