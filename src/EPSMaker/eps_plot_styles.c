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
#include "ppl_units.h"
#include "ppl_units_fns.h"

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

// UpdateUsage... get content of row X from data table
#define UUR(X) blk->data_real[X + Ncolumns*j]

// UpdateUsage... update axis X with ordinate value Y
#define UUU(X,Y) \
 z = Y; \
 if ( ((!X->MinUsedSet) || (X->MinUsed > z)) && ((X->log != SW_ONOFF_ON) || (z>0.0)) ) { X->MinUsedSet=1; X->MinUsed=z; } \
 if ( ((!X->MaxUsedSet) || (X->MaxUsed < z)) && ((X->log != SW_ONOFF_ON) || (z>0.0)) ) { X->MaxUsedSet=1; X->MaxUsed=z; }

// UpdateUsage... get physical unit of row X from data table
#define UURU(X) data->FirstEntries[X]

// UpdateUsage... assert that axis X should be dimensionally compatible with unit Y
#define UUAU(XYZ,XYZN,X,Y) \
 if ((X->DataUnitSet) && (!ppl_units_DimEqual(&X->DataUnit , &(Y)))) { sprintf(temp_err_string, "Axis %c%d on plot %d has data plotted against it with conflicting physical units of <%s> and <%s>.", XYZ, XYZN, id,  ppl_units_GetUnitStr(&X->DataUnit,NULL,NULL,0,0),  ppl_units_GetUnitStr(&(Y),NULL,NULL,1,0)); ppl_error(ERR_GENERAL, temp_err_string); return 1; } \
 if (!X->DataUnitSet) \
  { \
   X->DataUnitSet = 1; \
   X->DataUnit = Y; \
  }

int eps_plot_styles_UpdateUsage(DataTable *data, int style, unsigned char ThreeDim, settings_axis *xa, settings_axis *ya, settings_axis *za, int xn, int yn, int zn, int id)
 {
  int i, j, Ncolumns;
  double z;
  DataBlock *blk;

  // Cycle through data table acting upon the physical units of all of the columns
  if      (style == SW_STYLE_POINTS         ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_LINES          ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_LINESPOINTS    ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_XERRORBARS     ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_YERRORBARS     ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('y',yn,ya,UURU(2+ThreeDim)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_XYERRORBARS    ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); UUAU('y',yn,ya,UURU(3+ThreeDim)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_XERRORRANGE    ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); UUAU('x',xn,xa,UURU(3+ThreeDim)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_YERRORRANGE    ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('y',yn,ya,UURU(2+ThreeDim)); UUAU('y',yn,ya,UURU(3+ThreeDim)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_XYERRORRANGE   ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); UUAU('x',xn,xa,UURU(3+ThreeDim)); UUAU('y',yn,ya,UURU(4+ThreeDim)); UUAU('y',yn,ya,UURU(5+ThreeDim)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_DOTS           ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_IMPULSES       ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_BOXES          ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); }
  else if (style == SW_STYLE_WBOXES         ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2)); }
  else if (style == SW_STYLE_STEPS          ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); }
  else if (style == SW_STYLE_FSTEPS         ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); }
  else if (style == SW_STYLE_HISTEPS        ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); }
  else if (style == SW_STYLE_ARROWS_HEAD    ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); UUAU('y',yn,ya,UURU(3+ThreeDim)); if (ThreeDim) { UUAU('z',zn,za,UURU(2)); UUAU('z',zn,za,UURU(5)); } }
  else if (style == SW_STYLE_ARROWS_NOHEAD  ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); UUAU('y',yn,ya,UURU(3+ThreeDim)); if (ThreeDim) { UUAU('z',zn,za,UURU(2)); UUAU('z',zn,za,UURU(5)); } }
  else if (style == SW_STYLE_ARROWS_TWOHEAD ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); UUAU('x',xn,xa,UURU(2+ThreeDim)); UUAU('y',yn,ya,UURU(3+ThreeDim)); if (ThreeDim) { UUAU('z',zn,za,UURU(2)); UUAU('z',zn,za,UURU(5)); } }
  else if (style == SW_STYLE_CSPLINES       ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }
  else if (style == SW_STYLE_ACSPLINES      ) { UUAU('x',xn,xa,UURU(0)); UUAU('y',yn,ya,UURU(1)); if (ThreeDim) UUAU('z',zn,za,UURU(2)); }

  // Cycle through data table, ensuring that axis ranges are sufficient to include all data
  Ncolumns = data->Ncolumns;
  blk = data->first;
  i=0;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      if      (style == SW_STYLE_POINTS         ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_LINES          ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_LINESPOINTS    ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_XERRORBARS     ) { UUU(xa, UUR(0)); UUU(xa, UUR(0)-UUR(2+ThreeDim)); UUU(xa, UUR(0)+UUR(2+ThreeDim)); UUU(ya, UUR(1)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_YERRORBARS     ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(ya, UUR(1)-UUR(2+ThreeDim)); UUU(ya, UUR(1)+UUR(2+ThreeDim)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_XYERRORBARS    ) { UUU(xa, UUR(0)); UUU(xa, UUR(0)-UUR(2+ThreeDim)); UUU(xa, UUR(0)+UUR(2+ThreeDim)); UUU(ya, UUR(1)); UUU(ya, UUR(1)-UUR(3+ThreeDim)); UUU(ya, UUR(1)+UUR(3+ThreeDim)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_XERRORRANGE    ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(xa, UUR(2+ThreeDim)); UUU(xa, UUR(3+ThreeDim)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_YERRORRANGE    ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(ya, UUR(2+ThreeDim)); UUU(ya, UUR(3+ThreeDim)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_XYERRORRANGE   ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(xa, UUR(2+ThreeDim)); UUU(xa, UUR(3+ThreeDim)); UUU(ya, UUR(4+ThreeDim)); UUU(ya, UUR(5+ThreeDim)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_DOTS           ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_IMPULSES       ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); if (ThreeDim) UUU(za, UUR(2)); }
      else if (style == SW_STYLE_BOXES          ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); }
      else if (style == SW_STYLE_WBOXES         ) { UUU(xa, UUR(0)); UUU(xa, UUR(0)-UUR(2)); UUU(xa, UUR(0)+UUR(2)); UUU(ya, UUR(1)); }
      else if (style == SW_STYLE_STEPS          ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); }
      else if (style == SW_STYLE_FSTEPS         ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); }
      else if (style == SW_STYLE_HISTEPS        ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); }
      else if (style == SW_STYLE_ARROWS_HEAD    ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(xa, UUR(2+ThreeDim)); UUU(ya, UUR(3+ThreeDim)); if (ThreeDim) { UUU(za, UUR(2)); UUU(za, UUR(5)); } }
      else if (style == SW_STYLE_ARROWS_NOHEAD  ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(xa, UUR(2+ThreeDim)); UUU(ya, UUR(3+ThreeDim)); if (ThreeDim) { UUU(za, UUR(2)); UUU(za, UUR(5)); } }
      else if (style == SW_STYLE_ARROWS_TWOHEAD ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); UUU(xa, UUR(2+ThreeDim)); UUU(ya, UUR(3+ThreeDim)); if (ThreeDim) { UUU(za, UUR(2)); UUU(za, UUR(5)); } }
      else if (style == SW_STYLE_CSPLINES       ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); }
      else if (style == SW_STYLE_ACSPLINES      ) { UUU(xa, UUR(0)); UUU(ya, UUR(1)); }
      i++;
     }
    blk=blk->next;
   }

  return 0;
 }

