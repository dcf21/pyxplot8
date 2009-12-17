// bmp_gifread.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
//
//               2009-2010 Michael Rutter
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

// This file is edited from code which was kindly contributed to PyXPlot by
// Michael Rutter. It reads in data from GIF files.

#ifndef _PPL_BMP_GIFREAD_H
#define _PPL_BMP_GIFREAD_H 1

#include <stdio.h>

#include "eps_image.h"

void          bmp_gifread        (FILE *in, bitmap_data *image);
int           bmp_de_gifinterlace(bitmap_data *image);
unsigned long bmp_de_lzw         (unsigned char *buff, unsigned char *out, unsigned long len, int cs);
unsigned int  bmp_de_lzw_bits    (unsigned char *c,int st, int len);

#endif

