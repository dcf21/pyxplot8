// kpse_wrap.h
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

#ifndef _KPSE_WRAP_H
#define _KPSE_WRAP_H 1

void  ppl_kpse_wrap_init    ();
char *ppl_kpse_wrap_find_pfa(char *s);
char *ppl_kpse_wrap_find_pfb(char *s);
char *ppl_kpse_wrap_find_tfm(char *s);

#endif

