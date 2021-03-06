// lt_memory.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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

#define PPL_MAX_CONTEXTS 250

// lt_memory functions
// These provide simple wrapper for fastmalloc which keep track of the current memory allocation context

int lt_mem_context = -1;

char temp_merr_string[LSTR_LENGTH]; // Storage buffer for error messages

void (*mem_error)(int, int, int, char *); // Handler for errors
void (*mem_log)  (char *); // Handler for logging events


// ---------------------------------------------------------
// lt_memory functions
// These provide simple wrapper for fastmalloc which keep track of the current memory allocation context
// --------------------------------------------------------

// lt_MemoryInit() -- Call this before using any lt_memory, lt_list or lt_dist functions.

void lt_MemoryInit( void(*mem_error_handler)(int, int, int, char *) , void(*mem_log_handler)(char *) )
 {
  mem_error = mem_error_handler;
  mem_log   = mem_log_handler;
  if (MEMDEBUG1) (*mem_log)("Initialising memory management system.");
  lt_mem_context = 0;
  fastmalloc_init();
  lt_FreeAll(0);
  _lt_SetMemContext(0);
  return;
 }

// lt_MemoryStop() -- Call this when lt_memory, lt_list and lt_dict are all finished with and should be cleaned up

void lt_MemoryStop()
 {
  if (MEMDEBUG1) (*mem_log)("Shutting down memory management system.");
  fastmalloc_close();
  lt_mem_context = -1;
  return;
 }

// lt_DescendIntoNewContext() -- Create a new memory allocation context for future calls to lt_malloc()
//  [returns the context number of the allocation context which has been assigned to future lt_malloc calls]

int lt_DescendIntoNewContext()
 {
  if (lt_mem_context <                    0 ) { (*mem_error)(100, -1, -1, "Call to lt_DescendIntoNewContext() before call to lt_MemoryInit()."); return -1; }
  if (lt_mem_context >= (PPL_MAX_CONTEXTS+1)) { (*mem_error)(100, -1, -1, "Too many memory contexts.");                                          return -1; }
  _lt_SetMemContext(lt_mem_context+1);
  if (MEMDEBUG1) { sprintf(temp_merr_string, "Descended into memory context %d.", lt_mem_context); (*mem_log)(temp_merr_string); }
  return lt_mem_context;
 }

// lt_AscendOutOfContext() -- Call when the current memory allocation context is finished with and can be freed.
//   [call with the number of the allocation context which is to be freed. Returns the number of the current allocation context after the freeing operation.]

int  lt_AscendOutOfContext(int context)
 {
  if (lt_mem_context <               0 ) { (*mem_error)(100, -1, -1, "Call to lt_AscendOutOfContext() before call to lt_MemoryInit()."); return -1; }
  if (context        >  lt_mem_context ) return lt_mem_context;
  if (context        <=              0 ) { (*mem_error)(100, -1, -1, "Call to lt_AscendOutOfContext() attempting to ascend out of lowest possible memory context."); return -1; }
  if (MEMDEBUG1) { sprintf(temp_merr_string, "Ascending out of memory context %d.", lt_mem_context); (*mem_log)(temp_merr_string); }
  lt_FreeAll(context);
  _lt_SetMemContext(context-1);
  return lt_mem_context;
 }

// _lt_SetMemContext() -- PRIVATE FUNCTION.

void _lt_SetMemContext(int context)
 {
  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "lt_SetMemContext passed unrecognised context number %d.", context); (*mem_error)(100, -1, -1, temp_merr_string); return; }
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
  if (lt_mem_context<0) return; // Memory management not initialised
  latch=1;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "lt_FreeAll() passed unrecognised context %d.", context); (*mem_error)(100, -1, -1, temp_merr_string); return; }

  if (MEMDEBUG1) { sprintf(temp_merr_string, "Freeing all memory down to level %d.", context); (*mem_log)(temp_merr_string); }
  fastmalloc_freeall(context);
  latch=0;
  return;
 }

// lt_Free() -- Free all memory which has been allocated in the specified allocation context, but not in deeper levels.

void lt_Free(int context)
 {
  static int latch=0;

  if (latch==1) return; // Prevent recursive calls
  latch=1;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "lt_Free() passed unrecognised context %d.", context); (*mem_error)(100, -1, -1, temp_merr_string); return; }

  if (MEMDEBUG1) { sprintf(temp_merr_string, "Freeing all memory down in level %d.", context); (*mem_log)(temp_merr_string); }
  fastmalloc_free(context);
  latch=0;
  return;
 }

// lt_malloc() -- Malloc some memory in the present allocation context.

void *lt_malloc(int size)
 {
  void *out;

  if ((lt_mem_context<0) || (lt_mem_context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "lt_malloc() using unrecognised context %d.", lt_mem_context); (*mem_error)(100, -1, -1, temp_merr_string); return NULL; }

  if (MEMDEBUG2) { sprintf(temp_merr_string, "Request to malloc %d bytes at memory level %d.", size, lt_mem_context); (*mem_log)(temp_merr_string); }
  out = fastmalloc(lt_mem_context, size);
  if (out == NULL) { (*mem_error)(100, -1, -1, "Out of memory."); return NULL; }
  return out;
 }

// lt_malloc_incontext() -- Malloc some memory in the specified context. Use sparingly.

void *lt_malloc_incontext(int size, int context)
 {
  void *out;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "lt_malloc_incontext() passed unrecognised context %d.", context); (*mem_error)(100, -1, -1, temp_merr_string); return NULL; }

  if (MEMDEBUG2) { sprintf(temp_merr_string, "Request to malloc %d bytes at memory level %d.", size, context); (*mem_log)(temp_merr_string); }
  out = fastmalloc(context, size);
  if (out == NULL) { (*mem_error)(100, -1, -1, "Out of memory."); return NULL; }
  return out;
 }

// ------------------------------------------------------------------
// ALL CALLS BELOW THIS POINT ARE PRIVATE FUNCTIONS. DO NOT USE.
// ------------------------------------------------------------------

// Implementation of FASTMALLOC

// For each allocation context, a pointer to the first chunk of memory which we have malloced
void **_fastmalloc_firstblocklist;

// For each allocation context, a pointer to the chunk of memory which we are currently allocating from
void **_fastmalloc_currentblocklist;

// For each allocation context, integers recording how many bytes have been allocated from the current block
long *_fastmalloc_currentblock_alloc_ptr;

// Keep statistics on numbers of malloc calls
long long _fastmalloc_callcount;
long long _fastmalloc_bytecount;
long long _fastmalloc_malloccount;

static int _fastmalloc_initialised = 0;

void fastmalloc_init()
 {
  int i;
  if (_fastmalloc_initialised == 1) return;

  _fastmalloc_firstblocklist         = (void **)malloc(PPL_MAX_CONTEXTS * sizeof(void *));
  _fastmalloc_currentblocklist       = (void **)malloc(PPL_MAX_CONTEXTS * sizeof(void *));
  _fastmalloc_currentblock_alloc_ptr = (long  *)malloc(PPL_MAX_CONTEXTS * sizeof(long  ));

  for (i=0; i<PPL_MAX_CONTEXTS; i++) _fastmalloc_firstblocklist        [i] = NULL;
  for (i=0; i<PPL_MAX_CONTEXTS; i++) _fastmalloc_currentblocklist      [i] = NULL;
  for (i=0; i<PPL_MAX_CONTEXTS; i++) _fastmalloc_currentblock_alloc_ptr[i] = 0;

  _fastmalloc_callcount = 0;
  _fastmalloc_bytecount = 0.0;
  _fastmalloc_malloccount = 0;
  _fastmalloc_initialised = 1;
 }

void fastmalloc_close()
 {
  if (_fastmalloc_initialised == 0) return;
  if (DEBUG) { sprintf(temp_merr_string, "FastMalloc shutting down: Reduced %lld calls to fastmalloc, for a total of %lld bytes, to %lld calls to malloc.", _fastmalloc_callcount, _fastmalloc_bytecount, _fastmalloc_malloccount); (*mem_log)(temp_merr_string); }
  fastmalloc_freeall(0);
  free(_fastmalloc_firstblocklist);
  free(_fastmalloc_currentblocklist);
  free(_fastmalloc_currentblock_alloc_ptr);
  _fastmalloc_initialised = 0;
  return;
 }

void *fastmalloc(int context, int size)
 {
  void *ptr,*out;

  _fastmalloc_callcount++;
  _fastmalloc_bytecount += size;

  if ((context<0) || (context>=PPL_MAX_CONTEXTS))
   { sprintf(temp_merr_string, "FastMalloc asked to malloc memory in an unrecognised context %d.", context); (*mem_error)(100, -1, -1, temp_merr_string); return NULL; }

  if ((_fastmalloc_currentblocklist[context] == NULL) || (size > (FM_BLOCKSIZE - 2 - _fastmalloc_currentblock_alloc_ptr[context]))) // We need to malloc a new block
   {
    _fastmalloc_malloccount++;
    if (size > FM_BLOCKSIZE - sizeof(void **))
     {
      if (MEMDEBUG1) { sprintf(temp_merr_string, "Fastmalloc creating block of size %d bytes at memory level %d.", size, context); (*mem_log)(temp_merr_string); }
      if ((ptr = malloc(size + SYNCSTEP + sizeof(void **))) == NULL) { (*mem_error)(100, -1, -1, "Out of memory."); return NULL; } // This is a big malloc which needs to new block to itself
     } else {
      if (MEMDEBUG1) { sprintf(temp_merr_string, "Fastmalloc creating block of size %d bytes at memory level %d.", FM_BLOCKSIZE, context); (*mem_log)(temp_merr_string); }
      if ((ptr = malloc(FM_BLOCKSIZE                     )) == NULL) { (*mem_error)(100, -1, -1, "Out of memory."); return NULL; } // Malloc a new standard sized block
     }
    *((void **)ptr) = NULL; // Link to next block in chain
    if (_fastmalloc_currentblocklist[context] == NULL) _fastmalloc_firstblocklist[context]               = ptr; // Insert link into previous block in chain
    else                                               *((void **)_fastmalloc_currentblocklist[context]) = ptr;
    _fastmalloc_currentblocklist[context]        = ptr;
    _fastmalloc_currentblock_alloc_ptr[context]  = (sizeof(void **) + (SYNCSTEP-1));                         // Fastforward over link to next block
    _fastmalloc_currentblock_alloc_ptr[context] -= (_fastmalloc_currentblock_alloc_ptr[context] % SYNCSTEP);
    out                                          = ptr + _fastmalloc_currentblock_alloc_ptr[context];
    _fastmalloc_currentblock_alloc_ptr[context] += (size            + (SYNCSTEP-1));                         // Fastfoward over block we have just allocated
    _fastmalloc_currentblock_alloc_ptr[context] -= (_fastmalloc_currentblock_alloc_ptr[context] % SYNCSTEP);
   } else { // There is room for this malloc in the old block
    out                                          = _fastmalloc_currentblocklist[context] + _fastmalloc_currentblock_alloc_ptr[context];
    _fastmalloc_currentblock_alloc_ptr[context] += (size            + (SYNCSTEP-1));                         // Fastfoward over block we have just allocated
    _fastmalloc_currentblock_alloc_ptr[context] -= (_fastmalloc_currentblock_alloc_ptr[context] % SYNCSTEP);
   }
  return out;
 }

void fastmalloc_freeall(int context)
 {
  int   i;
  void *ptr, *ptr2;
  for (i=context; i<PPL_MAX_CONTEXTS; i++)
   {
    ptr = _fastmalloc_firstblocklist[i];
    while (ptr != NULL) { ptr2=*((void **)ptr); free(ptr); ptr=ptr2; }
    _fastmalloc_firstblocklist        [i] = NULL;
    _fastmalloc_currentblocklist      [i] = NULL;
    _fastmalloc_currentblock_alloc_ptr[i] = 0;
   }
  return;
 }

void fastmalloc_free(int context)
 {
  void *ptr, *ptr2;
  ptr = _fastmalloc_firstblocklist[context];
  while (ptr != NULL) { ptr2=*((void **)ptr); free(ptr); ptr=ptr2; }
  _fastmalloc_firstblocklist        [context] = NULL;
  _fastmalloc_currentblocklist      [context] = NULL;
  _fastmalloc_currentblock_alloc_ptr[context] = 0;
  return;
 }

