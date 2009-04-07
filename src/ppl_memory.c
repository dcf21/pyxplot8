// ppl_memory.c
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

// Functions for memory management

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ppl_list.h"
#include "ppl_memory.h"
#include "ppl_error.h"
#include "ppl_constants.h"

#define PPL_MAX_CONTEXTS 16

// ppl_memory functions
// These provide simple wrapper for fastmalloc which keep track of the current memory allocation context

int ppl_mem_context = -1;

void ppl_MemoryInit()
 {
  if (MEMDEBUG) ppl_log("Initialising memory management system.");
  ppl_mem_context = 0;
  fastmalloc_init();
  ppl_FreeAll(0);
  _ppl_SetMemContext(0);
  return;
 }

void ppl_MemoryStop()
 {
  fastmalloc_close();
  ppl_mem_context = -1;
  return;
 }

int ppl_DescendIntoNewContext()
 {
  if (ppl_mem_context <                    0 ) ppl_fatal(__FILE__,__LINE__,"Call to ppl_DescendIntoNewContext() before call to ppl_MemoryInit().");
  if (ppl_mem_context >= (PPL_MAX_CONTEXTS+1)) ppl_fatal(__FILE__,__LINE__,"Too many memory contexts.");
  _ppl_SetMemContext(ppl_mem_context+1);
  return ppl_mem_context;
 }

int  ppl_AscendOutOfContext(int context)
 {
  int i;
  if (ppl_mem_context <                    0 ) ppl_fatal(__FILE__,__LINE__,"Call to ppl_AscendOutOfContext() before call to ppl_MemoryInit().");
  if (context         >=     ppl_mem_context ) return ppl_mem_context;
  if (context         <=                   0 ) ppl_fatal(__FILE__,__LINE__,"Call to ppl_AscendOutOfContext() attempting to descend out of lowest possible memory context.");
  for (i=context; i<=ppl_mem_context; i++)     ppl_FreeAll(i);
  _ppl_SetMemContext(context-1);
  return ppl_mem_context;
 }

void _ppl_SetMemContext(int context)
 {
  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_err_string, "ppl_SetMemContext passed unrecognised context number %d.", context); ppl_fatal(__FILE__,__LINE__,temp_err_string); }
  if (MEMDEBUG) { sprintf(temp_err_string, "Setting memory allocation context to level %d.", context); ppl_log(temp_err_string); }
  ppl_mem_context = context;
  return; 
 }

int ppl_GetMemContext()
 {
  return ppl_mem_context;
 }

void ppl_FreeAll(int context)
 {
  static int latch=0;

  if (latch==1) return; // Prevent recursive calls
  latch=1;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS)) 
   { sprintf(temp_err_string, "ppl_FreeAll() passed unrecognised context %d.", context); ppl_fatal(__FILE__,__LINE__,temp_err_string); }

  if (MEMDEBUG) { sprintf(temp_err_string, "Freeing all memory down to level %d.", context); ppl_log(temp_err_string); }
  fastmalloc_freeall(context);
  latch=0;
  return;
 }

void *ppl_malloc(int size)
 {
  void *out;

  if ((ppl_mem_context<0) || (ppl_mem_context>=PPL_MAX_CONTEXTS)) 
   { sprintf(temp_err_string, "ppl_malloc() using unrecognised context %d.", ppl_mem_context); ppl_fatal(__FILE__,__LINE__,temp_err_string); }

  if (MEMDEBUG) { sprintf(temp_err_string, "Request to malloc %d bytes at memory level %d.", size, ppl_mem_context); ppl_log(temp_err_string); }
  out = fastmalloc(ppl_mem_context, size);
  if (out == NULL) ppl_fatal(__FILE__,__LINE__,"Malloc failure.");
  return out;
 }

void *ppl_malloc_incontext(int size, int context)
 {
  void *out;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS)) 
   { sprintf(temp_err_string, "ppl_malloc_incontext() passed unrecognised context %d.", context); ppl_fatal(__FILE__,__LINE__,temp_err_string); }

  if (MEMDEBUG) { sprintf(temp_err_string, "Request to malloc %d bytes at memory level %d.", size, context); ppl_log(temp_err_string); }
  out = fastmalloc(context, size);
  if (out == NULL) ppl_fatal(__FILE__,__LINE__,"Malloc failure.");
  return out;
 }

// Implementation of FASTMALLOC

// For each allocation context, a pointer to a list of big chunks of memory which we have malloced
void ***_fastmalloc_bufferlist;

// Pointers to current positions in the above lists, where the next malloced block will get recorded
int *_fastmalloc_bufferlist_pos;

// Integers recording how many bytes have been fastmalloced in the current block of memory
int *_fastmalloc_block_alloc_ptr;

// Keep statistics on numbers of malloc calls
int _fastmalloc_callcount;
int _fastmalloc_malloccount;

static int _fastmalloc_initialised = 0;

void fastmalloc_init()
 {
  int j,k;
  if (_fastmalloc_initialised == 1) return;

  _fastmalloc_bufferlist      = (void ***)malloc(PPL_MAX_CONTEXTS * sizeof(void **));
  _fastmalloc_bufferlist_pos  = (int    *)malloc(PPL_MAX_CONTEXTS * sizeof(int    ));
  _fastmalloc_block_alloc_ptr = (int    *)malloc(PPL_MAX_CONTEXTS * sizeof(int    ));
  for (j=0; j<PPL_MAX_CONTEXTS; j++)
   {
    _fastmalloc_bufferlist[j] = (void **)malloc(FM_BLOCKMAX * sizeof(void *));
    for (k=0; k<FM_BLOCKMAX; k++) _fastmalloc_bufferlist[j][k] = NULL;
    _fastmalloc_bufferlist_pos [j] = 0;
    _fastmalloc_block_alloc_ptr[j] =-1;
   }
  _fastmalloc_callcount = 0;
  _fastmalloc_malloccount = 0;
  _fastmalloc_initialised = 1;
 }

void fastmalloc_close()
 {
  int j;
  if (_fastmalloc_initialised == 0) return;
  if (DEBUG) { sprintf(temp_err_string, "FastMalloc shutting down: Reduced %d calls to fastmalloc to %d calls to malloc.", _fastmalloc_callcount, _fastmalloc_malloccount); ppl_log(temp_err_string); }
  fastmalloc_freeall(0);

  for (j=0; j<PPL_MAX_CONTEXTS; j++) free(_fastmalloc_bufferlist[j]);
  free(_fastmalloc_bufferlist);
  free(_fastmalloc_bufferlist_pos);
  free(_fastmalloc_block_alloc_ptr);
  return;
 }

void *fastmalloc(int context, int size)
 {
  void **blklist_pointer;
  int   *blklist_position;
  int   *blk_current_position;
  void  *out;

  _fastmalloc_callcount++;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_err_string, "FastMalloc asked to malloc memory in an unrecognised context %d.", context); ppl_fatal(__FILE__,__LINE__,temp_err_string); }

  blklist_pointer      = _fastmalloc_bufferlist       [context];
  blklist_position     = _fastmalloc_bufferlist_pos  + context ;
  blk_current_position = _fastmalloc_block_alloc_ptr + context ;

  if ((*blk_current_position == -1) || (size > (FM_BLOCKSIZE - 2 - *blk_current_position))) // We need to malloc a new block
   {
    _fastmalloc_malloccount++;
    if (*blklist_position == FM_BLOCKMAX-2) {  sprintf(temp_err_string, "Memory allocation table overflow in context %d.", context); ppl_fatal(__FILE__,__LINE__,temp_err_string); }
    if (size > FM_BLOCKMAX)
     {
      if (MEMDEBUG) { sprintf(temp_err_string, "Fastmalloc creating block of size %d bytes at memory level %d.", size, context); ppl_log(temp_err_string); }
      if ((out = blklist_pointer[(*blklist_position)++] = malloc(size        )) == NULL) ppl_fatal(__FILE__,__LINE__,"Malloc failure."); // This is a big malloc which needs to new block to itself
      *blk_current_position=-1;
     } else {
      if (MEMDEBUG) { sprintf(temp_err_string, "Fastmalloc creating block of size %d bytes at memory level %d.", FM_BLOCKSIZE, context); ppl_log(temp_err_string); }
      if ((out = blklist_pointer[(*blklist_position)++] = malloc(FM_BLOCKSIZE)) == NULL) ppl_fatal(__FILE__,__LINE__,"Malloc failure."); // Malloc a new standard sized block
      *blk_current_position=size;
     }
    return out;
   } else { // There is room for this malloc in the old block
    out = blklist_pointer[*(blklist_position)-1] + *blk_current_position;
    *blk_current_position += size;
    return out;
   }
  return NULL;
 }

void fastmalloc_freeall(int context)
 {
  int i,j;
  void **blklist_pointer;
  int   *blklist_position;
  int   *blk_current_position;

  for (i=context; i<PPL_MAX_CONTEXTS; i++)
   {
    blklist_pointer      = _fastmalloc_bufferlist       [i];
    blklist_position     = _fastmalloc_bufferlist_pos  + i ;
    blk_current_position = _fastmalloc_block_alloc_ptr + i ;

    for (j=0; j<*blklist_position; j++)
     {
      free( *(blklist_pointer + j) );
      *(blklist_pointer + j) = NULL;
     }
    *blklist_position = 0;
    *blk_current_position = -1;
   }
  return;
 }

