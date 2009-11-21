// eps_core.c
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

// This file contains various numerical constants which are used by the eps
// generation routines

#define _PPL_EPS_CORE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_colours.h"
#include "eps_comm.h"

// Clear EPS state information to ensure that we restate the linewidth, colour, etc we are using for the next item
void eps_core_clear(EPSComm *x)
 {
  x->LastLinewidth = -1;
  strcpy(x->LastEPSColour, "");
  return;
 }

// Set the colour of the EPS we are painting
void eps_core_SetColour(EPSComm *x, with_words *colour)
 {
  char NewColour[256];

  // Colour may be specified as a named colour, or as RGB components, or may not be specified at all, in which case we use black
  if      (x->current->with_data.USEcolourRGB) sprintf(NewColour, "%d %d %d setrgbcolor", x->current->with_data.colourR, x->current->with_data.colourG, x->current->with_data.colourB);
  else if (x->current->with_data.USEcolour   ) sprintf(NewColour, "%f %f %f %f setcmykcolor",
                                                        *(double *)FetchSettingName(x->current->with_data.colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_C, sizeof(double)),
                                                        *(double *)FetchSettingName(x->current->with_data.colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_M, sizeof(double)),
                                                        *(double *)FetchSettingName(x->current->with_data.colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_Y, sizeof(double)),
                                                        *(double *)FetchSettingName(x->current->with_data.colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_K, sizeof(double)) );
  else                                         sprintf(NewColour, "0 0 0 setrgbcolor");

  // Only change postscript colour if the colour we want isn't the one we are already using
  if (strcmp(NewColour, x->LastEPSColour) != 0)
   {
    fprintf(x->epsbuffer, "%s\n", NewColour);
    strcpy(x->LastEPSColour, NewColour);
   }
  return;
 }

// Set the linewidth of the EPS we are painting
void eps_core_SetLinewidth(EPSComm *x, double lw)
 {
  if (lw == x->LastLinewidth) return;
  fprintf(x->epsbuffer, "%f setlinewidth\n", lw);
  x->LastLinewidth = lw;
  return;
 }

// Update the EPS bounding box
void eps_core_BoundingBox(EPSComm *x, double xpos, double ypos, double lw)
 {
  if ((!x->bb_set) || (x->bb_left   > (xpos-lw/2))) x->bb_left   = (xpos-lw/2);
  if ((!x->bb_set) || (x->bb_right  < (xpos+lw/2))) x->bb_right  = (xpos+lw/2);
  if ((!x->bb_set) || (x->bb_bottom > (ypos-lw/2))) x->bb_bottom = (ypos-lw/2);
  if ((!x->bb_set) || (x->bb_top    < (ypos+lw/2))) x->bb_top    = (ypos+lw/2);
  x->bb_set = 1;
  return;
 }

