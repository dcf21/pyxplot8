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

#include <gsl/gsl_math.h>

#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_colours.h"
#include "eps_plot_canvas.h"
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

// Return the number of columns of data which are required to plot in any given plot style
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
  else if (style == SW_STYLE_XYERRORBARS    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_XERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz1,n1,a1,UURU(3+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_YERRORRANGE    ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz2,n2,a2,UURU(2+ThreeDim)); UUAU(xyz2,n2,a2,UURU(3+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_XYERRORRANGE   ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); UUAU(xyz1,n1,a1,UURU(2+ThreeDim)); UUAU(xyz1,n1,a1,UURU(3+ThreeDim)); UUAU(xyz2,n2,a2,UURU(4+ThreeDim)); UUAU(xyz2,n2,a2,UURU(5+ThreeDim)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
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
  else if (style == SW_STYLE_CSPLINES       ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }
  else if (style == SW_STYLE_ACSPLINES      ) { UUAU(xyz1,n1,a1,UURU(0)); UUAU(xyz2,n2,a2,UURU(1)); if (ThreeDim) UUAU(xyz3,n3,a3,UURU(2)); }

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
      else if (style == SW_STYLE_XYERRORBARS    ) { UUU(a1, UUR(0)); UUU(a1, UUR(0)-UUR(2+ThreeDim)); UUU(a1, UUR(0)+UUR(2+ThreeDim)); UUU(a2, UUR(1)); UUU(a2, UUR(1)-UUR(3+ThreeDim)); UUU(a2, UUR(1)+UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XERRORRANGE    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a1, UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_YERRORRANGE    ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a2, UUR(2+ThreeDim)); UUU(a2, UUR(3+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
      else if (style == SW_STYLE_XYERRORRANGE   ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); UUU(a1, UUR(2+ThreeDim)); UUU(a1, UUR(3+ThreeDim)); UUU(a2, UUR(4+ThreeDim)); UUU(a2, UUR(5+ThreeDim)); if (ThreeDim) UUU(a3, UUR(2)); }
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
      else if (style == SW_STYLE_CSPLINES       ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      else if (style == SW_STYLE_ACSPLINES      ) { UUU(a1, UUR(0)); UUU(a2, UUR(1)); }
      i++;
     }
    blk=blk->next;
   }

  return 0;
 }

// Render a dataset to postscript
int  eps_plot_dataset(EPSComm *x, DataTable *data, int style, unsigned char ThreeDim, settings_axis *a1, settings_axis *a2, settings_axis *a3, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, double origin_x, double origin_y, double width, double height)
 {
  int        j, Ncolumns, lt, pt;
  double     xpos, ypos, lw, ps;
  settings_axis *a[3] = {a1,a2,a3};
  DataBlock *blk;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present

  // Set colour of dataset
  eps_core_SetColour(x, &pd->ww_final);

  if ((style == SW_STYLE_LINES) || (style == SW_STYLE_LINESPOINTS)) // LINES
   {
    Ncolumns = data->Ncolumns;
    blk = data->first;
    while (blk != NULL)
     {
      for (j=0; j<blk->BlockPosition; j++)
       {
       }
      blk=blk->next;
     }
   }

  if ((style == SW_STYLE_POINTS) || (style == SW_STYLE_LINESPOINTS)) // POINTS
   {
    IF_NOT_INVISIBLE
     {
      lw = pd->ww_final.pointlinewidth;
      lt = 0; // linetype is always zero when drawing points
      ps = pd->ww_final.pointsize;
      pt = pd->ww_final.pointtype % N_POINTTYPES;
      eps_core_SetLinewidth(x, lw, lt);
      x->PointTypesUsed[pt] = 1;
      fprintf(x->epsbuffer, "/ps { %f } def\n", ps*3); // Scale up all pointsizes by 3

      Ncolumns = data->Ncolumns;
      blk = data->first;
      while (blk != NULL)
       {
        for (j=0; j<blk->BlockPosition; j++)
         {
          eps_plot_GetPosition(&xpos, &ypos, ThreeDim, UUR(xn), UUR(yn), ThreeDim ? UUR(zn) : 0.0, a[xn], a[yn], a[zn], sg, origin_x, origin_y, width, height);
          if (!gsl_finite(xpos)) continue; // Position of point is off side of graph
          fprintf(x->epsbuffer, "%.2f %.2f pt%d\n", xpos, ypos, pt+1);
         }
        blk=blk->next;
       }
     }
   }

  if      (style == SW_STYLE_XERRORBARS     ) // XERRORBARS
   {
   }

  else if (style == SW_STYLE_YERRORBARS     ) // YERRORBARS
   {
   }

  else if (style == SW_STYLE_XYERRORBARS    ) // XYERRORBARS
   {
   }

  else if (style == SW_STYLE_XERRORRANGE    ) // XERRORRANGE
   {
   }

  else if (style == SW_STYLE_YERRORRANGE    ) // YERRORRANGE
   {
   }

  else if (style == SW_STYLE_XYERRORRANGE   ) // XYERRORRANGE
   {
   }

  else if (style == SW_STYLE_DOTS           ) // DOTS
   {
   }

  else if (style == SW_STYLE_IMPULSES       ) // IMPULSES
   {
   }

  else if (style == SW_STYLE_BOXES          ) // BOXES
   {
   }

  else if (style == SW_STYLE_WBOXES         ) // WBOXES
   {
   }

  else if (style == SW_STYLE_STEPS          ) // STEPS
   {
   }

  else if (style == SW_STYLE_FSTEPS         ) // FSTEPS
   {
   }

  else if (style == SW_STYLE_HISTEPS        ) // HISTEPS
   {
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

  else if (style == SW_STYLE_CSPLINES       ) // CSPLINES
   {
   }

  else if (style == SW_STYLE_ACSPLINES      ) // ACSPLINES
   {
   }

  return 0;
 }

