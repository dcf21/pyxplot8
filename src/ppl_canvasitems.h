// ppl_canvasitems.h
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

#ifndef _PPL_CANVASITEMS_H
#define _PPL_CANVASITEMS_H 1

#include "ListTools/lt_dict.h"

#include "ppl_settings.h"

#define CANVAS_ARROW 22001
#define CANVAS_BOX   22002
#define CANVAS_CIRC  22003
#define CANVAS_ELLPS 22004
#define CANVAS_EPS   22005
#define CANVAS_IMAGE 22006
#define CANVAS_PLOT  22007
#define CANVAS_TEXT  22008

typedef struct plot_descriptor {
 unsigned char           function;
 with_words              with_data;
 char                   *text;
 struct plot_descriptor *next;
} plot_descriptor;

typedef struct canvas_item {
 int                 id, type, ArrowType, TransColR, TransColG, TransColB;
 double              xpos, ypos, xpos2, ypos2, rotation;
 double              x1,y1,x2,y2,xc,yc,xf,yf,a,b,ecc,slr; // Parameters which can be used to define ellipses
 unsigned char       x1set, xcset, xfset, aset, bset, eccset, slrset;
 char               *text;
 unsigned char       deleted, xpos2set, ypos2set, clip, calcbbox, smooth, NoTransparency, CustomTransparency;
 with_words          with_data;
 plot_descriptor    *plot_items;
 settings_graph      settings;
 settings_axis      *XAxes, *YAxes, *ZAxes;
 arrow_object       *arrow_list;
 label_object       *label_list;
 struct canvas_item *next;
} canvas_item;

typedef struct canvas_itemlist {
 canvas_item  *first;
} canvas_itemlist;

#ifndef _PPL_CANVASITEMS_C
extern canvas_itemlist *canvas_items;
#endif

int directive_clear   ();
int directive_list    ();
int directive_delete  (Dict *command);
int directive_undelete(Dict *command);
int directive_move    (Dict *command);
int directive_arrow   (Dict *command, int interactive);
int directive_box     (Dict *command, int interactive);
int directive_circle  (Dict *command, int interactive);
int directive_ellipse (Dict *command, int interactive);
int directive_eps     (Dict *command, int interactive);
int directive_text    (Dict *command, int interactive);
int directive_image   (Dict *command, int interactive);
int directive_plot    (Dict *command, int interactive, int replot);

#endif

