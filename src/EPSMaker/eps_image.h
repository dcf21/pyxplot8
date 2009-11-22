// eps_image.h
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

#ifndef _PPL_EPS_IMAGE_H
#define _PPL_EPS_IMAGE_H 1

#include "eps_comm.h"

void eps_image_RenderEPS(EPSComm *x);

// Colour channel configurations

#define BMP_COLOUR_BMP     1001
#define BMP_COLOUR_PALETTE 1002
#define BMP_COLOUR_GREY    1003
#define BMP_COLOUR_RGB     1004

// Image compression types

#define BMP_ENCODING_RLE    1
#define BMP_ENCODING_LZW    2
#define BMP_ENCODING_CCITT  4
#define BMP_ENCODING_FLATE  8
#define BMP_ENCODING_RLE24 16
#define BMP_ENCODING_DCT   32

typedef struct bitmap_data
 {
  unsigned char *data, *palette, *trans;
  unsigned       data_len;
  int            pal_len, width, height, depth, type, colour, compression, flags;
 } bitmap_data;

#endif

