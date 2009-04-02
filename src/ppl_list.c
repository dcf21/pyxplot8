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

#include "ppl_list.h"
#include "ppl_memory.h"

List *ListInit()
 {
  List *out;
  out = ppl_malloc(sizeof(List));
  out->first = NULL;
  out->last  = NULL;
  out->length= 0;
  return out;
 }

void ListKill(List *in)
 {
  ListItem *ptr, *ptrnext;
  if (in==NULL) return;
  ptr = in->first;
  ppl_free(in);
  while (ptr != NULL)
   {
    if (ptr->data != NULL) ppl_free(ptr->data);
    ptrnext = ptr->next;
    ppl_free(ptr);
    ptr = ptrnext;
   }
  return;
 }

int ListLen(List *in)
 {
  return in->length;
 }

void ListAppend(List *in, void *item)
 {
  ListItem *ptrnew;
  ptrnew = ppl_malloc(sizeof(ListItem));
  ptrnew->prev = in->last;
  ptrnew->next = NULL;
  ptrnew->data = item;
  if (in->first == NULL) in->first = ptrnew;
  in->last = ptrnew;
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
      if (item!=NULL) ppl_free(item);
      if (ptr->next != NULL) // We are not the last item in the list
       {
        ptrnext   = ptr->next;
        ptr->data = ptrnext->data;
        ptr->next = ptrnext->next;
        ppl_free(ptrnext);
        if (in->last == ptrnext) in->last = ptr;
        else ptr->next->prev = ptr;
       }
      else if (ptr->prev != NULL) // We are the last item in the list, but not the first item
       {
        ptrnext   = ptr->prev;
        ptr->data = ptrnext->data;
        ptr->prev = ptrnext->prev;
        ppl_free(ptrnext);
        if (in->first == ptrnext) in->first = ptr;
        else ptr->prev->next = ptr;
       }
      else // We are the only item in the list
       {
        ppl_free(ptr);
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
    ppl_free(in->last->next);
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
  if (in==NULL) { *item = NULL; return NULL; }
  in = in->next;
  if (in==NULL) { *item = NULL; return NULL; }
  *item = in->data;
  return in;
 }
