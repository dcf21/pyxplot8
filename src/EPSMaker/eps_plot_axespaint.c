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
#include "ppl_units_fns.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_settings.h"

void eps_plot_LabelAlignment(double theta_pinpoint, int *HALIGN, int *VALIGN)
 {
  while (theta_pinpoint < 0.0) theta_pinpoint += 2*M_PI;
  theta_pinpoint = fmod(theta_pinpoint, 2*M_PI);
  if      (ppl_units_DblEqual(theta_pinpoint,   0.0   )) { *HALIGN = SW_HALIGN_CENT ; *VALIGN = SW_VALIGN_TOP ; }
  else if (ppl_units_DblEqual(theta_pinpoint,   M_PI/2)) { *HALIGN = SW_HALIGN_LEFT ; *VALIGN = SW_VALIGN_CENT; }
  else if (ppl_units_DblEqual(theta_pinpoint,   M_PI  )) { *HALIGN = SW_HALIGN_CENT ; *VALIGN = SW_VALIGN_BOT ; }
  else if (ppl_units_DblEqual(theta_pinpoint, 3*M_PI/2)) { *HALIGN = SW_HALIGN_RIGHT; *VALIGN = SW_VALIGN_CENT; }
  else if (theta_pinpoint <   M_PI/2)                    { *HALIGN = SW_HALIGN_LEFT ; *VALIGN = SW_VALIGN_TOP ; }
  else if (theta_pinpoint <   M_PI  )                    { *HALIGN = SW_HALIGN_LEFT ; *VALIGN = SW_VALIGN_BOT ; }
  else if (theta_pinpoint < 3*M_PI/2)                    { *HALIGN = SW_HALIGN_RIGHT; *VALIGN = SW_VALIGN_BOT ; }
  else                                                   { *HALIGN = SW_HALIGN_RIGHT; *VALIGN = SW_VALIGN_TOP ; }
  return;
 }

void eps_plot_2d_axispaint(EPSComm *x, settings_axis *a, const unsigned char Yx, const unsigned char TOPbottom, double *ypos, const double left, const double right, const unsigned char PrintLabels)
 {
  int    i, l;
  double TickMaxHeight = 0.0, height, width;
  int    HALIGN, VALIGN;
  double theta, theta_pinpoint, theta_widthcalc; // clockwise rotation

  // Draw line of axis
  IF_NOT_INVISIBLE
   {
    if (!Yx)
     {
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", left, *ypos, right, *ypos);
      eps_core_BoundingBox(x, left , *ypos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
      eps_core_BoundingBox(x, right, *ypos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
     }
    else
     {
      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", *ypos, left, *ypos, right);
      eps_core_BoundingBox(x, *ypos, left , EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
      eps_core_BoundingBox(x, *ypos, right, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
     }

    // Paint axis ticks
    for (i=0; i<2; i++)
     {
      int      TR;
      double  *TLP, TRA, TLEN;
      char   **TLS;

      if (i==0) { TLP=a-> TickListPositions; TLS=a-> TickListStrings; TRA=a->TickLabelRotate; TR=a->TickLabelRotation; TLEN = EPS_AXES_MAJTICKLEN; } // Major ticks
      else      { TLP=a->MTickListPositions; TLS=a->MTickListStrings; TRA=a->TickLabelRotate; TR=a->TickLabelRotation; TLEN = EPS_AXES_MINTICKLEN; } // Minor ticks

      // Work out the rotation of the tick label
      if      (TR == SW_TICLABDIR_HORI) theta = 0.0;
      else if (TR == SW_TICLABDIR_VERT) theta = M_PI/2; // the clockwise rotation of the label relative to upright
      else                              theta = TRA;
      theta_pinpoint = theta - M_PI/2*Yx + M_PI*TOPbottom; // Angle around textbox where it is anchored
      eps_plot_LabelAlignment(theta_pinpoint, &HALIGN, &VALIGN);
      theta_widthcalc = theta + M_PI/2*Yx; // Angle used in calculating the height (or width) of the label

      if (TLP != NULL)
       {
        for (l=0; TLS[l]!=NULL; l++)
         {
          double lrpos  = left + (right-left) * TLP[l]; // left--right position of tick
          double udpos1 = *ypos + (TOPbottom ? 1.0 : -1.0) * (a->TickDir==SW_TICDIR_IN  ? 0.0 :  1.0) * TLEN * M_TO_PS; // top of tick
          double udpos2 = *ypos + (TOPbottom ? 1.0 : -1.0) * (a->TickDir==SW_TICDIR_OUT ? 0.0 : -1.0) * TLEN * M_TO_PS; // bottom of tick

          // Stroke the ticks
          if (!Yx)
           {
            fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", lrpos, udpos1, lrpos, udpos2);
            eps_core_BoundingBox(x, lrpos, udpos1, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
            eps_core_BoundingBox(x, lrpos, udpos2, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
           }
          else
           {
            fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", udpos1, lrpos, udpos2, lrpos);
            eps_core_BoundingBox(x, udpos1, lrpos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
            eps_core_BoundingBox(x, udpos2, lrpos, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
           }

          // Paint the axis labels
          if (PrintLabels && (TLS[l][0] != '\0'))
           {
            int pageno = x->LaTeXpageno++;
            double xlab, ylab;

            if (!Yx) { xlab = lrpos/M_TO_PS; ylab = *ypos/M_TO_PS + (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP; }
            else     { ylab = lrpos/M_TO_PS; xlab = *ypos/M_TO_PS + (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP; }

            IF_NOT_INVISIBLE
             {
              canvas_EPSRenderTextItem(x, pageno, xlab, ylab, HALIGN, VALIGN, x->LastEPSColour, x->current->settings.FontSize, -theta, &width, &height);
              height = height*fabs(cos(theta_widthcalc)) + width*fabs(sin(theta_widthcalc));
              if (height > TickMaxHeight) TickMaxHeight = height;
             }
           }
         }
       }
     }
    if (TickMaxHeight>0.0) *ypos += (TOPbottom ? 1.0 : -1.0) * (EPS_AXES_TEXTGAP * M_TO_PS + TickMaxHeight); // Allow a gap after axis labels

    // Write axis label
    if (PrintLabels && (a->FinalAxisLabel != NULL) && (a->FinalAxisLabel[0]!='\0'))
     {
      int pageno = x->LaTeXpageno++;
      double xlab, ylab;
      double width, height;

      // Work out the rotation of the tick label
      theta = a->LabelRotate;
      theta_pinpoint = theta + M_PI*(TOPbottom ^ Yx); // Angle around textbox where it is anchored
      eps_plot_LabelAlignment(theta_pinpoint, &HALIGN, &VALIGN);
      theta_widthcalc = theta; // Angle used in calculating the height (or width) of the label

      if (!Yx) { xlab = (left+right)/2/M_TO_PS; ylab = *ypos/M_TO_PS + (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP; }
      else     { ylab = (left+right)/2/M_TO_PS; xlab = *ypos/M_TO_PS + (TOPbottom ? 1.0 : -1.0) * EPS_AXES_TEXTGAP; }

      IF_NOT_INVISIBLE
       {
        canvas_EPSRenderTextItem(x, pageno, xlab, ylab, HALIGN, VALIGN, x->LastEPSColour, x->current->settings.FontSize, -theta + M_PI/2*Yx, &width, &height);
        *ypos += (TOPbottom ? 1.0 : -1.0) * (EPS_AXES_TEXTGAP * M_TO_PS + height*fabs(cos(theta_widthcalc)) + width*fabs(sin(theta_widthcalc)) ); // Allow gap after label
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

  x->LaTeXpageno = x->current->AxesTextID;

  // Set colour for painting axes
  with_words_zero(&ww,0);
  if (x->current->settings.AxesColour > 0) { ww.USEcolour = 1; ww.colour = x->current->settings.AxesColour; }
  else                                     { ww.USEcolourRGB = 1; ww.colourR = x->current->settings.AxesColourR; ww.colourG = x->current->settings.AxesColourG; ww.colourB = x->current->settings.AxesColourB; }
  eps_core_SetColour(x, &ww, 1);
  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH, 1, 0.0);

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
  x->LaTeXpageno = x->current->TitleTextID;
  if ((x->current->settings.title != NULL) && (x->current->settings.title[0] != '\0'))
   {
    int pageno = x->LaTeXpageno++;
    with_words_zero(&ww,0);
    if (x->current->settings.TextColour > 0) { ww.USEcolour = 1; ww.colour = x->current->settings.TextColour; }
    else                                     { ww.USEcolourRGB = 1; ww.colourR = x->current->settings.TextColourR; ww.colourG = x->current->settings.TextColourG; ww.colourB = x->current->settings.TextColourB; }
    eps_core_SetColour(x, &ww, 1);
    IF_NOT_INVISIBLE canvas_EPSRenderTextItem(x, pageno, title_x, title_y, SW_HALIGN_CENT, SW_VALIGN_BOT, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
   }

  return;
 }

