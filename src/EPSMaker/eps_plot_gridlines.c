// eps_plot_gridlines.c
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

#define _PPL_EPS_PLOT_GRIDLINES 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_canvasdraw.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units_fns.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_gridlines.h"
#include "eps_settings.h"

void eps_plot_gridlines(EPSComm *x, double origin_x, double origin_y, double width, double height)
 {
  int            i, j, k, l;
  double         left, right;
  settings_axis *axes;
  unsigned char *GridLines;
  with_words     ww;

#define MAJOR_GRIDCOL \
  /* Set colour for major axis axes */ \
  with_words_zero(&ww,0); \
  if (x->current->settings.GridMajColour > 0) { ww.colour = x->current->settings.GridMajColour; ww.USEcolour = 1; } \
  else                                        { ww.Col1234Space = x->current->settings.GridMajCol1234Space; ww.colour1 = x->current->settings.GridMajColour1; ww.colour2 = x->current->settings.GridMajColour2; ww.colour3 = x->current->settings.GridMajColour3; ww.colour4 = x->current->settings.GridMajColour4; ww.USEcolour1234 = 1; } \
  eps_core_SetColour(x, &ww, 1); \
  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, EPS_GRID_MAJLINEWIDTH * EPS_DEFAULT_LINEWIDTH, 1, 0.0);

#define MINOR_GRIDCOL \
  /* Set colour for minor axis axes */ \
  with_words_zero(&ww,0); \
  if (x->current->settings.GridMinColour > 0) { ww.colour = x->current->settings.GridMinColour; ww.USEcolour = 1; } \
  else                                        { ww.Col1234Space = x->current->settings.GridMinCol1234Space; ww.colour1 = x->current->settings.GridMinColour1; ww.colour2 = x->current->settings.GridMinColour2; ww.colour3 = x->current->settings.GridMinColour3; ww.colour4 = x->current->settings.GridMinColour4; ww.USEcolour1234 = 1; } \
  eps_core_SetColour(x, &ww, 1); \
  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, EPS_GRID_MINLINEWIDTH * EPS_DEFAULT_LINEWIDTH, 1, 0.0);

  if (x->current->settings.grid != SW_ONOFF_ON) return; // Grid lines are off

  for (j=0; j<3; j++)
   {
    if ((j==2) && (!x->current->ThreeDim)) continue;

    if      (j==2) { axes = x->current->ZAxes; GridLines = x->current->settings.GridAxisZ; }
    else if (j==1) { axes = x->current->YAxes; GridLines = x->current->settings.GridAxisY; left = origin_y; right = origin_y+height; }
    else           { axes = x->current->XAxes; GridLines = x->current->settings.GridAxisX; left = origin_x; right = origin_x+width;  }

    for (i=0; i<MAX_AXES; i++)
     if ((axes[i].FinalActive) && (!axes[i].invisible) && (GridLines[i]))
      {


       // Three-dimensional axes
       if (x->current->ThreeDim)
        {
        }

       // 2D Gnomonic axes
       else if (x->current->settings.projection == SW_PROJ_GNOM)
        {
        }

       // 2D flat axes
      else
       {
        for (k=0; k<2; k++)
         {
          double  *TLP;
          char   **TLS;
          if (k==0) { TLP=axes[i]. TickListPositions; TLS=axes[i]. TickListStrings; MAJOR_GRIDCOL; } // Major ticks
          else      { TLP=axes[i].MTickListPositions; TLS=axes[i].MTickListStrings; MINOR_GRIDCOL; } // Minor ticks
          if (TLP != NULL)
           {
            for (l=0; TLS[l]!=NULL; l++)
             {
              double lrpos  = left + (right-left) * TLP[l]; // left--right position of tick

              // Stroke the grid lines
              if (j==1)
               {
                fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", origin_x, lrpos, origin_x+width, lrpos);
                eps_core_BoundingBox(x, origin_x       , lrpos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
                eps_core_BoundingBox(x, origin_x+width , lrpos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
               } else {
                fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", lrpos, origin_y, lrpos, origin_y+height);
                eps_core_BoundingBox(x, lrpos, origin_y        , EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
                eps_core_BoundingBox(x, lrpos, origin_y+height , EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
               }
             }
           }
         }
       }
     }
   }

  return;
 }

