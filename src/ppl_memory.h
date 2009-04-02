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

#define MEMORY_SYSTEM 30000
#define MEMORY_GLOBAL 30001
#define MEMORY_PLOT   30002
#define MEMORY_SCRAP  30003

void  ppl_MemoryInit     ();
void  ppl_SetMemContext  (int context);
void  ppl_FreeAll        (int context);

void *ppl_malloc(int size);
void  ppl_free  (void *ptr);

#endif

