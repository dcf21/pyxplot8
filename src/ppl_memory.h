// ppl_memory.h
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

#ifndef _PPL_MEMORY
#define _PPL_MEMORY 1

void  ppl_MemoryInit            ();
void  ppl_MemoryStop            ();
int   ppl_DescendIntoNewContext ();
int   ppl_AscendOutOfContext    (int context);
void _ppl_SetMemContext         (int context);
int   ppl_GetMemContext         ();
void  ppl_FreeAll               (int context);

void *ppl_malloc                (int size);
void *ppl_malloc_incontext      (int size, int context);

// Fastmalloc functions

// Allocate memory in 256kb blocks
#define FM_BLOCKSIZE  262144
#define FM_BLOCKMAX    65536

void  fastmalloc_init   ();
void  fastmalloc_close  ();
void *fastmalloc        (int context, int size);
void  fastmalloc_freeall(int context);

#endif

