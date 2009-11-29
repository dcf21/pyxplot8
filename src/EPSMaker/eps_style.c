// eps_style.c
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

#include "eps_style.h"

// Line types
char *eps_LineType(int lt, double lw)
 {
  static char output[256];
  lt = lt % 9;
  if      (lt==0) sprintf(output, "0 setlinecap [] 0 setdash");                                   // solid
  else if (lt==1) sprintf(output, "0 setlinecap [%.2f] 0 setdash", 2*lw);                         // dashed
  else if (lt==2) sprintf(output, "1 setlinecap [0 %.2f] 0 setdash", 2*lw);                       // dotted
  else if (lt==3) sprintf(output, "1 setlinecap [0 %.2f %.2f %.2f] 0 setdash", 2*lw, 2*lw, 2*lw); // dash-dotted
  else if (lt==4) sprintf(output, "0 setlinecap [%.2f %.2f] 0 setdash", 7*lw, 2*lw);              // long dash
  else if (lt==5) sprintf(output, "1 setlinecap [%.2f %.2f 0 %.2f] 0 setdash", 7*lw, 2*lw, 2*lw); // long dash - dot
  else if (lt==6) sprintf(output, "1 setlinecap [%.2f %.2f 0 %.2f 0 %.2f] 0 setdash", 7*lw, 2*lw, 2*lw, 2*lw); // long dash - dot dot
  else if (lt==7) sprintf(output, "1 setlinecap [%.2f %.2f 0 %.2f 0 %.2f 0 %.2f] 0 setdash", 7*lw, 2*lw, 2*lw, 2*lw, 2*lw); // long dash - dot dot dot
  else if (lt==8) sprintf(output, "0 setlinecap [%.2f %.2f %.2f %.2f] 0 setdash", 7*lw, 2*lw, 2*lw, 2*lw); // long dash - dash
  return output;
 }

// Point types
// NB: The fact that pt3 depends upon pt1 and pt2 must be accounted for in ppl_canvasdraw.c
char *eps_PointTypes[N_POINTTYPES] = {
       "/pt1 {2 copy ps75 sub exch ps75 sub exch moveto ps75 2 mul dup rlineto closepath stroke ps75 add exch ps75 sub exch moveto ps75 2 mul dup -1 mul rlineto closepath stroke } bind def",
       "/pt2 {2 copy exch ps sub exch moveto ps 2 mul 0 rlineto closepath stroke ps sub moveto 0 ps 2 mul rlineto closepath stroke } bind def",
       "/pt3 {2 copy pt1 pt2} bind def",
       "/pt4 {ps75 add exch ps75 add exch moveto ps75 -2 mul 0 rlineto 0 ps75 -2 mul rlineto ps75 2 mul 0 rlineto closepath stroke} bind def",
       "/pt5 {exch ps 1.183 mul add exch ps75 sub moveto ps -1.183 mul ps 2.049 mul 2 copy rlineto -1 mul rlineto closepath stroke} bind def",
       "/pt6 {2 copy exch ps75 1.1 mul add exch moveto ps75 1.1 mul 0 360 arc stroke} bind def",
       "/pt7 {exch ps75 add exch moveto ps75 -1 mul ps 1.3 mul 2 copy 2 copy rlineto -1 mul rlineto -1 mul exch -1 mul exch rlineto closepath stroke} bind def",
       "/pt8 {ps75 add exch ps75 add exch moveto ps75 -2 mul 0 rlineto 0 ps75 -2 mul rlineto ps75 2 mul 0 rlineto closepath fill} bind def",
       "/pt9 {exch ps 1.183 mul add exch ps75 sub moveto ps -1.183 mul ps 2.049 mul 2 copy rlineto -1 mul rlineto closepath fill} bind def",
       "/pt10 {2 copy exch ps75 1.1 mul add exch moveto ps75 1.1 mul 0 360 arc fill} bind def",
       "/pt11 {exch ps75 add exch moveto ps75 -1 mul ps 1.3 mul 2 copy 2 copy rlineto -1 mul rlineto -1 mul exch -1 mul exch rlineto closepath fill} bind def",
       "/pt12 {exch ps add exch moveto ps -.5 mul ps 0.866 mul rlineto ps -1 mul 0 rlineto ps -.5 mul ps -.866 mul 2 copy rlineto exch -1 mul exch rlineto ps 0 rlineto closepath stroke} bind def",
       "/pt13 { 2 copy ps add moveto /theta 162 def 4 { 2 copy exch theta cos ps mul add exch theta sin ps mul add lineto /theta theta 72 add def } repeat closepath stroke pop pop} bind def",
       "/pt14 {exch ps 1.183 mul add exch ps75 add moveto ps -1.183 mul ps -2.049 mul 2 copy rlineto -1 mul rlineto closepath stroke} bind def",
       "/pt15 {ps 1.183 mul add exch ps75 sub exch moveto ps 2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath stroke} bind def",
       "/pt16 {ps 1.183 mul add exch ps75 add exch moveto ps -2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath stroke} bind def",
       "/pt17 {10 {2 copy} repeat ps add moveto /theta 126 def 5 { exch ps 4 div theta cos mul add exch ps 4 div theta sin mul add lineto /theta theta 36 add def exch ps theta cos mul add exch ps theta sin mul add lineto /theta theta 36 add def } repeat closepath stroke} bind def",
       "/pt18 { moveto ps75 dup rlineto ps -1.5 mul 0 rlineto ps 1.5 mul dup dup neg rlineto neg 0 rlineto closepath stroke} bind def",
       "/pt19 { moveto ps75 dup rlineto 0 ps -1.5 mul rlineto ps 1.5 mul neg dup dup neg rlineto 0 exch rlineto closepath stroke} bind def",
       "/pt20 {5 {2 copy} repeat /theta 90 def 3 { exch ps theta cos mul add exch ps theta sin mul add moveto lineto stroke /theta theta 120 add def} repeat } bind def",
       "/pt21 {5 {2 copy} repeat /theta 270 def 3 { exch ps theta cos mul add exch ps theta sin mul add moveto lineto stroke /theta theta 120 add def} repeat } bind def",
       "/pt22 {exch ps add exch moveto ps -.5 mul ps 0.866 mul rlineto ps -1 mul 0 rlineto ps -.5 mul ps -.866 mul 2 copy rlineto exch -1 mul exch rlineto ps 0 rlineto closepath fill} bind def",
       "/pt23 { 2 copy ps add moveto /theta 162 def 4 { 2 copy exch theta cos ps mul add exch theta sin ps mul add lineto /theta theta 72 add def } repeat closepath fill pop pop} bind def",
       "/pt24 {exch ps 1.183 mul add exch ps75 add moveto ps -1.183 mul ps -2.049 mul 2 copy rlineto -1 mul rlineto closepath fill} bind def",
       "/pt25 {ps 1.183 mul add exch ps75 sub exch moveto ps 2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath fill} bind def",
       "/pt26 {ps 1.183 mul add exch ps75 add exch moveto ps -2.049 mul ps -1.183 mul 2 copy rlineto exch -1 mul exch rlineto closepath fill} bind def",
       "/pt27 {10 {2 copy} repeat ps add moveto /theta 126 def 5 { exch ps 4 div theta cos mul add exch ps 4 div theta sin mul add lineto /theta theta 36 add def exch ps theta cos mul add exch ps theta sin mul add lineto /theta theta 36 add def } repeat closepath fill} bind def",
       "/pt28 { moveto ps75 dup rlineto ps -1.5 mul 0 rlineto ps 1.5 mul dup dup neg rlineto neg 0 rlineto closepath fill} bind def",
       "/pt29 { moveto ps75 dup rlineto 0 ps -1.5 mul rlineto ps 1.5 mul neg dup dup neg rlineto 0 exch rlineto closepath fill} bind def",
       "/pt30 {11 {2 copy} repeat /theta 90 def 6 { exch ps theta cos mul add exch ps theta sin mul add moveto lineto stroke /theta theta 60 add def} repeat } bind def",
      };

// Star types
char *eps_StarTypes [N_STARTYPES] = {
       ""
      };

