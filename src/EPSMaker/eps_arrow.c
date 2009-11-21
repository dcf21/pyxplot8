// eps_arrow.c
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

#define _PPL_EPS_ARROW 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_colours.h"
#include "eps_comm.h"
#include "eps_core.h"
#include "eps_arrow.h"
#include "eps_settings.h"

void eps_arrow_RenderEPS(EPSComm *x)
 {
  double lw, lw_scale, x1, x2, y1, y2, x3, y3, x4, y4, x5, y5, direction;

  // Print label at top of postscript description of arrow
  fprintf(x->epsbuffer, "%% Canvas item %d [arrow]\n", x->current->id);
  eps_core_clear(x);

  // Set colour of arrow
  eps_core_SetColour(x, &x->current->with_data);

  // Set linewidth
  if (x->current->with_data.USElinewidth) lw_scale = x->current->with_data.linewidth;
  else                                    lw_scale = 1.0;
  lw = EPS_DEFAULT_LINEWIDTH * lw_scale;
  eps_core_SetLinewidth(x, lw);

  // Draw stalk of arrow
  x1 =  x->current->xpos                      * M_TO_PS;
  y1 =  x->current->ypos                      * M_TO_PS;
  x2 = (x->current->xpos2 + x->current->xpos) * M_TO_PS;
  y2 = (x->current->ypos2 + x->current->ypos) * M_TO_PS;

  fprintf(x->epsbuffer, "newpath\n%f %f moveto\n%f %f lineto\nstroke\n", x1, y1, x2, y2);
  eps_core_BoundingBox(x, x1, y1, lw);
  eps_core_BoundingBox(x, x2, y2, lw);

  // Work out direction of arrow
  if (hypot(x2-x1,y2-y1) < 1e-200) direction = 0.0;
  else                             direction = atan2(x2-x1,y2-y1);

  // Draw head of arrow if desired
  if ((x->current->ArrowType == SW_ARROWTYPE_HEAD) || (x->current->ArrowType == SW_ARROWTYPE_TWOWAY))
   {
    x3 = x2 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction - EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on one side
    y3 = y2 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction - EPS_ARROW_ANGLE / 2);
    x5 = x2 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction + EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on other side
    y5 = y2 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction + EPS_ARROW_ANGLE / 2);

    x4 = x2 - EPS_ARROW_HEADSIZE * lw_scale * sin(direction) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2); // Point where back of arrowhead crosses stalk
    y4 = y2 - EPS_ARROW_HEADSIZE * lw_scale * cos(direction) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2);

    fprintf(x->epsbuffer, "newpath\n%f %f moveto\n%f %f lineto\n%f %f lineto\n%f %f lineto\nclosepath\nfill\n", x2, y2, x3, y3, x4, y4, x5, y5);
    eps_core_BoundingBox(x, x3, y3, lw);
    eps_core_BoundingBox(x, x5, y5, lw);
   }

  // Draw second head of arrow is desired
  if (x->current->ArrowType == SW_ARROWTYPE_TWOWAY)
   {
   }

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

