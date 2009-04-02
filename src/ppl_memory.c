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

static int _context = MEMORY_SYSTEM;

static List *_MemoryGlobal  = NULL;
static List *_MemoryPlot    = NULL;
static List *_MemoryScrap   = NULL;

void ppl_MemoryInit()
 {
  if (MEMDEBUG) ppl_log("Initialising memory management system.");
  ppl_FreeAll(MEMORY_SYSTEM);
  ppl_SetMemContext(MEMORY_SYSTEM);
  return;
 }

void ppl_SetMemContext(int context)
 {
  if (MEMDEBUG) { sprintf(temp_err_string, "Setting memory allocation context to level %d.", context); ppl_log(temp_err_string); }
  _context = MEMORY_SYSTEM;
  if ((context >= MEMORY_SCRAP ) && (_MemoryScrap  == NULL)) _MemoryScrap  = ListInit();
  if ((context >= MEMORY_PLOT  ) && (_MemoryPlot   == NULL)) _MemoryPlot   = ListInit();
  if ((context >= MEMORY_GLOBAL) && (_MemoryGlobal == NULL)) _MemoryGlobal = ListInit();
  _context = context;
  return; 
 }

void ppl_FreeAll(int context)
 {
  if (MEMDEBUG) { sprintf(temp_err_string, "Freeing all memory down to level %d.", context); ppl_log(temp_err_string); }
  int oldcontext = _context;
  _context = MEMORY_SYSTEM;
  if (context <= MEMORY_SCRAP ) { ListKill(_MemoryScrap ) ; _MemoryScrap  = NULL; }
  if (context <= MEMORY_PLOT  ) { ListKill(_MemoryPlot  ) ; _MemoryPlot   = NULL; }
  if (context <= MEMORY_GLOBAL) { ListKill(_MemoryGlobal) ; _MemoryGlobal = NULL; }
  _context = oldcontext;
 }

void *ppl_malloc(int size)
 {
  void *out;
  int oldcontext;
  oldcontext = _context;
  _context = MEMORY_SYSTEM;
  if (MEMDEBUG) { sprintf(temp_err_string, "Request to malloc %d bytes at memory level %d.", size, oldcontext); ppl_log(temp_err_string); }
  out = malloc(size);
  if (out == NULL) ppl_fatal(__FILE__,__LINE__,"Malloc failure.");
  if (oldcontext == MEMORY_GLOBAL) ListAppend(_MemoryGlobal , out);
  if (oldcontext == MEMORY_PLOT  ) ListAppend(_MemoryPlot   , out);
  if (oldcontext == MEMORY_SCRAP ) ListAppend(_MemoryScrap  , out);
  _context = oldcontext;
  return out;
 }

void ppl_free(void *ptr)
 {
  int oldcontext;
  oldcontext = _context;
  _context = MEMORY_SYSTEM;
  if (MEMDEBUG) { sprintf(temp_err_string, "Freeing pointer at memory level %d.", oldcontext); ppl_log(temp_err_string); }
  if      (oldcontext == MEMORY_GLOBAL) ListRemovePtr(_MemoryGlobal , ptr);
  else if (oldcontext == MEMORY_PLOT  ) ListRemovePtr(_MemoryPlot   , ptr);
  else if (oldcontext == MEMORY_SCRAP ) ListRemovePtr(_MemoryScrap  , ptr);
  else                                free(ptr);
  _context = oldcontext;
  return;
 }

