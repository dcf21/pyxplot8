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
  if (ptr->commandline != NULL) free(ptr->commandline);
  if (ptr->text        != NULL) free(ptr->text);
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(ptr->XAxes[i]), NULL);
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(ptr->YAxes[i]), NULL);
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(ptr->ZAxes[i]), NULL);
  // !!! Delete plot descriptor !!!
  free(ptr);
  return;
 }

// Add a new multiplot canvas item to the list above
static int canvas_itemlist_add(Dict *command, int type, char *line, canvas_item **output, int *id)
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
  while ((*insertpoint != NULL) && ((EditNo==NULL) ? ((*insertpoint)->id <= PrevId+1) : ((*insertpoint)->id <= *EditNo))) { PrevId=(*insertpoint)->id; insertpoint = &((*insertpoint)->next); }
  if ((EditNo != NULL) && (*insertpoint != NULL) && ((*insertpoint)->id = *EditNo))
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

  // Store the commandline that the user typed
  ptr->commandline = (char *)malloc(strlen(line)+1);
  if (ptr->commandline==NULL) { free(ptr); *insertpoint = next; return 1; }
  strcpy(ptr->commandline, line);

  // Copy the user's current settings
  ptr->settings = settings_graph_current;
  for (i=0; i<MAX_AXES; i++) CopyAxis(&(ptr->XAxes[i]), &(XAxes[i]));
  for (i=0; i<MAX_AXES; i++) CopyAxis(&(ptr->YAxes[i]), &(YAxes[i]));
  for (i=0; i<MAX_AXES; i++) CopyAxis(&(ptr->ZAxes[i]), &(ZAxes[i]));

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

// Implementation of the list command.
int directive_list()
 {
  canvas_item *ptr;

  ppl_report("# ID   Command");
  if (canvas_items == NULL) return 0;
  ptr = canvas_items->first;
  while (ptr != NULL)
   {
    sprintf(temp_err_string, "%5d  %s%s", ptr->id, (ptr->deleted) ? "[deleted] " : "", ptr->commandline);
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
  List         *DelList;
  ListIterator *ListIter;
  int          *id;

  if (canvas_items==NULL) { sprintf(temp_err_string, "There are currently no items on the multiplot canvas."); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  DictLookup(command, "deleteno,", NULL, (void *)&DelList);
  ListIter = ListIterateInit(DelList);
  while (ListIter != NULL)
   {
    DictLookup(ListIter->data, "number", NULL, (void *)&id);
    canvas_delete(*id);
    ListIter = ListIterate(ListIter, NULL);
   }
  return 0;
 }

// Implementation of the undelete command.
int directive_undelete(Dict *command)
 {
  List         *UndelList;
  ListIterator *ListIter;
  int          *id;
  canvas_item  *ptr;

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
  int          *moveno, i;
  value        *x, *y, *ang;
  unsigned char rotatable;
  canvas_item  *ptr;

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
  rotatable = (ptr->type!=CANVAS_ARROW);
  if ((ang != NULL) && (!rotatable)) { sprintf(temp_err_string, "It is not possible to rotate the specified multiplot item."); ppl_warning(ERR_GENERAL, temp_err_string); }
  ptr->xpos = x->real;
  ptr->ypos = y->real;
  if ((ang != NULL) && (rotatable)) ptr->rotation = ang->real;
  return 0;
 }

// Implementation of the arrow command.
int directive_arrow(Dict *command, char *line, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x1, *x2, *y1, *y2;
  unsigned char *unsuccessful_ops;

  DictLookup(command, "x1", NULL, (void *)&x1); DictLookup(command, "y1", NULL, (void *)&y1);
  DictLookup(command, "x2", NULL, (void *)&x2); DictLookup(command, "y2", NULL, (void *)&y2);

  ASSERT_LENGTH(x1,"arrow","x1"); ASSERT_LENGTH(y1,"arrow","y1");
  ASSERT_LENGTH(x2,"arrow","x2"); ASSERT_LENGTH(y2,"arrow","y2");

  if (canvas_itemlist_add(command,CANVAS_ARROW,line,&ptr,&id)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  ptr->xpos  = x1->real;
  ptr->ypos  = y1->real;
  ptr->xpos2 = x2->real - x1->real;
  ptr->ypos2 = y2->real - y1->real;

  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Arrow has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the eps command.
int directive_eps(Dict *command, char *line, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x, *y, *ang;
  unsigned char *unsuccessful_ops;
  char          *text, *fname;

  DictLookup(command, "x"     , NULL, (void *)&x  );
  DictLookup(command, "y"     , NULL, (void *)&y  );
  DictLookup(command, "rotate", NULL, (void *)&ang);

  if (x  !=NULL) { ASSERT_LENGTH(x  ,"eps","x"); }
  if (y  !=NULL) { ASSERT_LENGTH(y  ,"eps","y"); }
  if (ang!=NULL) { ASSERT_ANGLE (ang,"eps"    ); }

  DictLookup(command, "filename", NULL, (void *)&fname);
  text = (char *)malloc(strlen(fname)+1);
  if (text == NULL) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  strcpy(text, fname);

  if (canvas_itemlist_add(command,CANVAS_EPS,line,&ptr,&id)) { ppl_error(ERR_MEMORY,"Out of memory."); free(text); return 1; }

  if (x  !=NULL) { ptr->xpos     = x  ->real; } else { ptr->xpos      = settings_graph_current.OriginX.real; }
  if (y  !=NULL) { ptr->ypos     = y  ->real; } else { ptr->ypos      = settings_graph_current.OriginY.real; }
  if (ang!=NULL) { ptr->rotation = ang->real; } else { ptr->rotation  = 0.0;                                 }
  ptr->text = text;

  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("EPS image has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the text command.
int directive_text(Dict *command, char *line, int interactive)
 {
  canvas_item   *ptr;
  int            i, id;
  value         *x, *y, *ang;
  unsigned char *unsuccessful_ops;
  char          *text, *fname;

  DictLookup(command, "x"     , NULL, (void *)&x  );
  DictLookup(command, "y"     , NULL, (void *)&y  );
  DictLookup(command, "rotate", NULL, (void *)&ang);

  if (x  !=NULL) { ASSERT_LENGTH(x  ,"eps","x"); }
  if (y  !=NULL) { ASSERT_LENGTH(y  ,"eps","y"); }
  if (ang!=NULL) { ASSERT_ANGLE (ang,"eps"    ); }

  DictLookup(command, "string", NULL, (void *)&fname);
  text = (char *)malloc(strlen(fname)+1);
  if (text == NULL) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  strcpy(text, fname);

  if (canvas_itemlist_add(command,CANVAS_TEXT,line,&ptr, &id)) { ppl_error(ERR_MEMORY,"Out of memory."); free(text); return 1; }

  if (x  !=NULL) { ptr->xpos     = x  ->real; } else { ptr->xpos      = settings_graph_current.OriginX.real; }
  if (y  !=NULL) { ptr->ypos     = y  ->real; } else { ptr->ypos      = settings_graph_current.OriginY.real; }
  if (ang!=NULL) { ptr->rotation = ang->real; } else { ptr->rotation  = 0.0;                                 }
  ptr->text = text;

  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Text item has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

// Implementation of the jpeg command.
int directive_jpeg(Dict *command, char *line, int interactive)
 {
  canvas_item *ptr;
  int            i, id;
  value         *x, *y, *ang;
  unsigned char *unsuccessful_ops;
  char          *text, *fname;

  DictLookup(command, "x"     , NULL, (void *)&x  );
  DictLookup(command, "y"     , NULL, (void *)&y  );
  DictLookup(command, "rotate", NULL, (void *)&ang);

  if (x  !=NULL) { ASSERT_LENGTH(x  ,"eps","x"); }
  if (y  !=NULL) { ASSERT_LENGTH(y  ,"eps","y"); }
  if (ang!=NULL) { ASSERT_ANGLE (ang,"eps"    ); }

  DictLookup(command, "filename", NULL, (void *)&fname);
  text = (char *)malloc(strlen(fname)+1);
  if (text == NULL) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
  strcpy(text, fname);

  if (canvas_itemlist_add(command,CANVAS_JPEG,line,&ptr,&id)) { ppl_error(ERR_MEMORY,"Out of memory."); free(text); return 1; }

  if (x  !=NULL) { ptr->xpos     = x  ->real; } else { ptr->xpos      = settings_graph_current.OriginX.real; }
  if (y  !=NULL) { ptr->ypos     = y  ->real; } else { ptr->ypos      = settings_graph_current.OriginY.real; }
  if (ang!=NULL) { ptr->rotation = ang->real; } else { ptr->rotation  = 0.0;                                 }
  ptr->text = text;

  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("JPEG image has been removed from multiplot, because it generated an error.")); return 1; }
   }
  ppl_report(DictPrint(command, temp_err_string, LSTR_LENGTH));
  return 0;
 }

// Implementation of the plot and replot commands.
int directive_plot(Dict *command, char *line, int interactive, int replot)
 {
  canvas_item   *ptr;
  int            id;
  unsigned char *unsuccessful_ops;

  if (!replot)
   {
    if (canvas_itemlist_add(command,CANVAS_PLOT,line,&ptr,&id)) { ppl_error(ERR_MEMORY,"Out of memory."); return 1; }
   }

  if (settings_term_current.display == SW_ONOFF_ON)
   {
    unsuccessful_ops = (unsigned char *)lt_malloc(MULTIPLOT_MAXINDEX);
    canvas_draw(unsuccessful_ops);
    if (unsuccessful_ops[id]) { canvas_delete(id); ppl_error(ERR_GENERAL, ("Plot has been removed from multiplot, because it generated an error.")); return 1; }
   }
  return 0;
 }

