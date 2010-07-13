// ppl_interpolation2d.c
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

#define _PPL_INTERPOLATION2D_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <gsl/gsl_math.h>

#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"

#include "EPSMaker/eps_plot_canvas.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_interpolation2d.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

void ppl_interp2d_eval(double *output, const settings_graph *sg, const double *in, const long InSize, const int ColNum, const int NCols, const double x, const double y)
 {
  long          i;
  const double *inX = in;
  const double *inY = in +        InSize;
  const double *inZ = in + ColNum*InSize;

  const double Xscl = in[NCols*(InSize+1)     ] - in[NCols*InSize     ];
  const double Yscl = in[NCols*(InSize+1) + 1 ] - in[NCols*InSize + 1 ];

  switch (sg->Sample2DMethod)
   {
    case SW_SAMPLEMETHOD_NEAREST:
     {
      double DistBest=0;
      unsigned char first=1;
      *output = 0.0;
      for (i=0; i<InSize; i++)
       {
        double dist = hypot( (inX[i] - x)/Xscl , (inY[i] - y)/Yscl );
        if (first || (dist<DistBest)) { DistBest=dist; *output = inZ[i]; first=0; }
       }
      break;
     }
    case SW_SAMPLEMETHOD_INVSQ:
     {
      double WeightSum = 0.0;
      *output = 0.0;
      for (i=0; i<InSize; i++)
       {
        double dist = gsl_pow_2((inX[i] - x)/Xscl) + gsl_pow_2((inY[i] - y)/Yscl);
        double weight;
        if (dist<1e-200) { *output = inZ[i]; WeightSum=1.0; break; }
        weight = 1.0/dist;
        *output   += weight * inZ[i];
        WeightSum += weight;
       }
      if (WeightSum>0.0) *output /= WeightSum;
      break;
     }
    case SW_SAMPLEMETHOD_ML:
     {
      double WeightSum = 0.0;
      double h = sqrt( 1.0/InSize );
      *output = 0.0;
      for (i=0; i<InSize; i++)
       {
        double v = hypot( (inX[i] - x)/Xscl , (inY[i] - y)/Yscl ) / h;
        double w = (v>=2)?0.0:((v>=1)?(0.25*gsl_pow_3(2.0-v)):(1.0-1.5*gsl_pow_2(v)+0.75*gsl_pow_3(v)));
        if (!gsl_finite(w)) continue;
        *output   += w * inZ[i];
        WeightSum += w;
       }
      if (WeightSum>0.0) *output /= WeightSum;
      else               *output = GSL_NAN;
      break;
     }
    default:
     {
      *output = 0.0;
      break;
     }
   }
  return;
 }

void ppl_interp2d_grid(DataTable **output, const settings_graph *sg, DataTable *in, settings_axis *axis_x, settings_axis *axis_y, unsigned char SampleToEdge)
 {
  int        i, imax, j, jmax, ims, jms, c, k, TempContext;
  double    *indata, *MinList, *MaxList, *d[USING_ITEMS_MAX+4];
  long       p, p2, pc, InSize;
  DataBlock *blk;
  imax = (sg->SamplesXAuto == SW_BOOL_TRUE) ? sg->samples : sg->SamplesX;
  jmax = (sg->SamplesYAuto == SW_BOOL_TRUE) ? sg->samples : sg->SamplesY;

  k = in->Ncolumns;
  *output = DataFile_NewDataTable(k, lt_GetMemContext(), imax*jmax);
  if (*output == NULL) return; // Memory fail

  InSize = in->Nrows;
  (*output)->FirstEntries = in->FirstEntries;
  p2 = (*output)->current->BlockPosition = (*output)->Nrows = imax*jmax;
  for (pc=0; pc<p2; pc++) (*output)->current->split   [pc] = 0;
  for (pc=0; pc<p2; pc++) (*output)->current->text    [pc] = NULL;
  for (pc=0; pc<p2; pc++) (*output)->current->FileLine[pc] = 0;

  // Extract data into a temporary array
  TempContext = lt_DescendIntoNewContext();

  indata = (double *)lt_malloc(k * (InSize+2) * sizeof(double));
  if (indata==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst resampling data."); *output=NULL; return; }
  for (c=0; c<k; c++) d[c] = indata + c*InSize;
  MinList = indata + k* InSize;
  MaxList = indata + k*(InSize+1);

  // Copy input data table into temporary array
  blk = in->first;
  while (blk != NULL)
   {
    for (c=0; c<k; c++) for (j=0; j<blk->BlockPosition; j++) *(d[c]++) = blk->data_real[c + k*j].d;
    blk=blk->next;
   }

  // Fill out minimum and maximum of data
  for (jms=0; jms<k; jms++)
   {
    MinList[jms] = MaxList[jms] = 0.0;
    for (ims=0; ims<InSize; ims++)
     {
      if ((ims==0)||(MinList[jms]>indata[jms*InSize+ims])) MinList[jms] = indata[jms*InSize+ims];
      if ((ims==0)||(MaxList[jms]<indata[jms*InSize+ims])) MaxList[jms] = indata[jms*InSize+ims];
     }
    if (MaxList[jms]<=MinList[jms]) { double t=MinList[jms]; MinList[jms]=t*0.999;  MaxList[jms]=t*1.001; }
   }

  // Resample data into new DataTable
  for (j=0, p=0; j<jmax; j++)
   {
    double y = SampleToEdge ? eps_plot_axis_InvGetPosition( (((double)j    )/(jmax-1)) , axis_y)
                            : eps_plot_axis_InvGetPosition( (((double)j+0.5)/(jmax  )) , axis_y);
    for (i=0; i<imax; i++)
     {
      double x = SampleToEdge ? eps_plot_axis_InvGetPosition( (((double)i    )/(imax-1)) , axis_x)
                              : eps_plot_axis_InvGetPosition( (((double)i+0.5)/(imax  )) , axis_x);
      (*output)->current->data_real[p++].d = x;
      (*output)->current->data_real[p++].d = y;

      for (c=2; c<k; c++)
        ppl_interp2d_eval(&(*output)->current->data_real[p++].d, sg, indata, InSize, c, k, x, y);
     }
   }

  // Delete temporary array
  lt_AscendOutOfContext(TempContext);
  return;
 }

