// eps_point.c
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

#define _PPL_EPS_POINT 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ListTools/lt_memory.h"

#include "ppl_canvasdraw.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_plot_styles.h"
#include "eps_point.h"
#include "eps_settings.h"
#include "eps_style.h"

void eps_point_YieldUpText(EPSComm *x)
 {
  CanvasTextItem *i;

  if (x->current->text != NULL)
   {
    x->current->FirstTextID = x->NTextItems;
    if (x->current->text[0]=='\0') return;

    i = (CanvasTextItem *)lt_malloc(sizeof(CanvasTextItem));
    if (i==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); *(x->status) = 1; return; }
    i->text              = x->current->text;
    i->CanvasMultiplotID = x->current->id;
    ListAppendPtr(x->TextItems, i, sizeof(CanvasTextItem), 0, DATATYPE_VOID);
    x->NTextItems++;
   }
  return;
 }

void eps_point_RenderEPS(EPSComm *x)
 {
  int    pt, lt;
  double lw, lw_scale, xpos, ypos;
  with_words ww, ww_default;
  int pageno;

  pageno = x->LaTeXpageno = x->current->FirstTextID;
  x->LaTeXpageno++;

  // Print label at top of postscript description of box
  fprintf(x->epsbuffer, "%% Canvas item %d [point]\n", x->current->id);
  eps_core_clear(x);

  // Look up position of point
  xpos =  x->current->xpos  * M_TO_PS;
  ypos =  x->current->ypos  * M_TO_PS;

  // Expand any numbered styles which may appear in the with words we are passed
  eps_withwords_default(&ww_default, &x->current->settings, 0, 0, 0, 0);
  with_words_merge(&ww, &x->current->with_data, &ww_default, NULL, NULL, NULL, 1);

  // Display point
  eps_core_SetColour(x, &ww, 0);
  IF_NOT_INVISIBLE
   {
    // Set linewidth and linetype of point
    if (ww.USElinewidth) lw_scale = ww.pointlinewidth;
    else                 lw_scale = x->current->settings.LineWidth;
    lw = EPS_DEFAULT_LINEWIDTH * lw_scale;
    lt = 1;
    eps_core_SetLinewidth(x, lw, lt, 0.0);
    fprintf(x->epsbuffer, "/ps { %f } def\n", ww.pointsize * EPS_DEFAULT_PS);

    pt = (ww.pointtype-1) % N_POINTTYPES;
    while (pt<0) pt+=N_POINTTYPES;
    x->PointTypesUsed[pt] = 1;
    fprintf(x->epsbuffer, "%.2f %.2f pt%d\n", xpos, ypos, pt+1);
    eps_core_BoundingBox(x, xpos, ypos, 2 * ww.pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS);

    // Label point if it is labelled
    if ((x->current->text != NULL) && (x->current->text[0]!='\0'))
     {
      canvas_EPSRenderTextItem(x, pageno,
              x->current->xpos - (x->current->settings.TextHAlign - SW_HALIGN_CENT) * ww.pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS / M_TO_PS * 1.1,
              x->current->ypos + (x->current->settings.TextVAlign - SW_VALIGN_CENT) * ww.pointsize * eps_PointSize[pt] * EPS_DEFAULT_PS / M_TO_PS * 1.1,
              x->current->settings.TextHAlign, x->current->settings.TextVAlign, x->LastEPSColour, x->current->settings.FontSize, 0.0, NULL, NULL);
     }
   }

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

