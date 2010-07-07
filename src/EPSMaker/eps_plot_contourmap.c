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

// Codes for the four sides of a cell
#define FACE_ALL -1
#define FACE_T    1
#define FACE_R    2
#define FACE_B    3
#define FACE_L    4

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

// Routines for tracking the paths of contours

// -----+-----+-----+-----+
//      |     |     |     |  blk->split[X] contains flags indicating whether crossing points on lines
//      |     |     |     |  A (in bit 1) and B (in bit 2) have been used.
//      |     |--A--|     |  on successive passes, bits (3,4), (5,6) are used, etc.
// -----+-----X-----+-----+
//      |    ||     |     |
//      |    B|     |     |
//      |    ||     |     |
// -----+-----+-----+-----+
//      |     |     |     |
//      |     |     |     |
//      |     |     |     |
// -----+-----+-----+-----+


static int IsBetween(double x, double a, double b, double *f)
 {
  *f=0.5;
  if  (a==b)                  {                 return x==a; }
  if ((a< b)&&(x>=a)&&(x<=b)) { *f=(x-a)/(b-a); return 1;    }
  if ((a> b)&&(x<=a)&&(x>=b)) { *f=(x-a)/(b-a); return 1;    }
  return 0;
 }

// See whether cell X contains any unused starting points for contours
static int GetStartPoint(double c1, DataTable *data, int pass, int XSize, int YSize, int x0, int y0, int EntryFace, int *ExitFace, int *xcell, int *ycell, double *Xout, double *Yout)
 {
  DataBlock *blk  = data->first;
  int        Ncol = data->Ncolumns;
  double     f;

  *xcell = x0;
  *ycell = y0;

  if (((EntryFace<0)||(EntryFace==FACE_T)) && (x0<=XSize-2) && (x0>= 0) && (y0>= 0) && (y0<=YSize-1) &&
       (((blk->split[(x0  )+(y0  )*XSize]>>(0+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0  )*XSize)].d,&f))
   { blk->split[(x0  )+(y0  )*XSize] |= 1<<(0+2*pass); *ExitFace=FACE_T; *Xout=x0+f; *Yout=y0; return 1; }

  if (((EntryFace<0)||(EntryFace==FACE_R)) && (x0<=XSize-2) && (x0>=-1) && (y0>= 0) && (y0<=YSize-2) &&
       (((blk->split[(x0+1)+(y0  )*XSize]>>(1+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0+1)+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0+1)*XSize)].d,&f))
   { blk->split[(x0+1)+(y0  )*XSize] |= 1<<(1+2*pass); *ExitFace=FACE_R; *Xout=x0+1; *Yout=y0+f; return 1; }

  if (((EntryFace<0)||(EntryFace==FACE_B)) && (x0<=XSize-2) && (x0>= 0) && (y0>=-1) && (y0<=YSize-2) &&
       (((blk->split[(x0  )+(y0+1)*XSize]>>(0+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0+1)*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0+1)*XSize)].d,&f))
   { blk->split[(x0  )+(y0+1)*XSize] |= 1<<(0+2*pass); *ExitFace=FACE_B; *Xout=x0+f; *Yout=y0+1; return 1; }

  if (((EntryFace<0)||(EntryFace==FACE_L)) && (x0<=XSize-1) && (x0>= 0) && (y0>= 0) && (y0<=YSize-2) &&
       (((blk->split[(x0  )+(y0  )*XSize]>>(1+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0  )+(y0+1)*XSize)].d,&f))
   { blk->split[(x0  )+(y0  )*XSize] |= 1<<(1+2*pass); *ExitFace=FACE_L; *Xout=x0; *Yout=y0+f; return 1; }
  return 0;
 }

// Given a contour already tracking through cell X from a given face, where to go next?
static int GetNextPoint(double c1, DataTable *data, int pass, int XSize, int YSize, int x0, int y0, int EntryFace, int *ExitFace, int *xcell, int *ycell, double *Xout, double *Yout)
 {
  DataBlock *blk  = data->first;
  int        Ncol = data->Ncolumns;
  int        j;
  double     f;

  for (j=0; j<2; j++)
   {
    if (((j==1       )||(EntryFace==FACE_B)) && (x0<=XSize-2) && (x0>= 0) && (y0>= 0) && (y0<=YSize-1) &&
         (((blk->split[(x0  )+(y0  )*XSize]>>(0+2*pass))&1)==0) &&
         IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0  )*XSize)].d,&f))
     { blk->split[(x0  )+(y0  )*XSize] |= 1<<(0+2*pass); *ExitFace=FACE_T; *xcell=x0; *ycell=y0-1; *Xout=x0+f; *Yout=y0; return 1; }

    if (((j==1       )||(EntryFace==FACE_L)) && (x0<=XSize-2) && (x0>=-1) && (y0>= 0) && (y0<=YSize-2) &&
         (((blk->split[(x0+1)+(y0  )*XSize]>>(1+2*pass))&1)==0) &&
         IsBetween(c1, blk->data_real[2+Ncol*((x0+1)+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0+1)*XSize)].d,&f))
     { blk->split[(x0+1)+(y0  )*XSize] |= 1<<(1+2*pass); *ExitFace=FACE_R; *xcell=x0+1; *ycell=y0; *Xout=x0+1; *Yout=y0+f; return 1; }

    if (((j==1       )||(EntryFace==FACE_T)) && (x0<=XSize-2) && (x0>= 0) && (y0>=-1) && (y0<=YSize-2) &&
         (((blk->split[(x0  )+(y0+1)*XSize]>>(0+2*pass))&1)==0) &&
         IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0+1)*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0+1)*XSize)].d,&f))
     { blk->split[(x0  )+(y0+1)*XSize] |= 1<<(0+2*pass); *ExitFace=FACE_B; *xcell=x0; *ycell=y0+1; *Xout=x0+f; *Yout=y0+1; return 1; }

    if (((j==1       )||(EntryFace==FACE_R)) && (x0<=XSize-1) && (x0>= 0) && (y0>= 0) && (y0<=YSize-2) &&
         (((blk->split[(x0  )+(y0  )*XSize]>>(1+2*pass))&1)==0) &&
         IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0  )+(y0+1)*XSize)].d,&f))
     { blk->split[(x0  )+(y0  )*XSize] |= 1<<(1+2*pass); *ExitFace=FACE_L; *xcell=x0-1; *ycell=y0; *Xout=x0; *Yout=y0+f; return 1; }
   }
  return 0;
 }

#define XPOS_TO_POSTSCRIPT \
 { \
  xps = xo + Lx*xpos/(XSize-1)*sin(ThetaX) + Ly*ypos/(YSize-1)*sin(ThetaY); \
  yps = yo + Lx*xpos/(XSize-1)*cos(ThetaX) + Ly*ypos/(YSize-1)*cos(ThetaY); \
 }

#define FOLLOW_CONTOUR \
 { \
  int c; \
  double xps,yps; \
  XPOS_TO_POSTSCRIPT; \
  fprintf(x->epsbuffer, "newpath %.2f %.2f moveto\n", xps, yps); \
  do \
   { \
    if ((c = GetNextPoint(v.real, data, 0, XSize, YSize, xcell, ycell, face, &face, &xcell, &ycell, &xpos, &ypos))!=0) \
     { \
      XPOS_TO_POSTSCRIPT; \
      fprintf(x->epsbuffer, "%.2f %.2f lineto\n", xps, yps); \
     } \
   } \
  while (c); \
  fprintf(x->epsbuffer, "%sstroke\n", closepath?"closepath ":""); \
 }

// Render a contourmap to postscript
int  eps_plot_contourmap(EPSComm *x, DataTable *data, unsigned char ThreeDim, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, double origin_x, double origin_y, double width, double height, double zdepth)
 {
  double         scale_x, scale_y, scale_z;
  DataBlock     *blk;
  int            XSize = (x->current->settings.SamplesXAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesX;
  int            YSize = (x->current->settings.SamplesYAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesY;
  int            i, j, k, Ncol, face, xcell, ycell;
  double         xo, yo, Lx, Ly, ThetaX, ThetaY, CMin, CMax, xpos, ypos;
  unsigned char  CLog, CMinAuto, CMaxAuto, CRenorm, closepath;
  char          *errtext, c1name[]="c1";
  value         *CVar=NULL, CDummy;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present
  Ncol = data->Ncolumns;
  if (eps_plot_WithWordsCheckUsingItemsDimLess(&pd->ww_final, data->FirstEntries, Ncol, NULL)) return 1;
  if (!ThreeDim) { scale_x=width; scale_y=height; scale_z=1.0;    }
  else           { scale_x=width; scale_y=height; scale_z=zdepth; }
  blk = data->first;

  errtext = lt_malloc(LSTR_LENGTH);
  if (errtext==NULL) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); return 1; }

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
  CRenorm  = (sg->Crenorm [0]==SW_BOOL_TRUE);

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

  // Get pointer to variable c1 in the user's variable space
  DictLookup(_ppl_UserSpace_Vars, c1name, NULL, (void **)&CVar);
  if (CVar!=NULL)
   {
    CDummy = *CVar;
   }
  else // If variable is not defined, create it now
   {
    ppl_units_zero(&CDummy);
    DictAppendValue(_ppl_UserSpace_Vars, c1name, CDummy);
    DictLookup(_ppl_UserSpace_Vars, c1name, NULL, (void **)&CVar);
    CDummy.modified = 2;
   }
  if (!CRenorm) { *CVar = pd->CRangeUnit; CVar->FlagComplex=0; CVar->imag=0.0; }
  else ppl_units_zero(CVar); // c1 is a dimensionless number in range 0-1, regardless of units of input data

  // Loop over contours
  for (k=0;
       (k<MAX_CONTOURS) && (  ((sg->ContoursListLen< 0) && (k<sg->ContoursN))  ||
                              ((sg->ContoursListLen>=0) && (k<sg->ContoursListLen))  );
       k++)
   {
    value v = pd->CRangeUnit;

    if (sg->ContoursListLen< 0) v.real = CLog?(CMin*pow(CMax/CMin,((double)(k+1)/(sg->ContoursN+1))))
                                             :(CMin+(CMax-CMin)*((double)(k+1)/(sg->ContoursN+1)));
    else                        v.real = sg->ContoursList[k];

    // Write debugging output
    if (DEBUG)
     {
      sprintf(temp_err_string, "Beginning to draw contour at c1=%g.", v.real);
      ppl_log(temp_err_string);
     }

    // Reset contour map usage flags
    for (j=0; j<YSize; j++)
     for (i=0; i<XSize; i++)
      blk->split[i+XSize*j] = 0;

    // Set value of c1
    if (CRenorm)
     {
      if (sg->ContoursListLen< 0)  CVar->real = ((double)(k+1)/(sg->ContoursN+1));
      else                         CVar->real = CLog?(log(v.real/CMin) / log(CMax/CMin))
                                                    :((v.real-CMin)/(CMax-CMin));
      if (CVar->real<0.0)          CVar->real=0.0;
      if (CVar->real>1.0)          CVar->real=1.0;
      if (!gsl_finite(CVar->real)) CVar->real=0.5;
     }
    else                           CVar->real = v.real;

    // Evaluate any expressions in style information for next contour
    for (i=0 ; ; i++)
     {
      char          *expr[] = { pd->ww_final.STRlinetype,  pd->ww_final.STRlinewidth,  pd->ww_final.STRpointlinewidth,  pd->ww_final.STRpointsize,  pd->ww_final.STRpointtype,  pd->ww_final.STRcolour1   ,  pd->ww_final.STRcolour2   ,  pd->ww_final.STRcolour3   ,  pd->ww_final.STRcolour4   ,  pd->ww_final.STRfillcolour1   ,  pd->ww_final.STRfillcolour2   ,  pd->ww_final.STRfillcolour3   ,  pd->ww_final.STRfillcolour4   , NULL};
      double        *outD[] = { NULL                    , &pd->ww_final.linewidth   , &pd->ww_final.pointlinewidth   , &pd->ww_final.pointsize   ,  NULL                     , &pd->ww_final.colour1      , &pd->ww_final.colour2      , &pd->ww_final.colour3      , &pd->ww_final.colour4      , &pd->ww_final.fillcolour1      , &pd->ww_final.fillcolour2      , &pd->ww_final.fillcolour3      , &pd->ww_final.fillcolour4      , NULL};
      int           *outI[] = {&pd->ww_final.linetype   ,  NULL                     ,  NULL                          ,  NULL                     , &pd->ww_final.pointtype   ,  NULL                      ,  NULL                      ,  NULL                      ,  NULL                      ,  NULL                          ,  NULL                          ,  NULL                          ,  NULL                          , NULL};
      unsigned char *flag[] = {&pd->ww_final.USElinetype, &pd->ww_final.USElinewidth, &pd->ww_final.USEpointlinewidth, &pd->ww_final.USEpointsize, &pd->ww_final.USEpointtype, &pd->ww_final.USEcolour1234, &pd->ww_final.USEcolour1234, &pd->ww_final.USEcolour1234, &pd->ww_final.USEcolour1234, &pd->ww_final.USEfillcolour1234, &pd->ww_final.USEfillcolour1234, &pd->ww_final.USEfillcolour1234, &pd->ww_final.USEfillcolour1234, NULL};
      unsigned char  clip[] = {0,0,0,0,0,1,1,1,1,1,1,1,1,-1};
      value outval; double dbl; int end=-1, errpos=-1;

      if (flag[i]==NULL) break;
      if (expr[i]==NULL) continue;

      ppl_EvaluateAlgebra(expr[i], &outval, 0, &end, 0, &errpos, errtext, 1);

      if (errpos>=0) { sprintf(temp_err_string, "Could not evaluate the style expression <%s>. The error, encountered at character position %d, was: '%s'", expr[i], errpos, errtext); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); continue; }
      if (!outval.dimensionless) { sprintf(temp_err_string, "The style expression <%s> yielded a result which was not a dimensionless number.", expr[i]); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); continue; }
      if (outval.FlagComplex) { sprintf(temp_err_string, "The style expression <%s> yielded a result which was a complex number.", expr[i]); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); continue; }
      if (!gsl_finite(outval.real)) { sprintf(temp_err_string, "The style expression <%s> yielded a result which was not a finite number.", expr[i]); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); continue; }
      dbl = outval.real;

      if (clip[i]) { if (dbl<0.0) dbl=0.0; if (dbl>1.0) dbl=1.0; }
      if (outD[i]!=NULL) *outD[i] = dbl;
      if (outI[i]!=NULL) { if (dbl<INT_MIN) dbl=INT_MIN+1; if (dbl>INT_MAX) dbl=INT_MAX-1; *outI[i] = (int)dbl; }
      *flag[i] = 1;
     }

    // Work out style information for next contour
    eps_core_SetColour(x, &pd->ww_final, 1);
    eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.linewidth, pd->ww_final.linetype, 0);
    IF_NOT_INVISIBLE
     {

      // Scan edges of plot looking for contour start points
      closepath=0;
      for (i=0; i<XSize-1; i++) // Top face
       if (GetStartPoint(v.real, data, 0, XSize, YSize, i        , 0        , FACE_T,&face,&xcell,&ycell,&xpos,&ypos)) { FOLLOW_CONTOUR; }
      for (i=0; i<YSize-1; i++) // Right face
       if (GetStartPoint(v.real, data, 0, XSize, YSize, XSize-2  , i        , FACE_R,&face,&xcell,&ycell,&xpos,&ypos)) { FOLLOW_CONTOUR; }
      for (i=0; i<XSize-1; i++) // Bottom face
       if (GetStartPoint(v.real, data, 0, XSize, YSize, XSize-2-i, YSize-2  , FACE_B,&face,&xcell,&ycell,&xpos,&ypos)) { FOLLOW_CONTOUR; }
      for (i=0; i<YSize-1; i++) // Left face
       if (GetStartPoint(v.real, data, 0, XSize, YSize, 0        , YSize-2-i, FACE_L,&face,&xcell,&ycell,&xpos,&ypos)) { FOLLOW_CONTOUR; }

      // Scan body of plot looking for undrawn contours
      closepath=1;
      for (j=0; j<YSize-1; j++)
       for (i=0; i<XSize-1; i++)
        if (GetStartPoint(v.real, data, 0, XSize, YSize, i, j, FACE_ALL,&face,&xcell,&ycell,&xpos,&ypos)) { FOLLOW_CONTOUR; }

     }

    // Advance plot styles before drawing next contour
    if (pd->ww_final.AUTOcolour)
     {
      int i,j;
      for (j=0; j<PALETTE_LENGTH; j++) if (settings_palette_current[j]==-1) break; // j now contains length of palette
      i = (++pd->ww_final.AUTOcolour-5) % j; // i is now the palette colour number to use
      while (i<0) i+=j;
      if (settings_palette_current[i] > 0) { pd->ww_final.colour  = settings_palette_current[i]; pd->ww_final.USEcolour = 1; }
      else                                 { pd->ww_final.Col1234Space = settings_paletteS_current[i]; pd->ww_final.colour1 = settings_palette1_current[i]; pd->ww_final.colour2 = settings_palette2_current[i]; pd->ww_final.colour3 = settings_palette3_current[i]; pd->ww_final.colour4 = settings_palette4_current[i]; pd->ww_final.USEcolour1234 = 1; }
     }
    else if (pd->ww_final.AUTOlinetype) pd->ww_final.linetype++;
   }

  // Reset value of variable c1
  if (CVar!=NULL) *CVar = CDummy;

  return 0;
 }

