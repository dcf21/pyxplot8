// eps_plot.c
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

#define _PPL_EPS_PLOT 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"

#include "MathsTools/dcfmath.h"

#include "ppl_canvasdraw.h"
#include "ppl_datafile.h"
#include "ppl_datafile_rasters.h"
#include "ppl_error.h"
#include "ppl_interpolation2d.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot.h"
#include "eps_plot_axespaint.h"
#include "eps_plot_canvas.h"
#include "eps_plot_colourmap.h"
#include "eps_plot_contourmap.h"
#include "eps_plot_gridlines.h"
#include "eps_plot_labelsarrows.h"
#include "eps_plot_legend.h"
#include "eps_plot_linkedaxes.h"
#include "eps_plot_styles.h"
#include "eps_plot_threedimbuff.h"
#include "eps_settings.h"

// If a plot dataset has any with_words of the form "with linewidth $4", these
// need to be evaluated for every datapoint. We do this by adding additional
// items to the UsingList for these datasets. First, we need to check that the
// UsingList supplied by the user is of an acceptable form. If it is of the
// wrong length, we do nothing; it will fail in due course in ppl_datafile
// anyway. If the list is empty, we auto-generate a default list.

int eps_plot_AddUsingItemsForWithWords(with_words *ww, int *NExpect, unsigned char *AutoUsingList, List *UsingList)
 {
  int i, UsingLen;
  char *AutoItem, *temp, *temp2;
  Dict *tempdict;

  *AutoUsingList = 0;
  UsingLen = ListLen(UsingList);
  if (ww->linespoints == SW_STYLE_CONTOURMAP) return 0; // Contourplot evaluate expressions in terms of c1

  // If using list was empty, generate an automatic list before we start
  if (UsingLen==0)
   {
    for (i=0; i<*NExpect; i++)
     {
      AutoItem = (char *)lt_malloc(10);
      if (AutoItem == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return 1; }
      sprintf(AutoItem, "%d", i+1);
      tempdict = DictInit(HASHSIZE_SMALL);
      DictAppendPtr(tempdict, "using_item", (void *)AutoItem, 0, 0, DATATYPE_VOID);
      ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID);
     }
    UsingLen = *NExpect;
    *AutoUsingList = 1;
   }
  else if ((UsingLen==1) && (*NExpect==2)) // Prepend data point number if only one number specified in using statement
   {
    temp = (char *)ListPop(UsingList);
    tempdict = DictInit(HASHSIZE_SMALL);
    temp2 = (char *)lt_malloc(2);
    if (temp2==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return 1; }
    strcpy(temp2, "0");
    DictAppendPtr(tempdict, "using_item", temp2, 0, 0, DATATYPE_VOID);
    ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID);
    ListAppendPtr(UsingList, (void *)temp, 0, 0, DATATYPE_VOID);
    UsingLen++;
   }

  // If using list is wrong length, give up and let ppl_datafile return an error
  if (UsingLen != *NExpect) return 0;

#define ADD_FAKE_USING_ITEM(X) \
 { \
  int l = strlen(X); \
  char *tempstr = lt_malloc(l+3); \
  if (tempstr==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return 1; } \
  tempstr[0] = '('; \
  strcpy(tempstr+  1, X ); \
  strcpy(tempstr+l+1,")"); \
  tempdict = DictInit(HASHSIZE_SMALL); \
  if (tempdict==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return 1; } \
  DictAppendPtr(tempdict, "using_item", (void *)tempstr, 0, 0, DATATYPE_VOID); \
  ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); \
  (*NExpect)++; \
 }

  // Now cycle through all with_words which can be item-specific
  if (ww->STRlinetype       != NULL) ADD_FAKE_USING_ITEM(ww->STRlinetype      );
  if (ww->STRlinewidth      != NULL) ADD_FAKE_USING_ITEM(ww->STRlinewidth     );
  if (ww->STRpointlinewidth != NULL) ADD_FAKE_USING_ITEM(ww->STRpointlinewidth);
  if (ww->STRpointsize      != NULL) ADD_FAKE_USING_ITEM(ww->STRpointsize     );
  if (ww->STRpointtype      != NULL) ADD_FAKE_USING_ITEM(ww->STRpointtype     );
  if (ww->STRcolour1        != NULL) ADD_FAKE_USING_ITEM(ww->STRcolour1       );
  if (ww->STRcolour2        != NULL) ADD_FAKE_USING_ITEM(ww->STRcolour2       );
  if (ww->STRcolour3        != NULL) ADD_FAKE_USING_ITEM(ww->STRcolour3       );
  if (ww->STRcolour4        != NULL) ADD_FAKE_USING_ITEM(ww->STRcolour4       );
  if (ww->STRfillcolour1    != NULL) ADD_FAKE_USING_ITEM(ww->STRfillcolour1   );
  if (ww->STRfillcolour2    != NULL) ADD_FAKE_USING_ITEM(ww->STRfillcolour2   );
  if (ww->STRfillcolour3    != NULL) ADD_FAKE_USING_ITEM(ww->STRfillcolour3   );
  if (ww->STRfillcolour4    != NULL) ADD_FAKE_USING_ITEM(ww->STRfillcolour4   );

  return 0;
 }

#define PROJ_DBL \
 { \
  dbl = DataRow[i--]; \
  if (i<0) i=0; \
  if (!gsl_finite(dbl)) dbl=0.0; \
 }

#define PROJ0_1 \
 { \
  PROJ_DBL; \
  if (dbl < 0.0) dbl= 0.0; \
  if (dbl > 1.0) dbl= 1.0; \
 }

#define PROJ_INT \
 { \
  PROJ_DBL; \
  if (dbl < INT_MIN) dbl=INT_MIN+1; \
  if (dbl > INT_MAX) dbl=INT_MAX-1; \
 }

void eps_plot_WithWordsFromUsingItems(with_words *ww, double *DataRow, int Ncolumns)
 {
  int i = Ncolumns-1;
  double dbl;

  if (ww->linespoints == SW_STYLE_CONTOURMAP) return; // Contourplot evaluate expressions in terms of c1

  if (ww->STRfillcolour4    != NULL) { PROJ0_1  ; ww->USEfillcolour1234 = 1; ww->fillcolour4    = dbl; }
  if (ww->STRfillcolour3    != NULL) { PROJ0_1  ; ww->USEfillcolour1234 = 1; ww->fillcolour3    = dbl; }
  if (ww->STRfillcolour2    != NULL) { PROJ0_1  ; ww->USEfillcolour1234 = 1; ww->fillcolour2    = dbl; }
  if (ww->STRfillcolour1    != NULL) { PROJ0_1  ; ww->USEfillcolour1234 = 1; ww->fillcolour1    = dbl; }
  if (ww->STRcolour4        != NULL) { PROJ0_1  ; ww->USEcolour1234     = 1; ww->colour4        = dbl;      ww->AUTOcolour = 0; ww->USEcolour=0; }
  if (ww->STRcolour3        != NULL) { PROJ0_1  ; ww->USEcolour1234     = 1; ww->colour3        = dbl;      ww->AUTOcolour = 0; ww->USEcolour=0; }
  if (ww->STRcolour2        != NULL) { PROJ0_1  ; ww->USEcolour1234     = 1; ww->colour2        = dbl;      ww->AUTOcolour = 0; ww->USEcolour=0; }
  if (ww->STRcolour1        != NULL) { PROJ0_1  ; ww->USEcolour1234     = 1; ww->colour1        = dbl;      ww->AUTOcolour = 0; ww->USEcolour=0; }
  if (ww->STRpointtype      != NULL) { PROJ_INT ; ww->USEpointtype      = 1; ww->pointtype      = (int)dbl; ww->AUTOpointtype = 0; }
  if (ww->STRpointsize      != NULL) { PROJ_DBL ; ww->USEpointsize      = 1; ww->pointsize      = dbl; }
  if (ww->STRpointlinewidth != NULL) { PROJ_DBL ; ww->USEpointlinewidth = 1; ww->pointlinewidth = dbl; }
  if (ww->STRlinewidth      != NULL) { PROJ_DBL ; ww->USElinewidth      = 1; ww->linewidth      = dbl; }
  if (ww->STRlinetype       != NULL) { PROJ_INT ; ww->USElinetype       = 1; ww->linetype       = (int)dbl; ww->AUTOlinetype  = 0; }

  return;
 }

#define WWCUID(X) \
 if (!FirstValues[i].dimensionless) { sprintf(temp_err_string, "The expression specified for the %s should have been dimensionless, but instead had units of <%s>. Cannot plot this dataset.", X, ppl_units_GetUnitStr(FirstValues+i, NULL, NULL, 0, 1, 0)); ppl_error(ERR_NUMERIC, -1, -1,temp_err_string); return 1; } \
 i--; \
 if (i<0) i=0;

int eps_plot_WithWordsCheckUsingItemsDimLess(with_words *ww, value *FirstValues, int Ncolumns, int *NDataCols)
 {
  int i = Ncolumns-1;

  if (ww->linespoints == SW_STYLE_CONTOURMAP) return 0; // Contourplot evaluate expressions in terms of c1

  if (ww->STRfillcolour4    != NULL) { WWCUID("fourth component of the fillcolour"); }
  if (ww->STRfillcolour3    != NULL) { WWCUID("third component of the fillcolour"); }
  if (ww->STRfillcolour2    != NULL) { WWCUID("second component of the fillcolour"); }
  if (ww->STRfillcolour1    != NULL) { WWCUID("first component of the fillcolour"); }
  if (ww->STRcolour4        != NULL) { WWCUID("fourth component of the colour"); }
  if (ww->STRcolour3        != NULL) { WWCUID("third component of the colour"); }
  if (ww->STRcolour2        != NULL) { WWCUID("second component of the colour"); }
  if (ww->STRcolour1        != NULL) { WWCUID("first component of the colour"); }
  if (ww->STRpointtype      != NULL) { WWCUID("point type"); }
  if (ww->STRpointsize      != NULL) { WWCUID("point size"); }
  if (ww->STRpointlinewidth != NULL) { WWCUID("point line width"); }
  if (ww->STRlinewidth      != NULL) { WWCUID("line width"); }
  if (ww->STRlinetype       != NULL) { WWCUID("line type"); }
  if (NDataCols!=NULL) *NDataCols=i+1; // The number of columns which contain data which is not from with .... expressions
  return 0;
 }

// Private routines for sorting 3D positions by depth and azimuth when clipping 3D plots

static int SortByDepth(const void *x, const void *y)
 {
  const double *xd = (const double *)x;
  const double *yd = (const double *)y;
  if (xd[2]>yd[2]) return  1;
  if (xd[2]<yd[2]) return -1;
  return 0;
 }

static double SortByAzimuthXCentre, SortByAzimuthYCentre;

static int SortByAzimuth(const void *x, const void *y)
 {
  const double *xd = (const double *)x;
  const double *yd = (const double *)y;
  double ax,ay;
  ax = atan2(xd[0]-SortByAzimuthXCentre , xd[1]-SortByAzimuthYCentre);
  ay = atan2(yd[0]-SortByAzimuthXCentre , yd[1]-SortByAzimuthYCentre);
  if (ax>ay) return  1;
  if (ax<ay) return -1;
  return 0;
 }

static void TriSwap(double *a, double *b)
 {
  double t[3];
  memcpy(t,a,3*sizeof(double));
  memcpy(a,b,3*sizeof(double));
  memcpy(b,t,3*sizeof(double));
  return;
 }

// Loop through all of the datasets plotted in a single plot command.
// Initialise the datastructures for the plot command which we will fill in the
// process of deciding how to render the graph. Then read in data from
// datafiles and parametric functions. Do not read in data from non-parametric
// functions as we need to finalise axis ranges first, before we know what
// raster we will sample them on.

void eps_plot_ReadAccessibleData(EPSComm *x)
 {
  int               i, j, k, Ndatasets, Ccounter=0, LTcounter=0, PTcounter=0, status, ErrCount, NExpect;
  canvas_plotdesc  *pd;
  canvas_plotrange *pr;
  settings_axis    *axis, *axissets[3];
  List             *UsingList, *EveryList;
  unsigned char     AutoUsingList=0;
  Dict             *tempdict;
  char              errbuffer[LSTR_LENGTH];
  with_words        ww_default;
  double           *ordinate_raster, size[3], ScreenSize[3], ScreenBearing[3];

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

  // Work out lengths of x-, y- and z-axes
  size[0] = x->current->settings.width.real;
  if (x->current->settings.AutoAspect  == SW_ONOFF_ON) size[1] = size[0] * 2.0/(1.0+sqrt(5));
  else                                                 size[1] = size[0] * x->current->settings.aspect;
  if (x->current->settings.AutoZAspect == SW_ONOFF_ON) size[2] = size[0] * 2.0/(1.0+sqrt(5));
  else                                                 size[2] = size[0] * x->current->settings.zaspect;

  // Work out projected lengths of these axes on screen
  if (!x->current->ThreeDim)
   {
    memcpy(ScreenSize, size, 3*sizeof(double));
    ScreenBearing[0] = M_PI/2;
    ScreenBearing[1] = 0.0;
    ScreenBearing[2] = 0.0;
   }
  else
   for (j=0; j<3; j++)
    {
     double x1,y1,z1,x2,y2,z2;
     eps_plot_ThreeDimProject((j==0)?0:0.5,(j==1)?0:0.5,(j==2)?0:0.5,&x->current->settings,0,0,size[0],size[1],size[2],&x1,&y1,&z1);
     eps_plot_ThreeDimProject((j==0)?1:0.5,(j==1)?1:0.5,(j==2)?1:0.5,&x->current->settings,0,0,size[0],size[1],size[2],&x2,&y2,&z2);
     ScreenSize   [j] = hypot(x2-x1,y2-y1);
     ScreenBearing[j] = atan2(x2-x1,y2-y1);
     if (!gsl_finite(ScreenSize   [j])) ScreenSize   [j] = 0.0;
     if (!gsl_finite(ScreenBearing[j])) ScreenBearing[j] = 0.0;
    }

  // First clear all range information from all axes.
  // Also, transfer range information from [Min,Max,unit] to [HardMin,HardMax,HardUnit].
  for (j=0; j<3; j++)
   {
    settings_axis *axes = axissets[j];
    double PhysicalLengthMajor = ScreenSize[j] / (0.015 + 0.01*fabs(sin(ScreenBearing[j])));
    double PhysicalLengthMinor = ScreenSize[j] / 0.004;

    for (i=0; i<MAX_AXES; i++)
     {
      axes[i].AxisValueTurnings = 0;
      axes[i].AxisLinearInterpolation = NULL;
      axes[i].CrossedAtZero = 0;
      axes[i].MinUsedSet = axes[i].MaxUsedSet = axes[i].DataUnitSet = axes[i].RangeFinalised = 0;
      axes[i].FinalActive = axes[i].enabled;
      axes[i].MinUsed    = axes[i].MaxUsed    = axes[i].MinFinal = axes[i].MaxFinal = 0.0;
      axes[i].LogFinal   = axes[i].log;
      axes[i].HardMin    = axes[i].min;
      axes[i].HardMax    = axes[i].max;
      axes[i].HardMinSet = (axes[i].MinSet==SW_BOOL_TRUE);
      axes[i].HardMaxSet = (axes[i].MaxSet==SW_BOOL_TRUE);
      axes[i].HardUnit   = axes[i].unit;
      axes[i].HardUnitSet= (axes[i].MinSet==SW_BOOL_TRUE) || (axes[i].MaxSet==SW_BOOL_TRUE) || (axes[i].TickList!=NULL) || (((axes[i].log==SW_BOOL_TRUE)?(axes[i].TickMinSet):(axes[i].TickStepSet))!=0) || (axes[i].MTickList!=NULL) || (((axes[i].log==SW_BOOL_TRUE)?(axes[i].MTickMinSet):(axes[i].MTickStepSet))!=0);
      axes[i].HardAutoMinSet = axes[i].HardAutoMaxSet = 0;
      axes[i].Mode0BackPropagated = 0;
      axes[i].OrdinateRasterLen = 0;
      axes[i].OrdinateRaster = NULL;
      axes[i].FinalAxisLabel = NULL;
      axes[i].PhysicalLengthMajor = PhysicalLengthMajor;
      axes[i].PhysicalLengthMinor = PhysicalLengthMinor;
      axes[i].xyz            = j;
      axes[i].axis_n         = i;
      axes[i].canvas_id      = x->current->id;
      axes[i].TickListFinalised = 0;
      axes[i].TickListPositions = axes[i].MTickListPositions = NULL;
      axes[i].TickListStrings   = axes[i].MTickListStrings   = NULL;
      ppl_units_zero(&axes[i].DataUnit);
     }
   }

  // Transfer range information from plot [foo:bar] to HardMin and HardMax variables in axes
  pr = x->current->plotranges;
  k  = 0;
  while (pr != NULL)
   {
    int Ndirs, xyz, axis_n;
    Ndirs  = x->current->ThreeDim ? 3 : 2;
    xyz    = (k%Ndirs);
    axis_n = 1+k/Ndirs;
    if (axis_n > MAX_AXES) axis_n=0;
    axis   = &axissets[xyz][axis_n];

    // Check if we have partial range which conflicts with units of range of axis
    if ((pr->MinSet && (!pr->MaxSet)) && axis->HardMaxSet && (!pr->AutoMaxSet) && (!ppl_units_DimEqual(&axis->HardUnit, &pr->unit))) { sprintf(temp_err_string, "The minimum limit specified for axis %c%d in the plot command has conflicting units with the maximum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&axis->HardUnit,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); *(x->status) = 1; return; }
    if (((!pr->MinSet) && pr->MaxSet) && axis->HardMinSet && (!pr->AutoMinSet) && (!ppl_units_DimEqual(&axis->HardUnit, &pr->unit))) { sprintf(temp_err_string, "The maximum limit specified for axis %c%d in the plot command has conflicting units with the minimum limit of that axis: the former has units of <%s> whilst the latter has units of <%s>.", "xyz"[xyz], axis_n, ppl_units_GetUnitStr(&pr->unit,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&axis->HardUnit,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); *(x->status) = 1; return; }

    // Read information about axis range out of list of ranges supplied to the plot command, ready to pass to eps_plot_ticking
    if (pr->MinSet)     { axis->HardMinSet = 1; axis->HardMin = pr->min; }
    if (pr->MaxSet)     { axis->HardMaxSet = 1; axis->HardMax = pr->max; }
    if (pr->AutoMinSet) { axis->HardAutoMinSet = 1; axis->HardMinSet = 0; }
    if (pr->AutoMaxSet) { axis->HardAutoMaxSet = 1; axis->HardMaxSet = 0; }

    // Update axis->HardUnit to be the canonical reference for the units of the range specified for this axis
    if (pr->MinSet || pr->MaxSet) { axis->HardUnitSet=1; axis->HardUnit=pr->unit; }
    if ((!axis->HardMinSet) && (!axis->HardMaxSet) && (axis->TickList==NULL) && (((axis->log==SW_BOOL_TRUE)?(axis->TickMinSet):(axis->TickStepSet))==0) && (axis->MTickList==NULL) && (((axis->log==SW_BOOL_TRUE)?(axis->MTickMinSet):(axis->MTickStepSet))==0)) { axis->HardUnitSet=0; }

    pr=pr->next; k++;
   }

  // Proprogate range information to linked axes
  for (j=0; j<3; j++)
   {
    settings_axis *axes = axissets[j];
    for (i=0; i<MAX_AXES; i++) eps_plot_LinkedAxisForwardPropagate(x, axes+i, 0);
   }

  // Count number of datasets which we are plotting
  pd = x->current->plotitems;
  Ndatasets = 0;
  while (pd != NULL) { pd=pd->next; Ndatasets++; }

  // Malloc pointers to data tables where data to be plotted will be stored
  if (Ndatasets>0)
   {
    x->current->plotdata      = (DataTable **)lt_malloc(Ndatasets * sizeof(DataTable *));
    x->current->DatasetTextID = (int *)lt_malloc(Ndatasets * sizeof(int));
    if (x->current->plotdata == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); *(x->status) = 1; return; }
   } else {
    x->current->plotdata = NULL;
   }

  // Make raster on which to evaluate parametric functions
  ordinate_raster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
  if (ordinate_raster == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); *(x->status) = 1; return; }
  if (x->current->settings.Tlog == SW_BOOL_TRUE) LogarithmicRaster(ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);
  else                                           LinearRaster     (ordinate_raster, x->current->settings.Tmin.real, x->current->settings.Tmax.real, x->current->settings.samples);

  // Loop through all datasets
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL)
   {
    pd->TitleFinal=NULL;
    pd->TitleFinal_col=0;
    pd->TitleFinal_height = pd->TitleFinal_width = pd->TitleFinal_xpos = pd->TitleFinal_ypos = 0;

    // Merge together with words to form a final set
    eps_withwords_default(&ww_default, &x->current->settings, pd->function, Ccounter, LTcounter, PTcounter, settings_term_current.colour==SW_ONOFF_ON);
    if (pd->function != 0) { with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.FuncStyle, &ww_default, NULL, NULL, 1); }
    else                   { with_words_merge(&pd->ww_final, &pd->ww, &x->current->settings.DataStyle, &ww_default, NULL, NULL, 1); }
    eps_withwords_default_counterinc(&Ccounter, &LTcounter, &PTcounter, settings_term_current.colour==SW_ONOFF_ON, &pd->ww_final, &x->current->settings);

    // Mark up axes which are going to be used for any dataset, from datafile or functions
    axissets[pd->axis1xyz][pd->axis1].FinalActive = 1;
    axissets[pd->axis2xyz][pd->axis2].FinalActive = 1;
    axissets[pd->axis3xyz][pd->axis3].FinalActive = 1;

    // If plotting a datafile, can read in data now, so do so
    if ((pd->function == 0) || (pd->parametric == 1))
     {
      UsingList = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(HASHSIZE_SMALL); DictAppendPtr(tempdict, "using_item", (void *)pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(HASHSIZE_SMALL); DictAppendPtr(tempdict, "every_item", (void *)(pd->EveryList+j), 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      status   = 0;
      ErrCount = DATAFILE_NERRS;
      NExpect  = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);

      if (pd->ww_final.linespoints==SW_STYLE_COLOURMAP)
       {
        int ll = ListLen(UsingList);
        if ((ll>=3)&&(ll<=6)) NExpect=ll; // Colour maps can take 3,4,5 or 6 columns of data
        else if ((pd->function)&&(pd->NFunctions>=1)&&(pd->NFunctions<=4)) NExpect=pd->NFunctions+2;
       }

      if (eps_plot_AddUsingItemsForWithWords(&pd->ww_final, &NExpect, &AutoUsingList, UsingList)) { *(x->status) = 1; return; } // Add extra using items for, e.g. "linewidth $3".

      if (pd->function == 0) // Read data from file
       {
        if (DEBUG) { sprintf(temp_err_string, "Reading data from file '%s' for dataset %d in plot item %d", pd->filename, i+1, x->current->id); ppl_log(temp_err_string); }
        DataFile_read(x->current->plotdata+i, &status, errbuffer, pd->filename, pd->index, pd->UsingRowCols, UsingList, AutoUsingList, EveryList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, (pd->ww_final.linespoints==SW_STYLE_BOXES)?"@":NULL, DATAFILE_DISCONTINUOUS, &ErrCount);
       } else {
        double *special_raster = ordinate_raster;
        int     Nsamples       = x->current->settings.samples;
        value  *raster_unit    = &settings_graph_current.Tmin;
        DataFile_FromFunctions_CheckSpecialRaster(pd->functions, pd->NFunctions, "t", NULL, NULL, &special_raster, &Nsamples);

        if ((special_raster == ordinate_raster) && (pd->TRangeSet))
         {
          special_raster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
          if (special_raster == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); *(x->status) = 1; return; }
          if (x->current->settings.Tlog == SW_BOOL_TRUE) LogarithmicRaster(special_raster, pd->Tmin.real, pd->Tmax.real, x->current->settings.samples);
          else                                           LinearRaster     (special_raster, pd->Tmin.real, pd->Tmax.real, x->current->settings.samples);
         }

        if (DEBUG) { sprintf(temp_err_string, "Reading data from parametric functions for dataset %d in plot item %d", i+1, x->current->id); ppl_log(temp_err_string); }
        DataFile_FromFunctions(special_raster, 1, Nsamples, raster_unit, NULL, 0, NULL, x->current->plotdata+i, &status, errbuffer, pd->functions, pd->NFunctions, UsingList, AutoUsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, (pd->ww_final.linespoints==SW_STYLE_BOXES)?"@":NULL, DATAFILE_DISCONTINUOUS, &ErrCount);
       }
      if (status) { ppl_error(ERR_GENERAL, -1, -1, errbuffer); x->current->plotdata[i]=NULL; }
      else
       {
        // Update axes to reflect usage
        status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2], &axissets[pd->axis3xyz][pd->axis3], &x->current->settings, pd->axis1xyz, pd->axis2xyz, pd->axis3xyz, pd->axis1, pd->axis2, pd->axis3, x->current->id);
        if (status) { *(x->status) = 1; return; }
        eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis1xyz][pd->axis1]);
        eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis2xyz][pd->axis2]);
        eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis3xyz][pd->axis3]);
       }

      // If plotting surface, colourmap or contourmap, resample data onto grid
      if ((pd->ww_final.linespoints==SW_STYLE_SURFACE) || (pd->ww_final.linespoints==SW_STYLE_COLOURMAP) || (pd->ww_final.linespoints==SW_STYLE_CONTOURMAP))
       {
        DataTable *tmpdata = x->current->plotdata[i];

        // Fix range of axes
        eps_plot_LinkedAxisForwardPropagate(x, &axissets[pd->axis1xyz][pd->axis1], 1); if (*x->status) return;
        eps_plot_LinkedAxisForwardPropagate(x, &axissets[pd->axis2xyz][pd->axis2], 1); if (*x->status) return;

        ppl_interp2d_grid(x->current->plotdata+i, &x->current->settings, tmpdata,
                          &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2],
                          (pd->ww_final.linespoints!=SW_STYLE_COLOURMAP)                          );
       }
     }
    pd=pd->next; i++;
   }
  return;
 }

// Loop through all of the datasets plotting within a single plot command, and
// pick out those which are plotting functions which need to be rasterised
// along an ordinate axis (i.e. those which are not parametric). At this stage,
// we need to finalise the range of the ordinate axis, create a raster of
// values along it, and sample the function. After calling this function, all
// datasets plotted by the plot command will then have been sampled and be
// ready for plotting.

void eps_plot_SampleFunctions(EPSComm *x)
 {
  int              a, i, j, status, ErrCount, NExpect, OrdinateRasterLen, Nsamples, OrdinateRasterLen2;
  canvas_plotdesc *pd;
  settings_axis   *OrdinateAxis, *OrdinateAxis2, *axissets[3];
  List            *UsingList, *EveryList;
  unsigned char    AutoUsingList=0;
  Dict            *tempdict;
  char             errbuffer[LSTR_LENGTH];
  unsigned char    SampleGrid;
  double          *OrdinateRaster, *OrdinateRaster2;
  double          *SpecialRaster;

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

  // Loop through all datasets
  pd = x->current->plotitems;
  i  = 0;
  while (pd != NULL)
   {
    if ((pd->function == 1) && (pd->parametric == 0))
     {
      UsingList    = ListInit(); for (j=0; j<pd->NUsing  ; j++) { tempdict = DictInit(HASHSIZE_SMALL); DictAppendPtr(tempdict, "using_item", (void *)pd->UsingList[j], 0, 0, DATATYPE_VOID); ListAppendPtr(UsingList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      EveryList = ListInit(); for (j=0; j<pd->EverySet; j++) { tempdict = DictInit(HASHSIZE_SMALL); DictAppendPtr(tempdict, "every_item", (void *)(pd->EveryList+j), 0, 0, DATATYPE_VOID); ListAppendPtr(EveryList, (void *)tempdict, 0, 0, DATATYPE_VOID); }
      status       = 0;
      ErrCount     = DATAFILE_NERRS;
      NExpect      = eps_plot_styles_NDataColumns(pd->ww_final.linespoints, x->current->ThreeDim);
      OrdinateAxis = &axissets[pd->axis1xyz][pd->axis1];

      if (pd->ww_final.linespoints==SW_STYLE_COLOURMAP)
       {
        int ll = ListLen(UsingList);
        if ((ll>=3)&&(ll<=6)) NExpect=ll; // Colour maps can take 3,4,5 or 6 columns of data
        else if ((pd->function)&&(pd->NFunctions>=1)&&(pd->NFunctions<=4)) NExpect=pd->NFunctions+2;
       }

      if (eps_plot_AddUsingItemsForWithWords(&pd->ww_final, &NExpect, &AutoUsingList, UsingList)) { *(x->status) = 1; return; } // Add extra using items for, e.g. "linewidth $3".

      // Check whether we're sampling along one axis, or sampling a grid
      SampleGrid = (pd->ww_final.linespoints == SW_STYLE_SURFACE) || (pd->ww_final.linespoints == SW_STYLE_COLOURMAP) || (pd->ww_final.linespoints == SW_STYLE_CONTOURMAP);
      OrdinateAxis2      = SampleGrid ? (&axissets[pd->axis2xyz][pd->axis2]) : NULL;
      OrdinateRasterLen2 = 0;
      OrdinateRaster2    = NULL;

      // Loop over one (or two) ordinate axes
      for (a=0; a<=SampleGrid; a++)
       {
        int *_OrdinateRasterLen; double **_OrdinateRaster; settings_axis **_OrdinateAxis;

        if (!a) { _OrdinateRasterLen = &OrdinateRasterLen ; _OrdinateRaster = &OrdinateRaster ; _OrdinateAxis = &OrdinateAxis ; }
        else    { _OrdinateRasterLen = &OrdinateRasterLen2; _OrdinateRaster = &OrdinateRaster2; _OrdinateAxis = &OrdinateAxis2; }

        // Fix range of ordinate axis
        eps_plot_LinkedAxisForwardPropagate(x, *_OrdinateAxis, 1);
        if (*x->status) return;

        // Fetch ordinate raster to plot function along
        if ((pd->ww_final.linespoints == SW_STYLE_BOXES) || (pd->ww_final.linespoints == SW_STYLE_HISTEPS))
         {
          // boxes and histeps plot styles have their own special rasters
          int k;
          double left, right, left2=GSL_NAN, right2=GSL_NAN, width;
          left           = eps_plot_axis_InvGetPosition(0.0, *_OrdinateAxis);
          right          = eps_plot_axis_InvGetPosition(1.0, *_OrdinateAxis);
          if (x->current->settings.BoxWidth.real>0) width = x->current->settings.BoxWidth.real;
          else                                      width = fabs(left-right)/(x->current->settings.samples);
          left          += width/2 * ((right>left) ? 1.0 : -1.0);
          right         += width/2 * ((left>right) ? 1.0 : -1.0);
          for (k=0; k<=(*_OrdinateAxis)->AxisValueTurnings; k++) { left2  = eps_plot_axis_GetPosition(left, *_OrdinateAxis, k, 0); if (gsl_finite(left2 )) break; }
          for (k=(*_OrdinateAxis)->AxisValueTurnings; k>=0; k++) { right2 = eps_plot_axis_GetPosition(right,*_OrdinateAxis, k, 0); if (gsl_finite(right2)) break; }
          if ((!gsl_finite(left2))||(!gsl_finite(right2))||(right2<=left2)||(left2<0)||(left2>1)||(right2<0)||(right2>1)) { left2=0.0; right2=1.0; }

          *_OrdinateRaster = (double *)lt_malloc(x->current->settings.samples * sizeof(double));
          if (*_OrdinateRaster == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); *(x->status) = 1; return; }
          for (j=0; j<x->current->settings.samples; j++)
            (*_OrdinateRaster)[j] = eps_plot_axis_InvGetPosition(left2 + (right2-left2)*((double)j)/(x->current->settings.samples-1), *_OrdinateAxis);
          *_OrdinateRasterLen = x->current->settings.samples;
         }
        else // all other plot styles have rasters running from min -> max
         {
          if (SampleGrid || (*_OrdinateAxis)->OrdinateRaster == NULL) // Make ordinate raster if we don't already have one
           {
            int Nsamp = (!SampleGrid) ? x->current->settings.samples :
                  (a ? (  (x->current->settings.SamplesXAuto == SW_BOOL_TRUE) ?
                          x->current->settings.samples : x->current->settings.SamplesX )
                     : (  (x->current->settings.SamplesYAuto == SW_BOOL_TRUE) ?
                          x->current->settings.samples : x->current->settings.SamplesY ) );
            *_OrdinateRaster = (double *)lt_malloc(Nsamp * sizeof(double));
            if (*_OrdinateRaster == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); *(x->status) = 1; return; }
            for (j=0; j<Nsamp; j++)
              (*_OrdinateRaster)[j] = eps_plot_axis_InvGetPosition(((double)j)/(Nsamp-1), *_OrdinateAxis);
            *_OrdinateRasterLen = Nsamp;
            if (!SampleGrid)
             {
              (*_OrdinateAxis)->OrdinateRaster    = *_OrdinateRaster;
              (*_OrdinateAxis)->OrdinateRasterLen = *_OrdinateRasterLen;
             }
           }
          else
           {
            *_OrdinateRaster    = (*_OrdinateAxis)->OrdinateRaster;
            *_OrdinateRasterLen = (*_OrdinateAxis)->OrdinateRasterLen;
           }
         }
       }

      if (DEBUG) { sprintf(temp_err_string, "Reading data from functions for dataset %d in plot item %d", i+1, x->current->id); ppl_log(temp_err_string); }

      // Get data from functions
      SpecialRaster = OrdinateRaster;
      Nsamples      = OrdinateRasterLen;
      if (!SampleGrid)
        DataFile_FromFunctions_CheckSpecialRaster(pd->functions, pd->NFunctions, "x", NULL, NULL, &SpecialRaster, &Nsamples);
      DataFile_FromFunctions(SpecialRaster, 0, Nsamples, &OrdinateAxis->DataUnit, OrdinateRaster2, OrdinateRasterLen2, (OrdinateAxis2==NULL)?NULL:&OrdinateAxis2->DataUnit, x->current->plotdata+i, &status, errbuffer, pd->functions, pd->NFunctions, UsingList, AutoUsingList, pd->label, NExpect, pd->SelectCriterion, pd->continuity, (pd->ww_final.linespoints==SW_STYLE_BOXES)?"@":NULL, DATAFILE_DISCONTINUOUS, &ErrCount);
      if (status) { ppl_error(ERR_GENERAL, -1, -1, errbuffer); x->current->plotdata[i]=NULL; }

      // Update axes to reflect usage
      status=eps_plot_styles_UpdateUsage(x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, &axissets[pd->axis1xyz][pd->axis1], &axissets[pd->axis2xyz][pd->axis2], &axissets[pd->axis3xyz][pd->axis3], &x->current->settings, pd->axis1xyz, pd->axis2xyz, pd->axis3xyz, pd->axis1, pd->axis2, pd->axis3, x->current->id);
      if (status) { *(x->status) = 1; return; }
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis1xyz][pd->axis1]);
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis2xyz][pd->axis2]);
      eps_plot_LinkedAxisBackPropagate(x, &axissets[pd->axis3xyz][pd->axis3]);
      axissets[pd->axis1xyz][pd->axis1].RangeFinalised = axissets[pd->axis1xyz][pd->axis1].TickListFinalised = 0;
     }
    pd=pd->next; i++;
   }
  return;
 }

void eps_plot_YieldUpText(EPSComm *x)
 {
  int              j, k, l;
  canvas_plotdesc *pd;
  DataBlock       *blk;
  settings_axis   *axes;
  CanvasTextItem  *i;

  x->current->FirstTextID = x->NTextItems;

  // Labels attached to data points
  pd = x->current->plotitems;
  k  = 0;
  while (pd != NULL) // loop over all datasets
   {
    pd->CRangeDisplay = 0;
    x->current->DatasetTextID[k] = x->NTextItems;
    if (pd->ww_final.linespoints == SW_STYLE_COLOURMAP)
     {
      eps_plot_colourmap_YieldText(x, x->current->plotdata[k], &x->current->settings, pd);
     }
    else if (pd->ww_final.linespoints == SW_STYLE_CONTOURMAP)
     {
      eps_plot_contourmap_YieldText(x, x->current->plotdata[k], &x->current->settings, pd);
     }
    else if (x->current->plotdata[k] != NULL)
     {
      blk = x->current->plotdata[k]->first;
      while (blk != NULL)
       {
        if (blk->text != NULL) for (j=0; j<blk->BlockPosition; j++) { YIELD_TEXTITEM(blk->text[j]); }
        blk=blk->next;
       }
     }
    pd=pd->next; k++;
   }

  // Text for the 'set label' command
  x->current->SetLabelTextID = x->NTextItems;
  eps_plot_labelsarrows_YieldUpText(x);

  // Graph legend
  x->current->LegendTextID = x->NTextItems;
  GraphLegend_YieldUpText(x);

  // Axis labels and titles
  for (j=0; j<2+(x->current->ThreeDim); j++)
   {
    if      (j==0) axes = x->current->XAxes;
    else if (j==1) axes = x->current->YAxes;
    else           axes = x->current->ZAxes;

    for (k=0; k<MAX_AXES; k++)
     if ((axes[k].FinalActive) && (!axes[k].invisible))
      {
       axes[k].FirstTextID = x->NTextItems;
       if (axes[k]. TickListPositions != NULL) for (l=0; axes[k]. TickListStrings[l]!=NULL; l++) { YIELD_TEXTITEM(axes[k]. TickListStrings[l]); } // Major tick labels
       if (axes[k].MTickListPositions != NULL) for (l=0; axes[k].MTickListStrings[l]!=NULL; l++) { YIELD_TEXTITEM(axes[k].MTickListStrings[l]); } // Minor tick labels
       YIELD_TEXTITEM(axes[k].FinalAxisLabel);
      }
   }

  // Title of plot
  x->current->TitleTextID = x->NTextItems;
  YIELD_TEXTITEM(x->current->settings.title);

  return;
 }

// Finally, produce a postscript plot

void eps_plot_RenderEPS(EPSComm *x)
 {
  int              i, j, status, xyzaxis[3];
  double           origin_x, origin_y, width, height, zdepth;
  canvas_plotdesc *pd;
  settings_axis   *a1, *a2, *a3, *axissets[3];

  x->LaTeXpageno = x->current->FirstTextID;

  axissets[0] = x->current->XAxes;
  axissets[1] = x->current->YAxes;
  axissets[2] = x->current->ZAxes;

  // Write header at top of postscript
  fprintf(x->epsbuffer, "%% Canvas item %d [plot]\n", x->current->id);

  // Calculate positions of the four corners of graph
  origin_x = x->current->settings.OriginX.real * M_TO_PS;
  origin_y = x->current->settings.OriginY.real * M_TO_PS;
  width    = x->current->settings.width  .real * M_TO_PS;
  if (x->current->settings.AutoAspect  == SW_ONOFF_ON) height = width * 2.0/(1.0+sqrt(5));
  else                                                 height = width * x->current->settings.aspect;
  if (x->current->settings.AutoZAspect == SW_ONOFF_ON) zdepth = width * 2.0/(1.0+sqrt(5));
  else                                                 zdepth = width * x->current->settings.zaspect;

  // Turn on clipping if 'set clip' is set
  if (x->current->settings.clip == SW_ONOFF_ON)
   {
    if (x->current->ThreeDim) // 3D clip region is the edge of a cuboidal box
     {
      int i;
      double xap, yap, zap, data[3*8];
      for (i=0;i<8;i++)
       {
        xap=((i&1)!=0);
        yap=((i&2)!=0);
        zap=((i&4)!=0);
        eps_plot_ThreeDimProject(xap,yap,zap,&x->current->settings,origin_x,origin_y,width,height,zdepth,data+3*i,data+3*i+1,data+3*i+2);
       }
      SortByAzimuthXCentre = origin_x;
      SortByAzimuthYCentre = origin_y;
      qsort((void *)(data  ),8,3*sizeof(double),SortByDepth);
      if ((data[3*0+2]==data[3*1+2])&&(hypot(data[3*0+0]-origin_x,data[3*0+1]-origin_y)>hypot(data[3*1+0]-origin_x,data[3*1+1]-origin_y))) TriSwap(data+3*0,data+3*1);
      if ((data[3*7+2]==data[3*6+2])&&(hypot(data[3*7+0]-origin_x,data[3*7+1]-origin_y)>hypot(data[3*6+0]-origin_x,data[3*6+1]-origin_y))) TriSwap(data+3*7,data+3*6);
      qsort((void *)(data+3),6,3*sizeof(double),SortByAzimuth);
      fprintf(x->epsbuffer, "gsave\nnewpath\n");
      for (i=1;i<7;i++) fprintf(x->epsbuffer, "%.2f %.2f %sto\n", data[3*i], data[3*i+1], (i==1)?"move":"line");
      fprintf(x->epsbuffer, "closepath\nclip newpath\n");
     }
    else // 2D clip region is a simple rectangular box
     {
      fprintf(x->epsbuffer, "gsave\nnewpath\n%.2f %.2f moveto\n%.2f %.2f lineto\n%.2f %.2f lineto\n%.2f %.2f lineto\nclosepath\nclip newpath\n",origin_x,origin_y,origin_x+width,origin_y,origin_x+width,origin_y+height,origin_x,origin_y+height);
     }
   }

  // Render colourmaps and then contourmaps
  for (j=0; j<2; j++)
   {
    int style = j ? SW_STYLE_CONTOURMAP : SW_STYLE_COLOURMAP;
    for (pd = x->current->plotitems, i=0; pd != NULL; pd=pd->next, i++) // loop over all datasets
     if (pd->ww_final.linespoints == style)
      {
       x->LaTeXpageno = x->current->DatasetTextID[i];
       a1 = &axissets[pd->axis1xyz][pd->axis1];
       a2 = &axissets[pd->axis2xyz][pd->axis2];
       a3 = &axissets[pd->axis3xyz][pd->axis3];
       xyzaxis[pd->axis1xyz] = 0;
       xyzaxis[pd->axis2xyz] = 1;
       xyzaxis[pd->axis3xyz] = 2;

       status = (j?eps_plot_contourmap:eps_plot_colourmap)(x, x->current->plotdata[i], x->current->ThreeDim, xyzaxis[0], xyzaxis[1], xyzaxis[2], &x->current->settings, pd, i, origin_x, origin_y, width, height, zdepth);
       if (status) { *(x->status) = 1; return; }
      }
   }

  // Render gridlines
  eps_plot_gridlines(x, origin_x, origin_y, width, height, zdepth);

  // Activate three-dimensional buffer if graph is 3D
  if (x->current->ThreeDim) ThreeDimBuffer_Activate(x);

  // Render axes (back)
  eps_plot_axespaint(x, origin_x, origin_y, width, height, zdepth, 0);

  // Render each dataset in turn (except colourmaps, which we've already rendered)
  for (pd = x->current->plotitems, i=0; pd != NULL; pd=pd->next, i++) // loop over all datasets
   {
    x->LaTeXpageno = x->current->DatasetTextID[i];
    a1 = &axissets[pd->axis1xyz][pd->axis1];
    a2 = &axissets[pd->axis2xyz][pd->axis2];
    a3 = &axissets[pd->axis3xyz][pd->axis3];
    xyzaxis[pd->axis1xyz] = 0;
    xyzaxis[pd->axis2xyz] = 1;
    xyzaxis[pd->axis3xyz] = 2;

    status = eps_plot_dataset(x, x->current->plotdata[i], pd->ww_final.linespoints, x->current->ThreeDim, a1, a2, a3, xyzaxis[0], xyzaxis[1], xyzaxis[2], &x->current->settings, pd, origin_x, origin_y, width, height, zdepth);
    if (status) { *(x->status) = 1; return; }
   }

  // Render text labels and arrows
  eps_plot_labelsarrows(x, origin_x, origin_y, width, height, zdepth);

  // Deactivate three-dimensional buffer
  ThreeDimBuffer_Deactivate(x);

  // Turn off clipping if 'set clip' is set
  if (x->current->settings.clip == SW_ONOFF_ON)
   { fprintf(x->epsbuffer, "grestore\n"); x->LastLinewidth = -1; x->LastLinetype = -1; x->LastPSColour[0]='\0'; }

  // Render axes (front)
  eps_plot_axespaint(x, origin_x, origin_y, width, height, zdepth, 1);

  // Render colourmap scales
  eps_plot_colourmap_DrawScales(x, origin_x, origin_y, width, height, zdepth);

  // Render legend
  GraphLegend_Render(x, width, height, zdepth);

  // Put the title on the top of the graph
  x->LaTeXpageno = x->current->TitleTextID;
  if ((x->current->settings.title != NULL) && (x->current->settings.title[0] != '\0'))
   {
    int pageno = x->LaTeXpageno++;
    with_words ww;
    with_words_zero(&ww,0);
    if (x->current->settings.TextColour > 0) { ww.colour = x->current->settings.TextColour; ww.USEcolour = 1; }
    else                                     { ww.Col1234Space = x->current->settings.TextCol1234Space; ww.colour1 = x->current->settings.TextColour1; ww.colour2 = x->current->settings.TextColour2; ww.colour3 = x->current->settings.TextColour3; ww.colour4 = x->current->settings.TextColour4; ww.USEcolour1234 = 1; }
    eps_core_SetColour(x, &ww, 1);
    IF_NOT_INVISIBLE canvas_EPSRenderTextItem(x, NULL, pageno, (x->current->ThreeDim ? origin_x : (origin_x+width/2))/M_TO_PS, x->current->PlotTopMargin/M_TO_PS+4e-3, SW_HALIGN_CENT, SW_VALIGN_BOT, x->CurrentColour, x->current->settings.FontSize, 0.0, NULL, NULL);
   }

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

