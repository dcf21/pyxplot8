// lt_memory.c
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

// Functions for memory management

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "StringTools/str_constants.h"

#include "lt_memory.h"

#define PPL_MAX_CONTEXTS 16

// lt_memory functions
// These provide simple wrapper for fastmalloc which keep track of the current memory allocation context

int lt_mem_context = -1;

char temp_merr_string[LSTR_LENGTH]; // Storage buffer for error messages

void (*mem_fatal)(char *, int, char *); // Handler for fatal errors
void (*mem_log)  (char *);              // Handler for logging events


// ---------------------------------------------------------
// lt_memory functions
// These provide simple wrapper for fastmalloc which keep track of the current memory allocation context
// --------------------------------------------------------

// lt_MemoryInit() -- Call this before using any lt_memory, lt_list or lt_dist functions.

void lt_MemoryInit( void(*mem_fatal_handler)(char *, int, char *) , void(*mem_log_handler)(char *) )
 {
  mem_fatal = mem_fatal_handler;
  mem_log   = mem_log_handler;
  if (MEMDEBUG) (*mem_log)("Initialising memory management system.");
  lt_mem_context = 0;
  fastmalloc_init();
  lt_FreeAll(0);
  _lt_SetMemContext(0);
  return;
 }

// lt_MemoryStop() -- Call this when lt_memory, lt_list and lt_dict are all finished with and should be cleaned up

void lt_MemoryStop()
 {
  fastmalloc_close();
  lt_mem_context = -1;
  return;
 }

// lt_DescendIntoNewContext() -- Create a new memory allocation context for future calls to lt_malloc()
//  [returns the context number of the allocation context which has been assigned to future lt_malloc calls]

int lt_DescendIntoNewContext()
 {
  if (lt_mem_context <                    0 ) (*mem_fatal)(__FILE__,__LINE__,"Call to lt_DescendIntoNewContext() before call to lt_MemoryInit().");
  if (lt_mem_context >= (PPL_MAX_CONTEXTS+1)) (*mem_fatal)(__FILE__,__LINE__,"Too many memory contexts.");
  _lt_SetMemContext(lt_mem_context+1);
  return lt_mem_context;
 }

// lt_AscendOutOfContext() -- Call when the current memory allocation context is finished with and can be freed.
//   [call with the number of the allocation context which is to be freed. Returns the number of the current allocation context after the freeing operation.]

int  lt_AscendOutOfContext(int context)
 {
  int i;
  if (lt_mem_context <                    0 ) (*mem_fatal)(__FILE__,__LINE__,"Call to lt_AscendOutOfContext() before call to lt_MemoryInit().");
  if (context         >=     lt_mem_context ) return lt_mem_context;
  if (context         <=                   0 ) (*mem_fatal)(__FILE__,__LINE__,"Call to lt_AscendOutOfContext() attempting to descend out of lowest possible memory context.");
  for (i=context; i<=lt_mem_context; i++)     lt_FreeAll(i);
  _lt_SetMemContext(context-1);
  return lt_mem_context;
 }

// _lt_SetMemContext() -- PRIVATE FUNCTION.

void _lt_SetMemContext(int context)
 {
  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "lt_SetMemContext passed unrecognised context number %d.", context); (*mem_fatal)(__FILE__,__LINE__,temp_merr_string); }
  if (MEMDEBUG) { sprintf(temp_merr_string, "Setting memory allocation context to level %d.", context); (*mem_log)(temp_merr_string); }
  lt_mem_context = context;
  return; 
 }

// lt_GetMemContext() -- Returns the number of the current memory allocation context.

int lt_GetMemContext()
 {
  return lt_mem_context;
 }

// lt_FreeAll() -- Free all memory which has been allocated in the specified allocation context, and in deeper levels.

void lt_FreeAll(int context)
 {
  static int latch=0;

  if (latch==1) return; // Prevent recursive calls
  latch=1;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS)) 
   { sprintf(temp_merr_string, "lt_FreeAll() passed unrecognised context %d.", context); (*mem_fatal)(__FILE__,__LINE__,temp_merr_string); }

  if (MEMDEBUG) { sprintf(temp_merr_string, "Freeing all memory down to level %d.", context); (*mem_log)(temp_merr_string); }
  fastmalloc_freeall(context);
  latch=0;
  return;
 }

// lt_malloc() -- Malloc some memory in the present allocation context.

void *lt_malloc(int size)
 {
  void *out;

  if ((lt_mem_context<0) || (lt_mem_context>=PPL_MAX_CONTEXTS)) 
   { sprintf(temp_merr_string, "lt_malloc() using unrecognised context %d.", lt_mem_context); (*mem_fatal)(__FILE__,__LINE__,temp_merr_string); }

  if (MEMDEBUG) { sprintf(temp_merr_string, "Request to malloc %d bytes at memory level %d.", size, lt_mem_context); (*mem_log)(temp_merr_string); }
  out = fastmalloc(lt_mem_context, size);
  if (out == NULL) (*mem_fatal)(__FILE__,__LINE__,"Malloc failure.");
  return out;
 }

// lt_malloc_incontext() -- Malloc some memory in the specified context. Use sparingly.

void *lt_malloc_incontext(int size, int context)
 {
  void *out;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS)) 
   { sprintf(temp_merr_string, "lt_malloc_incontext() passed unrecognised context %d.", context); (*mem_fatal)(__FILE__,__LINE__,temp_merr_string); }

  if (MEMDEBUG) { sprintf(temp_merr_string, "Request to malloc %d bytes at memory level %d.", size, context); (*mem_log)(temp_merr_string); }
  out = fastmalloc(context, size);
  if (out == NULL) (*mem_fatal)(__FILE__,__LINE__,"Malloc failure.");
  return out;
 }

// Implementation of FASTMALLOC

// ------------------------------------------------------------------
// ALL CALLS BELOW THIS POINT ARE PRIVATE FUNCTIONS. DO NOT USE.
// ------------------------------------------------------------------

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
  if (DEBUG) { sprintf(temp_merr_string, "FastMalloc shutting down: Reduced %d calls to fastmalloc to %d calls to malloc.", _fastmalloc_callcount, _fastmalloc_malloccount); (*mem_log)(temp_merr_string); }
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
   { sprintf(temp_merr_string, "FastMalloc asked to malloc memory in an unrecognised context %d.", context); (*mem_fatal)(__FILE__,__LINE__,temp_merr_string); }

  blklist_pointer      = _fastmalloc_bufferlist       [context];
  blklist_position     = _fastmalloc_bufferlist_pos  + context ;
  blk_current_position = _fastmalloc_block_alloc_ptr + context ;

  if ((*blk_current_position == -1) || (size > (FM_BLOCKSIZE - 2 - *blk_current_position))) // We need to malloc a new block
   {
    _fastmalloc_malloccount++;
    if (*blklist_position == FM_BLOCKMAX-2) {  sprintf(temp_merr_string, "Memory allocation table overflow in context %d.", context); (*mem_fatal)(__FILE__,__LINE__,temp_merr_string); }
    if (size > FM_BLOCKMAX)
     {
      if (MEMDEBUG) { sprintf(temp_merr_string, "Fastmalloc creating block of size %d bytes at memory level %d.", size, context); (*mem_log)(temp_merr_string); }
      if ((out = blklist_pointer[(*blklist_position)++] = malloc(size        )) == NULL) (*mem_fatal)(__FILE__,__LINE__,"Malloc failure."); // This is a big malloc which needs to new block to itself
      *blk_current_position=-1;
     } else {
      if (MEMDEBUG) { sprintf(temp_merr_string, "Fastmalloc creating block of size %d bytes at memory level %d.", FM_BLOCKSIZE, context); (*mem_log)(temp_merr_string); }
      if ((out = blklist_pointer[(*blklist_position)++] = malloc(FM_BLOCKSIZE)) == NULL) (*mem_fatal)(__FILE__,__LINE__,"Malloc failure."); // Malloc a new standard sized block
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

