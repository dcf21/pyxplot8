// lt_list.c
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

// Functions for manupulating linked lists

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lt_dict.h"
#include "lt_list.h"
#include "lt_memory.h"

List *ListInit()
 {
  List *out;
  out = (List *)lt_malloc(sizeof(List));
  out->first  = NULL;
  out->last   = NULL;
  out->length = 0;
  out->memory_context = lt_GetMemContext();
  return out;
 }

List *ListCopy(List *in, int deep)
 {
  ListItem *item, *outitem;
  List *out;
  out = ListInit();
  item = in->first;
  while (item != NULL)
   {
    outitem           = (ListItem *)lt_malloc(sizeof(ListItem));
    outitem->prev     = out->last;
    outitem->next     = NULL;
    outitem->DataSize = item->DataSize;
    if (item->copyable != 0)
     {
      outitem->data     = (void *)lt_malloc_incontext(outitem->DataSize, out->memory_context);
      memcpy(outitem->data, item->data, outitem->DataSize);
     } else {
      if      ((deep!=0)&&(item->DataType == DATATYPE_LIST)) outitem->data = ListCopy((List *)item->data,1);
      else if ((deep!=0)&&(item->DataType == DATATYPE_DICT)) outitem->data = DictCopy((Dict *)item->data,1);
      else                                                   outitem->data =                  item->data   ;
     }
    outitem->copyable = item->copyable;
    outitem->DataType = item->DataType;
    if (out->first == NULL) out->first      = outitem;
    if (out->last  != NULL) out->last->next = outitem;
    out->last = outitem;
    out->length++;
    item = item->next;
   }
  return out;
 }

int ListLen(List *in)
 {
  return in->length;
 }

void ListAppendPtr(List *in, void *item, int size, int copyable, int DataType)
 {
  ListItem *ptrnew;
  ptrnew           = (ListItem *)lt_malloc_incontext(sizeof(ListItem), in->memory_context);
  ptrnew->prev     = in->last;
  ptrnew->next     = NULL;
  ptrnew->data     = item;
  ptrnew->DataSize = size;
  ptrnew->DataType = DataType;
  ptrnew->copyable = copyable;
  if (in->first == NULL) in->first = ptrnew;
  if (in->last  != NULL) in->last->next = ptrnew;
  in->last = ptrnew;
  in->length++;
  return;
 }

void ListAppendPtrCpy(List *in, void *item, int size, int DataType)
 {
  ListItem *ptrnew;
  ptrnew           = (ListItem *)lt_malloc_incontext(sizeof(ListItem), in->memory_context);
  ptrnew->prev     = in->last;
  ptrnew->next     = NULL;
  ptrnew->data     = (void *)lt_malloc_incontext(size, in->memory_context);
  memcpy(ptrnew->data, item, size);
  ptrnew->DataSize = size;
  ptrnew->DataType = DataType;
  ptrnew->copyable = 1;
  if (in->first == NULL) in->first = ptrnew;
  if (in->last  != NULL) in->last->next = ptrnew;
  in->last = ptrnew;
  in->length++;
  return;
 }

void ListAppendInt(List *in, int item)
 {
  ListAppendPtrCpy(in, (void *)&item, sizeof(int), DATATYPE_INT);
  return;
 }

void ListAppendFloat(List *in, double item)
 {
  ListAppendPtrCpy(in, (void *)&item, sizeof(double), DATATYPE_FLOAT);
  return;
 }

void ListAppendString(List *in, char *item)
 {
  ListAppendPtrCpy(in, (void *)item, (strlen(item)+1)*sizeof(char), DATATYPE_STRING);
  return;
 }

void ListAppendList(List *in, List *item)
 {
  ListAppendPtr(in, (void *)item, sizeof(List), 0, DATATYPE_LIST);
  return;
 }

void ListAppendDict(List *in, Dict *item)
 {
  ListAppendPtr(in, (void *)item, sizeof(Dict), 0, DATATYPE_DICT);
  return;
 }

int ListRemovePtr(List *in, void *item)
 {
  ListItem *ptr, *ptrnext;
  if (in==NULL) return -1;
  ptr = in->first;
  while (ptr != NULL)
   {
    if (ptr->data == item)
     {
      if (ptr->next != NULL) // We are not the last item in the list
       {
        ptrnext       = ptr->next;
        ptr->DataType = ptrnext->DataType;
        ptr->data     = ptrnext->data;
        ptr->next     = ptrnext->next;
        if (in->last == ptrnext) in->last = ptr;
        else ptr->next->prev = ptr;
       }
      else if (ptr->prev != NULL) // We are the last item in the list, but not the first item
       {
        ptrnext       = ptr->prev;
        ptr->DataType = ptrnext->DataType;
        ptr->data     = ptrnext->data;
        ptr->prev     = ptrnext->prev;
        if (in->first == ptrnext) in->first = ptr;
        else ptr->prev->next = ptr;
       }
      else // We are the only item in the list
       {
        in->first = NULL;
        in->last  = NULL;
       }
      in->length--;
      return 0;
     }
    ptr = ptr->next;
   }
  return -1;
 }

void ListRemovePtrAll(List *in, void *item)
 {
  while ( ListRemovePtr(in,item)!=-1 );
  return;
 }

void *ListGetItem(List *in, int N)
 {
  ListItem *ptr;
  int   i;
  if (in==NULL) return NULL;
  ptr = in->first;
  for (i=0; ((i<N) && (ptr!=NULL)); i++, ptr=ptr->next);
  if (ptr==NULL) return NULL;
  return ptr->data;
 }

void *ListPop(List *in)
 {
  void *out;
  if (in->last == NULL) return NULL;
  out = in->last->data;
  if (in->first == in->last)
   {
    in->first = in->last = NULL;
   } else {
    in->last = in->last->prev;
    in->last->next = NULL;
   }
  in->length--;
  return out;
 }

void *ListLast(List *in)
 {
  if (in->last == NULL) return NULL;
  return in->last->data;
 }

ListIterator *ListIterateInit(List *in)
 {
  return in->first;
 }

ListIterator *ListIterate(ListIterator *in, void **item)
 {
  if (in==NULL) { if (item!=NULL) *item = NULL; return NULL; }
  if (item!=NULL) *item = in->data;
  in = in->next;
  return in;
 }

char *ListPrint(List *in, char *out, int size)
 {
  ListIterator *iter;
  int pos,first;
  iter = ListIterateInit(in);
  pos=0; first=1;
  strcpy(out+pos, "["); pos += strlen(out+pos);
  while (iter != NULL)
   {
    if (pos > (size-30)) { strcpy(out+pos, ", ... ]"); return out; }// Truncate string as we're getting close to the end of the buffer
    if (first!=1) strcpy(out+(pos++), ",");
    if      (iter->DataType == DATATYPE_VOID  ) { strcpy (out+pos, "void"                         ); }
    else if (iter->DataType == DATATYPE_INT   ) { sprintf(out+pos, "%d"  , *((int    *)iter->data)); }
    else if (iter->DataType == DATATYPE_FLOAT ) { sprintf(out+pos, "%e"  , *((double *)iter->data)); }
    else if (iter->DataType == DATATYPE_STRING) { sprintf(out+pos, "'%s'",  ((char   *)iter->data)); }
    else if (iter->DataType == DATATYPE_LIST  ) { ListPrint( ((List *)iter->data), out+pos, size-pos); }
    else if (iter->DataType == DATATYPE_DICT  ) { DictPrint( ((Dict *)iter->data), out+pos, size-pos); } 
    pos += strlen(out+pos);
    first=0;
    iter = ListIterate(iter, NULL);
   }
  strcpy(out+pos, "]"); pos += strlen(out+pos);
  return out;
 }

