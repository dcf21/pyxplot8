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

#include "eps_arrow.h"
#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_axespaint.h"
#include "eps_plot_canvas.h"
#include "eps_settings.h"

void eps_plot_LabelAlignment(double theta_pinpoint, int *HALIGN, int *VALIGN)
 {
  theta_pinpoint = fmod(theta_pinpoint, 2*M_PI);
  while (theta_pinpoint < 0.0) theta_pinpoint += 2*M_PI;
  if      (ppl_units_DblApprox(theta_pinpoint,   0.0   , 1e-6)) { *HALIGN = SW_HALIGN_CENT ; *VALIGN = SW_VALIGN_TOP ; }
  else if (ppl_units_DblApprox(theta_pinpoint,   M_PI/2, 1e-6)) { *HALIGN = SW_HALIGN_RIGHT; *VALIGN = SW_VALIGN_CENT; }
  else if (ppl_units_DblApprox(theta_pinpoint,   M_PI  , 1e-6)) { *HALIGN = SW_HALIGN_CENT ; *VALIGN = SW_VALIGN_BOT ; }
  else if (ppl_units_DblApprox(theta_pinpoint, 3*M_PI/2, 1e-6)) { *HALIGN = SW_HALIGN_LEFT ; *VALIGN = SW_VALIGN_CENT; }
  else if (theta_pinpoint <   M_PI/2)                           { *HALIGN = SW_HALIGN_RIGHT; *VALIGN = SW_VALIGN_TOP ; }
  else if (theta_pinpoint <   M_PI  )                           { *HALIGN = SW_HALIGN_RIGHT; *VALIGN = SW_VALIGN_BOT ; }
  else if (theta_pinpoint < 3*M_PI/2)                           { *HALIGN = SW_HALIGN_LEFT ; *VALIGN = SW_VALIGN_BOT ; }
  else                                                          { *HALIGN = SW_HALIGN_LEFT ; *VALIGN = SW_VALIGN_TOP ; }
  return;
 }

void eps_plot_axispaint(EPSComm *x, with_words *ww, settings_axis *a, const int xyz, const double CP2, const unsigned char Lr, const double x1, const double y1, const double x2, const double y2, double *OutputWidth, const unsigned char PrintLabels)
 {
  int    i, l;
  double TickMaxHeight = 0.0, height, width, theta_axis;
  int    HALIGN, VALIGN, HALIGN_THIS, VALIGN_THIS;
  double theta, theta_pinpoint; // clockwise rotation

  *OutputWidth = 0.0;

  // Draw line of axis
  IF_NOT_INVISIBLE
   {
    if      (a->ArrowType == SW_AXISDISP_NOARR) eps_primitive_arrow(x, SW_ARROWTYPE_NOHEAD, x1, y1, x2, y2, ww);
    else if (a->ArrowType == SW_AXISDISP_ARROW) eps_primitive_arrow(x, SW_ARROWTYPE_HEAD  , x1, y1, x2, y2, ww);
    else if (a->ArrowType == SW_AXISDISP_TWOAR) eps_primitive_arrow(x, SW_ARROWTYPE_TWOWAY, x1, y1, x2, y2, ww);
    else if (a->ArrowType == SW_AXISDISP_BACKA) eps_primitive_arrow(x, SW_ARROWTYPE_HEAD  , x2, y2, x1, y1, ww);

    theta_axis = atan2(x2-x1,y2-y1);
    if (!gsl_finite(theta_axis)) theta_axis=0.0;

    // Paint axis ticks
    for (i=0; i<2; i++)
     {
      int      TR;
      double  *TLP, TRA, TLEN;
      char   **TLS;

      if (i==0) { TLP=a-> TickListPositions; TLS=a-> TickListStrings; TRA=a->TickLabelRotate; TR=a->TickLabelRotation; TLEN = EPS_AXES_MAJTICKLEN; } // Major ticks
      else      { TLP=a->MTickListPositions; TLS=a->MTickListStrings; TRA=a->TickLabelRotate; TR=a->TickLabelRotation; TLEN = EPS_AXES_MINTICKLEN; } // Minor ticks

      // Work out the rotation of the tick labels
      if      (TR == SW_TICLABDIR_HORI) theta =  0.0;
      else if (TR == SW_TICLABDIR_VERT) theta = -M_PI/2; // the clockwise rotation of the labels relative to upright
      else                              theta = -TRA;
      theta_pinpoint = theta + M_PI/2 + theta_axis + M_PI*(!Lr); // Angle around textboxes where it is anchored
      eps_plot_LabelAlignment(theta_pinpoint, &HALIGN, &VALIGN);

      if (TLP != NULL)
       {
        for (l=0; TLS[l]!=NULL; l++)
         {
          double tic_lab_xoff=0.0;
          double tic_x1 = x1 + (x2-x1) * TLP[l];
          double tic_y1 = y1 + (y2-y1) * TLP[l];
          double tic_x2 , tic_y2, tic_x3, tic_y3;
          tic_x2 = tic_x1 + (a->TickDir==SW_TICDIR_IN  ? 0.0 :  1.0) * (Lr ? -1.0 : 1.0) * sin(theta_axis + M_PI/2) * TLEN * M_TO_PS; // top of tick
          tic_y2 = tic_y1 + (a->TickDir==SW_TICDIR_IN  ? 0.0 :  1.0) * (Lr ? -1.0 : 1.0) * cos(theta_axis + M_PI/2) * TLEN * M_TO_PS;
          tic_x3 = tic_x1 + (a->TickDir==SW_TICDIR_OUT ? 0.0 : -1.0) * (Lr ? -1.0 : 1.0) * sin(theta_axis + M_PI/2) * TLEN * M_TO_PS; // bottom of tick
          tic_y3 = tic_y1 + (a->TickDir==SW_TICDIR_OUT ? 0.0 : -1.0) * (Lr ? -1.0 : 1.0) * cos(theta_axis + M_PI/2) * TLEN * M_TO_PS;

          // Check for special case of ticks at zero on axes crossed by atzero axes
          HALIGN_THIS = HALIGN;
          VALIGN_THIS = VALIGN;
          if ((a->CrossedAtZero) && (a->atzero))
           {
            double CP1 = TLP[l];
            double xl = TLP[l] - 1e-3;
            double xr = TLP[l] + 1e-3;
            if (xl<0.0) xl=0.0; if (xl>1.0) xl=1.0;
            if (xr<0.0) xr=0.0; if (xr>1.0) xr=1.0;
            xl = eps_plot_axis_InvGetPosition(xl, a);
            xr = eps_plot_axis_InvGetPosition(xr, a);
            if ((gsl_finite(xl)) && (gsl_finite(xr)) && ((xl==0)||(xr==0)||((xl<0)&&(xr>0))||((xl>0)&&(xr<0))))
             {
              if (xyz != 0) // y/z-axis -- only put label at zero if axis is at edge of graph
               {
                if (!( ((CP2<1e-3)&&( Lr)) || ((CP2>0.999)&&(!Lr)) )) { if (PrintLabels && (TLS[l][0] != '\0')) x->LaTeXpageno++; continue; }
               }
              else // x-axis -- if at edge of graph, put label at zero. If y-axis is labelled at zero, don't put label at zero...
               {
                if (!( ((CP2<1e-3)&&(!Lr)) || ((CP2>0.999)&&( Lr)) ))
                 {
                  int Lr1 = (xyz==0) ^ (CP1>0.999);
                  if ( ((CP1<1e-3)&&( Lr1)) || ((CP1>0.999)&&(!Lr1)) ) { if (PrintLabels && (TLS[l][0] != '\0')) x->LaTeXpageno++; continue; }
                  else // ... otherwise, displace label at zero to one side
                   {
                    double theta_pinpoint = theta + 1.5 * M_PI/2 + theta_axis + M_PI*(!Lr); // Angle around textboxes where it is anchored
                    eps_plot_LabelAlignment(theta_pinpoint, &HALIGN_THIS, &VALIGN_THIS);
                    tic_lab_xoff = -TLEN; // Move label to the left
                   }
                 }
               }
             }
           }

          // Stroke the tick, unless it is at the end of an axis with an arrowhead
          if (!( ((TLP[l]<1e-3)&&((a->ArrowType == SW_AXISDISP_BACKA)||(a->ArrowType == SW_AXISDISP_TWOAR))) || ((TLP[l]>0.999)&&((a->ArrowType == SW_AXISDISP_ARROW)||(a->ArrowType == SW_AXISDISP_TWOAR))) ))
           {
            fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto stroke\n", tic_x2, tic_y2, tic_x3, tic_y3);
            eps_core_PlotBoundingBox(x, tic_x2, tic_y2, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
            eps_core_PlotBoundingBox(x, tic_x3, tic_y3, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH);
           }

          // Paint the tick label
          if (PrintLabels && (TLS[l][0] != '\0'))
           {
            int pageno = x->LaTeXpageno++;
            double xlab, ylab;

            xlab = tic_x1/M_TO_PS + (Lr ? -1.0 : 1.0) * sin(theta_axis + M_PI/2) * EPS_AXES_TEXTGAP + tic_lab_xoff;
            ylab = tic_y1/M_TO_PS + (Lr ? -1.0 : 1.0) * cos(theta_axis + M_PI/2) * EPS_AXES_TEXTGAP;

            IF_NOT_INVISIBLE
             {
              canvas_EPSRenderTextItem(x, pageno, xlab, ylab, HALIGN_THIS, VALIGN_THIS, x->CurrentColour, x->current->settings.FontSize, theta, &width, &height);
              height = height*fabs(cos(theta_pinpoint)) + width*fabs(sin(theta_pinpoint));
              if (height > TickMaxHeight) TickMaxHeight = height;
             }
           }
         }
       }
     }
    if (TickMaxHeight>0.0) *OutputWidth = EPS_AXES_TEXTGAP * M_TO_PS + TickMaxHeight; // Allow a gap after axis labels

    // Write axis label
    if (PrintLabels && (a->FinalAxisLabel != NULL) && (a->FinalAxisLabel[0]!='\0'))
     {
      int pageno = x->LaTeXpageno++;
      double xlab, ylab;
      double width, height;

      // Work out the rotation of the tick label
      theta = -a->LabelRotate;
      theta_pinpoint = theta + M_PI*Lr; // Angle around textbox where it is anchored
      eps_plot_LabelAlignment(theta_pinpoint, &HALIGN, &VALIGN);

      xlab = (x1+x2)/2/M_TO_PS + (Lr ? -1.0 : 1.0) * (2*EPS_AXES_TEXTGAP+TickMaxHeight/M_TO_PS) * sin(theta_axis+M_PI/2);
      ylab = (y1+y2)/2/M_TO_PS + (Lr ? -1.0 : 1.0) * (2*EPS_AXES_TEXTGAP+TickMaxHeight/M_TO_PS) * cos(theta_axis+M_PI/2);

      IF_NOT_INVISIBLE
       {
        double theta_text = theta + M_PI/2 - theta_axis;
        theta_text = fmod(theta_text , 2*M_PI);
        if (theta_text < -M_PI  ) theta_text += 2*M_PI;
        if (theta_text >  M_PI  ) theta_text -= 2*M_PI;
        if (theta_text >  M_PI/2) theta_text -=   M_PI;
        if (theta_text < -M_PI/2) theta_text +=   M_PI;
        canvas_EPSRenderTextItem(x, pageno, xlab, ylab, HALIGN, VALIGN, x->CurrentColour, x->current->settings.FontSize, theta_text, &width, &height);
        *OutputWidth += (EPS_AXES_TEXTGAP * M_TO_PS + height*fabs(cos(theta_pinpoint)) + width*fabs(sin(theta_pinpoint)) ); // Allow gap after label
       }
     }

    // Allow a gap before next axis
    *OutputWidth += EPS_AXES_SEPARATION * M_TO_PS;
   }
 }

void eps_plot_axespaint(EPSComm *x, double origin_x, double origin_y, double width, double height, double zdepth)
 {
  int            i, j, Naxes[2], FirstAutoMirror[2];
  double         TopPos, BotPos;
  settings_axis *axes;
  with_words     ww;

  x->LaTeXpageno = x->current->AxesTextID;

  // Set colour for painting axes
  with_words_zero(&ww,0);
  if (x->current->settings.AxesColour > 0) { ww.colour = x->current->settings.AxesColour; ww.USEcolour = 1; }
  else                                     { ww.Col1234Space = x->current->settings.AxesCol1234Space; ww.colour1 = x->current->settings.AxesColour1; ww.colour2 = x->current->settings.AxesColour2; ww.colour3 = x->current->settings.AxesColour3; ww.colour4 = x->current->settings.AxesColour4; ww.USEcolour1234 = 1; }
  ww.linewidth = EPS_AXES_LINEWIDTH; ww.USElinewidth = 1;
  ww.linetype  = 1;                  ww.USElinetype  = 1;
  eps_core_SetColour(x, &ww, 1);
  IF_NOT_INVISIBLE eps_core_SetLinewidth(x, EPS_AXES_LINEWIDTH * EPS_DEFAULT_LINEWIDTH, 1, 0.0);

  x->current->PlotLeftMargin   = x->current->ThreeDim ? origin_x : (origin_x       );
  x->current->PlotRightMargin  = x->current->ThreeDim ? origin_x : (origin_x+width );
  x->current->PlotTopMargin    = x->current->ThreeDim ? origin_y : (origin_y+height);
  x->current->PlotBottomMargin = x->current->ThreeDim ? origin_y : (origin_y       );

  // Set CrossedAtZero flags
  for (j=0; j<3; j++)
   {
    if      (j==0) { axes = x->current->XAxes; }
    else if (j==1) { axes = x->current->YAxes; }
    else           { axes = x->current->ZAxes; }

    for (i=0; i<MAX_AXES; i++)
     if ((axes[i].FinalActive) && (!axes[i].invisible) && (axes[i].atzero))
      {
       if (j!=0) x->current->XAxes[1].CrossedAtZero = 1;
       if (j!=1) x->current->YAxes[1].CrossedAtZero = 1;
       if (j!=2) x->current->ZAxes[1].CrossedAtZero = 1;
       break;
      }
   }

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

      if (j==0) { axes = x->current->XAxes; BotPos = origin_y; TopPos = origin_y + height; }
      else      { axes = x->current->YAxes; BotPos = origin_x; TopPos = origin_x + width ; }

      for (i=0; i<MAX_AXES; i++)
       if ((axes[i].FinalActive) && (!axes[i].invisible))
        {
         int pageno = x->LaTeXpageno;
         if (axes[i].atzero)
          {
           settings_axis *PerpAxis = &(((j==0)?(x->current->YAxes):(x->current->XAxes))[1]); // Put axis at zero of either x- or y-axis
           int xrn;
           double ypos=0.5, ypos2, dummy;
           if (PerpAxis->FinalActive)
            {
             for (xrn=0; xrn<=PerpAxis->AxisValueTurnings; xrn++) // Perhaps more than once if x- or y-axis is non-monotonic
              {
               ypos = eps_plot_axis_GetPosition(0.0, PerpAxis, xrn, 0);
               if (gsl_finite(ypos))
                {
                 if (j==0) { ypos2=origin_y+ypos*height; x->LaTeXpageno=pageno; eps_plot_axispaint(x, &ww, axes+i, j, ypos, 0 ^ (ypos>0.999), origin_x, ypos2   , origin_x+width, ypos2          , &dummy, 1); }
                 else      { ypos2=origin_x+ypos*width ; x->LaTeXpageno=pageno; eps_plot_axispaint(x, &ww, axes+i, j, ypos, 1 ^ (ypos>0.999), ypos2   , origin_y, ypos2         , origin_y+height, &dummy, 1); }
                }
              }
            }
           else
            {
             if (j==0) { ypos2=origin_y+ypos*height; x->LaTeXpageno=pageno; eps_plot_axispaint(x, &ww, axes+i, j, ypos, 0 ^ (ypos>0.999), origin_x, ypos2   , origin_x+width, ypos2          , &dummy, 1); }
             else      { ypos2=origin_x+ypos*width ; x->LaTeXpageno=pageno; eps_plot_axispaint(x, &ww, axes+i, j, ypos, 1 ^ (ypos>0.999), ypos2   , origin_y, ypos2         , origin_y+height, &dummy, 1); }
            }
          }
         else // axis is notatzero... i.e. it is either at top or bottom of graph
          {
           if ((axes[i].MirrorType == SW_AXISMIRROR_AUTO) && (FirstAutoMirror[axes[i].topbottom]<0)) FirstAutoMirror[axes[i].topbottom] = i;

           if ((!axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_MIRROR) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR))
            {
             double BotPosInc;
             if (j==0) eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 0, origin_x, BotPos, origin_x+width, BotPos, &BotPosInc, (!axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR)); // Axis on bottom
             else      eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 1, BotPos, origin_y, BotPos, origin_y+height, &BotPosInc, (!axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR)); // Axis on left
             BotPos -= BotPosInc;
             Naxes[0]++;
            }
           if (( axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_MIRROR) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR))
            {
             double TopPosInc;
             if (j==0) eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 1, origin_x, TopPos, origin_x+width, TopPos, &TopPosInc, ( axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR)); // Axis on top
             else      eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 0, TopPos, origin_y, TopPos, origin_y+height, &TopPosInc, ( axes[i].topbottom) || (axes[i].MirrorType == SW_AXISMIRROR_FULLMIRROR)); // Axis on right
             TopPos += TopPosInc;
             Naxes[1]++;
            }
          }
        }

      // If there is no axis on bottom/left, but was an auto-mirrored axis on top/right, mirror it now
      if ((Naxes[0]==0) && (FirstAutoMirror[1]>=0))
       {
        double BotPosInc;
        i = FirstAutoMirror[1];
        if (j==0) eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 0, origin_x, BotPos, origin_x+width, BotPos , &BotPosInc, 0); // Axis on bottom
        else      eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 1, BotPos, origin_y, BotPos, origin_y+height, &BotPosInc, 0); // Axis on left
        BotPos -= BotPosInc;
        Naxes[0]++;
       }

      // If there is no axis on top/right, but was an auto-mirrored axis on bottom/left, mirror it now
      if ((Naxes[1]==0) && (FirstAutoMirror[0]>=0))
       {
        double TopPosInc;
        i = FirstAutoMirror[0];
        if (j==0) eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 1, origin_x, TopPos, origin_x+width, TopPos , &TopPosInc, 0); // Axis on top
        else      eps_plot_axispaint(x, &ww, axes+i, j, GSL_NAN, 0, TopPos, origin_y, TopPos, origin_y+height, &TopPosInc, 0); // Axis on right
        TopPos += TopPosInc;
        Naxes[0]++;
       }
     }
   }

  return;
 }

