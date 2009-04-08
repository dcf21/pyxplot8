// lt_dict.c
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

Dict *DictInit()
 {
  Dict *out;
  out = lt_malloc(sizeof(Dict));
  out->first  = NULL;
  out->last   = NULL;
  out->length = 0;
  out->memory_context = lt_GetMemContext();
  return out;
 }

int DictLen(Dict *in)
 {
  return in->length;
 }

void DictAppendPtr(Dict *in, char *key, int UserData, void *item)
 {
  DictItem *ptrnew;
  DictRemoveKey(in, key);
  ptrnew           = lt_malloc(sizeof(DictItem));
  ptrnew->prev     = in->last;
  ptrnew->next     = NULL;
  ptrnew->key      = (char *)lt_malloc_incontext((strlen(key)+1)*sizeof(char), in->memory_context); strcpy(ptrnew->key, key);
  ptrnew->UserData = UserData;
  ptrnew->data     = item;
  ptrnew->DataType = DATATYPE_VOID;
  if (in->first == NULL) in->first      = ptrnew;
  if (in->last  != NULL) in->last->next = ptrnew;
  in->last = ptrnew;
  in->length++;
  return;
 }

void DictAppendInt(Dict *in, char *key, int UserData, int item)
 {
  int *ptr = (int *)lt_malloc_incontext(sizeof(int), in->memory_context);
  *ptr = item;
  DictAppendPtr(in, key, UserData, (void *)ptr);
  in->last->DataType = DATATYPE_INT;
  return;
 }

void DictAppendFloat(Dict *in, char *key, int UserData, double item)
 {
  double *ptr = (double *)lt_malloc_incontext(sizeof(double), in->memory_context);
  *ptr = item;
  DictAppendPtr(in, key, UserData, (void *)ptr);
  in->last->DataType = DATATYPE_FLOAT;
  return;
 }

void DictAppendString(Dict *in, char *key, int UserData, char *item)
 {
  int length = strlen(item);
  char  *ptr = (char *)lt_malloc_incontext((length+1)*sizeof(char), in->memory_context);
  strcpy(ptr, item);
  DictAppendPtr(in, key, UserData, (void *)ptr);
  in->last->DataType = DATATYPE_STRING;
  return;
 }

void DictAppendList(Dict *in, char *key, int UserData, List *item)
 {
  DictAppendPtr(in, key, UserData, (void *)item);
  in->last->DataType = DATATYPE_LIST;
  return;
 }

void DictAppendDict(Dict *in, char *key, int UserData, Dict *item)
 {
  DictAppendPtr(in, key, UserData, (void *)item);
  in->last->DataType = DATATYPE_DICT;
  return;
 }

void DictLookup(Dict *in, char *key, int *UserDataOut, int *DataTypeOut, void **ptrout)
 {
  DictItem *ptr;
  if (in==NULL) { *ptrout=NULL; return; }
  ptr = in->first;
  while (ptr != NULL)
   {
    if (strcmp(ptr->key, key)==0)
     {
      if (UserDataOut != NULL) *UserDataOut = ptr->UserData;
      if (DataTypeOut != NULL) *DataTypeOut = ptr->DataType ;
      if (ptrout      != NULL) *ptrout = ptr->data;
      return;
     }
    ptr = ptr->next;
   }
  *ptrout = NULL;
  return;
 }

int DictContains(Dict *in, char *key)
 {
  DictItem *ptr;
  if (in==NULL) return 0;
  ptr = in->first;
  while (ptr != NULL)
   {
    if (strcmp(ptr->key, key)==0) return 1;
    ptr = ptr->next;
   }
  return 0;
 }

int  DictRemoveKey(Dict *in, char *key)
 {
  DictItem *ptr;
  if (in==NULL) return -1;
  ptr = in->first;
  while (ptr != NULL)
   {
    if (strcmp(ptr->key, key)==0)
     {
      _DictRemoveEngine(in, ptr);
      return 0;
     }
    ptr = ptr->next;
   }
  return -1;
 }

int DictRemovePtr(Dict *in, void *item)
 {
  DictItem *ptr;
  if (in==NULL) return -1;
  ptr = in->first;
  while (ptr != NULL)
   {
    if (ptr->data == item)
     {
      _DictRemoveEngine(in, ptr);
      return 0;
     }
    ptr = ptr->next;
   }
  return -1;
 }

void _DictRemoveEngine(Dict *in, DictItem *ptr)
 {
  DictItem *ptrnext;

  if (in ==NULL) return;
  if (ptr==NULL) return;

  if (ptr->next != NULL) // We are not the last item in the list
   {
    ptrnext       = ptr->next;
    ptr->key      = ptrnext->key;
    ptr->DataType = ptrnext->DataType;
    ptr->UserData = ptrnext->UserData;
    ptr->data     = ptrnext->data;
    ptr->next     = ptrnext->next;
    if (in->last == ptrnext) in->last = ptr;
    else ptr->next->prev = ptr;
   }
  else if (ptr->prev != NULL) // We are the last item in the list, but not the first item
   {
    ptrnext       = ptr->prev;
    ptr->key      = ptrnext->key;
    ptr->DataType = ptrnext->DataType;
    ptr->UserData = ptrnext->UserData;
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
  return;
 }

void DictRemovePtrAll(Dict *in, void *item)
 {
  while ( DictRemovePtr(in,item)!=-1 );
  return;
 }

DictIterator *DictIterateInit(Dict *in)
 {
  return in->first;
 }

DictIterator *DictIterate(DictIterator *in, int *UserDataOut, int *DataTypeOut, void **ptrout)
 {
  if (in==NULL) { if (ptrout!=NULL) *ptrout = NULL; return NULL; }
  if (UserDataOut != NULL) *UserDataOut = in->UserData;
  if (DataTypeOut != NULL) *DataTypeOut = in->DataType;
  if (ptrout      != NULL) *ptrout = in->data;
  in = in->next;
  return in;
 }

char *DictPrint(Dict *in, char *out, int size)
 {
  DictIterator *iter;
  int pos,first;
  iter = DictIterateInit(in);
  pos=0; first=1;
  strcpy(out+pos, "{"); pos += strlen(out+pos);
  while (iter != NULL)
   {
    if (pos > (size-30)) { strcpy(out+pos, ", ... }"); return out; } // Truncate string as we're getting close to the end of the buffer
    if (first!=1) strcpy(out+(pos++), ",");
    if      (iter->DataType == DATATYPE_VOID  ) { sprintf(out+pos, "'%s':[%d,void]", iter->key, iter->UserData                         ); }
    else if (iter->DataType == DATATYPE_INT   ) { sprintf(out+pos, "'%s':[%d,%d]"  , iter->key, iter->UserData, *((int    *)iter->data)); }
    else if (iter->DataType == DATATYPE_FLOAT ) { sprintf(out+pos, "'%s':[%d,%e]"  , iter->key, iter->UserData, *((double *)iter->data)); }
    else if (iter->DataType == DATATYPE_STRING) { sprintf(out+pos, "'%s':[%d,'%s']", iter->key, iter->UserData,  ((char   *)iter->data)); }
    else if (iter->DataType == DATATYPE_LIST  )
     {
      sprintf(out+pos, "%s:[%d,", iter->key, iter->UserData);
      pos += strlen(out+pos);
      ListPrint( ((List *)iter->data), out+pos, size-pos);
      pos += strlen(out+pos); strcpy(out+pos, "]");
     }
    else if (iter->DataType == DATATYPE_DICT  )
      {
       sprintf(out+pos, "%s:[%d,", iter->key, iter->UserData);
       pos += strlen(out+pos);
       DictPrint( ((Dict *)iter->data), out+pos, size-pos);
       pos += strlen(out+pos); strcpy(out+pos, "]");
      }
    pos += strlen(out+pos);
    first=0;
    iter = DictIterate(iter, NULL, NULL, NULL);
   }
  strcpy(out+pos, "}"); pos += strlen(out+pos);
  return out;
 }

