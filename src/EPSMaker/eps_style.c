// eps_style.c
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

#define _PPL_EPS_STYLE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eps_style.h"

// Line types
char *eps_LineType(int lt, double lw, double offset)
 {
  static char output[256];
  lt = (lt-1) % 9;
  while (lt<0) lt+=9;
  if      (lt==0) sprintf(output, "0 setlinecap [] %.2f setdash", offset);                                   // solid
  else if (lt==1) sprintf(output, "0 setlinecap [%.2f] %.2f setdash", 2*lw, offset);                         // dashed
  else if (lt==2) sprintf(output, "1 setlinecap [0 %.2f] %.2f setdash", 2*lw, offset);                       // dotted
  else if (lt==3) sprintf(output, "1 setlinecap [0 %.2f %.2f %.2f] %.2f setdash", 2*lw, 2*lw, 2*lw, offset); // dash-dotted
  else if (lt==4) sprintf(output, "0 setlinecap [%.2f %.2f] %.2f setdash", 7*lw, 2*lw, offset);              // long dash
  else if (lt==5) sprintf(output, "1 setlinecap [%.2f %.2f 0 %.2f] %.2f setdash", 7*lw, 2*lw, 2*lw, offset); // long dash - dot
  else if (lt==6) sprintf(output, "1 setlinecap [%.2f %.2f 0 %.2f 0 %.2f] %.2f setdash", 7*lw, 2*lw, 2*lw, 2*lw, offset); // long dash - dot dot
  else if (lt==7) sprintf(output, "1 setlinecap [%.2f %.2f 0 %.2f 0 %.2f 0 %.2f] %.2f setdash", 7*lw, 2*lw, 2*lw, 2*lw, 2*lw, offset); // long dash - dot dot dot
  else if (lt==8) sprintf(output, "0 setlinecap [%.2f %.2f %.2f %.2f] %.2f setdash", 7*lw, 2*lw, 2*lw, 2*lw, offset); // long dash - dash
  return output;
 }

// Point types
// NB: The fact that pt3 depends upon pt1 and pt2 must be accounted for in ppl_canvasdraw.c
char *eps_PointTypes[N_POINTTYPES] = {
       "/pt1 {newpath 2 copy ps75 sub exch ps75 sub exch moveto ps75 2 mul dup rlineto closepath stroke ps75 add exch ps75 sub exch moveto ps75 2 mul dup -1 mul rlineto closepath stroke } bind def",
       "/pt2 {newpath 2 copy exch ps sub exch moveto ps 2 mul 0 rlineto closepath stroke ps sub moveto 0 ps 2 mul rlineto closepath stroke } bind def",
       "/pt3 {newpath 2 copy pt1 pt2} bind def",
       "/pt4 {newpath ps75 add exch ps75 add exch moveto ps75 -2 mul 0 rlineto 0 ps75 -2 mul rlineto ps75 2 mul 0 rlineto closepath stroke} bind def",
       "/pt5 {newpath exch ps 1.183 mul add exch ps75 sub moveto ps -1.183 mul ps 2.049 mul 2 copy rlineto -1 mul rlineto closepath stroke} bind def",
       "/pt6 {newpath 2 copy exch ps75 1.1 mul add exch moveto ps75 1.1 mul 0 360 arc stroke} bind def",
       "/pt7 {newpath exch ps75 add exch moveto ps75 -1 mul ps 1.3 mul 2 copy 2 copy rlineto -1 mul rlineto -1 mul exch -1 mul exch rlineto closepath stroke} bind def",
       "/pt8 {newpath ps75 add exch ps75 add exch moveto ps75 -2 mul 0 rlineto 0 ps75 -2 mul rlineto ps75 2 mul 0 rlineto closepath fill} bind def",
       "/pt9 {newpath exch ps 1.183 mul add exch ps75 sub moveto ps -1.183 mul ps 2.049 mul 2 copy rlineto -1 mul rlineto closepath fill} bind def",
       "/pt10 {newpath 2 copy exch ps75 1.1 mul add exch moveto ps75 1.1 mul 0 360 arc fill} bind def",
       "/pt11 {newpath exch ps75 add exch moveto ps75 -1 mul ps 1.3 mul 2 copy 2 copy rlineto -1 mul rlineto -1 mul exch -1 mul exch rlineto closepath fill} bind def",
       "/pt12 {newpath exch ps add exch moveto ps -.5 mul ps 0.866 mul rlineto ps -1 mul 0 rlineto ps -.5 mul ps -.866 mul 2 copy rlineto exch -1 mul exch rlineto ps 0 rlineto closepath stroke} bind def",
       "/pt13 {newpath 2 copy ps add moveto /theta 162 def 4 { 2 copy exch theta cos ps mul add exch theta sin ps mul add lineto /theta theta 72 add def } repeat closepath stroke pop pop} bind def",
       "/pt14 {newpath exch ps 1.183 mul add exch ps75 add moveto ps -1.183 mul ps -2.049 mul 2 copy rlineto -1 mul rlineto closepath stroke} bind def",
       "/pt15 {newpath ps 1.183 mul add exch ps75 sub exch moveto ps 2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath stroke} bind def",
       "/pt16 {newpath ps 1.183 mul add exch ps75 add exch moveto ps -2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath stroke} bind def",
       "/pt17 {newpath 10 {2 copy} repeat ps add moveto /theta 126 def 5 { exch ps 4 div theta cos mul add exch ps 4 div theta sin mul add lineto /theta theta 36 add def exch ps theta cos mul add exch ps theta sin mul add lineto /theta theta 36 add def } repeat closepath stroke} bind def",
       "/pt18 {newpath moveto ps75 dup rlineto ps -1.5 mul 0 rlineto ps 1.5 mul dup dup neg rlineto neg 0 rlineto closepath stroke} bind def",
       "/pt19 {newpath moveto ps75 dup rlineto 0 ps -1.5 mul rlineto ps 1.5 mul neg dup dup neg rlineto 0 exch rlineto closepath stroke} bind def",
       "/pt20 {newpath 5 {2 copy} repeat /theta 90 def 3 { exch ps theta cos mul add exch ps theta sin mul add moveto lineto stroke /theta theta 120 add def} repeat } bind def",
       "/pt21 {newpath 5 {2 copy} repeat /theta 270 def 3 { exch ps theta cos mul add exch ps theta sin mul add moveto lineto stroke /theta theta 120 add def} repeat } bind def",
       "/pt22 {newpath exch ps add exch moveto ps -.5 mul ps 0.866 mul rlineto ps -1 mul 0 rlineto ps -.5 mul ps -.866 mul 2 copy rlineto exch -1 mul exch rlineto ps 0 rlineto closepath fill} bind def",
       "/pt23 {newpath 2 copy ps add moveto /theta 162 def 4 { 2 copy exch theta cos ps mul add exch theta sin ps mul add lineto /theta theta 72 add def } repeat closepath fill pop pop} bind def",
       "/pt24 {newpath exch ps 1.183 mul add exch ps75 add moveto ps -1.183 mul ps -2.049 mul 2 copy rlineto -1 mul rlineto closepath fill} bind def",
       "/pt25 {newpath ps 1.183 mul add exch ps75 sub exch moveto ps 2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath fill} bind def",
       "/pt26 {newpath ps 1.183 mul add exch ps75 add exch moveto ps -2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath fill} bind def",
       "/pt27 {newpath 10 {2 copy} repeat ps add moveto /theta 126 def 5 { exch ps 4 div theta cos mul add exch ps 4 div theta sin mul add lineto /theta theta 36 add def exch ps theta cos mul add exch ps theta sin mul add lineto /theta theta 36 add def } repeat closepath fill} bind def",
       "/pt28 {newpath moveto ps75 dup rlineto ps -1.5 mul 0 rlineto ps 1.5 mul dup dup neg rlineto neg 0 rlineto closepath fill} bind def",
       "/pt29 {newpath moveto ps75 dup rlineto 0 ps -1.5 mul rlineto ps 1.5 mul neg dup dup neg rlineto 0 exch rlineto closepath fill} bind def",
       "/pt30 {newpath 11 {2 copy} repeat /theta 90 def 6 { exch ps theta cos mul add exch ps theta sin mul add moveto lineto stroke /theta theta 60 add def} repeat } bind def",
      };

double eps_PointSize[N_POINTTYPES] = {0.75 , 1.0 , 1.0 , 0.75 , 1.183 , 0.75 , 1.3 , 0.75 , 1.183 , 0.75 , 1.3 , 1.0 , 1.0 , 1.183 , 1.183 , 1.183 , 1.2 , 1.0 , 1.0 , 1.0 , 1.0 ,  1.0 , 1.0 , 1.183 , 1.183 , 1.183 , 1.2 , 1.0 , 1.0};

// Star types
char *eps_StarTypes [N_STARTYPES] = {
       ""
      };

