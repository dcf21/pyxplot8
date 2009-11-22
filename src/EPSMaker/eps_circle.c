// eps_circle.c
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

#define _PPL_EPS_CIRCLE 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_circle.h"
#include "eps_settings.h"

void eps_circ_RenderEPS(EPSComm *x)
 {
  double xpos, ypos, r;

  // Print label at top of postscript description of circle
  fprintf(x->epsbuffer, "%% Canvas item %d [circle]\n", x->current->id);
  eps_core_clear(x);

  // Calculate position of centre of circle
  xpos = x->current->xpos  * M_TO_PS;
  ypos = x->current->ypos  * M_TO_PS;
  r    = x->current->ypos2 * M_TO_PS;

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

