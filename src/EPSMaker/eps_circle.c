// eps_circle.c
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

#define _PPL_EPS_CIRCLE 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_settings.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_circle.h"
#include "eps_settings.h"

void eps_circ_RenderEPS(EPSComm *x)
 {
  int    lt;
  double lw, lw_scale, xpos, ypos, r;
  with_words ww;

  // Print label at top of postscript description of circle
  fprintf(x->epsbuffer, "%% Canvas item %d [circle]\n", x->current->id);
  eps_core_clear(x);

  // Calculate position of centre of circle and its radius in TeX points
  xpos = x->current->xpos  * M_TO_PS;
  ypos = x->current->ypos  * M_TO_PS;
  r    = x->current->xpos2 * M_TO_PS;

  // Expand any numbered styles which may appear in the with words we are passed
  with_words_merge(&ww, &x->current->with_data, NULL, NULL, NULL, NULL, 1);

  // Set fill colour of circle
  eps_core_SetFillColour(x, &ww);
  eps_core_SwitchTo_FillColour(x);

  // Fill circle
  IF_NOT_INVISIBLE fprintf(x->epsbuffer, "%.2f %.2f %.2f 0 360 arc\nclosepath\nfill\n", xpos,ypos,r);

  // Set colour of outline of circle
  eps_core_SetColour(x, &ww, 1);

  // Set linewidth and linetype of outline
  if (ww.USElinewidth) lw_scale = ww.linewidth;
  else                 lw_scale = x->current->settings.LineWidth;
  lw = EPS_DEFAULT_LINEWIDTH * lw_scale;

  if (ww.USElinetype)  lt = ww.linetype;
  else                 lt = 1;

  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, lw, lt, 0.0);

  // Stroke outline of circle
  IF_NOT_INVISIBLE fprintf(x->epsbuffer, "%.2f %.2f %.2f 0 360 arc\nclosepath\nstroke\n", xpos,ypos,r);

  // Factor circle into EPS file's bounding box
  eps_core_BoundingBox(x, xpos-r, ypos  , lw);
  eps_core_BoundingBox(x, xpos+r, ypos  , lw);
  eps_core_BoundingBox(x, xpos  , ypos-r, lw);
  eps_core_BoundingBox(x, xpos  , ypos+r, lw);

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

