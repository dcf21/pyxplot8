// eps_style.h
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

#define _PPL_EPS_STYLE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Line types
char *eps_LineType(int lt, double lw)
 {
  static char output[256];
  lt = lt % 4;
  if      (lt==0) sprintf(output, "0 setlinecap [] 0 setdash");                                   // solid
  else if (lt==1) sprintf(output, "0 setlinecap [%.2f] 0 setdash", 2*lw);                         // dashed
  else if (lt==2) sprintf(output, "1 setlinecap [0 %.2f] 0 setdash", 2*lw);                       // dotted
  else if (lt==3) sprintf(output, "1 setlinecap [0 %.2f %.2f %.2f] 0 setdash", 2*lw, 2*lw, 2*lw); // dash-dotted
  return output;
 }

// Point types

