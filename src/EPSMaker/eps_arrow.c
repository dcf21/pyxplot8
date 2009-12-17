// eps_arrow.c
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

#define _PPL_EPS_ARROW 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_arrow.h"
#include "eps_settings.h"

void eps_arrow_RenderEPS(EPSComm *x)
 {
  double x1, x2, y1, y2;
  with_words ww;

  // Print label at top of postscript description of arrow
  fprintf(x->epsbuffer, "%% Canvas item %d [arrow]\n", x->current->id);
  eps_core_clear(x);

  // Calculate positions of start and end of arrow
  x1 =  x->current->xpos                      * M_TO_PS; // Start of arrow
  y1 =  x->current->ypos                      * M_TO_PS;
  x2 = (x->current->xpos2 + x->current->xpos) * M_TO_PS; // End of arrow
  y2 = (x->current->ypos2 + x->current->ypos) * M_TO_PS;

  // Expand any numbered styles which may appear in the with words we are passed
  with_words_merge(&ww, &x->current->with_data, NULL, NULL, NULL, NULL, 1);

  // Call primitive routine
  eps_primitive_arrow(x, x->current->ArrowType, x1, y1, x2, y2, &ww);

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

// Primitive routine for drawing arrow, suitable for use elsewhere in the EPS library
void eps_primitive_arrow(EPSComm *x, int ArrowType, double x1, double y1, double x2, double y2, with_words *with_data)
 {
  int    lt;
  double lw, lw_scale, x3, y3, x4, y4, x5, y5, xstart, ystart, xend, yend, direction;

  // Set colour of arrow
  eps_core_SetColour(x, with_data);

  // Set linewidth and linetype
  if (with_data->USElinewidth) lw_scale = with_data->linewidth;
  else                         lw_scale = x->current->settings.LineWidth;
  lw = EPS_DEFAULT_LINEWIDTH * lw_scale;

  if (with_data->USElinetype)  lt = with_data->linetype;
  else                         lt = 0;

  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, lw, lt);

  // Factor two ends of arrow into EPS file's bounding box
  eps_core_BoundingBox(x, x1, y1, lw);
  eps_core_BoundingBox(x, x2, y2, lw);

  // Work out direction of arrow
  if (hypot(x2-x1,y2-y1) < 1e-200) direction = 0.0;
  else                             direction = atan2(x2-x1,y2-y1);

  // Draw arrowhead on beginning of arrow if desired
  if (ArrowType == SW_ARROWTYPE_TWOWAY)
   {
    x3 = x1 - EPS_ARROW_HEADSIZE * lw_scale * sin((direction+M_PI) - EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on one side
    y3 = y1 - EPS_ARROW_HEADSIZE * lw_scale * cos((direction+M_PI) - EPS_ARROW_ANGLE / 2);
    x5 = x1 - EPS_ARROW_HEADSIZE * lw_scale * sin((direction+M_PI) + EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on other side
    y5 = y1 - EPS_ARROW_HEADSIZE * lw_scale * cos((direction+M_PI) + EPS_ARROW_ANGLE / 2);

    x4 = x1 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction+M_PI) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2); // Point where back of arrowhead crosses stalk
    y4 = y1 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction+M_PI) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2);

    IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nfill\n", x4,y4,x3,y3,x1,y1,x5,y5);
    eps_core_BoundingBox(x, x3, y3, lw);
    eps_core_BoundingBox(x, x5, y5, lw);
    xstart = x4;
    ystart = y4;
   } else {
    xstart = x1;
    ystart = y1;
   }

  // Draw arrowhead on end of arrow if desired
  if ((ArrowType == SW_ARROWTYPE_HEAD) || (ArrowType == SW_ARROWTYPE_TWOWAY))
   {
    x3 = x2 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction - EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on one side
    y3 = y2 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction - EPS_ARROW_ANGLE / 2);
    x5 = x2 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction + EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on other side
    y5 = y2 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction + EPS_ARROW_ANGLE / 2);

    x4 = x2 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2); // Point where back of arrowhead crosses stalk
    y4 = y2 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2);

    IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nfill\n", x4,y4,x3,y3,x2,y2,x5,y5);
    eps_core_BoundingBox(x, x3, y3, lw);
    eps_core_BoundingBox(x, x5, y5, lw);
    xend = x4;
    yend = y4;
   } else {
    xend = x2;
    yend = y2;
   }

  // Draw stalk of arrow
  IF_NOT_INVISIBLE fprintf(x->epsbuffer, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\nstroke\n", xstart, ystart, xend, yend);
  return;
 }

