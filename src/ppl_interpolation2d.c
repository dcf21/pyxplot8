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

#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_memory.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_interpolation2d.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

void ppl_interp2d_eval(double *output, const settings_graph *sg, const double *in, const long InSize, const double x, const double y)
 {
  long          i;
  const double *inX = in;
  const double *inY = in + 1*InSize;
  const double *inZ = in + 2*InSize;

  switch (sg->Sample2DMethod)
   {
    case SW_SAMPLEMETHOD_NEAREST:
     {
      double DistBest;
      unsigned char first=1;
      *output = 0.0;
      for (i=0; i<InSize; i++)
       {
        double dist = hypot( inX[i] - x , inY[i] - y );
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
        double dist = gsl_pow_2(inX[i] - x) + gsl_pow_2(inY[i] - y);
        double weight;
        if (dist<1e-200) { *output = inZ[i]; break; }
        weight = 1.0/dist;
        *output   += weight * inZ[i];
        WeightSum += weight;
       }
      if (WeightSum>0.0) *output /= WeightSum;
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

void ppl_interp2d_grid(DataTable **output, const int MemoryContext, const settings_graph *sg, const double *in, const long InSize, const double xmin, const double xmax, const unsigned char logx, const double ymin, const double ymax, const unsigned char logy)
 {
  int    i,imax,j,jmax;
  long   p=0, p2, pc;
  double xmin2, xmax2, ymin2, ymax2, xgap, ygap;
  imax = (sg->SamplesXAuto == SW_BOOL_TRUE) ? sg->samples : sg->SamplesX;
  jmax = (sg->SamplesYAuto == SW_BOOL_TRUE) ? sg->samples : sg->SamplesY;

  *output = DataFile_NewDataTable(3, MemoryContext, imax*jmax);
  if (*output == NULL) return; // Memory fail

  p2 = (*output)->current->BlockPosition = imax*jmax;
  for (pc=0; pc<p2; pc++) (*output)->current->split   [pc] = 0;
  for (pc=0; pc<p2; pc++) (*output)->current->text    [pc] = NULL;
  for (pc=0; pc<p2; pc++) (*output)->current->FileLine[pc] = -1;

  // Log limits in advance for speed
  xmin2 = logx ? log(xmin) : xmin;
  xmax2 = logx ? log(xmax) : xmax;
  ymin2 = logy ? log(ymin) : ymin;
  ymax2 = logy ? log(ymax) : ymax;
  xgap  = xmax2 - xmin2;
  ygap  = ymax2 - ymin2;

  for (j=0; j<jmax; j++)
   {
    double y = ymin2 + ygap * (((double)j)/jmax);
    if (logy) y=exp(y);
    for (i=0; i<imax; i++)
     {
      double x = xmin2 + xgap * (((double)i)/imax);
      if (logx) x=exp(x);
      (*output)->current->data_real[p++].d = x;
      (*output)->current->data_real[p++].d = y;
      ppl_interp2d_eval(&(*output)->current->data_real[p++].d, sg, in, InSize, x, y);
     }
   }
  return;
 }

