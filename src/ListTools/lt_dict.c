// lt_dict.c
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

// Functions for manupulating linked lists

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "lt_dict.h"
#include "lt_list.h"
#include "lt_memory.h"

#include "StringTools/asciidouble.h"

#include "ppl_units.h"

Dict *DictInit(int HashSize)
 {
  Dict *out;
  out = (Dict *)lt_malloc(sizeof(Dict));
  if (out==NULL) return NULL;
  out->first     = NULL;
  out->last      = NULL;
  out->length    = 0;
  out->HashSize  = HashSize;
  out->HashTable = (DictItem **)lt_malloc(HashSize * sizeof(DictItem *));
  memset(out->HashTable, 0, HashSize * sizeof(DictItem *));
  if (out->HashTable==NULL) return NULL;
  out->memory_context = lt_GetMemContext();
  return out;
 }

static int DictHash(const char *str, int HashSize)
 {
  unsigned int hash = 5381;
  int c;
  while ((c = *str++)) hash = ((hash << 5) + hash) + c;
  return hash % HashSize;
 }


Dict *DictCopy(Dict *in, int deep)
 {
  int hash;
  DictItem *item, *outitem;
  Dict     *out;
  out  = DictInit(in->HashSize);
  if (out==NULL) return NULL;
  item = in->first;
  while (item != NULL)
   {
    outitem           = (DictItem *)lt_malloc_incontext(sizeof(DictItem), out->memory_context);
    if (outitem==NULL) return NULL;
    outitem->prev     = out->last;
    outitem->next     = NULL;
    outitem->key      = (char *)lt_malloc_incontext((strlen(item->key)+1)*sizeof(char), out->memory_context);
    if (outitem->key==NULL) return NULL;
    strcpy(outitem->key, item->key);
    outitem->DataSize = item->DataSize;
    if (item->copyable != 0)
     {
      outitem->data     = (void *)lt_malloc_incontext(outitem->DataSize, out->memory_context);
      if (outitem->data==NULL) return NULL;
      memcpy(outitem->data, item->data, outitem->DataSize);
      outitem->MallocedByUs = 1;
     } else {
      if      ((deep!=0)&&(item->DataType == DATATYPE_LIST)) outitem->data = ListCopy((List *)item->data,1);
      else if ((deep!=0)&&(item->DataType == DATATYPE_DICT)) outitem->data = DictCopy((Dict *)item->data,1);
      else                                                   outitem->data =                  item->data   ;
      outitem->MallocedByUs = 0;
     }
    outitem->copyable = item->copyable;
    outitem->DataType = item->DataType;
    if (out->first == NULL) out->first      = outitem;
    if (out->last  != NULL) out->last->next = outitem;
    out->last = outitem;
    out->length++;
    hash = DictHash(item->key, in->HashSize);
    out->HashTable[hash] = outitem;
    item = item->next;
   }
  return out;
 }

int DictLen(Dict *in)
 {
  return in->length;
 }

void DictAppendPtr(Dict *in, char *key, void *item, int size, int copyable, int DataType)
 {
  DictItem *ptr=NULL, *ptrnew=NULL, *prev=NULL;
  int       cmp = -1, hash;

  ptr = in->first;
  while (ptr != NULL)
   {
    if ( ((cmp = StrCmpNoCase(ptr->key, key)) > 0) || ((cmp = strcmp(ptr->key, key)) == 0) ) break;
    prev = ptr;
    ptr  = ptr->next;
   }
  if (cmp == 0) // Overwrite an existing entry in dictionary
   {
    ptr->data     = item;
    ptr->DataSize = size;
    ptr->DataType = DataType;
    ptr->copyable = copyable;
    ptr->MallocedByUs = 0;
   }
  else
   {
    ptrnew           = (DictItem *)lt_malloc_incontext(sizeof(DictItem), in->memory_context);
    if (ptrnew==NULL) return;
    ptrnew->prev     = prev;
    ptrnew->next     = ptr;
    ptrnew->key      = (char *)lt_malloc_incontext((strlen(key)+1)*sizeof(char), in->memory_context);
    if (ptrnew->key==NULL) return;
    strcpy(ptrnew->key, key);
    ptrnew->data     = item;
    ptrnew->MallocedByUs = 0;
    ptrnew->DataType = DataType;
    ptrnew->copyable = copyable;
    ptrnew->DataSize = size;
    if (prev == NULL) in->first = ptrnew; else prev->next = ptrnew;
    if (ptr  == NULL) in->last  = ptrnew; else ptr ->prev = ptrnew;
    in->length++;

    hash = DictHash(key, in->HashSize);
    in->HashTable[hash] = ptrnew;
   }
 }

void DictAppendPtrCpy(Dict *in, char *key, void *item, int size, int DataType)
 {
  DictItem *ptr=NULL, *ptrnew=NULL, *prev=NULL;
  char     *newstr;
  int       cmp = -1, hash;

  ptr = in->first;
  while (ptr != NULL)
   {
    if ( ((cmp = StrCmpNoCase(ptr->key, key)) > 0) || ((cmp = strcmp(ptr->key, key)) == 0) ) break;
    prev = ptr;
    ptr  = ptr->next;
   }
  if (cmp == 0) // Overwrite an existing entry in dictionary
   {
    if ((size != ptr->DataSize) || (ptr->MallocedByUs == 0))
     {
      ptr->data     = (void *)lt_malloc_incontext(size , in->memory_context);
      ptr->DataSize = size;
      ptr->MallocedByUs = 1;
     }
    memcpy(ptr->data , item, size);
    ptr->DataType = DataType;
    ptrnew = ptr;
   }
  else
   {
    ptrnew           = (DictItem *)lt_malloc_incontext(sizeof(DictItem), in->memory_context);
    if (ptrnew==NULL) return;
    ptrnew->prev     = prev;
    ptrnew->next     = ptr;
    ptrnew->key      = (char *)lt_malloc_incontext((strlen(key)+1)*sizeof(char), in->memory_context);
    if (ptrnew->key==NULL) return;
    strcpy(ptrnew->key, key);
    ptrnew->data     = (void *)lt_malloc_incontext(size, in->memory_context);
    if (ptrnew->data==NULL) return;
    memcpy(ptrnew->data, item, size);
    ptrnew->MallocedByUs = 1;
    ptrnew->copyable = 1;
    ptrnew->DataType = DataType;
    ptrnew->DataSize = size;
    if (prev == NULL) in->first = ptrnew; else prev->next = ptrnew;
    if (ptr  == NULL) in->last  = ptrnew; else ptr ->prev = ptrnew;
    in->length++;

    hash = DictHash(key, in->HashSize);
    in->HashTable[hash] = ptrnew;
   }
  if ((ptrnew->DataType == DATATYPE_VALUE) && (((value *)ptrnew->data)->string != NULL))
   {
    newstr = (char *)lt_malloc_incontext(strlen(((value *)ptrnew->data)->string)+1, in->memory_context); // Copy strings in string values
    strcpy(newstr, ((value *)ptrnew->data)->string);
    ((value *)ptrnew->data)->string = newstr;
   }
 }

void DictAppendInt(Dict *in, char *key, int item)
 {
  DictAppendPtrCpy(in, key, (void *)&item, sizeof(int), DATATYPE_INT);
  return;
 }

void DictAppendFloat(Dict *in, char *key, double item)
 {
  DictAppendPtrCpy(in, key, (void *)&item, sizeof(double), DATATYPE_FLOAT);
  return;
 }

void DictAppendValue(Dict *in, char *key, value item)
 {
  DictAppendPtrCpy(in, key, (void *)&item, sizeof(value), DATATYPE_VALUE);
  return;
 }

void DictAppendString(Dict *in, char *key, char *item)
 {
  int length = strlen(item);
  DictAppendPtrCpy(in, key, (void *)item, (length+1)*sizeof(char), DATATYPE_STRING);
  return;
 }

void DictAppendList(Dict *in, char *key, List *item)
 {
  DictAppendPtr(in, key, (void *)item, sizeof(List), 0, DATATYPE_LIST);
  return;
 }

void DictAppendDict(Dict *in, char *key, Dict *item)
 {
  DictAppendPtr(in, key, (void *)item, sizeof(Dict), 0, DATATYPE_DICT);
  return;
 }

void DictLookup(Dict *in, char *key, int *DataTypeOut, void **ptrout)
 {
  int hash;
  DictItem *ptr;

  if (in==NULL) { *ptrout=NULL; return; }

#define DICTLOOKUP_TEST \
   if (strcmp(ptr->key, key) == 0) \
    { \
     if (DataTypeOut != NULL) *DataTypeOut = ptr->DataType; \
     *ptrout = ptr->data; \
     return; \
    }

  // Check hash table
  hash = DictHash(key, in->HashSize);
  ptr  = in->HashTable[hash];
  if (ptr==NULL) { *ptrout=NULL; return; }
  DICTLOOKUP_TEST;

  // Hash table clash; need to exhaustively search dictionary
  ptr = in->first;
  while (ptr != NULL)
   {
    DICTLOOKUP_TEST
    else if (StrCmpNoCase(ptr->key, key) > 0) break;
    ptr = ptr->next;
   }
  *ptrout = NULL;
  return;
 }

void DictLookupWithWildcard(Dict *in, Dict *in_w, char *key, char *SubsString, int SubsMaxLen, DictItem **ptrout)
 {
  int hash, k, l;
  char tmp;
  DictItem *ptr;

  SubsString[0]='\0';
  if (in==NULL) { *ptrout=NULL; return; }

  // Check hash table
  for (k=0; (isalnum(key[k]) || (key[k]=='_')); k++);
  tmp=key[k];
  key[k]='\0';
  hash = DictHash(key, in->HashSize);
  key[k]=tmp;
  ptr  = in->HashTable[hash];
  if (ptr!=NULL)
   {
    for (k=0; ((ptr->key[k]>' ')&&(ptr->key[k]!='?')&&(ptr->key[k]==key[k])); k++);
    if (!((ptr->key[k]>' ') || (isalnum(key[k])) || (key[k]=='_')) ) { *ptrout = ptr;  return; }

    // Hash table clash; need to exhaustively search dictionary
    ptr = in->first;
    while (ptr != NULL)
     {
      for (k=0; ((ptr->key[k]>' ')&&(ptr->key[k]!='?')&&(ptr->key[k]==key[k])); k++);
      if (!((ptr->key[k]>' ') || (isalnum(key[k])) || (key[k]=='_')) ) { *ptrout = ptr;  return; }
      ptr = ptr->next;
     }
   }

  // Need to start exhaustive search of dictionary of "int_d?"-like wildcards
  for (ptr=in_w->first; ptr!=NULL; ptr=ptr->next)
   {
    for (k=0; ((ptr->key[k]>' ')&&(ptr->key[k]!='?')&&(ptr->key[k]==key[k])); k++);
    if (ptr->key[k]=='?') // Dictionary key ends with a ?, e.g. "int_d?"
     {
      for (l=0; ((isalnum(key[k+l]) || (key[k+l]=='_')) && (l<SubsMaxLen)); l++) SubsString[l]=key[k+l];
      if (l==SubsMaxLen) continue; // Dummy variable name was too long
      if (l==0) continue; // Dummy variable was too short
      SubsString[l]='\0';
      *ptrout = ptr;
      return;
     } else { // Otherwise, we have to match function name exactly
      if ((ptr->key[k]>' ') || (isalnum(key[k])) || (key[k]=='_')) continue; // Nope...
      *ptrout = ptr;
      SubsString[0]='\0';
      return;
     }
   }
  SubsString[0]='\0';
  *ptrout = NULL;
  return;
 }

int DictContains(Dict *in, char *key)
 {
  int hash;
  DictItem *ptr;
  if (in==NULL) return 0;

  // Check hash table
  hash = DictHash(key, in->HashSize);
  ptr  = in->HashTable[hash];
  if (ptr==NULL) return 0;
  if (strcmp(ptr->key, key)==0) return 1;

  // Hash table clash; need to exhaustively search dictionary
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
  int hash;
  DictItem *ptr;

  if (in==NULL) return -1;

  // Check hash table
  hash = DictHash(key, in->HashSize);
  ptr  = in->HashTable[hash];
  if (ptr==NULL) return -1;
  if (strcmp(ptr->key, key)==0) { _DictRemoveEngine(in, ptr); return 0; }

  // Hash table clash; need to exhaustively search dictionary
  ptr = in->first;
  while (ptr != NULL)
   {
    if (strcmp(ptr->key, key)==0) { _DictRemoveEngine(in, ptr); return 0; }
    else if (StrCmpNoCase(ptr->key, key) > 0) break;
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
    if (ptr->data == item) { _DictRemoveEngine(in, ptr); return 0; }
    ptr = ptr->next;
   }
  return -1;
 }

void _DictRemoveEngine(Dict *in, DictItem *ptr)
 {
  int hash;
  DictItem *ptrnext;

  if (in ==NULL) return;
  if (ptr==NULL) return;

  // Remove hash table entry
  hash = DictHash(ptr->key,in->HashSize);
  if (in->HashTable[hash]==ptr) in->HashTable[hash]=NULL;

  if (ptr->next != NULL) // We are not the last item in the list
   {
    ptrnext       = ptr->next;
    ptr->key      = ptrnext->key;
    ptr->DataType = ptrnext->DataType;
    ptr->DataSize = ptrnext->DataSize;
    ptr->copyable = ptrnext->copyable;
    ptr->MallocedByUs = ptrnext->MallocedByUs;
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
    ptr->DataSize = ptrnext->DataSize;
    ptr->copyable = ptrnext->copyable;
    ptr->MallocedByUs = ptrnext->MallocedByUs;
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

  // Fix up hash table in case another dictionary entry shared our hash entry
  if (in->HashTable[hash]==NULL)
   {
    DictItem *iter=in->first;
    while (iter != NULL)
     {
      if (DictHash(iter->key,in->HashSize)==hash) { in->HashTable[hash]=iter; break; }
      iter = iter->next;
     }
   }

  return;
 }

void DictRemovePtrAll(Dict *in, void *item)
 {
  while ( DictRemovePtr(in,item)!=-1 );
  return;
 }

DictIterator *DictIterateInit(Dict *in)
 {
  if (in==NULL) return NULL;
  return in->first;
 }

DictIterator *DictIterate(DictIterator *in, int *DataTypeOut, void **ptrout)
 {
  if (in==NULL) { if (ptrout!=NULL) *ptrout = NULL; return NULL; }
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
    if      (iter->DataType == DATATYPE_VOID  ) { sprintf(out+pos, "'%s':void", iter->key                         ); }
    else if (iter->DataType == DATATYPE_INT   ) { sprintf(out+pos, "'%s':%d"  , iter->key, *((int    *)iter->data)); }
    else if (iter->DataType == DATATYPE_FLOAT ) { sprintf(out+pos, "'%s':%e"  , iter->key, *((double *)iter->data)); }
    else if (iter->DataType == DATATYPE_VALUE ) { sprintf(out+pos, "'%s':%e+%ei <unit>", iter->key, ((value *)iter->data)->real, ((value *)iter->data)->imag); }
    else if (iter->DataType == DATATYPE_STRING) { sprintf(out+pos, "'%s':'%s'", iter->key,  ((char   *)iter->data)); }
    else if (iter->DataType == DATATYPE_LIST  )
     {
      sprintf(out+pos, "'%s':", iter->key);
      pos += strlen(out+pos);
      ListPrint( ((List *)iter->data), out+pos, size-pos);
     }
    else if (iter->DataType == DATATYPE_DICT  )
      {
       sprintf(out+pos, "'%s':", iter->key);
       pos += strlen(out+pos);
       DictPrint( ((Dict *)iter->data), out+pos, size-pos);
      }
    pos += strlen(out+pos);
    first=0;
    iter = DictIterate(iter, NULL, NULL);
   }
  strcpy(out+pos, "}"); pos += strlen(out+pos);
  return out;
 }

