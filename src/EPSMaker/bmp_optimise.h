// bmp_optimise.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-9 Dominic Ford <coders@pyxplot.org.uk>
//               2008-9 Ross Church
//
//               2009   Michael Rutter
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
// Michael Rutter. It checks paletted images for possible optimisations of the
// palette size if there are unused entries, and checks RGB images to see if
// they can efficiently be reduced to paletted images.

#ifndef _PPL_BMP_OPTIMISE_H
#define _PPL_BMP_OPTIMISE_H 1

#include "eps_image.h"

void bmp_colour_count (bitmap_data *image);
void bmp_palette_check(bitmap_data *image);
void bmp_grey_check   (bitmap_data *image);
void bmp_compact      (bitmap_data *image);

#endif

