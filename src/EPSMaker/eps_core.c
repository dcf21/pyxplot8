// eps_core.c
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
#include "eps_style.h"

// Clear EPS state information to ensure that we restate the linewidth, colour, etc we are using for the next item
void eps_core_clear(EPSComm *x)
 {
  x->LastLinewidth = -1;
  strcpy(x->LastPSColour, "");
  strcpy(x->CurrentColour, "");
  strcpy(x->CurrentFillColour, "");
  x->LastLinetype = 0;
  return;
 }

// Set the colour of the EPS we are painting
void eps_core_SetColour(EPSComm *x, with_words *ww, unsigned char WritePS)
 {
  // Colour may be specified as a named colour, or as RGB components, or may not be specified at all, in which case we use black
  if      (ww->USEcolourRGB)  sprintf(x->CurrentColour, "%.3f %.3f %.3f setrgbcolor", (double)ww->colourR/255,
                                                                                      (double)ww->colourG/255,
                                                                                      (double)ww->colourB/255  );
  else if((ww->USEcolour   ) && ((ww->colour==COLOUR_NULL)||(ww->colour==COLOUR_INVISIBLE)||(ww->colour==COLOUR_TRANSPARENT)))
                              strcpy (x->CurrentColour, ""); // This is code to tell us we're writing in invisible ink
  else if (ww->USEcolour   )  sprintf(x->CurrentColour, "%.3f %.3f %.3f %.3f setcmykcolor",
                                          *(double *)FetchSettingName(ww->colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_C, sizeof(double)),
                                          *(double *)FetchSettingName(ww->colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_M, sizeof(double)),
                                          *(double *)FetchSettingName(ww->colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_Y, sizeof(double)),
                                          *(double *)FetchSettingName(ww->colour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_K, sizeof(double)) );
  else                        sprintf(x->CurrentColour, "0 0 0 setrgbcolor");

  // Only change postscript colour if the colour we want isn't the one we are already using
  if ((strcmp(x->CurrentColour, x->LastPSColour) != 0) && WritePS && (x->CurrentColour[0]!='\0'))
   { strcpy(x->LastPSColour, x->CurrentColour); fprintf(x->epsbuffer, "%s\n", x->LastPSColour); }
  return;
 }

void eps_core_SetFillColour(EPSComm *x, with_words *ww)
 {
  // Colour may be specified as a named colour, or as RGB components, or may not be specified at all, in which case we use black
  if      (ww->USEfillcolourRGB)  sprintf(x->CurrentFillColour, "%.3f %.3f %.3f setrgbcolor", (double)ww->fillcolourR/255,
                                                                                              (double)ww->fillcolourG/255,
                                                                                              (double)ww->fillcolourB/255  );
  else if((ww->USEfillcolour   ) && ((ww->fillcolour==COLOUR_NULL)||(ww->fillcolour==COLOUR_INVISIBLE)||(ww->fillcolour==COLOUR_TRANSPARENT)))
                                  strcpy (x->CurrentFillColour, ""); // This is code to tell us we're writing in invisible ink
  else if (ww->USEfillcolour   )  sprintf(x->CurrentFillColour, "%.3f %.3f %.3f %.3f setcmykcolor",
                                              *(double *)FetchSettingName(ww->fillcolour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_C, sizeof(double)),
                                              *(double *)FetchSettingName(ww->fillcolour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_M, sizeof(double)),
                                              *(double *)FetchSettingName(ww->fillcolour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_Y, sizeof(double)),
                                              *(double *)FetchSettingName(ww->fillcolour, SW_COLOUR_INT, (void *)SW_COLOUR_CMYK_K, sizeof(double)) );
  else                            strcpy (x->CurrentFillColour, ""); // If no fill colour is specified, we don't fill

  return;
 }

static char TempColour[256];

void eps_core_SwitchTo_FillColour(EPSComm *x)
 {
  strcpy(TempColour , x->CurrentColour); // Buffer the colour we're stroking with so we can restore it in eps_core_SwitchFrom_FillColour
  if ((strcmp(x->CurrentFillColour, x->LastPSColour) != 0) && (x->CurrentFillColour[0]!='\0'))
   { strcpy(x->LastPSColour, x->CurrentFillColour); fprintf(x->epsbuffer, "%s\n", x->LastPSColour); }
  strcpy(x->CurrentColour , x->CurrentFillColour); // This make the supression of invisible ink work...
  return;
 }

void eps_core_SwitchFrom_FillColour(EPSComm *x)
 {
  if ((strcmp(TempColour, x->LastPSColour) != 0) && (TempColour[0]!='\0'))
   { strcpy(x->LastPSColour, TempColour); fprintf(x->epsbuffer, "%s\n", x->LastPSColour); }
  strcpy(x->CurrentColour, TempColour); // Restore the colour we're stroking with
  return;
 }

// Set the linewidth of the EPS we are painting
void eps_core_SetLinewidth(EPSComm *x, double lw, int lt, double offset)
 {
  if ((lw == x->LastLinewidth) && (lt == x->LastLinetype) && (offset==0.0)) return;
  if (lw != x->LastLinewidth) fprintf(x->epsbuffer, "%f setlinewidth\n", lw);
  fprintf(x->epsbuffer, "%s\n", eps_LineType(lt, lw, offset));
  x->LastLinewidth = lw;
  x->LastLinetype  = lt;
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

// Update plot item bounding box
void eps_core_PlotBoundingBox(EPSComm *x, double xpos, double ypos, double lw)
 {
  if (x->current->PlotLeftMargin   > (xpos-lw/2)) x->current->PlotLeftMargin   = (xpos-lw/2);
  if (x->current->PlotRightMargin  < (xpos+lw/2)) x->current->PlotRightMargin  = (xpos+lw/2);
  if (x->current->PlotBottomMargin > (ypos-lw/2)) x->current->PlotBottomMargin = (ypos-lw/2);
  if (x->current->PlotTopMargin    < (ypos+lw/2)) x->current->PlotTopMargin    = (ypos+lw/2);
  eps_core_BoundingBox(x, xpos, ypos, lw);
  return;
 }

