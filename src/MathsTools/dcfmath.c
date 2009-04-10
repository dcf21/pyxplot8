// dcfmath.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-9 Dominic Ford <coders@pyxplot.org.uk>
//               2008-9 Ross Church
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

// A selection of useful mathematical functions which are not included in the standard C math library

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

double max(double x, double y)
 {
  if (x>y) return x;
  return y;
 }

double min(double x, double y)
 {
  if (x<y) return x;
  return y;
 }

int sgn(double x)
 {
  if (x==0) return  0;
  if (x< 0) return -1;
  return 1;
 }

double degrees(double rad)
 {
  return rad*180/M_PI;
 }

double radians(double degrees)
 {
  return degrees*M_PI/180;
 }

double frandom()
 {
  return (double)rand() / RAND_MAX;
 }
