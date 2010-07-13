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

#define MAX_CONTOUR_PATHS 1024

typedef struct ContourDesc
 {
  long           Nvertices_min, Nvertices_max;
  long           i, segment_flatest;
  unsigned char  closepath;
  double        *posdata;
  double         area;
  double         vreal;
 } ContourDesc;

static int ContourCmp(const void *xv, const void *yv)
 {
  const ContourDesc *x = (const ContourDesc *)xv;
  const ContourDesc *y = (const ContourDesc *)yv;

  if      (x->area < y->area) return  1.0;
  else if (x->area > y->area) return -1.0;
  else                        return  0.0;
 }

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
  blk  = data->first;

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
    char *text;
    CanvasTextItem *i;
    value           v = pd->CRangeUnit;

    if (sg->ContoursListLen< 0) v.real = CLog?(CMin*pow(CMax/CMin,(k+1.0)/(sg->ContoursN+1.0)))
                                             :(CMin+(CMax-CMin)*(k+1.0)/(sg->ContoursN+1.0));
    else                        v.real = sg->ContoursList[k];

    sprintf(UnitString,"%s",ppl_units_NumericDisplay(&v,0,SW_DISPLAY_L,0));
    text = lt_malloc(strlen(UnitString+1));
    if (text!=NULL) { strcpy(text, UnitString); YIELD_TEXTITEM(text); }
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
static int GetStartPoint(double c1, DataTable *data, unsigned char **flags, int XSize, int YSize, int x0, int y0, int EntryFace, int *ExitFace, int *xcell, int *ycell, double *Xout, double *Yout)
 {
  DataBlock *blk  = data->first;
  int        Ncol = data->Ncolumns;
  const int  pass = 0;
  double     f;

  *xcell = x0;
  *ycell = y0;

  if (((EntryFace<0)||(EntryFace==FACE_T)) && (x0<=XSize-2) && (x0>= 0) && (y0>= 0) && (y0<=YSize-1) &&
       (((blk->split[(x0  )+(y0  )*XSize]>>(0+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0  )*XSize)].d,&f))
   { *flags = &blk->split[(x0  )+(y0  )*XSize]; *ExitFace=FACE_T; *Xout=x0+f; *Yout=y0; return 1; }

  if (((EntryFace<0)||(EntryFace==FACE_R)) && (x0<=XSize-2) && (x0>=-1) && (y0>= 0) && (y0<=YSize-2) &&
       (((blk->split[(x0+1)+(y0  )*XSize]>>(1+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0+1)+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0+1)*XSize)].d,&f))
   { *flags = &blk->split[(x0+1)+(y0  )*XSize]; *ExitFace=FACE_R; *Xout=x0+1; *Yout=y0+f; return 1; }

  if (((EntryFace<0)||(EntryFace==FACE_B)) && (x0<=XSize-2) && (x0>= 0) && (y0>=-1) && (y0<=YSize-2) &&
       (((blk->split[(x0  )+(y0+1)*XSize]>>(0+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0+1)*XSize)].d, blk->data_real[2+Ncol*((x0+1)+(y0+1)*XSize)].d,&f))
   { *flags = &blk->split[(x0  )+(y0+1)*XSize]; *ExitFace=FACE_B; *Xout=x0+f; *Yout=y0+1; return 1; }

  if (((EntryFace<0)||(EntryFace==FACE_L)) && (x0<=XSize-1) && (x0>= 0) && (y0>= 0) && (y0<=YSize-2) &&
       (((blk->split[(x0  )+(y0  )*XSize]>>(1+2*pass))&1)==0) &&
       IsBetween(c1, blk->data_real[2+Ncol*((x0  )+(y0  )*XSize)].d, blk->data_real[2+Ncol*((x0  )+(y0+1)*XSize)].d,&f))
   { *flags = &blk->split[(x0  )+(y0  )*XSize]; *ExitFace=FACE_L; *Xout=x0; *Yout=y0+f; return 1; }
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

static void FollowContour(EPSComm *x, DataTable *data, ContourDesc *cd, value *v, unsigned char *flags, int XSize, int YSize, int xcell, int ycell, int face, double xpos, double ypos)
 {
  long   i, j, i_flatest=0;
  int    xcelli=xcell, ycelli=ycell;
  double xposi =xpos , yposi =ypos , grad_flatest = 1e100;
  int    facei =face;
  double xold=xpos, yold=ypos;

  // Starting point has been used on pass zero
  *flags |= 1<<(((face==FACE_L)||(face==FACE_R)) + 0);

  // Trace path, looking for flattest segment and counting length
  for (i=0; (GetNextPoint(v->real, data, 0, XSize, YSize, xcell, ycell, face, &face, &xcell, &ycell, &xpos, &ypos)!=0); i++)
   {
    double grad;
    if (xpos!=xold)
     {
      grad = fabs((ypos-yold)/(xpos-xold));
      if (grad<grad_flatest) { grad_flatest=grad; i_flatest=i; }
     }
    xold=xpos; yold=ypos;
   }

  // Fill out information in contour descriptor
  cd->Nvertices_min = cd->Nvertices_max = i;
  cd->segment_flatest = i_flatest;
  cd->posdata = (double *)lt_malloc((i+8) * 2 * sizeof(double));
  if (cd->posdata==NULL) return;

  // Begin pass one
  xcell=xcelli; ycell=ycelli; xpos=xposi; ypos=yposi; face=facei;

  // Starting point has been used on pass one
  *flags |= 1<<(((face==FACE_L)||(face==FACE_R)) + 2);

  // Trace path, looking for flattest segment and counting length
  for (j=0; (GetNextPoint(v->real, data, 1, XSize, YSize, xcell, ycell, face, &face, &xcell, &ycell, &xpos, &ypos)!=0); j++)
   {
    cd->posdata[2*j  ] = xpos;
    cd->posdata[2*j+1] = ypos;
   }
  return;
 }

// Render a contourmap to postscript
int  eps_plot_contourmap(EPSComm *x, DataTable *data, unsigned char ThreeDim, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, int pdn, double origin_x, double origin_y, double width, double height, double zdepth)
 {
  double         scale_x, scale_y, scale_z;
  DataBlock     *blk;
  int            XSize = (x->current->settings.SamplesXAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesX;
  int            YSize = (x->current->settings.SamplesYAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesY;
  int            i, j, k, cn, pass, Ncol, face, xcell, ycell;
  double         xo, yo, Lx, Ly, ThetaX, ThetaY, CMin, CMax, xpos, ypos;
  double         col1=-1,col2=-1,col3=-1,col4=-1,fc1=-1,fc2=-1,fc3=-1,fc4=-1;
  unsigned char  CLog, CMinAuto, CMaxAuto, CRenorm, *flags;
  char          *errtext, c1name[]="c1";
  value         *CVar=NULL, CDummy;
  ContourDesc   *clist;
  int            cpos=0;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present
  Ncol = data->Ncolumns;
  // if (eps_plot_WithWordsCheckUsingItemsDimLess(&pd->ww_final, data->FirstEntries, Ncol, NULL)) return 1;
  if (!ThreeDim) { scale_x=width; scale_y=height; scale_z=1.0;    }
  else           { scale_x=width; scale_y=height; scale_z=zdepth; }
  blk = data->first;

  clist   = (ContourDesc *)lt_malloc(MAX_CONTOUR_PATHS * sizeof(ContourDesc));
  errtext = lt_malloc(LSTR_LENGTH);
  if ((clist==NULL)||(errtext==NULL)) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); return 1; }

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

    if (sg->ContoursListLen< 0) v.real = CLog?(CMin*pow(CMax/CMin,(k+1.0)/(sg->ContoursN+1)))
                                             :(CMin+(CMax-CMin)*(k+1.0)/(sg->ContoursN+1));
    else                        v.real = sg->ContoursList[k];

    // Write debugging output
    if (DEBUG)
     {
      sprintf(temp_err_string, "Beginning to trace path of contour at c1=%g.", v.real);
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

    // Scan edges of plot looking for contour start points
    for (i=0; i<XSize-1; i++) // Top face
     if (GetStartPoint(v.real, data, &flags, XSize, YSize, i        , 0        , FACE_T,&face,&xcell,&ycell,&xpos,&ypos))
      {
       clist[cpos].i=k; clist[cpos].vreal=CVar->real;
       clist[cpos].closepath = 0;
       FollowContour(x, data, &clist[cpos], &v, flags, XSize, YSize, xcell, ycell, face, xpos, ypos);
       if (++cpos>=MAX_CONTOUR_PATHS) goto GOT_CONTOURS;
      }
    for (i=0; i<YSize-1; i++) // Right face
     if (GetStartPoint(v.real, data, &flags, XSize, YSize, XSize-2  , i        , FACE_R,&face,&xcell,&ycell,&xpos,&ypos))
      {
       clist[cpos].i=k; clist[cpos].vreal=CVar->real;
       clist[cpos].closepath = 0;
       FollowContour(x, data, &clist[cpos], &v, flags, XSize, YSize, xcell, ycell, face, xpos, ypos);
       if (++cpos>=MAX_CONTOUR_PATHS) goto GOT_CONTOURS;
      }
    for (i=0; i<XSize-1; i++) // Bottom face
     if (GetStartPoint(v.real, data, &flags, XSize, YSize, XSize-2-i, YSize-2  , FACE_B,&face,&xcell,&ycell,&xpos,&ypos))
      {
       clist[cpos].i=k; clist[cpos].vreal=CVar->real;
       clist[cpos].closepath = 0;
       FollowContour(x, data, &clist[cpos], &v, flags, XSize, YSize, xcell, ycell, face, xpos, ypos);
       if (++cpos>=MAX_CONTOUR_PATHS) goto GOT_CONTOURS;
      }
    for (i=0; i<YSize-1; i++) // Left face
     if (GetStartPoint(v.real, data, &flags, XSize, YSize, 0        , YSize-2-i, FACE_L,&face,&xcell,&ycell,&xpos,&ypos))
      {
       clist[cpos].i=k; clist[cpos].vreal=CVar->real;
       clist[cpos].closepath = 0;
       FollowContour(x, data, &clist[cpos], &v, flags, XSize, YSize, xcell, ycell, face, xpos, ypos);
       if (++cpos>=MAX_CONTOUR_PATHS) goto GOT_CONTOURS;
      }

    // Scan body of plot looking for undrawn contours
    for (j=0; j<YSize-1; j++)
     for (i=0; i<XSize-1; i++)
      if (GetStartPoint(v.real, data, &flags, XSize, YSize, i, j, FACE_ALL,&face,&xcell,&ycell,&xpos,&ypos))
       {
        clist[cpos].i=k; clist[cpos].vreal=CVar->real;
        clist[cpos].closepath = 1;
        FollowContour(x, data, &clist[cpos], &v, flags, XSize, YSize, xcell, ycell, face, xpos, ypos);
        if (++cpos>=MAX_CONTOUR_PATHS) goto GOT_CONTOURS;
       }
   } // Finish looping over contours we are to trace

GOT_CONTOURS:

#define XPOS_TO_POSTSCRIPT \
 { \
  xps = xo + Lx*xpos/(XSize-1)*sin(ThetaX) + Ly*ypos/(YSize-1)*sin(ThetaY); \
  yps = yo + Lx*xpos/(XSize-1)*cos(ThetaX) + Ly*ypos/(YSize-1)*cos(ThetaY); \
 }

  // Add corners of plot to paths of non-closed contours
  for (cn=0; cn<cpos; cn++)
  if (!clist[cn].closepath)
   {
    int    faceA, faceB;
    int    n  = clist[cn].Nvertices_min;
    if (clist[cn].posdata==NULL) continue;

    if      (clist[cn].posdata[          0] <        0.0001 ) faceA = FACE_L;
    else if (clist[cn].posdata[          0] > (XSize-0.0001)) faceA = FACE_R;
    else if (clist[cn].posdata[          1] <        0.0001 ) faceA = FACE_T;
    else                                                      faceA = FACE_B;

    if      (clist[cn].posdata[2*(n-1) + 0] <        0.0001 ) faceB = FACE_L;
    else if (clist[cn].posdata[2*(n-1) + 0] > (XSize-0.0001)) faceB = FACE_R;
    else if (clist[cn].posdata[2*(n-1) + 1] <        0.0001 ) faceB = FACE_T;
    else                                                      faceB = FACE_B;

    if      ( ((faceA==FACE_L)&&(faceB==FACE_T)) || ((faceA==FACE_T)&&(faceB==FACE_L)) )
     {
      clist[cn].posdata[2*n+0] = 0;
      clist[cn].posdata[2*n+1] = 0;
      clist[cn].Nvertices_max += 1;
     }
    else if ( ((faceA==FACE_R)&&(faceB==FACE_T)) || ((faceA==FACE_T)&&(faceB==FACE_R)) )
     {
      clist[cn].posdata[2*n+0] = XSize;
      clist[cn].posdata[2*n+1] = 0;
      clist[cn].Nvertices_max += 1;
     }
    else if ( ((faceA==FACE_L)&&(faceB==FACE_B)) || ((faceA==FACE_B)&&(faceB==FACE_L)) )
     {
      clist[cn].posdata[2*n+0] = 0;
      clist[cn].posdata[2*n+1] = YSize;
      clist[cn].Nvertices_max += 1;
     }
    else if ( ((faceA==FACE_R)&&(faceB==FACE_B)) || ((faceA==FACE_B)&&(faceB==FACE_R)) )
     {
      clist[cn].posdata[2*n+0] = XSize;
      clist[cn].posdata[2*n+1] = YSize;
      clist[cn].Nvertices_max += 1;
     }
    else if ((faceA==FACE_L)&&(faceB==FACE_R))
     {
      clist[cn].posdata[2*n+0] = XSize;
      clist[cn].posdata[2*n+1] = 0;
      clist[cn].posdata[2*n+3] = 0;
      clist[cn].posdata[2*n+4] = 0;
      clist[cn].Nvertices_max += 2;
     }
    else if ((faceA==FACE_R)&&(faceB==FACE_L))
     {
      clist[cn].posdata[2*n+0] = 0;
      clist[cn].posdata[2*n+1] = 0;
      clist[cn].posdata[2*n+3] = XSize;
      clist[cn].posdata[2*n+4] = 0;
      clist[cn].Nvertices_max += 2;
     }
    else if ((faceA==FACE_T)&&(faceB==FACE_B))
     {
      clist[cn].posdata[2*n+0] = 0;
      clist[cn].posdata[2*n+1] = YSize;
      clist[cn].posdata[2*n+3] = 0;
      clist[cn].posdata[2*n+4] = 0;
      clist[cn].Nvertices_max += 2;
     }
    else if ((faceA==FACE_B)&&(faceB==FACE_T))
     {
      clist[cn].posdata[2*n+0] = 0;
      clist[cn].posdata[2*n+1] = 0;
      clist[cn].posdata[2*n+3] = 0;
      clist[cn].posdata[2*n+4] = YSize;
      clist[cn].Nvertices_max += 2;
     }
   }

  // Calculate area of each contour
  for (cn=0; cn<cpos; cn++)
   {
    int i;
    double area=0.0;
    if (clist[cn].posdata!=NULL)
     {
      for (i=0; i<clist[cn].Nvertices_max-1; i++)
       {
        area +=   clist[cn].posdata[2*(i  )+0] * clist[cn].posdata[2*(i+1)+1]
                - clist[cn].posdata[2*(i+1)+0] * clist[cn].posdata[2*(i  )+1];
       }
      area +=   clist[cn].posdata[2*(i  )+0] * clist[cn].posdata[2*(0  )+1]
              - clist[cn].posdata[2*(0  )+0] * clist[cn].posdata[2*(i  )+1]; // Close sum around a closed path
     }
    clist[cn].area=fabs(area/2);
   }

  // Sort contours into order of descending enclosed area
  qsort((void *)clist, cpos, sizeof(ContourDesc), ContourCmp);

  // Now loop over the contours that we have traced, drawing them
  for (pass=1; pass<4; pass++) // Fill contour, Stroke contour, Label contour
  for (cn=0; cn<cpos; cn++)
   {
    // Set value of c1
    if (clist[cn].posdata==NULL) continue;
    CVar->real = clist[cn].vreal;

    // Evaluate any expressions in style information for next contour
    for (i=0 ; ; i++)
     {
      char          *expr [] = { pd->ww_final.STRlinetype ,  pd->ww_final.STRlinewidth ,  pd->ww_final.STRpointlinewidth ,  pd->ww_final.STRpointsize ,  pd->ww_final.STRpointtype ,  pd->ww_final.STRcolour1    ,  pd->ww_final.STRcolour2    ,  pd->ww_final.STRcolour3    ,  pd->ww_final.STRcolour4    ,  pd->ww_final.STRfillcolour1    ,  pd->ww_final.STRfillcolour2    ,  pd->ww_final.STRfillcolour3    ,  pd->ww_final.STRfillcolour4    , NULL};
      double        *outD [] = { NULL                     , &pd->ww_final.linewidth    , &pd->ww_final.pointlinewidth    , &pd->ww_final.pointsize    ,  NULL                      , &col1                       , &col2                       , &col3                       , &col4                       , &fc1                            , &fc2                            , &fc3                            , &fc4                            , NULL};
      int           *outI [] = {&pd->ww_final.linetype    ,  NULL                      ,  NULL                           ,  NULL                      , &pd->ww_final.pointtype    ,  NULL                       ,  NULL                       ,  NULL                       ,  NULL                       ,  NULL                           ,  NULL                           ,  NULL                           ,  NULL                           , NULL};
      unsigned char *flagU[] = {&pd->ww_final.USElinetype , &pd->ww_final.USElinewidth , &pd->ww_final.USEpointlinewidth , &pd->ww_final.USEpointsize , &pd->ww_final.USEpointtype ,  NULL                       ,  NULL                       ,  NULL                       ,  NULL                       ,  NULL                           ,  NULL                           ,  NULL                           ,  NULL                           , NULL};
      int           *flagA[] = {&pd->ww_final.AUTOlinetype,  NULL                      ,  NULL                           ,  NULL                      , &pd->ww_final.AUTOpointtype,  NULL                       ,  NULL                       ,  NULL                       ,  NULL                       ,  NULL                           ,  NULL                           ,  NULL                           ,  NULL                           , NULL};
      unsigned char  clip [] = {0,0,0,0,0,1,1,1,1,1,1,1,1,2};
      value outval; double dbl; int end=-1, errpos=-1;

      if (clip[i]>1) break;
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
      if (flagU[i]!=NULL) *flagU[i] = 1;
      if (flagA[i]!=NULL) *flagA[i] = 0;
     }

    if ((col1>=0.0)&&(col2>=0.0)&&(col3>=0.0)&&((pd->ww_final.    Col1234Space!=SW_COLSPACE_CMYK)||(col4>=0.0))) { pd->ww_final.colour1=col1; pd->ww_final.colour2=col2; pd->ww_final.colour3=col3; pd->ww_final.colour4=col4; pd->ww_final.USEcolour=0; pd->ww_final.USEcolour1234=1; pd->ww_final.AUTOcolour=0; }
    if ((fc1 >=0.0)&&(fc2 >=0.0)&&(fc3 >=0.0)&&((pd->ww_final.FillCol1234Space!=SW_COLSPACE_CMYK)||(fc4 >=0.0))) { pd->ww_final.fillcolour1=fc1; pd->ww_final.fillcolour2=fc2; pd->ww_final.fillcolour3=fc3; pd->ww_final.fillcolour4=fc4; pd->ww_final.USEfillcolour=0; pd->ww_final.USEfillcolour1234=1; }

    // Advance automatic plot styles
    if (pd->ww_final.AUTOcolour)
     {
      int i,j;
      for (j=0; j<PALETTE_LENGTH; j++) if (settings_palette_current[j]==-1) break; // j now contains length of palette
      i = ((pd->ww_final.AUTOcolour+clist[cn].i)-5) % j; // i is now the palette colour number to use
      while (i<0) i+=j;
      if (settings_palette_current[i] > 0) { pd->ww_final.colour  = settings_palette_current[i]; pd->ww_final.USEcolour = 1; }
      else                                 { pd->ww_final.Col1234Space = settings_paletteS_current[i]; pd->ww_final.colour1 = settings_palette1_current[i]; pd->ww_final.colour2 = settings_palette2_current[i]; pd->ww_final.colour3 = settings_palette3_current[i]; pd->ww_final.colour4 = settings_palette4_current[i]; pd->ww_final.USEcolour1234 = 1; }
     }
    else if (pd->ww_final.AUTOlinetype) pd->ww_final.linetype = pd->ww_final.AUTOlinetype + clist[cn].i;

    // PASS 1: Fill path, if required
    if (pass==1)
     {
//    eps_core_SetFillColour(x, &pd->ww_final);
//    eps_core_SwitchTo_FillColour(x,1);
//    IF_NOT_INVISIBLE
//     {
//      double xps, yps; long c=0;
//      xpos = clist[cn].posdata[c++];
//      ypos = clist[cn].posdata[c++];
//      XPOS_TO_POSTSCRIPT;
//      fprintf(x->epsbuffer, "newpath %.2f %.2f moveto\n", xps, yps);
//      while (c<2*clist[cn].Nvertices_max)
//       {
//        xpos = clist[cn].posdata[c++];
//        ypos = clist[cn].posdata[c++];
//        XPOS_TO_POSTSCRIPT;
//        fprintf(x->epsbuffer, "%.2f %.2f lineto\n", xps, yps);
//       }
//      fprintf(x->epsbuffer, "closepath fill\n");
//     }
     }

    // PASS 2: Stroke path
    else if (pass==2)
     {
      eps_core_SetColour(x, &pd->ww_final, 1);
      eps_core_SetLinewidth(x, EPS_DEFAULT_LINEWIDTH * pd->ww_final.linewidth, pd->ww_final.linetype, 0);
      IF_NOT_INVISIBLE
       {
        double xps, yps; long c=0;
        long   n=clist[cn].Nvertices_max;

        if (sg->ContoursLabel==SW_ONOFF_ON)
         {
          int    page = x->current->DatasetTextID[pdn]+clist[cn].i;
          long   i    = clist[cn].segment_flatest;
          double xlab = (clist[cn].posdata[2*((i  )%n)+0] + clist[cn].posdata[2*((i+1)%n)+0] )/2;
          double ylab = (clist[cn].posdata[2*((i  )%n)+1] + clist[cn].posdata[2*((i+1)%n)+1] )/2;
          double wlab, hlab;
          double height1,height2,bb_left,bb_right,bb_top,bb_bottom,ab_left,ab_right,ab_top,ab_bottom;
          postscriptPage *dviPage;

          fprintf(x->epsbuffer, "gsave\n");

          xlab = xo + Lx*xlab/(XSize-1)*sin(ThetaX) + Ly*ylab/(YSize-1)*sin(ThetaY);
          ylab = yo + Lx*xlab/(XSize-1)*cos(ThetaX) + Ly*ylab/(YSize-1)*cos(ThetaY);
          if (x->dvi == NULL) { goto CLIP_FAIL; }
          dviPage = (postscriptPage *)ListGetItem(x->dvi->output->pages, page+1);
          if (dviPage== NULL) { goto CLIP_FAIL; }
          bb_left   = dviPage->boundingBox[0];
          bb_bottom = dviPage->boundingBox[1];
          bb_right  = dviPage->boundingBox[2];
          bb_top    = dviPage->boundingBox[3];
          ab_left   = dviPage->textSizeBox[0];
          ab_bottom = dviPage->textSizeBox[1];
          ab_right  = dviPage->textSizeBox[2];
          ab_top    = dviPage->textSizeBox[3];
          height1   = fabs(ab_top - ab_bottom) * AB_ENLARGE_FACTOR;
          height2   = fabs(bb_top - bb_bottom) * BB_ENLARGE_FACTOR;
          hlab      = ((height2<height1) ? height2 : height1) * x->current->settings.FontSize;
          wlab      = ((ab_right - ab_left) + MARGIN_HSIZE  ) * x->current->settings.FontSize;

          fprintf(x->epsbuffer, "newpath %.2f %.2f moveto %.2f %.2f lineto %.2f %.2f lineto %.2f %.2f lineto closepath %.2f %.2f %.2f 0 360 arc closepath eoclip\n", xlab-wlab/2, ylab-hlab/2, xlab+wlab/2, ylab-hlab/2, xlab+wlab/2, ylab+hlab/2, xlab-wlab/2, ylab+hlab/2, xlab, ylab, 2*(Lx+Ly));
         }

CLIP_FAIL:
        xpos = clist[cn].posdata[c++];
        ypos = clist[cn].posdata[c++];
        XPOS_TO_POSTSCRIPT;
        fprintf(x->epsbuffer, "newpath %.2f %.2f moveto\n", xps, yps);
        while (c<2*clist[cn].Nvertices_min)
         {
          xpos = clist[cn].posdata[c++];
          ypos = clist[cn].posdata[c++];
          XPOS_TO_POSTSCRIPT;
          fprintf(x->epsbuffer, "%.2f %.2f lineto\n", xps, yps);
         }
        fprintf(x->epsbuffer, "%sstroke\n", clist[cn].closepath?"closepath ":"");
        if (sg->ContoursLabel==SW_ONOFF_ON) fprintf(x->epsbuffer, "grestore\n");
       }
     }

    // PASS 3: Label contours
    else if ((pass==3) && (sg->ContoursLabel==SW_ONOFF_ON))
     {
      long   i = clist[cn].segment_flatest;
      long   n = clist[cn].Nvertices_max;
      with_words ww;

      with_words_zero(&ww,0);
      if (x->current->settings.TextColour > 0) { ww.colour = x->current->settings.TextColour; ww.USEcolour = 1; }
      else                                     { ww.Col1234Space = x->current->settings.TextCol1234Space; ww.colour1 = x->current->settings.TextColour1; ww.colour2 = x->current->settings.TextColour2; ww.colour3 = x->current->settings.TextColour3; ww.colour4 = x->current->settings.TextColour4; ww.USEcolour1234 = 1; }
      eps_core_SetColour(x, &ww, 1);

      IF_NOT_INVISIBLE
       {
        double xlab = (clist[cn].posdata[2*((i  )%n)+0] + clist[cn].posdata[2*((i+1)%n)+0] )/2;
        double ylab = (clist[cn].posdata[2*((i  )%n)+1] + clist[cn].posdata[2*((i+1)%n)+1] )/2;
        canvas_EPSRenderTextItem(x, NULL, x->current->DatasetTextID[pdn]+clist[cn].i,
                                 (xo + Lx*xlab/(XSize-1)*sin(ThetaX) + Ly*ylab/(YSize-1)*sin(ThetaY))/M_TO_PS,
                                 (yo + Lx*xlab/(XSize-1)*cos(ThetaX) + Ly*ylab/(YSize-1)*cos(ThetaY))/M_TO_PS,
                                 SW_HALIGN_CENT, SW_VALIGN_CENT, x->CurrentColour, x->current->settings.FontSize,
                                 0.0, NULL, NULL);
       }
     }
   }

  // Reset value of variable c1
  if (CVar!=NULL) *CVar = CDummy;

  return 0;
 }

