// ppl_list.c
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

// Functions for manupulating linked lists

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ppl_list.h"
#include "ppl_dict.h"
#include "ppl_memory.h"

List *ListInit()
 {
  List *out;
  out = ppl_malloc(sizeof(List));
  out->first  = NULL;
  out->last   = NULL;
  out->length = 0;
  out->memory_context = ppl_GetMemContext();
  return out;
 }

int ListLen(List *in)
 {
  return in->length;
 }

void ListAppendPtr(List *in, void *item)
 {
  ListItem *ptrnew;
  ptrnew = ppl_malloc(sizeof(ListItem));
  ptrnew->prev = in->last;
  ptrnew->next = NULL;
  ptrnew->data = item;
  ptrnew->DataType = DATATYPE_VOID;
  if (in->first == NULL) in->first = ptrnew;
  if (in->last  != NULL) in->last->next = ptrnew;
  in->last = ptrnew;
  in->length++;
  return;
 }

void ListAppendInt(List *in, int item)
 {
  int *ptr = (int *)ppl_malloc_incontext(sizeof(int), in->memory_context);
  *ptr = item;
  ListAppendPtr(in, (void *)ptr);
  in->last->DataType = DATATYPE_INT;
  return;
 }

void ListAppendFloat(List *in, double item)
 {
  double *ptr = (double *)ppl_malloc_incontext(sizeof(double), in->memory_context);
  *ptr = item;
  ListAppendPtr(in, (void *)ptr);
  in->last->DataType = DATATYPE_FLOAT;
  return;
 }

void ListAppendString(List *in, char *item)
 {
  int length = strlen(item);
  char  *ptr = (char *)ppl_malloc_incontext((length+1)*sizeof(char), in->memory_context);
  strcpy(ptr, item);
  ListAppendPtr(in, (void *)ptr);
  in->last->DataType = DATATYPE_STRING;
  return;
 }

void ListAppendList(List *in, List *item)
 {
  ListAppendPtr(in, (void *)item);
  in->last->DataType = DATATYPE_LIST;
  return;
 }

void ListAppendDict(List *in, Dict *item)
 {
  ListAppendPtr(in, (void *)item);
  in->last->DataType = DATATYPE_DICT;
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

