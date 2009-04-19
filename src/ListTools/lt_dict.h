// lt_dict.h
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

// Data structures for linked lists

#ifndef _LT_DICT_H
#define _LT_DICT_H 1

typedef struct DictItemS
 {
  char             *key;
  void             *data;
  int               UserData;
  int               DataType;
  int               DataSize;
  unsigned char     MallocedByUs;
  unsigned char     copyable;
  struct DictItemS *next;
  struct DictItemS *prev;
 } DictItem;


typedef struct DictS
 {
  struct DictItemS *first;
  struct DictItemS *last;
  int               length;
  int               memory_context;
 } Dict;

typedef DictItem DictIterator;

#include "lt_list.h"

// Functions defined in lt_dict.c
Dict *DictInit         ();
Dict *DictCopy         (Dict *in, int deep);
int   DictLen          (Dict *in);
void  DictAppendPtr    (Dict *in, char *key, int UserData, void *item, int size, int copyable, int DataType);
void  DictAppendPtrCpy (Dict *in, char *key, int UserData, void *item, int size, int DataType);
void  DictAppendInt    (Dict *in, char *key, int UserData, int   item);
void  DictAppendFloat  (Dict *in, char *key, int UserData, double item);
void  DictAppendString (Dict *in, char *key, int UserData, char *item);
void  DictAppendList   (Dict *in, char *key, int UserData, List *item);
void  DictAppendDict   (Dict *in, char *key, int UserData, Dict *item);
void  DictLookup       (Dict *in, char *key, int *UserDataOut, int *DataTypeOut, void **ptrout);
int   DictContains     (Dict *in, char *key);
int   DictRemoveKey    (Dict *in, char *key);
int   DictRemovePtr    (Dict *in, void *item);
void  DictRemovePtrAll (Dict *in, void *item);
DictIterator *DictIterateInit(Dict *in);
DictIterator *DictIterate(DictIterator *in, int *UserDataOut, int *DataTypeOut, void **ptrout);
char *DictPrint        (Dict *in, char *out, int size);

// Private
void _DictRemoveEngine (Dict *in, DictItem *ptr);

#endif

