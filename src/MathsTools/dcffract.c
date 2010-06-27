// dcffract.c
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

// A selection of mathematical functions which plot fractals

#define _PPL_DCFFRACT_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

#include "dcffract.h"

#include "dcfmath_macros.h"

void dcffract_julia(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "fractal_julia(z,cz,MaxIter)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  CHECK_NEEDLONG(in3, "MaxIter", "function's third argument must be");
   {
    double  y = in1->imag  ,  x = in1->real, x2;
    double cy = in2->imag  , cx = in2->real;
    long MaxIter=(long)(in3->real), iter;

    for (iter=0; ((iter<MaxIter)&&((x*x+y*y)<4)); iter++)
     {
      x2 = x*x - y*y + cx;
      y  = 2*x*y     + cy;
      x  = x2;
     }
    output->real = (double)iter;
   }
  CHECK_OUTPUT_OKAY;
 }

void dcffract_mandelbrot(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "fractal_mandelbrot(z,MaxIter)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDLONG(in2, "MaxIter", "function's second argument must be");
   {
    double  y = in1->imag  ,  x = in1->real, x2;
    double cy = in1->imag  , cx = in1->real;
    long MaxIter=(long)(in2->real), iter;

    for (iter=0; ((iter<MaxIter)&&((x*x+y*y)<4)); iter++)
     {
      x2 = x*x - y*y + cx;
      y  = 2*x*y     + cy;
      x  = x2;
     }
    output->real = (double)iter;
   }
  CHECK_OUTPUT_OKAY;
 }

