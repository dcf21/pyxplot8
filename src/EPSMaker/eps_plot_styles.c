// eps_plot_styles.c
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

#define _PPL_EPS_STYLE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_colours.h"
#include "eps_plot_styles.h"

// Work out the default set of with words for a plot item
void eps_withwords_default(with_words *output, unsigned char functions, int Fcounter, int Dcounter, unsigned char colour)
 {
  int i;

  with_words_zero(output, 0);
  if (!colour) { output->colour = COLOUR_BLACK; output->USEcolour = 1; }
  else
   {
    for (i=0; i<PALETTE_LENGTH; i++) if (settings_palette_current[i]==-1) break; // i now contains length of palette
    i = (functions ? Fcounter : Dcounter) % i; // i is now the palette colour number to use
    if (settings_palette_current[i] > 0) { output->colour  = settings_palette_current[i]; output->USEcolour = 1; }
    else                                 { output->colourR = settings_paletteR_current[i]; output->colourG = settings_paletteG_current[i]; output->colourB = settings_paletteB_current[i]; output->USEcolourRGB = 1; }
   }
  output->linespoints    = (functions ? SW_STYLE_LINES : SW_STYLE_POINTS);
  output->USElinespoints = 1;
  output->fillcolour     = COLOUR_NULL;
  output->USEfillcolour  = 1;
  output->linetype = output->pointtype = (functions ? Fcounter : Dcounter);
  output->USElinetype = output->USEpointtype = 1;
  output->linewidth = output->pointlinewidth = output->pointsize = 1.0;
  output->USElinewidth = output->USEpointlinewidth = output->USEpointsize = 1;
  return;
 }

int eps_plot_styles_NDataColumns(int style, unsigned char ThreeDim)
 {
  if      (style == SW_STYLE_POINTS         ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_LINES          ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_LINESPOINTS    ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_XERRORBARS     ) return 3 + (ThreeDim!=0);
  else if (style == SW_STYLE_YERRORBARS     ) return 3 + (ThreeDim!=0);
  else if (style == SW_STYLE_XYERRORBARS    ) return 4 + (ThreeDim!=0);
  else if (style == SW_STYLE_XERRORRANGE    ) return 4 + (ThreeDim!=0);
  else if (style == SW_STYLE_YERRORRANGE    ) return 4 + (ThreeDim!=0);
  else if (style == SW_STYLE_XYERRORRANGE   ) return 6 + (ThreeDim!=0);
  else if (style == SW_STYLE_DOTS           ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_IMPULSES       ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_BOXES          ) return 2;
  else if (style == SW_STYLE_WBOXES         ) return 3;
  else if (style == SW_STYLE_STEPS          ) return 2;
  else if (style == SW_STYLE_FSTEPS         ) return 2;
  else if (style == SW_STYLE_HISTEPS        ) return 2;
  else if (style == SW_STYLE_ARROWS_HEAD    ) return 4 + 2*(ThreeDim!=0);
  else if (style == SW_STYLE_ARROWS_NOHEAD  ) return 4 + 2*(ThreeDim!=0);
  else if (style == SW_STYLE_ARROWS_TWOHEAD ) return 4 + 2*(ThreeDim!=0);
  else if (style == SW_STYLE_CSPLINES       ) return 2;
  else if (style == SW_STYLE_ACSPLINES      ) return 2;

  ppl_fatal(__FILE__,__LINE__,"Unrecognised style type passed to eps_plot_styles_NDataColumns()");
  return -1;
 }

int eps_plot_styles_UpdateUsage(DataTable *data, int style, unsigned char ThreeDim, settings_axis *xa, settings_axis *ya, settings_axis *za, canvas_plotdesc *pd)
 {
  return 0;
 }
