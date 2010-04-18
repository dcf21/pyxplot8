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

#include "eps_arrow.h"
#include "eps_comm.h"
#include "eps_core.h"
#include "eps_colours.h"
#include "eps_plot.h"
#include "eps_plot_canvas.h"
#include "eps_plot_filledregion.h"
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
  else if (style == SW_STYLE_STARS          ) return 2 + (ThreeDim!=0);
  else if (style == SW_STYLE_ARROWS_HEAD    ) return 4 + 2*(ThreeDim!=0);
  else if (style == SW_STYLE_ARROWS_NOHEAD  ) return 4 + 2*(ThreeDim!=0);
  else if (style == SW_STYLE_ARROWS_TWOHEAD ) return 4 + 2*(ThreeDim!=0);

  ppl_fatal(__FILE__,__LINE__,"Unrecognised style type passed to eps_plot_styles_NDataColumns()");
  return -1;
 }

// UpdateUsage... get content of row X from data table
#define UUR(X) blk->data_real[X + Ncolumns*j].d

// UpdateUsage... check whether position is within range of axis
#define UUC(X,Y) \
{ \
 if (InRange && (!eps_plot_axis_InRange(X,Y))) InRange=0; \
}

// Memory recall on within-range flag, adding previous flag to ORed list of points to be checked
#define UUD(X,Y) \
{ \
 PartiallyInRange = PartiallyInRange || InRange; \
 InRange = InRangeMemory; \
 UUC(X,Y); \
}

// Store current within-range flag to memory
#define UUE(X,Y) \
{ \
 InRangeMemory = InRange; \
 UUC(X,Y); \
}

// Simultaneously update usage with UUU and check whether position is within range
#define UUF(X,Y) \
{ \
 UUC(X,logaxis?exp(Y):(Y)) \
 UUU(X,logaxis?exp(Y):(Y)) \
}

// Reset flags used to test whether a datapoint is within range before using it to update ranges of other axes
#define UUC_RESET \
{ \
 InRange=1; PartiallyInRange=0; InRangeMemory=1; \
}

// UpdateUsage... update axis X with ordinate value Y
#define UUU(X,Y) \
{ \
 if (InRange || PartiallyInRange) \
  { \
   z = Y; \
   if ( (gsl_finite(z)) && ((!X->MinUsedSet) || (X->MinUsed > z)) && ((X->log != SW_BOOL_TRUE) || (z>0.0)) ) { X->MinUsedSet=1; X->MinUsed=z; } \
   if ( (gsl_finite(z)) && ((!X->MaxUsedSet) || (X->MaxUsed < z)) && ((X->log != SW_BOOL_TRUE) || (z>0.0)) ) { X->MaxUsedSet=1; X->MaxUsed=z; } \
  } \
}

// UpdateUsage... get physical unit of row X from data table
#define UURU(X) data->FirstEntries[X]

// UpdateUsage... assert that axis X should be dimensionally compatible with unit Y
#define UUAU(XYZ,XYZN,X,Y) \
 if ((X->HardUnitSet) && (!ppl_units_DimEqual(&X->HardUnit , &(Y)))) { sprintf(temp_err_string, "Axis %c%d on plot %d has data plotted against it with conflicting physical units of <%s> as compared to range of axis, which has units of <%s>.", "xyz"[XYZ], XYZN, id,  ppl_units_GetUnitStr(&X->HardUnit,NULL,NULL,0,1,0),  ppl_units_GetUnitStr(&(Y),NULL,NULL,1,1,0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return 1; } \
 if ((X->DataUnitSet) && (!ppl_units_DimEqual(&X->DataUnit , &(Y)))) { sprintf(temp_err_string, "Axis %c%d on plot %d has data plotted against it with conflicting physical units of <%s> and <%s>.", "xyz"[XYZ], XYZN, id,  ppl_units_GetUnitStr(&X->DataUnit,NULL,NULL,0,1,0),  ppl_units_GetUnitStr(&(Y),NULL,NULL,1,1,0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return 1; } \
 if (!X->DataUnitSet) \
  { \
   X->DataUnitSet = 1; \
   X->DataUnit = Y; \
  }

// Update the usage of axes to include data from a particular data table, plotted in a particular style
int eps_plot_styles_UpdateUsage(DataTable *data, int style, unsigned char ThreeDim, settings_axis *a1, settings_axis *a2, settings_axis *a3, settings_graph *sg, int xyz1, int xyz2, int xyz3, int n1, int n2, int n3, int id)
 {
  int i, j, Ncolumns;
  double z;
  double ptAx, ptBx, ptCx, lasty;
  unsigned char ptAset=0, ptBset=0, ptCset=0;
  unsigned char InRange, PartiallyInRange, InRangeMemory;
  DataBlock *blk;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present

  // Cycle through data table acting upon the physical units of all of the columns
  if      (style == SW_STYLE_POINTS         ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_LINES          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_LINESPOINTS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_XERRORBARS     ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_YERRORBARS     ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_ZERRORBARS     ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(3)); }
  else if (style == SW_STYLE_XYERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_XZERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz3,n3,a3,UURU(4)); }
  else if (style == SW_STYLE_YZERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz2,n2,a2,UURU(3)); UUAU(xyz3,n3,a3,UURU(4)); }
  else if (style == SW_STYLE_XYZERRORBARS   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz2,n2,a2,UURU(4)); UUAU(xyz3,n3,a3,UURU(5)); }
  else if (style == SW_STYLE_XERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz1,n1,a1,UURU(3+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_YERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_ZERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz3,n3,a3,UURU(3)); UUAU(xyz3,n3,a3,UURU(4)); }
  else if (style == SW_STYLE_XYERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz1,n1,a1,UURU(3+ThreeDim)); UUAU(xyz2,n2,a2,UURU(4+ThreeDim)); UUAU(xyz2,n2,a2,UURU(5+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_XZERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz1,n1,a1,UURU(4)); UUAU(xyz3,n3,a3,UURU(5)); UUAU(xyz3,n3,a3,UURU(6)); }
  else if (style == SW_STYLE_YZERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz2,n2,a2,UURU(3)); UUAU(xyz2,n2,a2,UURU(4)); UUAU(xyz3,n3,a3,UURU(5)); UUAU(xyz3,n3,a3,UURU(6)); }
  else if (style == SW_STYLE_XYZERRORRANGE  ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz3,n3,a3,UURU(2)); UUAU(xyz1,n1,a1,UURU(3)); UUAU(xyz1,n1,a1,UURU(4)); UUAU(xyz2,n2,a2,UURU(5)); UUAU(xyz2,n2,a2,UURU(6)); UUAU(xyz3,n3,a3,UURU(7)); UUAU(xyz3,n3,a3,UURU(8)); }
  else if (style == SW_STYLE_FILLEDREGION   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_YERRORSHADED   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2)); }
  else if (style == SW_STYLE_UPPERLIMITS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_LOWERLIMITS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_DOTS           ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_IMPULSES       ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
  else if (style == SW_STYLE_BOXES          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_WBOXES         ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2)); }
  else if (style == SW_STYLE_STEPS          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_FSTEPS         ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_HISTEPS        ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); }
  else if (style == SW_STYLE_STARS          ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) { UUAU(xyz3,n3,a3,UURU(2)); } }
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
      UUC_RESET;
      if      (style == SW_STYLE_POINTS         ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_LINES          ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_LINESPOINTS    ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XERRORBARS     ) { UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2)); UUE(a1, UUR(0)-UUR(2+ThreeDim));
                                                                                                    UUD(a1, UUR(0)                );
                                                                                                    UUD(a1, UUR(0)+UUR(2+ThreeDim));
                                                    UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2+ThreeDim)); UUU(a1, UUR(0)+UUR(2+ThreeDim)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_YERRORBARS     ) { UUC(a1, UUR(0)); if (ThreeDim) UUC(a3, UUR(2)); UUE(a2, UUR(1)-UUR(2+ThreeDim));
                                                                                                    UUD(a2, UUR(1)                );
                                                                                                    UUD(a2, UUR(1)+UUR(2+ThreeDim));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(2+ThreeDim)); UUU(a2, UUR(1)+UUR(2+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_ZERRORBARS     ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); UUE(a3, UUR(2)); UUD(a3, UUR(2)-UUR(3)); UUD(a3, UUR(2)+UUR(3));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(3)); UUU(a3, UUR(2)+UUR(3)); }
      else if (style == SW_STYLE_XYERRORBARS    ) { if (ThreeDim) UUC(a3, UUR(2)); UUE(a1, UUR(0)                ); UUC(a2, UUR(1)-UUR(3+ThreeDim));
                                                                                   UUD(a1, UUR(0)                ); UUC(a2, UUR(1)                );
                                                                                   UUD(a1, UUR(0)                ); UUC(a2, UUR(1)+UUR(3+ThreeDim));
                                                                                   UUD(a1, UUR(0)-UUR(2+ThreeDim)); UUC(a2, UUR(1)                );
                                                                                   UUD(a1, UUR(0)+UUR(2+ThreeDim)); UUC(a2, UUR(1)                );
                                                    UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2+ThreeDim)); UUU(a1, UUR(0)+UUR(2+ThreeDim)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(3+ThreeDim)); UUU(a2, UUR(1)+UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XZERRORBARS    ) { UUC(a2, UUR(1)); UUE(a1, UUR(0)       ); UUC(a3, UUR(2)-UUR(4));
                                                                     UUD(a1, UUR(0)       ); UUC(a3, UUR(2)       );
                                                                     UUD(a1, UUR(0)       ); UUC(a3, UUR(2)+UUR(4));
                                                                     UUD(a1, UUR(0)-UUR(3)); UUC(a3, UUR(2)       );
                                                                     UUD(a1, UUR(0)+UUR(3)); UUC(a3, UUR(2)       );
                                                    UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(3)); UUU(a1, UUR(0)+UUR(3)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(4)); UUU(a3, UUR(2)+UUR(4)); }
      else if (style == SW_STYLE_YZERRORBARS    ) { UUC(a1, UUR(0)); UUE(a2, UUR(1)       ); UUC(a3, UUR(2)-UUR(4));
                                                                     UUD(a2, UUR(1)       ); UUC(a3, UUR(2)       );
                                                                     UUD(a2, UUR(1)       ); UUC(a3, UUR(2)+UUR(4));
                                                                     UUD(a2, UUR(1)-UUR(3)); UUC(a3, UUR(2)       );
                                                                     UUD(a2, UUR(1)+UUR(3)); UUC(a3, UUR(2)       );
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(3)); UUU(a2, UUR(1)+UUR(3)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(4)); UUU(a3, UUR(2)+UUR(4)); }
      else if (style == SW_STYLE_XYZERRORBARS   ) { UUC(a1, UUR(0)       ); UUC(a2, UUR(1)       ); UUC(a3, UUR(2)-UUR(5));
                                                    UUD(a1, UUR(0)       ); UUC(a2, UUR(1)       ); UUC(a3, UUR(2)       );
                                                    UUD(a1, UUR(0)       ); UUC(a2, UUR(1)       ); UUC(a3, UUR(2)+UUR(5));
                                                    UUD(a1, UUR(0)       ); UUC(a2, UUR(1)-UUR(4)); UUC(a3, UUR(2)       );
                                                    UUD(a1, UUR(0)       ); UUC(a2, UUR(1)+UUR(4)); UUC(a3, UUR(2)       );
                                                    UUD(a1, UUR(0)-UUR(3)); UUC(a2, UUR(1)       ); UUC(a3, UUR(2)       );
                                                    UUD(a1, UUR(0)+UUR(3)); UUC(a2, UUR(1)       ); UUC(a3, UUR(2)       );
                                                    UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(3)); UUU(a1, UUR(0)+UUR(3)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(4)); UUU(a2, UUR(1)+UUR(4)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(5)); UUU(a3, UUR(2)+UUR(5)); }
      else if (style == SW_STYLE_XERRORRANGE    ) { UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2)); UUE(a1, UUR(2+ThreeDim));
                                                                                                    UUD(a1, UUR(0)         );
                                                                                                    UUD(a1, UUR(3+ThreeDim));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a1, UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_YERRORRANGE    ) { UUC(a1, UUR(0)); if (ThreeDim) UUC(a3, UUR(2)); UUE(a2, UUR(2+ThreeDim));
                                                                                                    UUD(a2, UUR(1)         );
                                                                                                    UUD(a2, UUR(3+ThreeDim));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_ZERRORRANGE    ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); UUE(a3, UUR(2)); UUD(a3, UUR(3)); UUD(a3, UUR(4));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a3, UUR(2)); UUU(a3, UUR(3)); UUU(a3, UUR(4)); }
      else if (style == SW_STYLE_XYERRORRANGE   ) { if (ThreeDim) UUC(a3, UUR(2)); UUE(a1, UUR(0)         ); UUC(a2, UUR(4+ThreeDim));
                                                                                   UUD(a1, UUR(0)         ); UUC(a2, UUR(1)         );
                                                                                   UUD(a1, UUR(0)         ); UUC(a2, UUR(5+ThreeDim));
                                                                                   UUD(a1, UUR(2+ThreeDim)); UUC(a2, UUR(1)         );
                                                                                   UUD(a1, UUR(3+ThreeDim)); UUC(a2, UUR(1)         );
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a1, UUR(3+ThreeDim)); UUU(a2, UUR(4+ThreeDim)); UUU(a2, UUR(5+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XZERRORRANGE   ) { UUC(a2, UUR(1)); UUE(a1, UUR(0)); UUC(a3, UUR(5));
                                                                     UUD(a1, UUR(0)); UUC(a3, UUR(2));
                                                                     UUD(a1, UUR(0)); UUC(a3, UUR(6)); 
                                                                     UUD(a1, UUR(3)); UUC(a3, UUR(2)); 
                                                                     UUD(a1, UUR(4)); UUC(a3, UUR(2));
                                                    UUU(a2, UUR(1)); UUU(a1, UUR(0)); UUU(a1, UUR(3)); UUU(a1, UUR(4)); UUU(a3, UUR(2)); UUU(a3, UUR(5)); UUU(a3, UUR(6)); }
      else if (style == SW_STYLE_YZERRORRANGE   ) { UUC(a1, UUR(0)); UUE(a2, UUR(1)); UUC(a3, UUR(5));
                                                                     UUD(a2, UUR(1)); UUC(a3, UUR(2)); 
                                                                     UUD(a2, UUR(1)); UUC(a3, UUR(6)); 
                                                                     UUD(a2, UUR(3)); UUC(a3, UUR(2)); 
                                                                     UUD(a2, UUR(4)); UUC(a3, UUR(2)); 
                                                    UUU(a1, UUR(0)); UUU(a1, UUR(3)); UUU(a1, UUR(4)); UUU(a2, UUR(1)); UUU(a2, UUR(5)); UUU(a2, UUR(6)); UUU(a3, UUR(2)); UUU(a3, UUR(7)); UUU(a3, UUR(8)); }
      else if (style == SW_STYLE_XYZERRORRANGE  ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); UUC(a3, UUR(7));
                                                    UUD(a1, UUR(0)); UUC(a2, UUR(1)); UUC(a3, UUR(2));
                                                    UUD(a1, UUR(0)); UUC(a2, UUR(1)); UUC(a3, UUR(8));
                                                    UUD(a1, UUR(0)); UUC(a2, UUR(5)); UUC(a3, UUR(2));
                                                    UUD(a1, UUR(0)); UUC(a2, UUR(6)); UUC(a3, UUR(2));
                                                    UUD(a1, UUR(3)); UUC(a2, UUR(1)); UUC(a3, UUR(2));
                                                    UUD(a1, UUR(4)); UUC(a2, UUR(1)); UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(3)); UUU(a1, UUR(0)+UUR(3)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(4)); UUU(a2, UUR(1)+UUR(4)); UUU(a3, UUR(2)); UUU(a3, UUR(2)-UUR(5)); UUU(a3, UUR(2)+UUR(5)); }
      else if (style == SW_STYLE_FILLEDREGION   ) { UUC(a1, UUR(0)); UUC(a2, UUR(1));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_YERRORSHADED   ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); UUC(a2, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(2)); }
      else if (style == SW_STYLE_LOWERLIMITS    ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_UPPERLIMITS    ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_DOTS           ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_IMPULSES       ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_WBOXES         ) { UUC(a2, UUR(1)); UUE(a1, UUR(0)); UUD(a1, UUR(0)-UUR(2)); UUD(a1, UUR(0)+UUR(2));
                                                    UUU(a2, UUR(1)); UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2)); UUU(a1, UUR(0)+UUR(2)); }
      else if (style == SW_STYLE_STARS          ) { UUC(a1, UUR(0)); UUC(a2, UUR(1)); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if ((style == SW_STYLE_ARROWS_HEAD) || (style == SW_STYLE_ARROWS_NOHEAD) || (style == SW_STYLE_ARROWS_TWOHEAD))
                                                  { UUC(a1, UUR(0         )); UUC(a2, UUR(1         )); if (ThreeDim) UUC(a3, UUR(2));
                                                    UUD(a1, UUR(2+ThreeDim)); UUC(a2, UUR(3+ThreeDim)); if (ThreeDim) UUC(a3, UUR(5));
                                                    UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) { UUU(a3, UUR(2)); UUU(a3, UUR(5)); } }
      else if ((style == SW_STYLE_BOXES) || (style == SW_STYLE_STEPS) || (style == SW_STYLE_FSTEPS) || (style == SW_STYLE_HISTEPS))
       {
        // Boxes and steps need slightly more complicated logic to take into account finite width of boxes/steps
        unsigned char logaxis = (a1->log==SW_BOOL_TRUE);
        UUC(a2, UUR(1)); // y-coordinates are easy
        ptAx=ptBx; ptAset=ptBset;
        ptBx=ptCx; ptBset=ptCset;
        ptCx=logaxis?log(UUR(0)):(UUR(0)); ptCset=1;
        if (ptBset)
         {
          if (ptAset) // We are processing a box in the midst of many
           {
            if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { UUF(a1, ((ptBx+(ptAx+ptCx)/2)/2 - (ptCx-ptAx)/4)); UUF(a1, ((ptBx+(ptAx+ptCx)/2)/2 + (ptCx-ptAx)/4)); }
            else if (style == SW_STYLE_BOXES)                                 { UUF(a1, (ptBx - sg->BoxWidth.real/2)); UUF(a1, (ptBx + sg->BoxWidth.real/2)); }
            else if (style == SW_STYLE_HISTEPS)                               { UUF(a1, (ptBx - (ptCx-ptAx)/4      )); UUF(a1, (ptBx + (ptCx-ptAx)/4      )); }
            else if (style == SW_STYLE_STEPS)  { UUF(a1, ((ptAx+ptBx)/2 - (ptBx-ptAx)/2)); UUF(a1, ((ptAx+ptBx)/2 + (ptBx-ptAx)/2)); }
            else if (style == SW_STYLE_FSTEPS) { UUF(a1, ((ptBx+ptCx)/2 - (ptCx-ptBx)/2)); UUF(a1, ((ptBx+ptCx)/2 + (ptCx-ptBx)/2)); }
           }
          else // The first box/step we work out the width of
           {
            if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { UUF(a1, (ptBx - (ptCx-ptBx)/2));       UUF(a1, (ptBx + (ptCx-ptBx)/2)); }
            else if (style == SW_STYLE_BOXES)                                 { UUF(a1, (ptBx - sg->BoxWidth.real/2)); UUF(a1, (ptBx + sg->BoxWidth.real/2)); }
            else if (style == SW_STYLE_HISTEPS)                               { UUF(a1, (ptBx - (ptCx-ptBx)/2));       UUF(a1, (ptBx + (ptCx-ptBx)/2)); }
            else if (style == SW_STYLE_STEPS)  { UUF(a1, ptBx); }
            else if (style == SW_STYLE_FSTEPS) { UUF(a1, ((ptBx+ptCx)/2 - (ptCx-ptBx)/2)); UUF(a1, ((ptBx+ptCx)/2 + (ptCx-ptBx)/2)); }
           }
         }
        UUU(a2, UUR(1)); // y-coordinates are easy  
        lasty = UUR(1);
       }
      i++;
     }
    blk=blk->next;
   }

  // Logic to take account of final boxes/steps
  if (ptAset && ((style == SW_STYLE_BOXES) || (style == SW_STYLE_STEPS) || (style == SW_STYLE_FSTEPS) || (style == SW_STYLE_HISTEPS)))
   {
    unsigned char logaxis = (a1->log==SW_BOOL_TRUE);
    UUC_RESET;
    UUC(a2, lasty);
    if (ptBset) // We have one final box/step to process
     {
            if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { UUF(a1, (ptCx - (ptCx-ptBx)/2      )); UUF(a1, (ptCx + (ptCx-ptBx)/2      )); }
            else if (style == SW_STYLE_BOXES)                                 { UUF(a1, (ptCx - sg->BoxWidth.real/2)); UUF(a1, (ptCx + sg->BoxWidth.real/2)); }
            else if (style == SW_STYLE_HISTEPS)                               { UUF(a1, (ptCx - (ptCx-ptBx)/2      )); UUF(a1, (ptCx + (ptCx-ptBx)/2      )); }
            else if (style == SW_STYLE_STEPS)  { UUF(a1, ((ptBx+ptCx)/2 - (ptCx-ptBx)/2)); UUF(a1, ((ptBx+ptCx)/2 + (ptCx-ptBx)/2)); }
            else if (style == SW_STYLE_FSTEPS) { UUF(a1, ptCx); }
     }
    else // We have a dataset with only a single box/step
     {
      if     ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { UUF(a1, (ptCx - 0.5)); UUF(a1, (ptCx + 0.5)); }
      else if (style == SW_STYLE_BOXES)                                { UUF(a1, (ptCx - sg->BoxWidth.real/2)); UUF(a1, (ptCx + sg->BoxWidth.real/2)); }
      else if (sg->BoxWidth.real<1e-200)                               { UUF(a1, (ptCx - 0.5)); UUF(a1, (ptCx + 0.5)); }
      else                                                             { UUF(a1, (ptCx - sg->BoxWidth.real/2)); UUF(a1, (ptCx + sg->BoxWidth.real/2)); }
     }
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

  if ((style == SW_STYLE_POINTS) || (style == SW_STYLE_LINESPOINTS) || (style == SW_STYLE_STARS) || (style == SW_STYLE_DOTS)) // POINTS, DOTS, STARS
   {
    double final_pointsize = pd->ww_final.pointsize;
    if      (style == SW_STYLE_DOTS ) final_pointsize *=  0.05; // Dots are 1/20th size of points
    else if (style == SW_STYLE_STARS) final_pointsize *= 12.0 ; // Stars are BIG
    last_colstr=NULL;

    blk = data->first;
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
          if (style != SW_STYLE_STARS)
           {
            pt = (style == SW_STYLE_DOTS) ? 9 : ((pd->ww_final.pointtype-1) % N_POINTTYPES); // Dots are always pt 9 (circle)
            while (pt<0) pt+=N_POINTTYPES;
            x->PointTypesUsed[pt] = 1;
            sprintf(epsbuff, "%.2f %.2f pt%d", xpos, ypos, pt+1);
            eps_core_BoundingBox(x, xpos, ypos, 2 * final_pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS);
           } else {
            pt = ((pd->ww_final.pointtype-1) % N_STARTYPES);
            while (pt<0) pt+=N_STARTYPES;
            x->StarTypesUsed[pt] = 1;
            sprintf(epsbuff, "/angle { 40 } def %.2f %.2f st%d", xpos, ypos, pt+1);
            eps_core_BoundingBox(x, xpos, ypos, 2 * final_pointsize * eps_StarSize[pt] * EPS_DEFAULT_PS);
           }
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
    int NDirections=0;

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
          if      (style == SW_STYLE_XERRORBARS   ) { NDirections = 1; ac[0]=1; min[0] = UUR(0) - UUR(2+ThreeDim); max[0] = UUR(0) + UUR(2+ThreeDim); }
          else if (style == SW_STYLE_YERRORBARS   ) { NDirections = 1; ac[1]=1; min[1] = UUR(1) - UUR(2+ThreeDim); max[1] = UUR(1) + UUR(2+ThreeDim); }
          else if (style == SW_STYLE_ZERRORBARS   ) { NDirections = 1; ac[2]=1; min[2] = UUR(2) - UUR(3         ); max[2] = UUR(2) + UUR(3         ); }
          else if (style == SW_STYLE_XERRORRANGE  ) { NDirections = 1; ac[0]=1; min[0] = UUR(2+ThreeDim); max[0] = UUR(3+ThreeDim); }
          else if (style == SW_STYLE_YERRORRANGE  ) { NDirections = 1; ac[1]=1; min[1] = UUR(2+ThreeDim); max[1] = UUR(3+ThreeDim); }
          else if (style == SW_STYLE_ZERRORRANGE  ) { NDirections = 1; ac[2]=1; min[2] = UUR(3         ); max[2] = UUR(4         ); }
          else if (style == SW_STYLE_XYERRORBARS  ) { NDirections = 2; ac[0]=ac[1]=1; min[0] = UUR(0) - UUR(2+ThreeDim); max[0] = UUR(0) + UUR(2+ThreeDim); min[1] = UUR(1) - UUR(3+ThreeDim); max[1] = UUR(1) + UUR(3+ThreeDim); }
          else if (style == SW_STYLE_XZERRORBARS  ) { NDirections = 2; ac[0]=ac[2]=1; min[0] = UUR(0) - UUR(3         ); max[0] = UUR(0) + UUR(3         ); min[2] = UUR(2) - UUR(4         ); max[2] = UUR(2) + UUR(4         ); }
          else if (style == SW_STYLE_YZERRORBARS  ) { NDirections = 2; ac[1]=ac[2]=1; min[1] = UUR(1) - UUR(3         ); max[1] = UUR(1) + UUR(3         ); min[2] = UUR(2) - UUR(4         ); max[2] = UUR(2) + UUR(4         ); }
          else if (style == SW_STYLE_XYERRORRANGE ) { NDirections = 2; ac[0]=ac[1]=1; min[0] = UUR(2+ThreeDim); max[0] = UUR(3+ThreeDim); min[1] = UUR(4+ThreeDim); max[1] = UUR(5+ThreeDim); }
          else if (style == SW_STYLE_YZERRORRANGE ) { NDirections = 2; ac[1]=ac[2]=1; min[1] = UUR(3         ); max[1] = UUR(4         ); min[2] = UUR(5         ); max[2] = UUR(6         ); }
          else if (style == SW_STYLE_XZERRORRANGE ) { NDirections = 2; ac[0]=ac[2]=1; min[0] = UUR(3         ); max[0] = UUR(4         ); min[2] = UUR(5         ); max[2] = UUR(6         ); }
          else if (style == SW_STYLE_XYZERRORBARS ) { NDirections = 3; ac[0]=ac[1]=ac[2]=1; min[0] = UUR(0) - UUR(3); max[0] = UUR(0) + UUR(3); min[1] = UUR(1) - UUR(4); max[1] = UUR(1) + UUR(4); min[2] = UUR(2) - UUR(5); max[2] = UUR(2) + UUR(6); }
          else if (style == SW_STYLE_XYZERRORRANGE) { NDirections = 3; ac[0]=ac[1]=ac[2]=1; min[0] = UUR(3); max[0] = UUR(4); min[1] = UUR(5); max[1] = UUR(6); min[2] = UUR(7); max[2] = UUR(8); }

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
            if (NDirections!=1) continue; // Only put a central bar through errorbars which only go in a single direction
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

    if (a[yn]->DataUnitSet && (!ppl_units_DimEqual(&sg->BoxFrom, &a[yn]->DataUnit))) { sprintf(temp_err_string, "Data with units of <%s> plotted with impulses when BoxFrom is set to a value with units of <%s>.", ppl_units_GetUnitStr(&a[yn]->DataUnit,NULL,NULL,0,1,0),  ppl_units_GetUnitStr(&sg->BoxFrom,NULL,NULL,1,1,0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
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
     double xl,xr; \
     if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); } \
     xl=x0-width; \
     xr=x0+width; \
     if (logaxis) { xl=exp(xl); xr=exp(xr); } \
     LineDraw_Point(ld, xl, y0, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, xr, y0, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
    } \

#define MAKE_BOX(X0,Y0,WIDTH) \
   IF_NOT_INVISIBLE \
    { \
     double x0=X0,y0=Y0,width=WIDTH; \
     double xl,xr,yb,yt; \
     xl = x0-width; \
     xr = x0+width; \
     if (logaxis) { xl=exp(xl); xr=exp(xr); } \
\
     /* Set fill colour of box */ \
     eps_core_SetFillColour(x, &pd->ww_final); \
     eps_core_SwitchTo_FillColour(x); \
\
     /* Fill box */ \
     IF_NOT_INVISIBLE \
      { \
       FilledRegionHandle *fr; \
       yb = sg->BoxFrom.real; \
       yt = y0; \
       fr = FilledRegion_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z); \
       FilledRegion_Point(fr, xl, yb); \
       FilledRegion_Point(fr, xl, yt); \
       FilledRegion_Point(fr, xr, yt); \
       FilledRegion_Point(fr, xr, yb); \
       FilledRegion_Finish(fr, pd->ww_final.linetype, pd->ww_final.linewidth, 0); \
      } \
     eps_core_SwitchFrom_FillColour(x); \
\
     if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); } \
     LineDraw_Point(ld, xl, sg->BoxFrom.real, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, xl, y0              , 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, xr, y0              , 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, xr, sg->BoxFrom.real, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_Point(ld, xl, sg->BoxFrom.real, 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, pd->ww_final.linewidth, last_colstr); \
     LineDraw_PenUp(ld); \
    } \

    if (a[yn]->DataUnitSet && (!ppl_units_DimEqual(&sg->BoxFrom, &a[yn]->DataUnit))) { sprintf(temp_err_string, "Data with units of <%s> plotted as boxes/steps when BoxFrom is set to a value with units of <%s>.", ppl_units_GetUnitStr(&a[yn]->DataUnit,NULL,NULL,0,1,0),  ppl_units_GetUnitStr(&sg->BoxFrom,NULL,NULL,1,1,0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
    else if (a[xn]->DataUnitSet && (sg->BoxWidth.real>0.0) && (!ppl_units_DimEqual(&sg->BoxWidth, &a[xn]->DataUnit))) { sprintf(temp_err_string, "Data with ordinate units of <%s> plotted as boxes/steps when BoxWidth is set to a value with units of <%s>.", ppl_units_GetUnitStr(&a[xn]->DataUnit,NULL,NULL,0,1,0),  ppl_units_GetUnitStr(&sg->BoxWidth,NULL,NULL,1,1,0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
    else
     {
      unsigned char logaxis = (a[xn]->log==SW_BOOL_TRUE);
      while (blk != NULL)
       {
        for (j=0; j<blk->BlockPosition; j++)
         {
          ptAx=ptBx; ptAy=ptBy; ptAset=ptBset;
          ptBx=ptCx; ptBy=ptCy; ptBset=ptCset;
          ptCx=logaxis?log(UUR(0)):(UUR(0)); ptCy=UUR(1); ptCset=1;
          if (ptBset)
           {
            if (ptAset) // We are processing a box in the midst of many
             {
              if      ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { MAKE_BOX ((ptBx+(ptAx+ptCx)/2)/2, ptBy, (ptCx-ptAx)/4      ); }
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
      else if (ptCset) // We have a dataset with only a single box/step
       {
        if     ((style == SW_STYLE_BOXES) && (sg->BoxWidth.real<1e-200)) { MAKE_BOX (ptCx, ptCy, 0.5); }
        else if (style == SW_STYLE_BOXES)                                { MAKE_BOX (ptCx, ptCy, sg->BoxWidth.real/2); }
        else if (sg->BoxWidth.real<1e-200)                               { MAKE_STEP(ptCx, ptCy, 0.5); }
        else                                                             { MAKE_STEP(ptCx, ptCy, sg->BoxWidth.real/2); }
       }
     }
    LineDraw_PenUp(ld);
   }

  else if ((style == SW_STYLE_ARROWS_HEAD) || (style == SW_STYLE_ARROWS_NOHEAD) || (style == SW_STYLE_ARROWS_TWOHEAD)) // ARROWS_HEAD, ARROWS_NOHEAD, ARROWS_TWOHEAD
   {
    double xpos2,ypos2,depth2,xap2,yap2,zap2,lw,theta;
    lw  = pd->ww_final.linewidth;

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
          if ((last_colstr==NULL)||(strcmp(last_colstr,x->LastEPSColour)!=0)) { last_colstr = (char *)lt_malloc(strlen(x->LastEPSColour)+1); if (last_colstr==NULL) break; strcpy(last_colstr, x->LastEPSColour); }
          LineDraw_Point(ld, UUR(xn           ), UUR(yn           ), ThreeDim ? UUR(zn  ) : 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, lw, last_colstr);
          LineDraw_Point(ld, UUR(xn+2+ThreeDim), UUR(yn+2+ThreeDim), ThreeDim ? UUR(zn+3) : 0.0, 0,0,0,0,0,0, pd->ww_final.linetype, lw, last_colstr);
          LineDraw_PenUp(ld);

          eps_plot_GetPosition(&xpos , &ypos , &depth , &xap , &yap , &zap , NULL, NULL, NULL, ThreeDim, UUR(xn           ), UUR(yn           ), ThreeDim ? UUR(zn  ) : 0.0, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, origin_x, origin_y, scale_x, scale_y, scale_z, 0);
          eps_plot_GetPosition(&xpos2, &ypos2, &depth2, &xap2, &yap2, &zap2, NULL, NULL, NULL, ThreeDim, UUR(xn+2+ThreeDim), UUR(yn+2+ThreeDim), ThreeDim ? UUR(zn+3) : 0.0, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, origin_x, origin_y, scale_x, scale_y, scale_z, 0);
          theta = atan2(xpos2-xpos,ypos2-ypos);
          if (!gsl_finite(theta)) theta=0.0;

          if ((style == SW_STYLE_ARROWS_TWOHEAD) && (xap>=0.0)&&(xap<=1.0)&&(yap>=0.0)&&(yap<=1.0)&&((!ThreeDim)||((zap>=0.0)&&(zap<=1.0))))
           {
            double x2=xpos, y2=ypos, x3, y3, x4, y4, x5, y5, theta_y = theta + M_PI;
            x3 = x2 - EPS_ARROW_HEADSIZE * lw * sin(theta_y - EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on one side
            y3 = y2 - EPS_ARROW_HEADSIZE * lw * cos(theta_y - EPS_ARROW_ANGLE / 2);
            x5 = x2 - EPS_ARROW_HEADSIZE * lw * sin(theta_y + EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on other side
            y5 = y2 - EPS_ARROW_HEADSIZE * lw * cos(theta_y + EPS_ARROW_ANGLE / 2);
            x4 = x2 - EPS_ARROW_HEADSIZE * lw * sin(theta_y) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2); // Point where back of arrowhead crosses stalk
            y4 = y2 - EPS_ARROW_HEADSIZE * lw * cos(theta_y) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2);
            sprintf(epsbuff, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nfill\n", x4,y4,x3,y3,x2,y2,x5,y5);
            ThreeDimBuffer_writeps(x, depth, 1, lw, 1.0, last_colstr, epsbuff);
           }

          if (((style == SW_STYLE_ARROWS_TWOHEAD) || (style == SW_STYLE_ARROWS_HEAD)) && (xap2>=0.0)&&(xap2<=1.0)&&(yap2>=0.0)&&(yap2<=1.0)&&((!ThreeDim)||((zap2>=0.0)&&(zap2<=1.0))))
           {
            double x2=xpos2, y2=ypos2, x3, y3, x4, y4, x5, y5, theta_y = theta;
            x3 = x2 - EPS_ARROW_HEADSIZE * lw * sin(theta_y - EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on one side
            y3 = y2 - EPS_ARROW_HEADSIZE * lw * cos(theta_y - EPS_ARROW_ANGLE / 2);
            x5 = x2 - EPS_ARROW_HEADSIZE * lw * sin(theta_y + EPS_ARROW_ANGLE / 2); // Pointy back of arrowhead on other side
            y5 = y2 - EPS_ARROW_HEADSIZE * lw * cos(theta_y + EPS_ARROW_ANGLE / 2);
            x4 = x2 - EPS_ARROW_HEADSIZE * lw * sin(theta_y) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2); // Point where back of arrowhead crosses stalk
            y4 = y2 - EPS_ARROW_HEADSIZE * lw * cos(theta_y) * (1.0 - EPS_ARROW_CONSTRICT) * cos(EPS_ARROW_ANGLE / 2);
            sprintf(epsbuff, "newpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nfill\n", x4,y4,x3,y3,x2,y2,x5,y5);
            ThreeDimBuffer_writeps(x, depth2, 1, lw, 1.0, last_colstr, epsbuff);
           }
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

  else if (style == SW_STYLE_FILLEDREGION) // FILLEDREGION
   {
    FilledRegionHandle *fr;
    fr = FilledRegion_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);

    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
        // Work out style information for next point
        eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
        FilledRegion_Point(fr, UUR(xn), UUR(yn));
       }
      blk=blk->next;
     }
    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetFillColour(x, &pd->ww_final);
    eps_core_SwitchTo_FillColour(x);
    FilledRegion_Finish(fr, pd->ww_final.linetype, pd->ww_final.linewidth, 1);
    eps_core_SwitchFrom_FillColour(x);
    strcpy(x->LastEPSColour, ""); // Nullify last EPS colour
   }

  else if (style == SW_STYLE_YERRORSHADED) // YERRORSHADED
   {
    FilledRegionHandle *fr;
    int BlkNo=0;

    fr = FilledRegion_Init(x, a[xn], a[yn], a[zn], xrn, yrn, zrn, sg, ThreeDim, origin_x, origin_y, scale_x, scale_y, scale_z);

    // First add all of the points along the tops of the error bars, moving from left to right
    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++) FilledRegion_Point(fr, UUR(xn), UUR(yn));
      blk=blk->next;
      BlkNo++;
     }

    // Now add the points along the bottoms of all of the error bars, moving from right to left
    for (BlkNo-- ; BlkNo>=0; BlkNo--)
     {
      blk = data->first;
      for (j=0; j<BlkNo; j++) blk=blk->next;
      for (j=blk->BlockPosition-1; j>=0; j--)
       {
        // Work out style information for next point
        eps_plot_WithWordsFromUsingItems(&pd->ww_final, &blk->data_real[Ncolumns*j].d, Ncolumns);
        if (xn==0) FilledRegion_Point(fr, UUR(0), UUR(2));
        else       FilledRegion_Point(fr, UUR(2), UUR(0));
       }
     }

    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetFillColour(x, &pd->ww_final); 
    eps_core_SwitchTo_FillColour(x); 
    FilledRegion_Finish(fr, pd->ww_final.linetype, pd->ww_final.linewidth, 1);
    eps_core_SwitchFrom_FillColour(x);
    strcpy(x->LastEPSColour, ""); // Nullify last EPS colour
   }

  // End looping over monotonic regions of axis space
   }

  return 0;
 }

// Produce an icon representing a dataset on the graph's legend
void eps_plot_LegendIcon(EPSComm *x, int i, canvas_plotdesc *pd, double xpos, double ypos, double scale, settings_axis *a1, settings_axis *a2, settings_axis *a3, int xn, int yn, int zn)
 {
  int            style;
  DataTable     *data;
  settings_axis *a[3] = {a1,a2,a3};

  data  = x->current->plotdata[i];
  style = pd->ww_final.linespoints;
  if ((data==NULL) || (data->Nrows<1)) return; // No data present

  if ((style==SW_STYLE_LINES) || (style==SW_STYLE_LINESPOINTS) || (style==SW_STYLE_IMPULSES) || (style==SW_STYLE_BOXES) || (style==SW_STYLE_WBOXES) || (style==SW_STYLE_STEPS) || (style==SW_STYLE_FSTEPS) || (style==SW_STYLE_HISTEPS))
   {
    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.linewidth, pd->ww_final.linetype, 0);
    IF_NOT_INVISIBLE
     {
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", xpos-scale*0.60/2, ypos, xpos+scale*0.60/2, ypos);
      eps_core_BoundingBox(x, xpos-scale*0.60/2, ypos, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos+scale*0.60/2, ypos, pd->ww_final.linewidth);
     }
   }

  if ((style==SW_STYLE_POINTS) || (style==SW_STYLE_LINESPOINTS) || (style==SW_STYLE_STARS) || (style==SW_STYLE_DOTS))
   {
    double final_pointsize = pd->ww_final.pointsize; 
    if (style==SW_STYLE_DOTS) final_pointsize *= 0.05; // Dots are 1/20th size of points
    eps_core_SetColour(x, &pd->ww_final, 1);
    IF_NOT_INVISIBLE
     {
      int pt = (pd->ww_final.pointtype-1) % N_POINTTYPES;
      if (style==SW_STYLE_DOTS ) pt =  9; // Dots are always pt 9 (circle)
      if (style==SW_STYLE_STARS) pt = 26; // Stars are always pt 26 (star)
      while (pt<0) pt+=N_POINTTYPES;
      x->PointTypesUsed[pt] = 1;
      fprintf(x->epsbuffer, "/ps { %f } def %.2f %.2f pt%d\n", final_pointsize * EPS_DEFAULT_PS, xpos, ypos, pt+1);
      eps_core_BoundingBox(x, xpos, ypos, 2 * final_pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS);
     }
   }

  else if ((style==SW_STYLE_LOWERLIMITS) || (style==SW_STYLE_UPPERLIMITS))
   {
    double ps, sgn, eah_old=EPS_ARROW_HEADSIZE;
    ps  = pd->ww_final.pointsize * EPS_DEFAULT_PS;
    sgn = ( (style == SW_STYLE_UPPERLIMITS) ^ (a[yn]->MaxFinal > a[yn]->MinFinal) ) ? 1.0 : -1.0;
    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.pointlinewidth, pd->ww_final.linetype, 0);
    IF_NOT_INVISIBLE
     {
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", xpos-ps, ypos-ps*sgn, xpos+ps, ypos-ps*sgn);
      eps_core_BoundingBox(x, xpos-ps, ypos-ps*sgn, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos+ps, ypos-ps*sgn, pd->ww_final.linewidth);
      EPS_ARROW_HEADSIZE /=2;
      eps_primitive_arrow(x, SW_ARROWTYPE_HEAD, xpos, ypos-ps*sgn, xpos, ypos+ps*sgn, &pd->ww_final);
      EPS_ARROW_HEADSIZE = eah_old;
     }
   }

  else if ((style==SW_STYLE_ARROWS_HEAD) || (style==SW_STYLE_ARROWS_NOHEAD) || (style==SW_STYLE_ARROWS_TWOHEAD))
   {
    int ArrowStyle;
    if      (style==SW_STYLE_ARROWS_TWOHEAD) ArrowStyle = SW_ARROWTYPE_TWOWAY;
    else if (style==SW_STYLE_ARROWS_NOHEAD)  ArrowStyle = SW_ARROWTYPE_NOHEAD;
    else                                     ArrowStyle = SW_ARROWTYPE_HEAD;
    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.pointlinewidth, pd->ww_final.linetype, 0);
    IF_NOT_INVISIBLE eps_primitive_arrow(x, ArrowStyle, xpos-scale*0.60/2, ypos, xpos+scale*0.60/2, ypos, &pd->ww_final);
   }

  else if ((style == SW_STYLE_FILLEDREGION) || (style == SW_STYLE_YERRORSHADED))
   {
    double s=scale*0.45/2;
    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetFillColour(x, &pd->ww_final);
    eps_core_SwitchTo_FillColour(x);
    IF_NOT_INVISIBLE
     {
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath fill\n", xpos-s, ypos-s, xpos+s, ypos-s, xpos+s, ypos+s, xpos-s, ypos+s);
      eps_core_BoundingBox(x, xpos-s, ypos-s, 0);
      eps_core_BoundingBox(x, xpos+s, ypos-s, 0);
      eps_core_BoundingBox(x, xpos-s, ypos+s, 0);
      eps_core_BoundingBox(x, xpos+s, ypos+s, 0);
     }
    eps_core_SwitchFrom_FillColour(x);
    eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.pointlinewidth, pd->ww_final.linetype, 0);
    IF_NOT_INVISIBLE
     {
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath stroke\n", xpos-s, ypos-s, xpos+s, ypos-s, xpos+s, ypos+s, xpos-s, ypos+s);
      eps_core_BoundingBox(x, xpos-s, ypos-s, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos+s, ypos-s, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos-s, ypos+s, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos+s, ypos+s, pd->ww_final.linewidth);
     }
   }

  else if ((style==SW_STYLE_XERRORBARS)||(style==SW_STYLE_YERRORBARS)||(style==SW_STYLE_ZERRORBARS)||(style==SW_STYLE_XYERRORBARS)||(style==SW_STYLE_XZERRORBARS)||(style==SW_STYLE_XZERRORBARS)||(style==SW_STYLE_XZERRORBARS)||(style==SW_STYLE_XERRORRANGE)||(style==SW_STYLE_YERRORRANGE)||(style==SW_STYLE_ZERRORRANGE)||(style==SW_STYLE_XYERRORRANGE)||(style==SW_STYLE_XZERRORRANGE)||(style==SW_STYLE_YZERRORRANGE))
   {
    double s  = scale*0.6/2;
    double b  = 0.0005 * x->current->settings.bar * M_TO_PS;
    double ps = pd->ww_final.pointsize * EPS_DEFAULT_PS;

    eps_core_SetColour(x, &pd->ww_final, 1);
    IF_NOT_INVISIBLE
     {            
      eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.linewidth, pd->ww_final.linetype, 0);
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",xpos-s,ypos   ,xpos+s,ypos   );
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",xpos-s,ypos-b ,xpos-s,ypos+b );
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",xpos+s,ypos-b ,xpos+s,ypos+b );
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n",xpos  ,ypos-ps,xpos  ,ypos+ps);
      eps_core_BoundingBox(x, xpos-s, ypos-ps, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos-s, ypos+ps, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos+s, ypos-ps, pd->ww_final.linewidth);
      eps_core_BoundingBox(x, xpos+s, ypos+ps, pd->ww_final.linewidth);
     }
   }

  return;
 }

