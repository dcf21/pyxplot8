// ppl_canvasitems.c
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

#define _PPL_CANVASITEMS_C 1

#include <stdlib.h>
#include <stdio.h>

#include "ListTools/lt_dict.h"

#include "StringTools/str_constants.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"

canvas_itemlist *canvas_items = NULL;

int directive_clear()
 {
  return 0;
 }

int directive_list(Dict *command)
 {
  return 0;
 }

int directive_arrow(Dict *command, int interactive)
 {
  return 0;
 }

int directive_text(Dict *command, int interactive)
 {
  return 0;
 }

int directive_jpeg(Dict *command, int interactive)
 {
  ppl_report(DictPrint(command, temp_err_string, LSTR_LENGTH));
  return 0;
 }

int directive_plot(Dict *command, int interactive, int replot)
 {
  return 0;
 }

