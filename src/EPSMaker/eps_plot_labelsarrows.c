// eps_plot_labelsarrows.c
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

#define _PPL_EPS_PLOT_LABELSARROWS 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ListTools/lt_memory.h"

#include "ppl_canvasdraw.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units_fns.h"

#include "eps_arrow.h"
#include "eps_colours.h"
#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_canvas.h"
#include "eps_plot_labelsarrows.h"
#include "eps_plot_legend.h"
#include "eps_settings.h"

void eps_plot_labelsarrows_YieldUpText(EPSComm *x)
 {
  label_object   *li;
  CanvasTextItem *i;

  for (li=x->current->label_list; li!=NULL; li=li->next) { YIELD_TEXTITEM(li->text); }
  return;
 }

#define FETCH_AXES(SYSTEM, XA, XAXES, AXISN, XIN, XOUT) \
     XOUT = XIN.real; \
     if      (SYSTEM == SW_SYSTEM_FIRST ) { XA = &(XAXES[1]); } \
     else if (SYSTEM == SW_SYSTEM_SECOND) { XA = &(XAXES[2]); } \
     else if (SYSTEM == SW_SYSTEM_AXISN ) { if ((AXISN<0)||(AXISN>=MAX_AXES)) { XA = NULL; XOUT = 0.0; ppl_error(ERR_INTERNAL,"Axis number out of range"); } else { XA = &(XAXES[AXISN]); } } \
     else                                 { XA = NULL; XOUT = 0.0; } \
     if (XA != NULL) \
      { \
       if (XA->DataUnitSet && (!ppl_units_DimEqual(&(XIN),&(XA->DataUnit)))) { sprintf(temp_err_string, "Position specified for %s dimensionally incompatible with the axes used. Position has units of <%s> while axis has units of <%s>.", ItemName, ppl_units_GetUnitStr(&(XIN), NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr(&(XA->DataUnit), NULL, NULL, 1, 1, 0)); ppl_error(ERR_NUMERIC,temp_err_string); XA=NULL; XOUT=0.5; status=1; } \
       else if (!XA->DataUnitSet) { XA=NULL; XOUT=0.5; } \
      }

#define ADD_PAGE_COORDINATES(SYSTEM, XIN, THETA_X) \
     if ((SYSTEM==SW_SYSTEM_PAGE)||(SYSTEM==SW_SYSTEM_GRAPH)) \
      { \
       xpos += XIN.real * sin(THETA_X) * M_TO_PS; \
       ypos += XIN.real * cos(THETA_X) * M_TO_PS; \
      }

void eps_plot_labelsarrows(EPSComm *x, double origin_x, double origin_y, double width, double height)
 {
  int           pageno, hal, val;
  char          ItemName[64];
  arrow_object *ai;
  label_object *li;
  with_words    ww, ww_default;

  with_words_zero(&ww_default,0);
  ww_default.USEcolour   = 1;
  ww_default.colour      = COLOUR_BLACK;
  ww_default.USElinetype = ww_default.USElinewidth = 1;
  ww_default.linetype    = ww_default.linewidth = 1.0;

  pageno = x->LaTeXpageno = x->current->SetLabelTextID;

  // Loop through all arrows, rendering them in turn
  for (ai=x->current->arrow_list; ai!=NULL; ai=ai->next)
   {
    settings_axis *xa0, *ya0, *za0=NULL, *xa1, *ya1, *za1=NULL;
    double         xin0, yin0, zin0=0.5, xin1, yin1, zin1=0.5;
    double         xpos, ypos, xpos0, ypos0, xpos1, ypos1, depth, xap, yap, zap, theta_x, theta_y, theta_z;
    int            status=0, xrn0, yrn0, zrn0, xrn1, yrn1, zrn1;
    sprintf(ItemName, "arrow %d on plot %d", ai->id, x->current->id);
    FETCH_AXES(ai->system_x0, xa0, x->current->XAxes, ai->axis_x0, ai->x0, xin0);
    FETCH_AXES(ai->system_y0, ya0, x->current->YAxes, ai->axis_y0, ai->y0, yin0);
    if (x->current->ThreeDim) { FETCH_AXES(ai->system_z0, za0, x->current->ZAxes, ai->axis_z0, ai->z0, zin0); }
    FETCH_AXES(ai->system_x1, xa1, x->current->XAxes, ai->axis_x1, ai->x1, xin1);
    FETCH_AXES(ai->system_y1, ya1, x->current->YAxes, ai->axis_y1, ai->y1, yin1);
    if (x->current->ThreeDim) { FETCH_AXES(ai->system_z1, za1, x->current->ZAxes, ai->axis_z1, ai->z1, zin1); }
    if (!status)
     for (xrn0=0; xrn0<=(                         (xa0!=NULL)  ? xa0->AxisValueTurnings : 0); xrn0++)
     for (yrn0=0; yrn0<=(                         (ya0!=NULL)  ? ya0->AxisValueTurnings : 0); yrn0++)
     for (zrn0=0; zrn0<=((x->current->ThreeDim && (za0!=NULL)) ? za0->AxisValueTurnings : 0); zrn0++)
     for (xrn1=0; xrn1<=(                         (xa1!=NULL)  ? xa1->AxisValueTurnings : 0); xrn1++)
     for (yrn1=0; yrn1<=(                         (ya1!=NULL)  ? ya1->AxisValueTurnings : 0); yrn1++)
     for (zrn1=0; zrn1<=((x->current->ThreeDim && (za1!=NULL)) ? za1->AxisValueTurnings : 0); zrn1++)
      {
       if ((xa0==xa1)&&(xrn0!=xrn1)) continue;
       if ((ya0==ya1)&&(yrn0!=yrn1)) continue;
       if ((za0==za1)&&(zrn0!=zrn1)) continue;
       eps_plot_GetPosition(&xpos0, &ypos0, &depth, &xap, &yap, &zap, &theta_x, &theta_y, &theta_z, x->current->ThreeDim, xin0, yin0, zin0, xa0, ya0, za0, xrn0, yrn0, zrn0, &x->current->settings, origin_x, origin_y, width, height, width, 0);
       eps_plot_GetPosition(&xpos1, &ypos1, &depth, &xap, &yap, &zap, &theta_x, &theta_y, &theta_z, x->current->ThreeDim, xin1, yin1, zin1, xa1, ya1, za1, xrn1, yrn1, zrn1, &x->current->settings, origin_x, origin_y, width, height, width, 0);
       xpos=xpos0; ypos=ypos0;
       ADD_PAGE_COORDINATES(ai->system_x0, ai->x0, theta_x);
       ADD_PAGE_COORDINATES(ai->system_y0, ai->y0, theta_y);
       if (x->current->ThreeDim) { ADD_PAGE_COORDINATES(ai->system_z0, ai->z0, theta_z); }
       xpos0=xpos; ypos0=ypos; xpos=xpos1; ypos=ypos1;
       ADD_PAGE_COORDINATES(ai->system_x1, ai->x1, theta_x);
       ADD_PAGE_COORDINATES(ai->system_y1, ai->y1, theta_y);
       xpos1=xpos; ypos1=ypos;
       if (x->current->ThreeDim) { ADD_PAGE_COORDINATES(ai->system_z1, ai->z1, theta_z); }
       with_words_merge(&ww, &ai->style, &ww_default, NULL, NULL, NULL, 1);
       if ((gsl_finite(xpos0))&&(gsl_finite(ypos0))&&(gsl_finite(xpos1))&&(gsl_finite(ypos1))) eps_primitive_arrow(x, ai->arrow_style, xpos0, ypos0, xpos1, ypos1, &ww);
      }
   }

  // By default, use 'set textcolour' colour for text labels
  if (x->current->settings.TextColour > 0) { ww_default.USEcolourRGB = 0; ww_default.USEcolour = 1; ww_default.colour = x->current->settings.TextColour; }
  else                                     { ww_default.USEcolourRGB = 1; ww_default.USEcolour = 0; ww_default.colourR = x->current->settings.TextColourR; ww_default.colourG = x->current->settings.TextColourG; ww_default.colourB = x->current->settings.TextColourB; } 

  // Loop through all text labels, rendering them in turn
  for (li=x->current->label_list; li!=NULL; li=li->next)
   if ((li->text!=NULL)&&(li->text[0]!='\0'))
    {
     settings_axis *xa, *ya, *za=NULL;
     double         xin, yin, zin=0.5;
     double        xpos, ypos, depth, xap, yap, zap, theta_x, theta_y, theta_z;
     int            status=0, xrn, yrn, zrn;
     sprintf(ItemName, "label %d on plot %d", li->id, x->current->id);
     FETCH_AXES(li->system_x, xa, x->current->XAxes, li->axis_x, li->x, xin);
     FETCH_AXES(li->system_y, ya, x->current->YAxes, li->axis_y, li->y, yin);
     if (x->current->ThreeDim) { FETCH_AXES(li->system_z, za, x->current->ZAxes, li->axis_z, li->z, zin); }
     if (!status)
      for (xrn=0; xrn<=(                         (xa!=NULL)  ? xa->AxisValueTurnings : 0); xrn++)
      for (yrn=0; yrn<=(                         (ya!=NULL)  ? ya->AxisValueTurnings : 0); yrn++)
      for (zrn=0; zrn<=((x->current->ThreeDim && (za!=NULL)) ? za->AxisValueTurnings : 0); zrn++)
      {
       double xgap,ygap,xgap2,ygap2;
       eps_plot_GetPosition(&xpos, &ypos, &depth, &xap, &yap, &zap, &theta_x, &theta_y, &theta_z, x->current->ThreeDim, xin, yin, zin, xa, ya, za, xrn, yrn, zrn, &x->current->settings, origin_x, origin_y, width, height, width, 0);
       ADD_PAGE_COORDINATES(li->system_x, li->x, theta_x);
       ADD_PAGE_COORDINATES(li->system_y, li->y, theta_y);
       if (x->current->ThreeDim) { ADD_PAGE_COORDINATES(li->system_z, li->z, theta_z); }
       if (li->HAlign != 0) hal = li->HAlign;
       else                 hal = x->current->settings.TextHAlign;
       if (li->VAlign != 0) val = li->VAlign;
       else                 val = x->current->settings.TextVAlign;
       xgap  = -(hal - SW_HALIGN_CENT) * li->gap;
       ygap  =  (val - SW_VALIGN_CENT) * li->gap;
       xgap2 = xgap*cos(li->rotation) - ygap*sin(li->rotation);
       ygap2 = xgap*sin(li->rotation) + ygap*cos(li->rotation);
       with_words_merge(&ww, &li->style, &ww_default, NULL, NULL, NULL, 1);
       eps_core_SetColour(x, &ww, 1);
       IF_NOT_INVISIBLE if ((gsl_finite(xpos))&&(gsl_finite(ypos)))
           canvas_EPSRenderTextItem(x, pageno, xpos/M_TO_PS+xgap2, ypos/M_TO_PS+ygap2, hal, val, x->LastEPSColour, x->current->settings.FontSize, li->rotation, NULL, NULL);
      }
     pageno++;
    }

  return;
 }

