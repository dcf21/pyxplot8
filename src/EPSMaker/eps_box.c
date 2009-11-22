// eps_box.c
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

#define _PPL_EPS_BOX 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_box.h"
#include "eps_settings.h"

void eps_box_RenderEPS(EPSComm *x)
 {
  double x1, x2, y1, y2;

  // Print label at top of postscript description of box
  fprintf(x->epsbuffer, "%% Canvas item %d [box]\n", x->current->id);
  eps_core_clear(x);

  // Calculate positions of two opposite corners of box
  x1 =  x->current->xpos                      * M_TO_PS; // First corner
  y1 =  x->current->ypos                      * M_TO_PS;
  x2 = (x->current->xpos2 + x->current->xpos) * M_TO_PS; // Second corner
  y2 = (x->current->ypos2 + x->current->ypos) * M_TO_PS;

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

