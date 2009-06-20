// lt_memory.h
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

#ifndef _LT_MEMORY_H
#define _LT_MEMORY_H 1

void  lt_MemoryInit            ( void(*mem_fatal_handler)(char *, int, char *) , void(*mem_log_handler)(char *) );
void  lt_MemoryStop            ();
int   lt_DescendIntoNewContext ();
int   lt_AscendOutOfContext    (int context);
void _lt_SetMemContext         (int context);
int   lt_GetMemContext         ();
void  lt_FreeAll               (int context);

void *lt_malloc                (int size);
void *lt_malloc_incontext      (int size, int context);

// Fastmalloc functions

// Allocate memory in 256kb blocks (262144 bytes)
#define FM_BLOCKSIZE  262144
#define FM_BLOCKMAX    65536

void  fastmalloc_init   ();
void  fastmalloc_close  ();
void *fastmalloc        (int context, int size);
void  fastmalloc_freeall(int context);

#endif

