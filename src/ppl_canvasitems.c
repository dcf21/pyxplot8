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
#include <string.h>

#include "ListTools/lt_dict.h"

#include "StringTools/str_constants.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

canvas_itemlist *canvas_items = NULL;

// Add a new multiplot canvas item to the list above
static int canvas_itemlist_add(char *line)
 {
  canvas_item *ptr;
  int i;

  // If we're not in multiplot mode, clear the canvas now
  if (settings_term_current.multiplot == SW_ONOFF_OFF) directive_clear();

  // Ensure that multiplot canvas list is initialised before trying to use it
  if (canvas_items == NULL)
   {
    canvas_items = (canvas_itemlist *)malloc(sizeof(canvas_itemlist));
    if (canvas_items == NULL) return 1;
    canvas_items->first = canvas_items->last = NULL;
    canvas_items->insert_point = &(canvas_items->first);
   }
  ptr = *(canvas_items->insert_point) = (canvas_item *)malloc(sizeof(canvas_item));
  if (*(canvas_items->insert_point)==NULL) return 1;

  // Store the commandline that the user typed
  ptr->commandline = (char *)malloc(strlen(line)+1);
  if (ptr->commandline==NULL) { free(ptr); *(canvas_items->insert_point) = NULL; return 1; }
  strcpy(ptr->commandline, line);

  // Copy the user's current settings
  ptr->settings = settings_graph_current;
  for (i=0; i<MAX_AXES; i++) ptr->XAxes[i] = XAxes[i];
  for (i=0; i<MAX_AXES; i++) ptr->YAxes[i] = YAxes[i];
  for (i=0; i<MAX_AXES; i++) ptr->ZAxes[i] = ZAxes[i];

  // Doubly link linked list
  if (canvas_items->last != NULL) canvas_items->last->next = ptr;
  ptr->prev = canvas_items->last;
  canvas_items->last = ptr;
  canvas_items->insert_point = &(ptr->next);
  return 0;
 }

// Implementation of the clear command. Also called whenever the canvas is to be cleared.
int directive_clear()
 {
  canvas_item *ptr, *next;

  if (canvas_items == NULL) return 0;
  ptr = canvas_items->first;
  while (ptr != NULL)
   {
    free(ptr->commandline);
    next = ptr->next;
    free(ptr);
    ptr = next;
   }
  free(canvas_items);
  canvas_items = NULL;
  return 0;
 }

// Implementation of the list command.
int directive_list()
 {
  canvas_item *ptr;
  int i;

  ppl_report("# ID   Command");
  if (canvas_items == NULL) return 0;
  ptr = canvas_items->first;
  i=0;
  while (ptr != NULL)
   {
    i++;
    sprintf(temp_err_string, "%5d  %s", i, ptr->commandline);
    ppl_report(temp_err_string);
    ptr = ptr->next;
   }
  return 0;
 }

// Implementation of the arrow command.
int directive_arrow(Dict *command, char *line, int interactive)
 {
  canvas_item *ptr;

  if (canvas_itemlist_add(line)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ptr = canvas_items->last;
  ptr->type = CANVAS_ARROW;
  return 0;
 }

// Implementation of the text command.
int directive_text(Dict *command, char *line, int interactive)
 {
  canvas_item *ptr;

  if (canvas_itemlist_add(line)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ptr = canvas_items->last;
  ptr->type = CANVAS_TEXT;
  return 0;
 }

// Implementation of the jpeg command.
int directive_jpeg(Dict *command, char *line, int interactive)
 {
  canvas_item *ptr;

  if (canvas_itemlist_add(line)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ppl_report(DictPrint(command, temp_err_string, LSTR_LENGTH));
  ptr = canvas_items->last;
  ptr->type = CANVAS_JPEG;
  return 0;
 }

// Implementation of the plot and replot commands.
int directive_plot(Dict *command, char *line, int interactive, int replot)
 {
  canvas_item *ptr;

  if (!replot)
   {
    if (canvas_itemlist_add(line)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
    ptr = canvas_items->last;
    ptr->type = CANVAS_PLOT;
   }
  return 0;
 }

