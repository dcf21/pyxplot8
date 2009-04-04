// ppl_list.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-8 Dominic Ford <coders@pyxplot.org.uk>
//               2008   Ross Church
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

// Data structures for linked lists

#ifndef _PPL_LIST
#define _PPL_LIST 1

#define DATATYPE_VOID   31000
#define DATATYPE_INT    31001
#define DATATYPE_FLOAT  31002
#define DATATYPE_STRING 31003
#define DATATYPE_LIST   31004
#define DATATYPE_DICT   31005

typedef struct ListItemS
 {
  void             *data;
  int               DataType;
  struct ListItemS *next;
  struct ListItemS *prev;
 } ListItem;


typedef struct ListS
 {
  struct ListItemS *first;
  struct ListItemS *last;
  int               length;
  int               memory_context;
 } List;

typedef ListItem ListIterator;

#include "ppl_dict.h"

// Functions defined in ppl_list.c
List *ListInit         ();
int   ListLen          (List *in);
void  ListAppendPtr    (List *in, void *item);
void  ListAppendInt    (List *in, int   item);
void  ListAppendFloat  (List *in, double item);
void  ListAppendString (List *in, char *item);
void  ListAppendList   (List *in, List *item);
void  ListAppendDict   (List *in, Dict *item);
int   ListRemovePtr    (List *in, void *item);
void  ListRemovePtrAll (List *in, void *item);
void *ListPop          (List *in);
void *ListLast         (List *in);
ListIterator *ListIterateInit(List *in);
ListIterator *ListIterate(ListIterator *in, void **item);
char *ListPrint        (List *in, char *out, int size);

#endif
