// ppl_canvasitems.c
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

#define _PPL_CANVASITEMS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ListTools/lt_dict.h"
#include "ListTools/lt_memory.h"

#include "StringTools/str_constants.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"

canvas_itemlist *canvas_items = NULL;

// Free a node in the multiplot canvas item list
static void canvas_item_delete(canvas_item *ptr)
 {
  int i;
  if (ptr->text        != NULL) free(ptr->text);
  with_words_destroy(&(ptr->settings.DataStyle));
  with_words_destroy(&(ptr->settings.FuncStyle));
  if (ptr->XAxes != NULL) { for (i=0; i<MAX_AXES; i++) DestroyAxis( &(ptr->XAxes[i]) ); free(ptr->XAxes); }
  if (ptr->YAxes != NULL) { for (i=0; i<MAX_AXES; i++) DestroyAxis( &(ptr->YAxes[i]) ); free(ptr->YAxes); }
  if (ptr->ZAxes != NULL) { for (i=0; i<MAX_AXES; i++) DestroyAxis( &(ptr->ZAxes[i]) ); free(ptr->ZAxes); }
  arrow_list_destroy(&(ptr->arrow_list));
  label_list_destroy(&(ptr->label_list));
  with_words_destroy(&(ptr->with_data));
  // !!! Delete plot data structure !!!
  free(ptr);
  return;
 }

// Add a new multiplot canvas item to the list above
static int canvas_itemlist_add(Dict *command, int type, canvas_item **output, int *id, unsigned char IncludeAxes)
 {
  canvas_item *ptr, *next, **insertpoint;
  int i, PrevId, *EditNo;

  // If we're not in multiplot mode, clear the canvas now
  if (settings_term_current.multiplot == SW_ONOFF_OFF) directive_clear();

  // Ensure that multiplot canvas list is initialised before trying to use it
  if (canvas_items == NULL)
   {
    canvas_items = (canvas_itemlist *)malloc(sizeof(canvas_itemlist));
    if (canvas_items == NULL) return 1;
    canvas_items->first = NULL;
   }

  DictLookup(command, "editno", NULL, (void *)&EditNo);
  insertpoint = &(canvas_items->first);
  PrevId      = 0;
  while ((*insertpoint != NULL) && ((EditNo==NULL) ? ((*insertpoint)->id <= PrevId+1) : ((*insertpoint)->id < *EditNo))) { PrevId=(*insertpoint)->id; insertpoint = &((*insertpoint)->next); }
  if ((EditNo != NULL) && (*insertpoint != NULL) && ((*insertpoint)->id == *EditNo))
   {
    next = (*insertpoint)->next;
    canvas_item_delete(*insertpoint);
   } else {
    next = (*insertpoint);
   }
  ptr = *insertpoint = (canvas_item *)malloc(sizeof(canvas_item));
  if (ptr==NULL) return 1;
  ptr->next    = next; // Link singly-linked list
  ptr->text    = NULL;
  ptr->plot_items = NULL;
  ptr->id      = (EditNo == NULL) ? (PrevId+1) : (*EditNo);
  ptr->type    = type;
  ptr->deleted = 0;
  with_words_zero(&ptr->with_data, 0);

  // Copy the user's current settings
  ptr->settings = settings_graph_current;
  with_words_copy(&ptr->settings.DataStyle , &settings_graph_current.DataStyle);
  with_words_copy(&ptr->settings.FuncStyle , &settings_graph_current.FuncStyle);
  if (IncludeAxes)
   {
    ptr->XAxes = (settings_axis *)malloc(MAX_AXES * sizeof(settings_axis));
    ptr->YAxes = (settings_axis *)malloc(MAX_AXES * sizeof(settings_axis));
    ptr->ZAxes = (settings_axis *)malloc(MAX_AXES * sizeof(settings_axis));
    if ((ptr->XAxes==NULL)||(ptr->YAxes==NULL)||(ptr->ZAxes==NULL))
     {
      ppl_error(ERR_MEMORY,"Out of memory");
      if (ptr->XAxes!=NULL) { free(ptr->XAxes); ptr->XAxes = NULL; }
      if (ptr->YAxes!=NULL) { free(ptr->YAxes); ptr->YAxes = NULL; }
      if (ptr->ZAxes!=NULL) { free(ptr->ZAxes); ptr->ZAxes = NULL; }
     }
    else
     {
      for (i=0; i<MAX_AXES; i++) CopyAxis(&(ptr->XAxes[i]), &(XAxes[i]));
      for (i=0; i<MAX_AXES; i++) CopyAxis(&(ptr->YAxes[i]), &(YAxes[i]));
      for (i=0; i<MAX_AXES; i++) CopyAxis(&(ptr->ZAxes[i]), &(ZAxes[i]));
     }
    arrow_list_copy(&ptr->arrow_list , &arrow_list);
    label_list_copy(&ptr->label_list , &label_list);
   } else {
    ptr->XAxes = ptr->YAxes = ptr->ZAxes = NULL;
    ptr->arrow_list = NULL;
    ptr->label_list = NULL;
   }

  *output = ptr; // Return pointer to the newly-created canvas item
  *id     = ptr->id;
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
    next = ptr->next;
    canvas_item_delete(ptr);
    ptr = next;
   }
  free(canvas_items);
  canvas_items = NULL;
  return 0;
 }

// Produce a textual representation of the command which would need to be typed to produce any given canvas item
char *canvas_item_textify(canvas_item *ptr, char *output)
 {
  int i;
  if      (ptr->type == CANVAS_ARROW) // Produce textual representations of arrow commands
   {
    sprintf(output, "arrow item %d from %s,%s to %s,%s", ptr->id,
             NumericDisplay( ptr->xpos            *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->ypos            *100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay((ptr->xpos+ptr->xpos2)*100, 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay((ptr->ypos+ptr->ypos2)*100, 3, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i = strlen(output);
    with_words_print(&ptr->with_data, output+i+6);
    if (strlen(output+i+6)>0) { sprintf(output+i, " with"); output[i+5]=' '; }
    else                      { output[i]='\0'; }
   }
  else if (ptr->type == CANVAS_BOX  ) // Produce textual representations of box commands
   {
    sprintf(output, "box item %d from %s,%s to %s,%s", ptr->id,
             NumericDisplay( ptr->xpos            *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->ypos            *100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay((ptr->xpos+ptr->xpos2)*100, 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay((ptr->ypos+ptr->ypos2)*100, 3, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i = strlen(output);
    with_words_print(&ptr->with_data, output+i+6);
    if (strlen(output+i+6)>0) { sprintf(output+i, " with"); output[i+5]=' '; }
    else                      { output[i]='\0'; }
   }
  else if (ptr->type == CANVAS_CIRC ) // Produce textual representations of circle commands
   {
    sprintf(output, "circle item %d at %s,%s radius %s", ptr->id,
             NumericDisplay( ptr->xpos            *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->ypos            *100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->xpos2           *100, 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i = strlen(output);
    with_words_print(&ptr->with_data, output+i+6);
    if (strlen(output+i+6)>0) { sprintf(output+i, " with"); output[i+5]=' '; }
    else                      { output[i]='\0'; }
   }
  else if (ptr->type == CANVAS_ELLPS) // Produce textual representations of ellipse commands
   {
    sprintf(output, "ellipse item %d", ptr->id);
    i = strlen(output);
    if (ptr->x1set ) sprintf(output+i, " from %s,%s to %s,%s",
             NumericDisplay( ptr->x1  *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->y1  *100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->x2  *100, 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->y2  *100, 3, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->xcset ) sprintf(output+i, " centre %s,%s",
             NumericDisplay( ptr->xc  *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->yc  *100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->xfset ) sprintf(output+i, " focus %s,%s",
             NumericDisplay( ptr->xf  *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->yf  *100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->aset  ) sprintf(output+i, " MajorAxis %s",
             NumericDisplay( ptr->a   *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->bset  ) sprintf(output+i, " MinorAxis %s",
             NumericDisplay( ptr->b   *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->eccset) sprintf(output+i, " eccentricity %s",
             NumericDisplay( ptr->ecc     , 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->slrset) sprintf(output+i, " SemiLatusRectum %s",
             NumericDisplay( ptr->slr *100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    sprintf(output+i, " rotate %s",
             NumericDisplay( ptr->rotation *180/M_PI , 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    with_words_print(&ptr->with_data, output+i+6);
    if (strlen(output+i+6)>0) { sprintf(output+i, " with"); output[i+5]=' '; }
    else                      { output[i]='\0'; }
   }
  else if (ptr->type == CANVAS_EPS  ) // Produce textual representations of eps commands
   {
    sprintf(output, "eps item %d ", ptr->id);
    i = strlen(output);
    StrEscapify(ptr->text, output+i);
    i += strlen(output+i);
    sprintf(output+i, " at %s,%s",
             NumericDisplay(ptr->xpos*100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay(ptr->ypos*100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->xpos2set) sprintf(output+i, " width %s" , NumericDisplay(ptr->xpos2*100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)));
    i += strlen(output+i);
    if (ptr->ypos2set) sprintf(output+i, " height %s", NumericDisplay(ptr->ypos2*100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)));
    i += strlen(output+i);
    sprintf(output+i, " rotate %s",
             NumericDisplay(ptr->rotation * 180/M_PI , 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->clip    ) { sprintf(output+i, " clip"    ); i += strlen(output+i); }
    if (ptr->calcbbox) { sprintf(output+i, " calcbbox"); i += strlen(output+i); }
   }
  else if (ptr->type == CANVAS_IMAGE) // Produce textual representations of image commands
   {
    sprintf(output, "image item %d ", ptr->id);
    i = strlen(output);
    StrEscapify(ptr->text, output+i);
    i += strlen(output+i);
    sprintf(output+i, " at %s,%s",
             NumericDisplay(ptr->xpos*100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay(ptr->ypos*100, 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    if (ptr->smooth            ) { sprintf(output+i, " smooth");                                                                  i += strlen(output+i); }
    if (ptr->NoTransparency    ) { sprintf(output+i, " NoTransparency");                                                          i += strlen(output+i); }
    if (ptr->CustomTransparency) { sprintf(output+i, " transparent rgb%d:%d:%d", ptr->TransColR, ptr->TransColG, ptr->TransColB); i += strlen(output+i); }
    if (ptr->xpos2set) sprintf(output+i, " width %s" , NumericDisplay(ptr->xpos2*100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)));
    i += strlen(output+i);
    if (ptr->ypos2set) sprintf(output+i, " height %s", NumericDisplay(ptr->ypos2*100, 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)));
    i += strlen(output+i);
    sprintf(output+i, " rotate %s",
             NumericDisplay(ptr->rotation * 180/M_PI , 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
   }
  else if (ptr->type == CANVAS_PLOT ) // Produce textual representations of plot commands
   {
    sprintf(output, "[plot]");
   }
  else if (ptr->type == CANVAS_TEXT ) // Produce textual representations of text commands
   {
    sprintf(output, "text item %d ", ptr->id);
    i = strlen(output);
    StrEscapify(ptr->text, output+i);
    i += strlen(output+i);
    sprintf(output+i, " at %s,%s rotate %s",
             NumericDisplay( ptr->xpos     * 100     , 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->ypos     * 100     , 1, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L)),
             NumericDisplay( ptr->rotation * 180/M_PI, 2, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           );
    i += strlen(output+i);
    with_words_print(&ptr->with_data, output+i+6);
    if (strlen(output+i+6)>0) { sprintf(output+i, " with"); output[i+5]=' '; }
    else                      { output[i]='\0'; }
   }
  else
   { sprintf(output, "[unknown object]"); } // Ooops.
  return output;
 }

// Implementation of the list command.
int directive_list()
 {
  int i;
  canvas_item *ptr;

  ppl_report("# ID   Command");
  if (canvas_items == NULL) return 0;
  ptr = canvas_items->first;
  while (ptr != NULL)
   {
    sprintf(temp_err_string, "%5d  %s", ptr->id, (ptr->deleted) ? "[deleted] " : "");
    i = strlen(temp_err_string);
    canvas_item_textify(ptr, temp_err_string+i);
    ppl_report(temp_err_string);
    ptr = ptr->next;
   }
  return 0;
 }

// Implementation of the delete command.
static int canvas_delete(int id)
 {
  canvas_item  *ptr;

  ptr = canvas_items->first;
  while ((ptr!=NULL)&&(ptr->id!=id)) ptr=ptr->next;
  if (ptr==NULL) { sprintf(temp_err_string, "There is no multiplot item with ID %d.", id); ppl_warning(ERR_GENERAL, temp_err_string); return 1; }
  else           { ptr->deleted = 1; }
  return 0;
 }

int directive_delete(Dict *command)
 {
  List          *DelList;
  ListIterator  *ListIter;
  int           *id;
  unsigned char *unsuccessful_ops;

  if (canvas_items==NULL) { sprintf(temp_err_string, "There are currently no items on the multiplot canvas."); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  DictLookup(command, "deleteno,", NULL, (void *)&DelList);
  ListIter = ListIterateInit(DelList);
  while (ListIter != NULL)
   {
    DictLookup(ListIter->data, "number", NULL, (void *)&id);
    canvas_delete(*id);
    ListIter = ListIterate(ListIter, NULL);
   }

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
   }
  return 0;
 }

// Implementation of the undelete command.
int directive_undelete(Dict *command)
 {
  List          *UndelList;
  ListIterator  *ListIter;
  int           *id;
  unsigned char *unsuccessful_ops;
  canvas_item   *ptr;

  if (canvas_items==NULL) { sprintf(temp_err_string, "There are currently no items on the multiplot canvas."); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  DictLookup(command, "undeleteno,", NULL, (void *)&UndelList);
  ListIter = ListIterateInit(UndelList);
  while (ListIter != NULL)
   {
    DictLookup(ListIter->data, "number", NULL, (void *)&id);
    ptr = canvas_items->first;
    while ((ptr!=NULL)&&(ptr->id!=*id)) ptr=ptr->next;
    if (ptr==NULL) { sprintf(temp_err_string, "There is no multiplot item with ID %d.", *id); ppl_warning(ERR_GENERAL, temp_err_string); }
    else           { ptr->deleted = 0; }
    ListIter = ListIterate(ListIter, NULL);
   }

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
   }
  return 0;
 }

#define ASSERT_LENGTH(VAR,CMD,NAME) \
  if (!(VAR->dimensionless)) \
   { \
    for (i=0; i<UNITS_MAX_BASEUNITS; i++) \
     if (VAR->exponent[i] != (i==UNIT_LENGTH)) \
      { \
       sprintf(temp_err_string,"The position supplied to the '%s' command must have dimensions of length. Supplied %s input has units of <%s>.",CMD,NAME,ppl_units_GetUnitStr(VAR,NULL,NULL,1,0)); \
       ppl_error(ERR_NUMERIC, temp_err_string); \
       return 1; \
      } \
   } \
  else { VAR->real /= 100; } // By default, dimensionless positions are in centimetres

#define ASSERT_ANGLE(VAR,CMD) \
  if (!(VAR->dimensionless)) \
   { \
    for (i=0; i<UNITS_MAX_BASEUNITS; i++) \
     if (VAR->exponent[i] != (i==UNIT_ANGLE)) \
      { \
       sprintf(temp_err_string,"The rotation angle supplied to the '%s' command must have dimensions of angle. Supplied input has units of <%s>.",CMD,ppl_units_GetUnitStr(VAR,NULL,NULL,1,0)); \
       ppl_error(ERR_NUMERIC, temp_err_string); \
       return 1; \
      } \
   } \
  else { VAR->real *= M_PI/180.0; } // By default, dimensionless angles are in degrees

// Implementation of the move command.
int directive_move(Dict *command)
 {
  int           *moveno, i;
  value         *x, *y, *ang;
  unsigned char  rotatable;
  canvas_item   *ptr;
  unsigned char *unsuccessful_ops;

  if (canvas_items==NULL) { sprintf(temp_err_string, "There are currently no items on the multiplot canvas."); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  DictLookup(command, "moveno"  , NULL, (void *)&moveno);
  DictLookup(command, "x"       , NULL, (void *)&x);
  DictLookup(command, "y"       , NULL, (void *)&y);
  DictLookup(command, "rotation", NULL, (void *)&ang);

  ASSERT_LENGTH(x,"move","x");
  ASSERT_LENGTH(y,"move","y");
  if (ang!=NULL) { ASSERT_ANGLE(ang,"move"); }

  ptr = canvas_items->first;
  while ((ptr!=NULL)&&(ptr->id!=*moveno)) ptr=ptr->next;
  if (ptr==NULL) { sprintf(temp_err_string, "There is no multiplot item with ID %d.", *moveno); ppl_error(ERR_GENERAL, temp_err_string); return 1; }
  rotatable = ((ptr->type!=CANVAS_ARROW)&&(ptr->type!=CANVAS_BOX)&&(ptr->type!=CANVAS_CIRC));
  if ((ang != NULL) && (!rotatable)) { sprintf(temp_err_string, "It is not possible to rotate the specified multiplot item."); ppl_warning(ERR_GENERAL, temp_err_string); }
  ptr->xpos = x->real;
  ptr->ypos = y->real;
  if ((ang != NULL) && (rotatable)) ptr->rotation = ang->real;

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
   }
  return 0;
 }

// Implementation of the arrow command.
int directive_arrow(Dict *command, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x1, *x2, *y1, *y2;
  char          *tempstr;
  unsigned char *unsuccessful_ops;

  // Look up the start and end point of the arrow, and ensure that they are either dimensionless or in units of length
  DictLookup(command, "x1", NULL, (void *)&x1); DictLookup(command, "y1", NULL, (void *)&y1);
  DictLookup(command, "x2", NULL, (void *)&x2); DictLookup(command, "y2", NULL, (void *)&y2);

  ASSERT_LENGTH(x1,"arrow","x1"); ASSERT_LENGTH(y1,"arrow","y1");
  ASSERT_LENGTH(x2,"arrow","x2"); ASSERT_LENGTH(y2,"arrow","y2");

  // Add this arrow to the linked list which decribes the canvas
  if (canvas_itemlist_add(command,CANVAS_ARROW,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ptr->xpos  = x1->real;
  ptr->ypos  = y1->real;
  ptr->xpos2 = x2->real - x1->real;
  ptr->ypos2 = y2->real - y1->real;

  // Read in colour and linewidth information, if available
  with_words_fromdict(command, &ptr->with_data, 1);

  // Work out whether this arrow is in the 'head', 'nohead' or 'twoway' style
  DictLookup(command, "arrow_style", NULL, (void *)&tempstr);
  if (tempstr != NULL) ptr->ArrowType = FetchSettingByName(tempstr, SW_ARROWTYPE_INT, SW_ARROWTYPE_STR);
  else                 ptr->ArrowType = SW_ARROWTYPE_HEAD;

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Arrow has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the box command.
int directive_box(Dict *command, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x1, *x2, *y1, *y2;
  unsigned char *unsuccessful_ops;

  // Look up the positions of the two corners of the box, and ensure that they are either dimensionless or in units of length
  DictLookup(command, "x1", NULL, (void *)&x1); DictLookup(command, "y1", NULL, (void *)&y1);
  DictLookup(command, "x2", NULL, (void *)&x2); DictLookup(command, "y2", NULL, (void *)&y2);

  ASSERT_LENGTH(x1,"box","x1"); ASSERT_LENGTH(y1,"box","y1");
  ASSERT_LENGTH(x2,"box","x2"); ASSERT_LENGTH(y2,"box","y2");

  // Add this box to the linked list which decribes the canvas
  if (canvas_itemlist_add(command,CANVAS_BOX,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ptr->xpos  = x1->real;
  ptr->ypos  = y1->real;
  ptr->xpos2 = x2->real - x1->real;
  ptr->ypos2 = y2->real - y1->real;

  // Read in colour and linewidth information, if available
  with_words_fromdict(command, &ptr->with_data, 1);

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Arrow has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the circle command.
int directive_circle(Dict *command, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x1, *x2, *y1;
  unsigned char *unsuccessful_ops;

  // Look up the position of the centre of the circle and its radius
  DictLookup(command, "x", NULL, (void *)&x1);
  DictLookup(command, "y", NULL, (void *)&y1);
  DictLookup(command, "r", NULL, (void *)&x2);

  ASSERT_LENGTH(x1,"circle","x");
  ASSERT_LENGTH(y1,"circle","y");
  ASSERT_LENGTH(x2,"circle","r");

  // Add this circle to the linked list which decribes the canvas
  if (canvas_itemlist_add(command,CANVAS_CIRC,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ptr->xpos  = x1->real;
  ptr->ypos  = y1->real;
  ptr->xpos2 = x2->real;

  // Read in colour and linewidth information, if available
  with_words_fromdict(command, &ptr->with_data, 1);

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Circle has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the ellipse command.
int directive_ellipse(Dict *command, int interactive)
 {
  canvas_item   *ptr;
  int            i, e=0, r=0, p=0, id;
  value         *x1, *y1, *x2, *y2, *xc, *yc, *xf, *yf;
  value         *a, *b, *slr, *ang;
  double        *ecc;
  double         xc_dbl, yc_dbl, a_dbl, b_dbl, ecc_dbl, ang_dbl, ratio;
  unsigned char *unsuccessful_ops;

  // Look up the input parameters which define the ellipse
  DictLookup(command, "rotation"    , NULL, (void *)&ang);
  DictLookup(command, "x1"          , NULL, (void *)&x1);
  DictLookup(command, "y1"          , NULL, (void *)&y1);
  DictLookup(command, "x2"          , NULL, (void *)&x2);
  DictLookup(command, "y2"          , NULL, (void *)&y2);
  DictLookup(command, "xcentre"     , NULL, (void *)&xc);
  DictLookup(command, "ycentre"     , NULL, (void *)&yc);
  DictLookup(command, "xfocus"      , NULL, (void *)&xf);
  DictLookup(command, "yfocus"      , NULL, (void *)&yf);
  DictLookup(command, "majoraxis"   , NULL, (void *)&a);
  DictLookup(command, "minoraxis"   , NULL, (void *)&b);
  DictLookup(command, "slr"         , NULL, (void *)&slr);
  DictLookup(command, "eccentricity", NULL, (void *)&ecc);

  // Check that input parameters have the right units, and convert dimensionless lengths into cm
  if (ang!= NULL) { ASSERT_ANGLE (ang,"ellipse"); ang_dbl = ang->real; r++; } else { ang_dbl=0.0; }
  if (x1 != NULL) { ASSERT_LENGTH(x1 ,"ellipse","x1"); }
  if (y1 != NULL) { ASSERT_LENGTH(y1 ,"ellipse","y1"); }
  if (x2 != NULL) { ASSERT_LENGTH(x2 ,"ellipse","x2"); }
  if (y2 != NULL) { ASSERT_LENGTH(y2 ,"ellipse","y2"); }
  if (xc != NULL) { ASSERT_LENGTH(xc ,"ellipse","xcentre"); p++; }
  if (yc != NULL) { ASSERT_LENGTH(yc ,"ellipse","ycentre"); }
  if (xf != NULL) { ASSERT_LENGTH(xf ,"ellipse","xfocus");  p++; }
  if (yf != NULL) { ASSERT_LENGTH(yf ,"ellipse","yfocus"); }
  if (a  != NULL) { ASSERT_LENGTH(a  ,"ellipse","majoraxis");       e++; }
  if (b  != NULL) { ASSERT_LENGTH(b  ,"ellipse","minoraxis");       e++; }
  if (ecc!= NULL) {                                                 e++; if ((*ecc<0.0) || (*ecc>=1.0)) { ppl_error(ERR_NUMERIC, "Supplied eccentricity is not in the range 0 <= e < 1."); return 1; } }
  if (slr!= NULL) { ASSERT_LENGTH(slr,"ellipse","semilatusrectum"); e++; }

  // Check that we have been supplied an appropriate set of inputs
  if ( (x1==NULL) && (((p==2)&&((e!=1)||(r!=0))) || ((p<2)&&(e!=2))) )
   { printf("%d %d %d\n",p,e,r); ppl_error(ERR_GENERAL, "Ellipse command has received an inappropriate set of inputs. Must specify either the position of both the centre and focus of the ellipse, and one further piece of information out of the major axis length, the minor axis length, the eccentricity or the semi-latus rectum, or the position of one of these two points, the rotation angle of the major axis of the ellipse, and two further pieces of information."); return 1; }

  // Convert inputs such that we have the position of the centre of the ellipse and major/minor axes
  if (x1 != NULL) // User has specified two corners of the ellipse
   {
    xc_dbl = (x2->real + x1->real) / 2.0;
    yc_dbl = (y2->real + y1->real) / 2.0;
    a_dbl  = (x2->real - x1->real) / 2.0;
    b_dbl  = (y2->real - y1->real) / 2.0;
   }
  else if (p==2) // User has specified both centre and focus of the ellipse, and one further piece of information
   {
    if (ppl_units_DblEqual(xc->real, xf->real) && ppl_units_DblEqual(yc->real, yf->real)) { ang_dbl = 0.0; }
    else                                                                                  { ang_dbl = atan2(xc->real - xf->real , yc->real - yf->real); }
    xc_dbl = xc->real;
    yc_dbl = yc->real;

    if      (a != NULL) // Additional piece of information was major axis...
     {
      a_dbl   = fabs(a->real);
      ecc_dbl = hypot(xc->real - xf->real , yc->real - yf->real) / a_dbl;
      if ((ecc_dbl < 0.0) || (ecc_dbl >= 1.0)) { ppl_error(ERR_NUMERIC, "Supplied major axis length is longer than the distance between the supplied focus and centre of the ellipse. No ellipse may have such parameters."); return 1; }
      if (ppl_units_DblEqual(ecc_dbl,0.0)) { b_dbl = a_dbl; }
      else                                 { b_dbl = a_dbl * sqrt(1.0-pow(ecc_dbl,2)); }
     }
    else if (b != NULL) // minor axis...
     {
      b_dbl   = fabs(b->real);
      a_dbl   = hypot(hypot(xc->real - xf->real , yc->real - yf->real) , b_dbl);
      if (b_dbl > a_dbl) { ppl_error(ERR_NUMERIC, "Supplied minor axis length is longer than the implied major axis length of the ellipse."); return 1; }
      ecc_dbl = sqrt(1.0 - pow(b_dbl/a_dbl , 2.0));
     }
    else if (ecc != NULL) // eccentricity...
     {
      ecc_dbl = *ecc;
      a_dbl   = hypot(xc->real - xf->real , yc->real - yf->real) / ecc_dbl;
      if (ppl_units_DblEqual(ecc_dbl,0.0)) { b_dbl = a_dbl; }
      else                                 { b_dbl = a_dbl * sqrt(1.0-pow(ecc_dbl,2)); }
     }
    else if (slr != NULL) // or semi-latus rectum...
     {
      ratio   = hypot(xc->real - xf->real , yc->real - yf->real) / slr->real;
      ecc_dbl = (sqrt(1+4*pow(ratio,2))-1.0) / (2*ratio);
      if ((ecc_dbl<0.0) || (ecc_dbl>=1.0)) { ppl_error(ERR_NUMERIC, "Eccentricity implied for ellipse is not in the range 0 <= e < 1."); return 1; }
      a_dbl   = hypot(xc->real - xf->real , yc->real - yf->real) / ecc_dbl;
      b_dbl   = a_dbl * sqrt(1.0 - pow(ecc_dbl,2.0));
     }
    else { ppl_error(ERR_INTERNAL, "Flow control error in ellipse command."); return 1; }
   }
  else // User has specified centre / focus of ellipse and two further pieces of information...
   {
    if      ((a   != NULL) && (b   != NULL)) // major and minor axes...
     {
      a_dbl   = fabs(a->real);
      b_dbl   = fabs(b->real);
      if (b_dbl>a_dbl) { ppl_error(ERR_NUMERIC, "Supplied minor axis length is longer than the supplied major axis length of the ellipse."); return 1; }
      ecc_dbl = sqrt(1.0 - pow(b_dbl/a_dbl , 2.0));
     }
    else if ((a   != NULL) && (ecc != NULL)) // major axis and eccentricity...
     {
      a_dbl   = fabs(a->real);
      ecc_dbl = *ecc;
      b_dbl   = a_dbl * sqrt(1.0 - pow(ecc_dbl,2.0));
     }
    else if ((a   != NULL) && (slr != NULL)) // major axis and SLR...
     {
      a_dbl   = fabs(a->real);
      if (fabs(slr->real) > a_dbl) { ppl_error(ERR_NUMERIC, "Supplied semi-latus rectum is longer than the supplied major axis length of the ellipse."); return 1; }
      ecc_dbl = sqrt(1.0 - fabs(slr->real) / a_dbl);
      b_dbl   = a_dbl * sqrt(1.0 - pow(ecc_dbl,2.0));
     }
    else if ((b   != NULL) && (ecc != NULL)) // minor axis and eccentricity...
     {
      b_dbl   = b->real;
      ecc_dbl = *ecc;
      a_dbl   = b_dbl / sqrt(1.0 - pow(ecc_dbl,2.0));
     }
    else if ((b   != NULL) && (slr != NULL)) // minor axis and SLR...
     {
      b_dbl   = b->real;
      if (fabs(slr->real) > b_dbl) { ppl_error(ERR_NUMERIC, "Supplied semi-latus rectum is longer than the supplied minor axis length of the ellipse."); return 1; }
      ecc_dbl = sqrt(1.0 - pow(fabs(slr->real) / b_dbl,2.0));
      a_dbl   = b_dbl / sqrt(1.0 - pow(ecc_dbl,2.0));
     }
    else if ((ecc != NULL) && (slr != NULL)) // eccentricity and SLR...
     {
      ecc_dbl = *ecc;
      a_dbl   = fabs(slr->real) / (1.0 - pow(ecc_dbl,2.0));
      b_dbl   = a_dbl * sqrt(1.0 - pow(ecc_dbl,2.0));
     }
    else { ppl_error(ERR_INTERNAL, "Flow control error in ellipse command."); return 1; }

    if (xc != NULL) // User has specified the centre of the ellipse
     {
      xc_dbl = xc->real;
      yc_dbl = yc->real;
     } else { // User has specified the focus of the ellipse... convert to the centre by translating distance a * eccentricity
      xc_dbl = xf->real + a_dbl * ecc_dbl * cos( ang_dbl);
      yc_dbl = yf->real + a_dbl * ecc_dbl * sin( ang_dbl);
     }

   }

  // Add this ellipse to the linked list which decribes the canvas
  if (canvas_itemlist_add(command,CANVAS_ELLPS,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }

  // Add the exact parameterisation which we have been given to canvas item, so that "list" command prints it out in the form originally supplied
  ptr->x1set = ptr->xcset = ptr->xfset = ptr->aset = ptr->bset = ptr->eccset = ptr->slrset = 0;
  ptr->x1 = ptr->y1 = ptr->x2 = ptr->y2 = ptr->xc = ptr->yc = ptr->xf = ptr->yf = ptr->ecc = ptr->slr = 0.0;
  if (x1 != NULL) { ptr->x1set = 1; ptr->x1 = x1->real; ptr->y1 = y1->real; ptr->x2 = x2->real; ptr->y2 = y2->real; }
  if (xc != NULL) { ptr->xcset = 1; ptr->xc = xc->real; ptr->yc = yc->real; }
  if (xf != NULL) { ptr->xfset = 1; ptr->xf = xf->real; ptr->yf = yf->real; }
  if (a  != NULL) { ptr-> aset = 1; ptr->a  = a ->real; }
  if (b  != NULL) { ptr-> bset = 1; ptr->b  = b ->real; }
  if (ecc!= NULL) { ptr->eccset= 1; ptr->ecc= *ecc; }
  if (slr!= NULL) { ptr->slrset= 1; ptr->slr= slr->real; }

  // Set xpos, ypos, xpos2 and ypos2 to be centre position and major/minor axes of ellipse for quick drawing
  ptr->xpos     = xc_dbl;
  ptr->ypos     = yc_dbl;
  ptr->xpos2    = a_dbl;
  ptr->ypos2    = b_dbl;
  ptr->rotation = ang_dbl;

  // Read in colour and linewidth information, if available
  with_words_fromdict(command, &ptr->with_data, 1);

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Ellipse has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the eps command.
int directive_eps(Dict *command, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x, *y, *ang, *width, *height;
  unsigned char *unsuccessful_ops;
  char          *text, *fname, *tempstr1, *tempstr2;

  // Read in positional information for this eps image, and ensure that values are either dimensionless, or have units of length / angle as required
  DictLookup(command, "x"       , NULL, (void *)&x     );
  DictLookup(command, "y"       , NULL, (void *)&y     );
  DictLookup(command, "rotation", NULL, (void *)&ang   );
  DictLookup(command, "width"   , NULL, (void *)&width );
  DictLookup(command, "height"  , NULL, (void *)&height);
  DictLookup(command, "clip"    , NULL, (void *)&tempstr1);
  DictLookup(command, "calcbbox", NULL, (void *)&tempstr2);

  if (x     !=NULL) { ASSERT_LENGTH(x     ,"eps","x"     ); }
  if (y     !=NULL) { ASSERT_LENGTH(y     ,"eps","y"     ); }
  if (ang   !=NULL) { ASSERT_ANGLE (ang   ,"eps"         ); }
  if (width !=NULL) { ASSERT_LENGTH(width ,"eps","width" ); }
  if (height!=NULL) { ASSERT_LENGTH(height,"eps","height"); }

  DictLookup(command, "filename", NULL, (void *)&fname);
  text = (char *)malloc(strlen(fname)+1);
  if (text == NULL) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  strcpy(text, fname);

  if (canvas_itemlist_add(command,CANVAS_EPS,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); free(text); return 1; }

  if (x     !=NULL) { ptr->xpos     = x     ->real; }                    else { ptr->xpos     = settings_graph_current.OriginX.real; }
  if (y     !=NULL) { ptr->ypos     = y     ->real; }                    else { ptr->ypos     = settings_graph_current.OriginY.real; }
  if (ang   !=NULL) { ptr->rotation = ang   ->real; }                    else { ptr->rotation = 0.0;                                 }
  if (width !=NULL) { ptr->xpos2    = width ->real; ptr->xpos2set = 1; } else { ptr->xpos2    = 0.0; ptr->xpos2set = 0; }
  if (height!=NULL) { ptr->ypos2    = height->real; ptr->ypos2set = 1; } else { ptr->ypos2    = 0.0; ptr->ypos2set = 0; }
  ptr->text = text;
  if (tempstr1!=NULL) { ptr->clip     = 1; } else { ptr->clip     = 0; }
  if (tempstr2!=NULL) { ptr->calcbbox = 1; } else { ptr->calcbbox = 0; }

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("EPS image has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the text command.
int directive_text(Dict *command, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x, *y, *ang;
  unsigned char *unsuccessful_ops;
  char          *text, *fname;

  DictLookup(command, "x"       , NULL, (void *)&x  );
  DictLookup(command, "y"       , NULL, (void *)&y  );
  DictLookup(command, "rotation", NULL, (void *)&ang);

  if (x  !=NULL) { ASSERT_LENGTH(x  ,"eps","x"); }
  if (y  !=NULL) { ASSERT_LENGTH(y  ,"eps","y"); }
  if (ang!=NULL) { ASSERT_ANGLE (ang,"eps"    ); }

  DictLookup(command, "string", NULL, (void *)&fname);
  text = (char *)malloc(strlen(fname)+1);
  if (text == NULL) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  strcpy(text, fname);

  if (canvas_itemlist_add(command,CANVAS_TEXT,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); free(text); return 1; }

  if (x  !=NULL) { ptr->xpos     = x  ->real; } else { ptr->xpos      = settings_graph_current.OriginX.real; }
  if (y  !=NULL) { ptr->ypos     = y  ->real; } else { ptr->ypos      = settings_graph_current.OriginY.real; }
  if (ang!=NULL) { ptr->rotation = ang->real; } else { ptr->rotation  = 0.0;                                 }
  ptr->text = text;

  // Read in colour information, if available
  with_words_fromdict(command, &ptr->with_data, 1);

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Text item has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the image command.
int directive_image(Dict *command, int interactive)
 {
  canvas_item *ptr;
  int            i, id, *TransColR, *TransColG, *TransColB;
  value         *x, *y, *ang, *width, *height;
  unsigned char *unsuccessful_ops;
  char          *text, *fname, *smooth, *NoTransparency;

  // Read in positional information for this bitmap image, and ensure that values are either dimensionless, or have units of length / angle as required
  DictLookup(command, "x"       , NULL, (void *)&x     );
  DictLookup(command, "y"       , NULL, (void *)&y     );
  DictLookup(command, "smooth"  , NULL, (void *)&smooth);
  DictLookup(command, "rotation", NULL, (void *)&ang   );
  DictLookup(command, "width"   , NULL, (void *)&width );
  DictLookup(command, "height"  , NULL, (void *)&height);
  DictLookup(command, "notrans" , NULL, (void *)&NoTransparency);
  DictLookup(command, "colourR" , NULL, (void *)&TransColR     );
  DictLookup(command, "colourG" , NULL, (void *)&TransColG     );
  DictLookup(command, "colourB" , NULL, (void *)&TransColB     );

  if (x     !=NULL) { ASSERT_LENGTH(x     ,"eps","x"     ); }
  if (y     !=NULL) { ASSERT_LENGTH(y     ,"eps","y"     ); }
  if (ang   !=NULL) { ASSERT_ANGLE (ang   ,"eps"         ); }
  if (width !=NULL) { ASSERT_LENGTH(width ,"eps","width" ); }
  if (height!=NULL) { ASSERT_LENGTH(height,"eps","height"); }

  DictLookup(command, "filename", NULL, (void *)&fname);
  text = (char *)malloc(strlen(fname)+1);
  if (text == NULL) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  strcpy(text, fname);

  if (canvas_itemlist_add(command,CANVAS_IMAGE,&ptr,&id,0)) { ppl_error(ERR_MEMORY,"Out of memory."); free(text); return 1; }

  if (x     !=NULL) { ptr->xpos     = x     ->real; }                    else { ptr->xpos     = settings_graph_current.OriginX.real; }
  if (y     !=NULL) { ptr->ypos     = y     ->real; }                    else { ptr->ypos     = settings_graph_current.OriginY.real; }
  if (ang   !=NULL) { ptr->rotation = ang   ->real; }                    else { ptr->rotation = 0.0;                                 }
  if (width !=NULL) { ptr->xpos2    = width ->real; ptr->xpos2set = 1; } else { ptr->xpos2    = 0.0; ptr->xpos2set = 0; }
  if (height!=NULL) { ptr->ypos2    = height->real; ptr->ypos2set = 1; } else { ptr->ypos2    = 0.0; ptr->ypos2set = 0; }
  if (smooth!=NULL) { ptr->smooth   = 1; }                               else { ptr->smooth   = 0; }
  if (NoTransparency != NULL) { ptr->NoTransparency = 1; } else { ptr->NoTransparency = 0; }
  if (TransColR      != NULL)
   {
    ptr->CustomTransparency = 1;
    ptr->TransColR = (*TransColR <= 0) ? 0 : ((*TransColR >= 255) ? 255 : *TransColR); // Make sure that colour component is in the range 0-255
    ptr->TransColG = (*TransColG <= 0) ? 0 : ((*TransColG >= 255) ? 255 : *TransColG); // Make sure that colour component is in the range 0-255
    ptr->TransColB = (*TransColB <= 0) ? 0 : ((*TransColB >= 255) ? 255 : *TransColB); // Make sure that colour component is in the range 0-255
   } else { ptr->CustomTransparency = 0; }
  ptr->text      = text;

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Bitmap image has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the plot and replot commands.
int directive_plot(Dict *command, int interactive, int replot)
 {
  canvas_item   *ptr;
  int            id;
  unsigned char *unsuccessful_ops;

  if (!replot)
   {
    if (canvas_itemlist_add(command,CANVAS_PLOT,&ptr,&id,1)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
   }

  // Redisplay the canvas as required
  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Plot has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

