// eps_plot_axespaint.c
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

#define _PPL_EPS_PLOT_AXESPAINT 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_canvasdraw.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_settings.h"

void eps_plot_2d_axispaint(EPSComm *x, settings_axis *a, const unsigned char Yx, const unsigned char TOPbottom, double *ypos, const double left, const double right, const unsigned char PrintLabels)
 {
  // Draw line of axis
  IF_NOT_INVISIBLE
   {
    if (!Yx)
     {
      fprintf(x->epsbuffer, "%.2f %.2f moveto %.2f %.2f lineto stroke\n", left, *ypos, right, *ypos);
      eps_core_BoundingBox(x, left , *ypos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
      eps_core_BoundingBox(x, right, *ypos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
     }
    else
     {
      fprintf(x->epsbuffer, "%.2f %.2f moveto %.2f %.2f lineto stroke\n", *ypos, left, *ypos, right);
      eps_core_BoundingBox(x, *ypos, left , EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
      eps_core_BoundingBox(x, *ypos, right, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
     }

    // Write axis label
    if (PrintLabels && (a->label != NULL) && (a->label[0]!='\0'))
     {
      int pageno = x->LaTeXpageno++;
      double xlab, ylab;

      if (!Yx) { xlab = (left+right)/2/M_TO_PS; ylab = *ypos/M_TO_PS + (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP; }
      else     { ylab = (left+right)/2/M_TO_PS; xlab = *ypos/M_TO_PS + (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP; }

      IF_NOT_INVISIBLE
       {
        canvas_EPSRenderTextItem(x, pageno, xlab, ylab, SW_HALIGN_CENT, ( TOPbottom ^ Yx) ? SW_VALIGN_BOT : SW_VALIGN_TOP, x->LastEPSColour, x->current->settings.FontSize, M_PI/2*Yx);
        *ypos += (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP * M_TO_PS; // Allow gap after label
       }
     }

    // Allow a gap before next axis
    *ypos += (TOPbottom ? 1.0 : -1.0) * EPS_AXES_SEPARATION * M_TO_PS;
   }
 }

void eps_plot_axespaint(EPSComm *x, double origin_x, double origin_y, double width, double height)
 {
  int            i, j, Naxes[2], FirstAutoMirror[2];
  double         left, right, title_x, title_y, TopPos, BotPos;
  settings_axis *axes;
  with_words     ww;

  // Set colour for painting axes
  with_words_zero(&ww,0);
  if (x->current->settings.AxesColour > 0) { ww.USEcolour = 1; ww.colour = x->current->settings.AxesColour; }
  else                                     { ww.USEcolourRGB = 1; ww.colourR = x->current->settings.AxesColourR; ww.colourG = x->current->settings.AxesColourG; ww.colourB = x->current->settings.AxesColourB; }
  eps_core_SetColour(x, &ww);
  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH, 0);

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
    for (j=0; j<2; j++)
     {
      Naxes[0] = Naxes[1] = 0;
      FirstAutoMirror[0] = FirstAutoMirror[1] = -1;

      if (j==0) { axes = x->current->XAxes; BotPos = origin_y; TopPos = origin_y + height; left = origin_x; right = origin_x + width;  }
      else      { axes = x->current->YAxes; BotPos = origin_x; TopPos = origin_x + width ; left = origin_y; right = origin_y + height; }

      for (i=0; i<MAX_AXES; i++)
       if ((axes[i].FinalActive) && (!axes[i].invisible))
        {
         if ((axes[i].MirrorType == SW_AXISMIRROR_AUTO) && (FirstAutoMirror[axes[i].topbottom]<0)) FirstAutoMirror[axes[i].topbottom] = i;

         if ((!axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_MIRROR) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR))
          {
           eps_plot_2d_axispaint(x, axes+i, j, 0, &BotPos, left, right, (!axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR)); // Axis on bottom/left
           Naxes[0]++;
          }
         if (( axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_MIRROR) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR))
          {
           eps_plot_2d_axispaint(x, axes+i, j, 1, &TopPos, left, right, ( axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR)); // Axis on top/right
           Naxes[1]++;
          }
        }

      // If there is no axis on bottom/left, but was an auto-mirrored axis on top/right, mirror it now
      if ((Naxes[0]==0) && (FirstAutoMirror[1]>=0))
       {
        i = FirstAutoMirror[1];
        eps_plot_2d_axispaint(x, axes+i, j, 0, &BotPos, left, right, 0);
        Naxes[0]++;
       }

      // If there is no axis on top/right, but was an auto-mirrored axis on bottom/left, mirror it now
      if ((Naxes[1]==0) && (FirstAutoMirror[0]>=0))
       {
        i = FirstAutoMirror[0];
        eps_plot_2d_axispaint(x, axes+i, j, 1, &TopPos, left, right, 0); 
        Naxes[0]++;
       }

      // If we're dealing with x-axes, set the position of the title now
      if (j==0)
       {
        title_x  = (left+right)/2 / M_TO_PS;
        title_y  = TopPos         / M_TO_PS;
       }
     }
   }

  // Put the title on the top of the graph
  if ((x->current->settings.title != NULL) && (x->current->settings.title[0] != '\0'))
   {
    int pageno = x->LaTeXpageno++;
    with_words_zero(&ww,0);
    if (x->current->settings.TextColour > 0) { ww.USEcolour = 1; ww.colour = x->current->settings.TextColour; }
    else                                     { ww.USEcolourRGB = 1; ww.colourR = x->current->settings.TextColourR; ww.colourG = x->current->settings.TextColourG; ww.colourB = x->current->settings.TextColourB; }
    eps_core_SetColour(x, &ww);
    IF_NOT_INVISIBLE canvas_EPSRenderTextItem(x, pageno, title_x, title_y, SW_HALIGN_CENT, SW_VALIGN_BOT, x->LastEPSColour, x->current->settings.FontSize, 0.0);
   }

  return;
 }

