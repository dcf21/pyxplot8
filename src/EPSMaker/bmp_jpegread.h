// bmp_jpegread.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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
// Michael Rutter. It reads in data from JPEG files without performing any
// decompression, since the DCT-compressed data can be rewritten straight out
// to postscript for decompression by the postscript rasterising engine.

#ifndef _PPL_BMP_JPEGREAD_H
#define _PPL_BMP_JPEGREAD_H 1

#include <stdio.h>

#include "eps_image.h"

void bmp_jpegread(FILE *jpeg, bitmap_data *image);

#endif

