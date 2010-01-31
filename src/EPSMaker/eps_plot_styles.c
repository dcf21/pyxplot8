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

#define _PPL_EPS_PLOT_STYLES_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_colours.h"
#include "eps_plot.h"
#include "eps_plot_canvas.h"
#include "eps_plot_linedraw.h"
#include "eps_plot_styles.h"
#include "eps_plot_threedimbuff.h"
#include "eps_settings.h"
#include "eps_style.h"

// Work out the default set of with words for a plot item
void eps_withwords_default(with_words *output, settings_graph *sg, unsigned char functions, int Fcounter, int Dcounter, unsigned char colour)
 {
  int i,j;

  with_words_zero(output, 0);
  if (!colour) { output->colour = COLOUR_BLACK; output->USEcolour = 1; }
  else
   {
    for (j=0; j<PALETTE_LENGTH; j++) if (settings_palette_current[j]==-1) break; // j now contains length of palette
    i = (functions ? Fcounter : Dcounter) % j; // i is now the palette colour number to use
    while (i<0) i+=j;
    if (settings_palette_current[i] > 0) { output->colour  = settings_palette_current[i]; output->USEcolour = 1; }
    else                                 { output->colourR = settings_paletteR_current[i]; output->colourG = settings_paletteG_current[i]; output->colourB = settings_paletteB_current[i]; output->USEcolourRGB = 1; }
   }
  output->linespoints    = (functions ? SW_STYLE_LINES : SW_STYLE_POINTS);
  output->USElinespoints = 1;
  output->fillcolour     = COLOUR_NULL;
  output->USEfillcolour  = 1;
  output->linetype       = colour ? 1 : (functions ? Fcounter+1 : Dcounter+1);
  output->pointtype      =              (functions ? Fcounter+1 : Dcounter+1);
  output->USElinetype    = output->USEpointtype = 1;
  output->linewidth      = sg->LineWidth;
  output->pointlinewidth = sg->PointLineWidth;
  output->pointsize      = sg->PointSize;
  output->USElinewidth   = output->USEpointlinewidth = output->USEpointsize = 1;
  return;
 }

// Return the number of columns of data which are required to plot in any given plot style
int eps_plot_styles_NDataColumns(int style, unsigned char ThreeDim)
 {
  if      (style == SW_STYLE_POINTS         ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_LINES          ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_LINESPOINTS    ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_XERRORBARS     ) return 3 + (ThreeDim!=0);
  else if (style == SW_STYLE_YERRORBARS     ) return 3 + (ThreeDim!=0);
  else if (style == SW_STYLE_ZERRORBARS     ) return 3 + 1;
  else if (style == SW_STYLE_XYERRORBARS    ) return 4 + (ThreeDim!=0);
  else if (style == SW_STYLE_XZERRORBARS    ) return 4 + 1;
  else if (style == SW_STYLE_YZERRORBARS    ) return 4 + 1;
  else if (style == SW_STYLE_XYZERRORBARS   ) return 5 + 1;
  else if (style == SW_STYLE_XERRORRANGE    ) return 4 + (ThreeDim!=0);
  else if (style == SW_STYLE_YERRORRANGE    ) return 4 + (ThreeDim!=0);
  else if (style == SW_STYLE_ZERRORRANGE    ) return 4 + 1;
  else if (style == SW_STYLE_XYERRORRANGE   ) return 6 + (ThreeDim!=0);
  else if (style == SW_STYLE_XZERRORRANGE   ) return 6 + 1;
  else if (style == SW_STYLE_YZERRORRANGE   ) return 6 + 1;
  else if (style == SW_STYLE_XYZERRORRANGE  ) return 8 + 1;
  else if (style == SW_STYLE_FILLEDREGION   ) return 2;
  else if (style == SW_STYLE_YERRORSHADED   ) return 3;
  else if (style == SW_STYLE_UPPERLIMITS    ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_LOWERLIMITS    ) return 2 + (ThreeDim!=0);
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

  ppl_fatal(__FILE__,__LINE__,"Unrecognised style type passed to eps_plot_styles_NDataColumns()");
  return -1;
 }

// UpdateUsage... get content of row X from data table
#define UUR(X) blk->data_real[X + Ncolumns*j].d

// UpdateUsage... update axis X with ordinate value Y
#define UUU(X,Y) \
 z = Y; \
 if ( ((!X->MinUsedSet) || (X->MinUsed > z)) && ((X->log != SW_BOOL_TRUE) || (z>0.0)) ) { X->MinUsedSet=1; X->MinUsed=z; } \
 if ( ((!X->MaxUsedSet) || (X->MaxUsed < z)) && ((X->log != SW_BOOL_TRUE) || (z>0.0)) ) { X->MaxUsedSet=1; X->MaxUsed=z; }

// UpdateUsage... get physical unit of row X from data table
#define UURU(X) data->FirstEntries[X]

// UpdateUsage... assert that axis X should be dimensionally compatible with unit Y
#define UUAU(XYZ,XYZN,X,Y) \
 if ((X->DataUnitSet) && (!ppl_units_DimEqual(&X->DataUnit , &(Y)))) { sprintf(temp_err_string, "Axis %c%d on plot %d has data plotted against it with conflicting physical units of <%s> and <%s>.", "xyz"[XYZ], XYZN, id,  ppl_units_GetUnitStr(&X->DataUnit,NULL,NULL,0,0),  ppl_units_GetUnitStr(&(Y),NULL,NULL,1,0)); ppl_error(ERR_GENERAL, temp_err_string); return 1; } \
 if (!X->DataUnitSet) \
  { \
   X->DataUnitSet = 1; \
   X->DataUnit = Y; \
  }

// Update the usage of axes to include data from a particular data table, plotted in a particular style
int eps_plot_styles_UpdateUsage(DataTable *data, int style, unsigned char ThreeDim, settings_axis *a1, settings_axis *a2, settings_axis *a3, int xyz1, int xyz2, int xyz3, int n1, int n2, int n3, int id)
 {
  int i, j, Ncolumns;
  double z;
  DataBlock *blk;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present

  // Cycle through data table acting upon the physical units of all of the columns
  if      (style == SW_STYLE_POINTS         ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_LINES          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_LINESPOINTS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_XERRORBARS     ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_YERRORBARS     ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_ZERRORBARS     ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(3)); }
  else if (style == SW_STYLE_XYERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_XZERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz3,n3,a3,UURU(4)); }
  else if (style == SW_STYLE_YZERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz2,n2,a2,UURU(3)); UUAU(xyz3,n3,a3,UURU(4)); }
  else if (style == SW_STYLE_XERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz1,n1,a1,UURU(3+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_YERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_ZERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(3)); UUAU(xyz3,n3,a3,UURU(4)); }
  else if (style == SW_STYLE_XYERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz1,n1,a1,UURU(3+ThreeDim)); UUAU(xyz2,n2,a2,UURU(4+ThreeDim)); UUAU(xyz2,n2,a2,UURU(5+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_XZERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz1,n1,a1,UURU(4)); UUAU(xyz3,n3,a3,UURU(5)); UUAU(xyz3,n3,a3,UURU(6)); }
  else if (style == SW_STYLE_YZERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz2,n2,a2,UURU(3)); UUAU(xyz2,n2,a2,UURU(4)); UUAU(xyz3,n3,a3,UURU(5)); UUAU(xyz3,n3,a3,UURU(6)); }
  else if (style == SW_STYLE_XYZERRORRANGE  ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz1,n1,a1,UURU(4)); UUAU(xyz2,n2,a2,UURU(5)); UUAU(xyz2,n2,a2,UURU(6)); UUAU(xyz3,n3,a3,UURU(7)); UUAU(xyz3,n3,a3,UURU(8)); }
  else if (style == SW_STYLE_FILLEDREGION   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_YERRORSHADED   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2)); UUAU(xyz2,n2,a2,UURU(3)); }
  else if (style == SW_STYLE_UPPERLIMITS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_LOWERLIMITS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_DOTS           ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_IMPULSES       ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_BOXES          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_WBOXES         ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2)); }
  else if (style == SW_STYLE_STEPS          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_FSTEPS         ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_HISTEPS        ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_ARROWS_HEAD    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(5)); } }
  else if (style == SW_STYLE_ARROWS_NOHEAD  ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(5)); } }
  else if (style == SW_STYLE_ARROWS_TWOHEAD ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(5)); } }

  // Cycle through data table, ensuring that axis ranges are sufficient to include all data
  Ncolumns = data->Ncolumns;
  blk = data->first;
  i=0;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      if      (style == SW_STYLE_POINTS         ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_LINES          ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_LINESPOINTS    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XERRORBARS     ) { UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2+ThreeDim)); UUU(a1, UUR(0)+UUR(2+ThreeDim)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_YERRORBARS     ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(2+ThreeDim)); UUU(a2, UUR(1)+UUR(2+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_ZERRORBARS     ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(3)); UUU(a3, UUR(2)+UUR(3)); }
      else if (style == SW_STYLE_XYERRORBARS    ) { UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2+ThreeDim)); UUU(a1, UUR(0)+UUR(2+ThreeDim)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(3+ThreeDim)); UUU(a2, UUR(1)+UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XZERRORBARS    ) { UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(3)); UUU(a1, UUR(0)+UUR(3)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(4)); UUU(a3, UUR(2)+UUR(4)); }
      else if (style == SW_STYLE_XZERRORBARS    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(3)); UUU(a2, UUR(1)+UUR(3)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(4)); UUU(a3, UUR(2)+UUR(4)); }
      else if (style == SW_STYLE_XZERRORBARS    ) { UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(3)); UUU(a1, UUR(0)+UUR(3)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(4)); UUU(a2, UUR(1)+UUR(4)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(5)); UUU(a3, UUR(2)+UUR(5)); }
      else if (style == SW_STYLE_XERRORRANGE    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a1, UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_YERRORRANGE    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_ZERRORRANGE    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(3)); UUU(a3, UUR(4)); }
      else if (style == SW_STYLE_XYERRORRANGE   ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a1, UUR(3+ThreeDim)); UUU(a2, UUR(4+ThreeDim)); UUU(a2, UUR(5+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XZERRORRANGE   ) { UUU(a1, UUR(0)); UUU(a1, UUR(3)); UUU(a1, UUR(4)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(5)); UUU(a3, UUR(6)); }
      else if (style == SW_STYLE_YZERRORRANGE   ) { UUU(a1, UUR(0)); UUU(a1, UUR(3)); UUU(a1, UUR(4)); UUU(a2, UUR(1)); UUU(a2, UUR(5)); UUU(a2, UUR(6)); UUU(a3, UUR(2)); UUU(a3, UUR(7)); UUU(a3, UUR(8)); }
      else if (style == SW_STYLE_FILLEDREGION   ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_YERRORSHADED   ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(2)); UUU(a2, UUR(3)); }
      else if (style == SW_STYLE_LOWERLIMITS    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_UPPERLIMITS    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_DOTS           ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_IMPULSES       ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_BOXES          ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_WBOXES         ) { UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2)); UUU(a1, UUR(0)+UUR(2)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_STEPS          ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_FSTEPS         ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_HISTEPS        ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_ARROWS_HEAD    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) { UUU(a3, UUR(2)); UUU(a3, UUR(5)); } }
      else if (style == SW_STYLE_ARROWS_NOHEAD  ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) { UUU(a3, UUR(2)); UUU(a3, UUR(5)); } }
      else if (style == SW_STYLE_ARROWS_TWOHEAD ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) { UUU(a3, UUR(2)); UUU(a3, UUR(5)); } }
      i++;
     }
    blk=blk->next;
   }

  return 0;
 }

// Render a dataset to postscript
int  eps_plot_dataset(EPSComm *x, DataTable *data, int style, unsigned char ThreeDim, settings_axis *a1, settings_axis *a2, settings_axis *a3, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, double origin_x, double origin_y, double width, double height)
 {
  int             i, j, Ncolumns, pt, xrn, yrn, zrn;
  double          xpos, ypos, depth, xap, yap, zap, scale_x, scale_y, scale_z;
  char            epsbuff[FNAME_LENGTH], *last_colstr=NULL;
  LineDrawHandle *ld;
  settings_axis  *a[3] = {a1,a2,a3};
  DataBlock      *blk;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present

  Ncolumns = data->Ncolumns;
  if (eps_plot_WithWordsCheckUsingItemsDimLess(&pd->ww_final, data->FirstEntries, Ncolumns)) return 1;

  if (!ThreeDim) { scale_x=width; scale_y=height; scale_z=1.0;    }
  else           { scale_x=width; scale_y=width ; scale_z=height; }

  // If axes have value-turning points, loop over all monotonic regions of axis space
  for (xrn=0; xrn<=a[xn]->AxisValueTurnings; xrn++)
  for (yrn=0; yrn<=a[yn]->AxisValueTurnings; yrn++)
  for (zrn=0; zrn<=(ThreeDim ? a[zn]->AxisValueTurnings : 0); zrn++)
    {

  blk = data->first;

  if ((style == SW_STYLE_LINES) || (style == SW_STYLE_LINESPOINTS)) // LINES
   {
    ld = LineDraw_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);
    last_colstr=NULL;

    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        // Work out style information for next point
        eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
        eps_core_SetColour(x, &pd->ww_final, 0);
        if (blk->split[j]) { LineDraw_PenUp(ld); }
        IF_NOT_INVISIBLE
         {
          if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); }
          LineDraw_Point(ld, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
         } else { LineDraw_PenUp(ld); }
       }
      blk=blk->next;
     }
    LineDraw_PenUp(ld);
    strcpy(x->LastEPSColour, ""); // Nullify last EPS colour
   }

  if ((style == SW_STYLE_POINTS) || (style == SW_STYLE_LINESPOINTS) || (style == SW_STYLE_DOTS)) // POINTS
   {
    double final_pointsize = pd->ww_final.pointsize;
    if (style == SW_STYLE_DOTS) final_pointsize *= 0.05; // Dots are 1/20th size of points
    last_colstr=NULL;

    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, NULL, NULL, NULL, ThreeDim, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, origin_x, origin_y, scale_x, scale_y, scale_z, 0);
        if (!gsl_finite(xpos)) continue; // Position of point is off side of graph

        // Work out style information for next point
        eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
        eps_core_SetColour(x, &pd->ww_final, 0);
        IF_NOT_INVISIBLE
         {
          if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); }
          pt = (style == SW_STYLE_DOTS) ? 9 : ((pd->ww_final.pointtype-1) % N_POINTTYPES); // Dots are always pt 9 (circle)
          while (pt<0) pt+=N_POINTTYPES;
          x->PointTypesUsed[pt] = 1;
          sprintf(epsbuff, "%.2f %.2f pt%d", xpos, ypos, pt+1);
          eps_core_BoundingBox(x, xpos, ypos, 2 * final_pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS);
          ThreeDimBuffer_writeps(x, depth, 1, pd->ww_final.pointlinewidth, final_pointsize, last_colstr, epsbuff);
         }

        // label point if instructed to do so
        if ((blk->text[j] != NULL) && (blk->text[j][0] != '\0'))
         {
          canvas_EPSRenderTextItem(x, x->LaTeXpageno++,
             xpos/M_TO_PS - (x->current->settings.TextHAlign - SW_HALIGN_CENT) * final_pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS / M_TO_PS * 1.1,
             ypos/M_TO_PS + (x->current->settings.TextVAlign - SW_VALIGN_CENT) * final_pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS / M_TO_PS * 1.1,
             x->current->settings.TextHAlign, x->current->settings.TextVAlign, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
         }
       }
      blk=blk->next;
     }
   }

  if ((style == SW_STYLE_XERRORBARS) || (style == SW_STYLE_YERRORBARS) || (style == SW_STYLE_ZERRORBARS) || (style == SW_STYLE_XYERRORBARS) || (style == SW_STYLE_YZERRORBARS) || (style == SW_STYLE_XZERRORBARS) || (style == SW_STYLE_XYZERRORBARS) || (style == SW_STYLE_XERRORRANGE) || (style == SW_STYLE_YERRORRANGE) || (style == SW_STYLE_ZERRORRANGE) || (style == SW_STYLE_XYERRORRANGE) || (style == SW_STYLE_YZERRORRANGE) || (style == SW_STYLE_XZERRORRANGE) || (style == SW_STYLE_XYZERRORRANGE)) // XERRORBARS , YERRORBARS , ZERRORBARS
   {
    unsigned char ac[3]={0,0,0};
    double b,ps,min[3],max[3];

    b  = 0.0005 * sg->bar;
    ps = pd->ww_final.pointsize * EPS_DEFAULT_PS / M_TO_PS;

    ld = LineDraw_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);
    last_colstr=NULL;

    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        // Work out style information for next point
        eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
        eps_core_SetColour(x, &pd->ww_final, 0);
        LineDraw_PenUp(ld);
        IF_NOT_INVISIBLE
         {
          for (i=0;i<3;i++) { min[i]=max[i]=UUR(i); }
          if      (style == SW_STYLE_XERRORBARS   ) { ac[0]=1; min[0] = UUR(0) - UUR(2+ThreeDim); max[0] = UUR(0) + UUR(2+ThreeDim); }
          else if (style == SW_STYLE_YERRORBARS   ) { ac[1]=1; min[1] = UUR(1) - UUR(2+ThreeDim); max[1] = UUR(1) + UUR(2+ThreeDim); }
          else if (style == SW_STYLE_ZERRORBARS   ) { ac[2]=1; min[2] = UUR(2) - UUR(3         ); max[2] = UUR(2) + UUR(3         ); }
          else if (style == SW_STYLE_XERRORRANGE  ) { ac[0]=1; min[0] = UUR(2+ThreeDim); max[0] = UUR(3+ThreeDim); }
          else if (style == SW_STYLE_YERRORRANGE  ) { ac[1]=1; min[1] = UUR(2+ThreeDim); max[1] = UUR(3+ThreeDim); }
          else if (style == SW_STYLE_ZERRORRANGE  ) { ac[2]=1; min[2] = UUR(3         ); max[2] = UUR(4         ); }
          else if (style == SW_STYLE_XYERRORBARS  ) { ac[0]=ac[1]=1; min[0] = UUR(0) - UUR(2+ThreeDim); max[0] = UUR(0) + UUR(2+ThreeDim); min[1] = UUR(1) - UUR(3+ThreeDim); max[1] = UUR(1) + UUR(3+ThreeDim); }
          else if (style == SW_STYLE_XZERRORBARS  ) { ac[0]=ac[2]=1; min[0] = UUR(0) - UUR(3         ); max[0] = UUR(0) + UUR(3         ); min[2] = UUR(2) - UUR(4         ); max[2] = UUR(2) + UUR(4         ); }
          else if (style == SW_STYLE_YZERRORBARS  ) { ac[1]=ac[2]=1; min[1] = UUR(1) - UUR(3         ); max[1] = UUR(1) + UUR(3         ); min[2] = UUR(2) - UUR(4         ); max[2] = UUR(2) + UUR(4         ); }
          else if (style == SW_STYLE_XYERRORRANGE ) { ac[0]=ac[1]=1; min[0] = UUR(2+ThreeDim); max[0] = UUR(3+ThreeDim); min[1] = UUR(4+ThreeDim); max[1] = UUR(5+ThreeDim); }
          else if (style == SW_STYLE_YZERRORRANGE ) { ac[1]=ac[2]=1; min[1] = UUR(3         ); max[1] = UUR(4         ); min[2] = UUR(5         ); max[2] = UUR(6         ); }
          else if (style == SW_STYLE_XZERRORRANGE ) { ac[0]=ac[2]=1; min[0] = UUR(3         ); max[0] = UUR(4         ); min[2] = UUR(5         ); max[2] = UUR(6         ); }
          else if (style == SW_STYLE_XYZERRORBARS ) { ac[0]=ac[1]=ac[2]=1; min[0] = UUR(0) - UUR(3); max[0] = UUR(0) + UUR(3); min[1] = UUR(1) - UUR(4); max[1] = UUR(1) + UUR(4); min[2] = UUR(2) - UUR(5); max[2] = UUR(2) + UUR(6); }
          else if (style == SW_STYLE_XYZERRORRANGE) { ac[0]=ac[1]=ac[2]=1; min[0] = UUR(3); max[0] = UUR(4); min[1] = UUR(5); max[1] = UUR(6); min[2] = UUR(7); max[2] = UUR(8); }

          if (a[0]->log==SW_BOOL_TRUE) { if (min[0]<DBL_MIN) min[0]=DBL_MIN; if (max[0]<DBL_MIN) max[0]=DBL_MIN; }
          if (a[1]->log==SW_BOOL_TRUE) { if (min[1]<DBL_MIN) min[1]=DBL_MIN; if (max[1]<DBL_MIN) max[1]=DBL_MIN; }
          if (a[2]->log==SW_BOOL_TRUE) { if (min[2]<DBL_MIN) min[2]=DBL_MIN; if (max[2]<DBL_MIN) max[2]=DBL_MIN; }

          if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); }

          for (i=0; i<3; i++) if (ac[i])
           {
            LineDraw_Point(ld, (i==xn)?(min[xn]):(UUR(xn)), (i==yn)?(min[yn]):(UUR(yn)), ThreeDim ? ((i==zn)?(min[zn]):(UUR(zn))) : 0.0, 0,0,0,
                  (i==xn)?( b ):0, (i==yn)?( b ):0, (i==zn)?( b ):0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_Point(ld, (i==xn)?(min[xn]):(UUR(xn)), (i==yn)?(min[yn]):(UUR(yn)), ThreeDim ? ((i==zn)?(min[zn]):(UUR(zn))) : 0.0, 0,0,0,
                  (i==xn)?(-b ):0, (i==yn)?(-b ):0, (i==zn)?(-b ):0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_PenUp(ld);
            LineDraw_Point(ld, (i==xn)?(min[xn]):(UUR(xn)), (i==yn)?(min[yn]):(UUR(yn)), ThreeDim ? ((i==zn)?(min[zn]):(UUR(zn))) : 0.0, 0,0,0,
                                0,               0,               0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_Point(ld, (i==xn)?(max[xn]):(UUR(xn)), (i==yn)?(max[yn]):(UUR(yn)), ThreeDim ? ((i==zn)?(max[zn]):(UUR(zn))) : 0.0, 0,0,0,
                                0,               0,               0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_PenUp(ld);
            LineDraw_Point(ld, (i==xn)?(max[xn]):(UUR(xn)), (i==yn)?(max[yn]):(UUR(yn)), ThreeDim ? ((i==zn)?(max[zn]):(UUR(zn))) : 0.0, 0,0,0,
                  (i==xn)?( b ):0, (i==yn)?( b ):0, (i==zn)?( b ):0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_Point(ld, (i==xn)?(max[xn]):(UUR(xn)), (i==yn)?(max[yn]):(UUR(yn)), ThreeDim ? ((i==zn)?(max[zn]):(UUR(zn))) : 0.0, 0,0,0,
                  (i==xn)?(-b ):0, (i==yn)?(-b ):0, (i==zn)?(-b ):0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_PenUp(ld);
            LineDraw_Point(ld,                   (UUR(xn)),                   (UUR(yn)), ThreeDim ? (                   UUR(zn) ) : 0.0, 0,0,0,
                  (i==xn)?( ps):0, (i==yn)?( ps):0, (i==zn)?( ps):0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_Point(ld,                   (UUR(xn)),                   (UUR(yn)), ThreeDim ? (                   UUR(zn) ) : 0.0, 0,0,0,
                  (i==xn)?(-ps):0, (i==yn)?(-ps):0, (i==zn)?(-ps):0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_PenUp(ld);
           }
         }

        // label point if instructed to do so
        if ((blk->text[j] != NULL) && (blk->text[j][0] != '\0'))
         {
          eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, NULL, NULL, NULL, ThreeDim, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, origin_x, origin_y, scale_x, scale_y, scale_z, 0);
          if (!gsl_finite(xpos)) continue; // Position of point is off side of graph
          canvas_EPSRenderTextItem(x, x->LaTeXpageno++, xpos/M_TO_PS, ypos/M_TO_PS, x->current->settings.TextHAlign, x->current->settings.TextVAlign, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
         }
       }
      blk=blk->next;
     }
    strcpy(x->LastEPSColour, ""); // Nullify last EPS colour
   }

  else if (style == SW_STYLE_IMPULSES       ) // IMPULSES
   {
    ld = LineDraw_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);
    last_colstr=NULL;

    if (a[yn]->DataUnitSet && (!ppl_units_DimEqual(&sg->BoxFrom, &a[yn]->DataUnit))) { sprintf(temp_err_string, "Data with units of <%s> plotted with impulses when BoxFrom is set to a value with units of <%s>.", ppl_units_GetUnitStr(&a[yn]->DataUnit,NULL,NULL,0,0),  ppl_units_GetUnitStr(&sg->BoxFrom,NULL,NULL,1,0)); ppl_error(ERR_GENERAL, temp_err_string); }
    else
     {
      while (blk != NULL)
       {
        for (j=0; j<blk->BlockPosition; j++)
         {
          // Work out style information for next point
          eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
          eps_core_SetColour(x, &pd->ww_final, 0);
          LineDraw_PenUp(ld);
          IF_NOT_INVISIBLE
           {
            if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); }
            LineDraw_Point(ld, UUR(xn), sg->BoxFrom.real, ThreeDim ? UUR(zn) : 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
            LineDraw_Point(ld, UUR(xn), UUR(yn)         , ThreeDim ? UUR(zn) : 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr);
           }
          LineDraw_PenUp(ld);

          // label point if instructed to do so
          if ((blk->text[j] != NULL) && (blk->text[j][0] != '\0'))
           {
            eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, NULL, NULL, NULL, ThreeDim, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, origin_x, origin_y, scale_x, scale_y, scale_z, 0);
            if (!gsl_finite(xpos)) continue; // Position of point is off side of graph
            canvas_EPSRenderTextItem(x, x->LaTeXpageno++, xpos/M_TO_PS, ypos/M_TO_PS, x->current->settings.TextHAlign, x->current->settings.TextVAlign, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
           }
         }
        blk=blk->next;
       }
     }
    strcpy(x->LastEPSColour, ""); // Nullify last EPS colour
   }

  else if ((style == SW_STYLE_LOWERLIMITS) || (style == SW_STYLE_UPPERLIMITS)) // LOWERLIMITS, UPPERLIMITS
   {
    double ps, sgn, lw, theta_y, x2, y2, x3, y3, x4, y4, x5, y5;
    ps  = pd->ww_final.pointsize * EPS_DEFAULT_PS / M_TO_PS;
    sgn = ( (style == SW_STYLE_UPPERLIMITS) ^ (a[yn]->MaxFinal > a[yn]->MinFinal) ) ? 1.0 : -1.0;
    lw  = pd->ww_final.pointlinewidth;

    ld = LineDraw_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);
    last_colstr=NULL;

    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        // Work out style information for next point
        eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
        eps_core_SetColour(x, &pd->ww_final, 0);
        LineDraw_PenUp(ld);
        IF_NOT_INVISIBLE
         {
          eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, NULL, &theta_y, NULL, ThreeDim, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, origin_x, origin_y, scale_x, scale_y, scale_z, 0);
          if (!gsl_finite(xpos)) continue; // Position of point is off side of graph

          if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); }
          LineDraw_Point(ld, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0,-ps,       0,0,0,0,0, pd->ww_final.linetype, lw, last_colstr);
          LineDraw_Point(ld, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, ps,       0,0,0,0,0, pd->ww_final.linetype, lw, last_colstr);
          LineDraw_PenUp(ld);
          LineDraw_Point(ld, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0,  0,       0,0,0,0,0, pd->ww_final.linetype, lw, last_colstr);
          LineDraw_Point(ld, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0,  0,sgn*2*ps,0,0,0,0, pd->ww_final.linetype, lw, last_colstr);
          LineDraw_PenUp(ld);
          x2 = xpos + sgn*2*ps*M_TO_PS * sin(theta_y);
          y2 = ypos + sgn*2*ps*M_TO_PS * cos(theta_y);
          if (sgn<0.0) theta_y += M_PI;
          x3 = x2 - EPS_ARROW_HEADSIZE/2 * lw * sin(theta_y - EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on one side
          y3 = y2 - EPS_ARROW_HEADSIZE/2 * lw * cos(theta_y - EPS_ARROW_ANGLE / 2);
          x5 = x2 - EPS_ARROW_HEADSIZE/2 * lw * sin(theta_y + EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on other side
          y5 = y2 - EPS_ARROW_HEADSIZE/2 * lw * cos(theta_y + EPS_ARROW_ANGLE / 2);
          x4 = x2 - EPS_ARROW_HEADSIZE/2 * lw * sin(theta_y) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2); // Point where back of arrowhead crosses stalk
          y4 = y2 - EPS_ARROW_HEADSIZE/2 * lw * cos(theta_y) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2);
          sprintf(epsbuff, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nfill\n", x4,y4,x3,y3,x2,y2,x5,y5);
          ThreeDimBuffer_writeps(x, depth, 1, lw, 1.0, last_colstr, epsbuff);
         }

        // label point if instructed to do so
        if ((blk->text[j] != NULL) && (blk->text[j][0] != '\0'))
         {
          canvas_EPSRenderTextItem(x, x->LaTeXpageno++, xpos/M_TO_PS, ypos/M_TO_PS, x->current->settings.TextHAlign, x->current->settings.TextVAlign, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
         }
       }
      blk=blk->next;
     }
    strcpy(x->LastEPSColour, ""); // Nullify last EPS colour
   }

  else if ((style == SW_STYLE_BOXES) || (style == SW_STYLE_WBOXES) || (style == SW_STYLE_STEPS) || (style == SW_STYLE_FSTEPS) || (style == SW_STYLE_HISTEPS)) // BOXES, WBOXES, STEPS, FSTEPS, HISTEPS
   {
    double ptAx, ptAy, ptBx, ptBy, ptCx, ptCy;
    unsigned char ptAset=0, ptBset=0, ptCset=0;

    ld = LineDraw_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);
    last_colstr=NULL;

#define MAKE_STEP(X0,Y0,WIDTH) \
   IF_NOT_INVISIBLE \
    { \
     double x0=X0,y0=Y0,width=WIDTH; \
     if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); } \
     LineDraw_Point(ld, x0-width, y0, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, x0+width, y0, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
    } \

#define MAKE_BOX(X0,Y0,WIDTH) \
   IF_NOT_INVISIBLE \
    { \
     double x0=X0,y0=Y0,width=WIDTH; \
     if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); } \
     LineDraw_Point(ld, x0-width, sg->BoxFrom.real, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, x0-width, y0              , 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, x0+width, y0              , 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, x0+width, sg->BoxFrom.real, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, x0-width, sg->BoxFrom.real, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_PenUp(ld); \
    } \

    if (a[yn]->DataUnitSet && (!ppl_units_DimEqual(&sg->BoxFrom, &a[yn]->DataUnit))) { sprintf(temp_err_string, "Data with units of <%s> plotted as boxes/steps when BoxFrom is set to a value with units of <%s>.", ppl_units_GetUnitStr(&a[yn]->DataUnit,NULL,NULL,0,0),  ppl_units_GetUnitStr(&sg->BoxFrom,NULL,NULL,1,0)); ppl_error(ERR_GENERAL, temp_err_string); }
    else if (a[xn]->DataUnitSet && (sg->BoxWidth.real>DBL_MIN) && (!ppl_units_DimEqual(&sg->BoxWidth, &a[xn]->DataUnit))) { sprintf(temp_err_string, "Data with ordinate units of <%s> plotted as boxes/steps when BoxWidth is set to a value with units of <%s>.", ppl_units_GetUnitStr(&a[xn]->DataUnit,NULL,NULL,0,0),  ppl_units_GetUnitStr(&sg->BoxWidth,NULL,NULL,1,0)); ppl_error(ERR_GENERAL, temp_err_string); }
    else
     {
      while (blk != NULL)
       {
        for (j=0; j<blk->BlockPosition; j++)
         {
          ptAx=ptBx; ptAy=ptBy; ptAset=ptBset;
          ptBx=ptCx; ptBy=ptCy; ptBset=ptCset;
          ptCx=UUR(0); ptCy=UUR(1); ptCset=1;
          if (ptBset)
           {
            if (ptAset) // We are processing a box in the midst of many
             {
              if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { MAKE_BOX ((ptBx+(ptAx+ptBx)/2)/2, ptBy, (ptCx-ptAx)/4      ); }
              else if (style == SW_STYLE_BOXES)                                 { MAKE_BOX ( ptBx                 , ptBy, sg->BoxWidth.real/2); }
              else if (style == SW_STYLE_HISTEPS)                               { MAKE_STEP( ptBx                 , ptBy, (ptCx-ptAx)/4      ); }
              else if (style == SW_STYLE_STEPS)  { MAKE_STEP((ptAx+ptBx)/2, ptBy, (ptBx-ptAx)/2); }
              else if (style == SW_STYLE_FSTEPS) { MAKE_STEP((ptBx+ptCx)/2, ptBy, (ptCx-ptBx)/2); }
             }
            else // The first box/step we work out the width of
             {
              if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { MAKE_BOX ( ptBx                 , ptBy, (ptCx-ptBx)/2      ); }
              else if (style == SW_STYLE_BOXES)                                 { MAKE_BOX ( ptBx                 , ptBy, sg->BoxWidth.real/2); }
              else if (style == SW_STYLE_HISTEPS)                               { MAKE_STEP( ptBx                 , ptBy, (ptCx-ptBx)/2      ); }
              else if (style == SW_STYLE_STEPS)  { MAKE_STEP( ptBx        , ptBy, 0.0          ); }
              else if (style == SW_STYLE_FSTEPS) { MAKE_STEP((ptBx+ptCx)/2, ptBy, (ptCx-ptBx)/2); }
             }
           }
          // Work out style information for next point
          eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
          eps_core_SetColour(x, &pd->ww_final, 0);
          if (style == SW_STYLE_WBOXES) { MAKE_BOX(ptCx, ptCy, UUR(2)/2); }
         }
        blk=blk->next;
       }
      if (ptBset) // We have one final box/step to process
       {
              if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { MAKE_BOX ( ptCx                 , ptCy, (ptCx-ptBx)/2      ); }
              else if (style == SW_STYLE_BOXES)                                 { MAKE_BOX ( ptCx                 , ptCy, sg->BoxWidth.real/2); }
              else if (style == SW_STYLE_HISTEPS)                               { MAKE_STEP( ptCx                 , ptCy, (ptCx-ptBx)/2      ); }
              else if (style == SW_STYLE_STEPS)  { MAKE_STEP((ptBx+ptCx)/2, ptCy, (ptCx-ptBx)/2); }
              else if (style == SW_STYLE_FSTEPS) { MAKE_STEP( ptCx        , ptCy, 0.0          ); }
       }
      else // We have a dataset with only a single box/step
       {
        if     ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { MAKE_BOX (ptCx, ptCy, 0.5); }
        else if (style == SW_STYLE_BOXES)                                { MAKE_BOX (ptCx, ptCy, sg->BoxWidth.real/2); }
        else if (sg->BoxWidth.real<1e-200)                               { MAKE_STEP(ptCx, ptCy, 0.5); }
        else                                                             { MAKE_STEP(ptCx, ptCy, sg->BoxWidth.real/2); }
       }
     }
    LineDraw_PenUp(ld);
   }

  else if (style == SW_STYLE_ARROWS_HEAD    ) // ARROWS_HEAD
   {
   }

  else if (style == SW_STYLE_ARROWS_NOHEAD  ) // ARROWS_NOHEAD
   {
   }

  else if (style == SW_STYLE_ARROWS_TWOHEAD ) // ARROWS_TWOHEAD
   {
   }

  // End looping over monotonic regions of axis space
   }

  return 0;
 }

