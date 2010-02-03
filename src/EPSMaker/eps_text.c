// eps_text.c
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

#define _PPL_EPS_TEXT 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ListTools/lt_memory.h"

#include "ppl_error.h"
#include "ppl_canvasdraw.h"
#include "ppl_setting_types.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_text.h"
#include "eps_settings.h"

void eps_text_YieldUpText(EPSComm *x)
 {
  CanvasTextItem *i;

  x->current->FirstTextID = x->NTextItems;
  if (x->current->text[0]=='\0') return;

  i = (CanvasTextItem *)lt_malloc(sizeof(CanvasTextItem));
  if (i==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); *(x->status) = 1; return; }
  i->text              = x->current->text;
  i->CanvasMultiplotID = x->current->id;
  ListAppendPtr(x->TextItems, i, sizeof(CanvasTextItem), 0, DATATYPE_VOID);
  x->NTextItems++;
  return;
 }

void eps_text_RenderEPS(EPSComm *x)
 {
  with_words def, merged;
  char *colstr;
  double xgap, ygap, xgap2, ygap2;
  int pageno;

  pageno = x->LaTeXpageno = x->current->FirstTextID;
  x->LaTeXpageno++;

  if (x->current->text[0]=='\0') return;

  // Write header at top of postscript
  fprintf(x->epsbuffer, "%% Canvas item %d [text label]\n", x->current->id);

  // Work out text colour
  with_words_zero(&def,0);
  def.colour    = x->current->settings.TextColour;
  def.colourR   = x->current->settings.TextColourR;
  def.colourG   = x->current->settings.TextColourG;
  def.colourB   = x->current->settings.TextColourB;
  def.USEcolour = (def.colour!=0);
  def.USEcolourRGB = (def.colour==0);
  with_words_merge(&merged, &x->current->with_data, &def, NULL, NULL, NULL, 1);
  eps_core_SetColour(x, &merged, 1);
  colstr = x->LastEPSColour;

  // Render text item to eps
  xgap  = -(x->current->settings.TextHAlign - SW_HALIGN_CENT) * x->current->xpos2;
  ygap  =  (x->current->settings.TextVAlign - SW_VALIGN_CENT) * x->current->xpos2;

  xgap2 = xgap*cos(x->current->rotation) - ygap*sin(x->current->rotation);
  ygap2 = xgap*sin(x->current->rotation) + ygap*cos(x->current->rotation);

  canvas_EPSRenderTextItem(x, pageno, x->current->xpos + xgap2, x->current->ypos + ygap2,
      x->current->settings.TextHAlign, x->current->settings.TextVAlign, colstr, x->current->settings.FontSize, x->current->rotation, NULL, NULL);

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

