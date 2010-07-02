// eps_plot_contourmap.c
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

#define _PPL_EPS_PLOT_CONTOURMAP_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "ListTools/lt_memory.h"

#include "MathsTools/dcfmath.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_colours.h"
#include "eps_plot.h"
#include "eps_plot_canvas.h"
#include "eps_plot_legend.h"
#include "eps_settings.h"
#include "eps_style.h"

// Yield up text items which label contours on a contourmap
void eps_plot_contourmap_YieldText(EPSComm *x, DataTable *data, settings_graph *sg, canvas_plotdesc *pd)
 {
  DataBlock     *blk;
  int            XSize = (x->current->settings.SamplesXAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesX;
  int            YSize = (x->current->settings.SamplesYAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesY;
  int            i, j, k, Ncol;
  double         CMin, CMax, min_prelim, max_prelim, OoM, UnitMultiplier;
  unsigned char  CMinAuto, CMinSet, CMaxAuto, CMaxSet, CLog;
  char          *UnitString;

  // Check that we have some data
  if ((data==NULL) || (data->Nrows<1)) return; // No data present
  Ncol = data->Ncolumns;
  blk = data->first;

  // Work out normalisation of variable c1
  CMinAuto = (sg->Cminauto[0]==SW_BOOL_TRUE);
  CMinSet  = !CMinAuto;
  CMin     = sg->Cmin[0].real;
  CMaxAuto = (sg->Cmaxauto[0]==SW_BOOL_TRUE);
  CMaxSet  = !CMaxAuto;
  CMax     = sg->Cmax[0].real;
  CLog     = (sg->Clog[0]==SW_BOOL_TRUE);

  // Find extremal values
  if (CMinAuto || CMaxAuto)
   for (j=0; j<YSize; j++)
    for (i=0; i<XSize; i++)
     {
      double val = blk->data_real[2 + Ncol*(i+XSize*j)].d;
      if (!gsl_finite(val)) continue;
      if ((CMinAuto) && ((!CMinSet) || (CMin>val)) && ((!CLog)||(val>0.0))) { CMin=val; CMinSet=1; }
      if ((CMaxAuto) && ((!CMaxSet) || (CMax<val)) && ((!CLog)||(val>0.0))) { CMax=val; CMaxSet=1; }
     }

  // If no data present, stop now
  if ((!CMinSet)||(!CMaxSet)) return;

  // If log spacing, make sure range is strictly positive
  if (CLog && (CMin<1e-200)) CMin=1e-200;
  if (CLog && (CMax<1e-200)) CMax=1e-200;

  // If there's no spread of data, make a spread up
  if ( (fabs(CMax) <= fabs(CMin)) || (fabs(CMin-CMax) <= fabs(1e-14*CMax)) )
   {
    if (!CLog)
     {
      double step = max(1.0,1e-3*fabs(CMin));
      CMin -= step; CMax += step;
     }
    else
     {
      if (CMin > 1e-300) CMin /= 10.0;
      if (CMax <  1e300) CMax *= 10.0;
     }
   }

  // Work out units in which contours will be labelled
  pd->CRangeUnit             = data->FirstEntries[2];
  pd->CRangeUnit.FlagComplex = 0;
  pd->CRangeUnit.imag        = 0.0;
  pd->CRangeUnit.real        = CLog ?  (CMin * sqrt(CMax/CMin))
                                    : ((CMin + CMax)/2);
  UnitString = ppl_units_GetUnitStr(&pd->CRangeUnit,&UnitMultiplier,NULL,0,0,SW_DISPLAY_L);
  UnitMultiplier /= pd->CRangeUnit.real;
  if (!gsl_finite(UnitMultiplier)) UnitMultiplier=1.0;

  // Round range outwards to round endpoints
  min_prelim = CMin * UnitMultiplier;
  max_prelim = CMax * UnitMultiplier;

  if (CLog) { min_prelim = log10(min_prelim); max_prelim = log10(max_prelim); }

  OoM = pow(10.0, floor(log10(fabs(max_prelim - min_prelim)/5)));
  min_prelim = floor(min_prelim / OoM) * OoM;
  max_prelim = ceil (max_prelim / OoM) * OoM;

  if (CLog) { min_prelim = pow(10.0,min_prelim); max_prelim = pow(10.0,max_prelim); }

  min_prelim /= UnitMultiplier;
  max_prelim /= UnitMultiplier;

  if (gsl_finite(min_prelim) && (CLog||(min_prelim>1e-300))) CMin = min_prelim;
  if (gsl_finite(max_prelim) && (CLog||(min_prelim>1e-300))) CMax = max_prelim;

  // Loop over all contours submitting labels
  for (k=0;
       (k<MAX_CONTOURS) && (  ((sg->ContoursListLen< 0) && (k<sg->ContoursN))  ||
                              ((sg->ContoursListLen>=0) && (k<sg->ContoursListLen))  );
       k++)
   {
    CanvasTextItem *i;
    value           v = pd->CRangeUnit;

    if (sg->ContoursListLen< 0) v.real = CLog?(CMin+(CMax-CMin)*((double)(k+1)/(sg->ContoursN+1)))
                                             :(CMin+pow(CMax/CMin,((double)(k+1)/(sg->ContoursN+1))));
    else                        v.real = sg->ContoursList[j];

    sprintf(UnitString,"%s",ppl_units_NumericDisplay(&v,0,SW_DISPLAY_L,-1));
    YIELD_TEXTITEM(UnitString);
   }

  // Store range of values for which contours will be drawn
  pd->CMinFinal     = CMin;
  pd->CMaxFinal     = CMax;
  return;
 }

// Render a contourmap to postscript
int  eps_plot_contourmap(EPSComm *x, DataTable *data, unsigned char ThreeDim, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, double origin_x, double origin_y, double width, double height, double zdepth)
 {
  double         scale_x, scale_y, scale_z;
  DataBlock     *blk;
  int            XSize = (x->current->settings.SamplesXAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesX;
  int            YSize = (x->current->settings.SamplesYAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesY;
  int            i, j, k, Ncol;
  double         xo, yo, Lx, Ly, ThetaX, ThetaY, CMin, CMax;
  unsigned char  CLog, CMinAuto, CMaxAuto;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present
  Ncol = data->Ncolumns;
  if (eps_plot_WithWordsCheckUsingItemsDimLess(&pd->ww_final, data->FirstEntries, Ncol, NULL)) return 1;
  if (!ThreeDim) { scale_x=width; scale_y=height; scale_z=1.0;    }
  else           { scale_x=width; scale_y=height; scale_z=zdepth; }
  blk = data->first;

  // Work out orientation of contourmap
  if (!ThreeDim)
   {
    xo     = origin_x;
    yo     = origin_y;
    Lx     = scale_x;
    Ly     = scale_y;
    ThetaX = M_PI/2;
    ThetaY = 0.0;
    if (xn!=0) { double t1=Lx, t2=ThetaX; Lx=Ly; ThetaX=ThetaY; Ly=t1; ThetaY=t2; }
   }
  else // 3D case: put colour map on back face of cuboid
   {
    double ap[3]={0.5,0.5,0.0}, xtmp, ytmp, z0tmp, z1tmp;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z0tmp);
    ap[2]=1.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z1tmp);
    ap[2]=(z1tmp>z0tmp)?1.0:0.0; // Determine whether zap=0 or zap=1 represents back of cuboid

    ap[0]=ap[1]=0.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xo, &yo, &z0tmp);

    ap[0]=1.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z0tmp);
    Lx = hypot(xtmp-xo, ytmp-yo);
    ThetaX = atan2(xtmp-xo, ytmp-yo);
    if (!gsl_finite(ThetaX)) ThetaX = 0.0;

    ap[0]=0.0;
    ap[1]=1.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z0tmp);
    Ly = hypot(xtmp-xo, ytmp-yo);
    ThetaY = atan2(xtmp-xo, ytmp-yo);
    if (!gsl_finite(ThetaY)) ThetaY = 0.0;
   }

  // Look up normalisation of variable c1
  CLog     = (sg->Clog[0]==SW_BOOL_TRUE);
  CMin     = pd->CMinFinal;
  CMax     = pd->CMaxFinal;
  CMinAuto = (sg->Cminauto[0]==SW_BOOL_TRUE);
  CMaxAuto = (sg->Cmaxauto[0]==SW_BOOL_TRUE);

  // If no data present, stop now
  if ((CMin==0)&&(CMax==0))
   {
    sprintf(temp_err_string, "No data supplied to determine range for variable c1");
    return 0;
   }

  // Output result to debugging output
  if (DEBUG)
   {
    int SF = settings_term_current.SignificantFigures;
    sprintf(temp_err_string, "Range for variable c1 is [%s:%s]", NumericDisplay(CMin,0,SF,0), NumericDisplay(CMax,1,SF,0));
    ppl_log(temp_err_string);
   }

  // Check that variable c1 has appropriate units
  if ( ((!CMinAuto)||(!CMaxAuto)) && (!ppl_units_DimEqual(&data->FirstEntries[2] , (sg->Cminauto[0]==SW_BOOL_TRUE)?(&sg->Cmax[0]):(&sg->Cmin[0]))) )
   {
    sprintf(temp_err_string, "Column 3 of data supplied to the colourmap plot style has conflicting units with those set in the 'set crange' command. The former has units of <%s> whilst the latter has units of <%s>.", ppl_units_GetUnitStr(&data->FirstEntries[2], NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr((sg->Cminauto[0]==SW_BOOL_TRUE)?(&sg->Cmax[0]):(&sg->Cmin[0]), NULL, NULL, 1, 1, 0));
    ppl_error(ERR_NUMERIC,-1,-1,temp_err_string);
    return 1;
   }

  // Loop over contours
  for (k=0;
       (k<MAX_CONTOURS) && (  ((sg->ContoursListLen< 0) && (k<sg->ContoursN))  ||
                              ((sg->ContoursListLen>=0) && (k<sg->ContoursListLen))  );
       k++)
   {
    value v = pd->CRangeUnit;

    if (sg->ContoursListLen< 0) v.real = CLog?(CMin+(CMax-CMin)*((double)(k+1)/(sg->ContoursN+1)))
                                             :(CMin+pow(CMax/CMin,((double)(k+1)/(sg->ContoursN+1))));
    else                        v.real = sg->ContoursList[j];

    // Reset contour map usage flags
    for (j=0; j<YSize; j++)
     for (i=0; i<XSize; i++)
      blk->split[i+XSize*j] = 0;

   }

  return 0;
 }

