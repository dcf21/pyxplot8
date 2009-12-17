// lt_memory.h
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

// Functions for memory management

#ifndef _LT_MEMORY_H
#define _LT_MEMORY_H 1

void  lt_MemoryInit            ( void(*mem_error_handler)(int, char *) , void(*mem_log_handler)(char *) );
void  lt_MemoryStop            ();
int   lt_DescendIntoNewContext ();
int   lt_AscendOutOfContext    (int context);
void _lt_SetMemContext         (int context);
int   lt_GetMemContext         ();
void  lt_FreeAll               (int context);
void  lt_Free                  (int context);

void *lt_malloc                (int size);
void *lt_malloc_incontext      (int size, int context);

// Fastmalloc functions

// Allocate memory in 128kb blocks (131072 bytes)
#define FM_BLOCKSIZE  131072

// Always align mallocs to 8-byte boundaries; 64-bit processors do double arithmetic twice as fast when word-aligned
#define SYNCSTEP      8

void  fastmalloc_init   ();
void  fastmalloc_close  ();
void *fastmalloc        (int context, int size);
void  fastmalloc_freeall(int context);
void  fastmalloc_free   (int context);

#endif

