// eps_image.c
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

#define _PPL_EPS_IMAGE 1

#include <stdlib.h>
#include <stdio.h>

#include "eps_comm.h"
#include "eps_image.h"
#include "eps_settings.h"

void eps_image_RenderEPS(EPSComm *x)
 {
  fprintf(x->epsbuffer, "%% Canvas item %d [bitmap image]\n", x->current->id);
  return;
 }

