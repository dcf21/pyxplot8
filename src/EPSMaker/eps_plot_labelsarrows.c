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
#include "eps_plot.h"
#include "eps_plot_canvas.h"
#include "eps_plot_labelsarrows.h"
#include "eps_plot_legend.h"
#include "eps_plot_threedimbuff.h"
#include "eps_settings.h"

void eps_plot_labelsarrows_YieldUpText(EPSComm *x)
 {
  label_object   *li;
  CanvasTextItem *i;

  for (li=x->current->label_list; li!=NULL; li=li->next) { YIELD_TEXTITEM(li->text); }
  return;
 }

#define FETCH_AXES(SYSTEM, XA, XAXES, AXISN, XIN, XOUT) \
 { \
     XOUT = XIN.real; \
     if      (SYSTEM == SW_SYSTEM_FIRST ) { XA = &(XAXES[1]); } \
     else if (SYSTEM == SW_SYSTEM_SECOND) { XA = &(XAXES[2]); } \
     else if (SYSTEM == SW_SYSTEM_AXISN ) { if ((AXISN<0)||(AXISN>=MAX_AXES)) { XA = NULL; XOUT = 0.0; ppl_error(ERR_INTERNAL, -1, -1,"Axis number out of range"); } else { XA = &(XAXES[AXISN]); } } \
     else                                 { XA = NULL; XOUT = 0.0; } \
     if (XA != NULL) \
      { \
       if (!XA->RangeFinalised) { XA = &(XAXES[1]); } /* Default to first axis if desired axis doesn't exist */ \
       if (XA->HardUnitSet && (!ppl_units_DimEqual(&(XIN),&(XA->HardUnit)))) { sprintf(temp_err_string, "Position specified for %s dimensionally incompatible with the range specified for the axis. Position has units of <%s> while axis has units of <%s>.", ItemName, ppl_units_GetUnitStr(&(XIN), NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr(&(XA->HardUnit), NULL, NULL, 1, 1, 0)); ppl_error(ERR_NUMERIC, -1, -1,temp_err_string); XA=NULL; XOUT=0.5; status=1; } \
       if (XA->DataUnitSet && (!ppl_units_DimEqual(&(XIN),&(XA->DataUnit)))) { sprintf(temp_err_string, "Position specified for %s dimensionally incompatible with data plotted against the axis. Position has units of <%s> while axis has units of <%s>.", ItemName, ppl_units_GetUnitStr(&(XIN), NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr(&(XA->DataUnit), NULL, NULL, 1, 1, 0)); ppl_error(ERR_NUMERIC, -1, -1,temp_err_string); XA=NULL; XOUT=0.5; status=1; } \
       if (!XA->RangeFinalised) { XA=NULL; XOUT=0.5; } \
      } \
 }

#define ADD_PAGE_COORDINATES(SYSTEM, XIN, THETA_X) \
     if ((SYSTEM==SW_SYSTEM_PAGE)||(SYSTEM==SW_SYSTEM_GRAPH)) \
      { \
       xpos_ += XIN.real * sin(THETA_X) * M_TO_PS; \
       ypos_ += XIN.real * cos(THETA_X) * M_TO_PS; \
      }

void eps_plot_labelsarrows(EPSComm *x, double origin_x, double origin_y, double width, double height, double zdepth)
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
    double         xpos_, ypos_, xpos[2], ypos[2], depth[2];
    double         xap[2], yap[2], zap[2], theta_x, theta_y, theta_z;
    int            status=0, xrn0, yrn0, zrn0, xrn1, yrn1, zrn1;
    int            ArrowType=ai->arrow_style;
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
       eps_plot_GetPosition(&xpos[0], &ypos[0], &depth[0], &xap[0], &yap[0], &zap[0], &theta_x, &theta_y, &theta_z, x->current->ThreeDim, xin0, yin0, zin0, xa0, ya0, za0, xrn0, yrn0, zrn0, &x->current->settings, origin_x, origin_y, width, height, zdepth, 1);
       eps_plot_GetPosition(&xpos[1], &ypos[1], &depth[1], &xap[1], &yap[1], &zap[1], &theta_x, &theta_y, &theta_z, x->current->ThreeDim, xin1, yin1, zin1, xa1, ya1, za1, xrn1, yrn1, zrn1, &x->current->settings, origin_x, origin_y, width, height, zdepth, 1);
       if (((xap[0]<0)||(xap[0]>1)||(yap[0]<0)||(yap[0]>1)||(zap[0]<0)||(zap[0]>1)) && ((xa0==NULL)||(ya0==NULL)||(x->current->ThreeDim&&(za0==NULL))||(xa1==NULL)||(ya1==NULL)||(x->current->ThreeDim&&(za1==NULL)))) continue;
       xpos_=xpos[0]; ypos_=ypos[0];
       ADD_PAGE_COORDINATES(ai->system_x0, ai->x0, theta_x);
       ADD_PAGE_COORDINATES(ai->system_y0, ai->y0, theta_y);
       if (x->current->ThreeDim) { ADD_PAGE_COORDINATES(ai->system_z0, ai->z0, theta_z); }
       xpos[0]=xpos_; ypos[0]=ypos_; xpos_=xpos[1]; ypos_=ypos[1];
       ADD_PAGE_COORDINATES(ai->system_x1, ai->x1, theta_x);
       ADD_PAGE_COORDINATES(ai->system_y1, ai->y1, theta_y);
       if (x->current->ThreeDim) { ADD_PAGE_COORDINATES(ai->system_z1, ai->z1, theta_z); }
       xpos[1]=xpos_; ypos[1]=ypos_;

        {
         const double grad[3][6] = { // d[x y z xap yap zap]/d[xap yap zap]
           { (xap[1]==xap[0])?0.0:(xpos[1] -xpos[0] )/(xap[1]-xap[0]),
             (xap[1]==xap[0])?0.0:(ypos[1] -ypos[0] )/(xap[1]-xap[0]),
             (xap[1]==xap[0])?0.0:(depth[1]-depth[0])/(xap[1]-xap[0]),
             1.0,
             (xap[1]==xap[0])?0.0:(yap[1]  -yap[0]  )/(xap[1]-xap[0]),
             (xap[1]==xap[0])?0.0:(zap[1]  -zap[0]  )/(xap[1]-xap[0]),
         },{ (yap[1]==yap[0])?0.0:(xpos[1] -xpos[0] )/(yap[1]-yap[0]),
             (yap[1]==yap[0])?0.0:(ypos[1] -ypos[0] )/(yap[1]-yap[0]),
             (yap[1]==yap[0])?0.0:(depth[1]-depth[0])/(yap[1]-yap[0]),
             (yap[1]==yap[0])?0.0:(xap[1]  -xap[0]  )/(yap[1]-yap[0]),
             1.0,
             (yap[1]==yap[0])?0.0:(zap[1]  -zap[0]  )/(yap[1]-yap[0]),
         },{ (zap[1]==zap[0])?0.0:(xpos[1] -xpos[0] )/(zap[1]-zap[0]),
             (zap[1]==zap[0])?0.0:(ypos[1] -ypos[0] )/(zap[1]-zap[0]),
             (zap[1]==zap[0])?0.0:(depth[1]-depth[0])/(zap[1]-zap[0]),
             (zap[1]==zap[0])?0.0:(xap[1]  -xap[0]  )/(zap[1]-zap[0]),
             (zap[1]==zap[0])?0.0:(yap[1]  -yap[0]  )/(zap[1]-zap[0]),
             1.0
          }};

#define CLIP(XAP, XYZ, XAPV, P) \
         if (XAPV ? (XAP[P]>1) : (XAP[P]<0) ) \
          { \
           const double t=XAP[P]; \
           if (XAP[1-P]<0.0) continue; \
           xpos[P]  += grad[XYZ][0]*(XAPV-t); \
           ypos[P]  += grad[XYZ][1]*(XAPV-t); \
           depth[P] += grad[XYZ][2]*(XAPV-t); \
           xap[P]   += grad[XYZ][3]*(XAPV-t); \
           yap[P]   += grad[XYZ][4]*(XAPV-t); \
           zap[P]   += grad[XYZ][5]*(XAPV-t); \
 \
           if (P==0) \
            { \
             if (ArrowType==SW_ARROWTYPE_TWOWAY) ArrowType=SW_ARROWTYPE_HEAD; \
            } \
           else \
            { \
             if (ArrowType==SW_ARROWTYPE_HEAD  ) ArrowType=SW_ARROWTYPE_NOHEAD; \
             if (ArrowType==SW_ARROWTYPE_TWOWAY) \
              { \
               double t1=xpos[0],t2=ypos[0],t3=depth[0],t4=xap[0],t5=yap[0],t6=zap[0]; \
               xpos[0]=xpos[1]; ypos[0]=ypos[1]; depth[0]=depth[1]; xap[0]=xap[1]; yap[0]=yap[1]; zap[0]=zap[1]; \
               xpos[1]=t1;      ypos[1]=t2;      depth[1]=t3;       xap[1]=t4;     yap[1]=t5;     zap[1]=t6; \
               ArrowType=SW_ARROWTYPE_HEAD; \
              } \
            } \
          }

         CLIP(xap, 0, 0.0, 0); CLIP(xap, 0, 1.0, 0); CLIP(xap, 0, 0.0, 1); CLIP(xap, 0, 1.0, 1);
         CLIP(yap, 1, 0.0, 0); CLIP(yap, 1, 1.0, 0); CLIP(yap, 1, 0.0, 1); CLIP(yap, 1, 1.0, 1);
         CLIP(zap, 2, 0.0, 0); CLIP(zap, 2, 1.0, 0); CLIP(zap, 2, 0.0, 1); CLIP(zap, 2, 1.0, 1);
        }

       with_words_merge(&ww, &ai->style, &ww_default, NULL, NULL, NULL, 1);
       if ((gsl_finite(xpos[0]))&&(gsl_finite(ypos[0]))&&(gsl_finite(xpos[1]))&&(gsl_finite(ypos[1])))
         eps_primitive_arrow(x, ArrowType, xpos[0], ypos[0], x->current->ThreeDim?&depth[0]:NULL, xpos[1], ypos[1], x->current->ThreeDim?&depth[1]:NULL, &ww);
      }
   }

  // By default, use 'set textcolour' colour for text labels
  if (x->current->settings.TextColour > 0) { ww_default.colour = x->current->settings.TextColour; ww_default.USEcolour = 1; ww_default.USEcolour1234 = 0; }
  else                                     { ww_default.Col1234Space = x->current->settings.TextCol1234Space; ww_default.colour1 = x->current->settings.TextColour1; ww_default.colour2 = x->current->settings.TextColour2; ww_default.colour3 = x->current->settings.TextColour3; ww_default.colour4 = x->current->settings.TextColour4; ww_default.USEcolour = 0; ww_default.USEcolour1234 = 1; }

  // Loop through all text labels, rendering them in turn
  for (li=x->current->label_list; li!=NULL; li=li->next)
   if ((li->text!=NULL)&&(li->text[0]!='\0'))
    {
     settings_axis *xa, *ya, *za=NULL;
     double         xin, yin, zin=0.5;
     double         xpos_, ypos_, depth, xap, yap, zap, theta_x, theta_y, theta_z;
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
       eps_plot_GetPosition(&xpos_, &ypos_, &depth, &xap, &yap, &zap, &theta_x, &theta_y, &theta_z, x->current->ThreeDim, xin, yin, zin, xa, ya, za, xrn, yrn, zrn, &x->current->settings, origin_x, origin_y, width, height, zdepth, 0);
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
       eps_core_SetColour(x, &ww, 0);
       IF_NOT_INVISIBLE if ((gsl_finite(xpos_))&&(gsl_finite(ypos_))&&(gsl_finite(depth)))
        {
         char *colstr=NULL, *text=NULL;
         colstr = (char *)lt_malloc(strlen(x->CurrentColour)+1);
         if (colstr!=NULL)
          {
           strcpy(colstr, x->CurrentColour);
           canvas_EPSRenderTextItem(x, &text, pageno, xpos_/M_TO_PS+xgap2, ypos_/M_TO_PS+ygap2, hal, val, x->CurrentColour, x->current->settings.FontSize, li->rotation, NULL, NULL);
           if (text!=NULL) ThreeDimBuffer_writeps(x, depth, 1, 1, 0, 1, colstr, text);
          }
        }
      }
     pageno++;
    }

  return;
 }

