// bmp_pngread.c
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
// Michael Rutter. It reads in data from PNG files uses libpng.

#define _PPL_BMP_PNGREAD_C 1

#include <stdlib.h>
#include <stdio.h>
#include <png.h>

#include "ListTools/lt_memory.h"

#include "ppl_error.h"

#include "eps_image.h"
#include "bmp_pngread.h"

void bmp_pngread(FILE *in, bitmap_data *image)
 {
  int depth,ncols,ntrans,png_colour_type,i,j;
  unsigned width,height,row_bytes;
  static unsigned char index[3];
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_ptrs,trans_colours;
  png_color_16p trans_val;
  png_color_16p backgndp;
  png_color_16 background;
  png_colorp palette;

  if (DEBUG) ppl_log("Beginning to decode PNG image file");

  // Initialise libpng data structures
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (png_ptr == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  if (setjmp(png_jmpbuf(png_ptr))) { ppl_error(ERR_INTERNAL, "Unexpected error in libpng while trying to decode PNG image file"); return; }

  png_init_io(png_ptr, in);
  png_set_sig_bytes(png_ptr, 3); // Three bytes consumed before this routine was called

  // Let libpng read header, and deal with outcome
  png_read_info(png_ptr, info_ptr);

  width           = png_get_image_width (png_ptr,info_ptr);
  height          = png_get_image_height(png_ptr,info_ptr);
  depth           = png_get_bit_depth   (png_ptr,info_ptr);
  png_colour_type = png_get_color_type  (png_ptr,info_ptr);

  if (DEBUG) { sprintf(temp_err_string, "Size %dx%d", width, height); ppl_log(temp_err_string); }
  if (DEBUG) { sprintf(temp_err_string, "Depth %d", depth); ppl_log(temp_err_string); }

  if (depth==16)
   {
    if (DEBUG) ppl_log("Reducing 16 bit per components to 8 bits");
    png_set_strip_16(png_ptr);
    depth=8;
   }

  if (png_colour_type & PNG_COLOR_MASK_ALPHA)
   {
    if (DEBUG) ppl_log("PNG uses transparency");
    if (png_get_bKGD(png_ptr,info_ptr,&backgndp))
     {
      if (DEBUG) ppl_log("PNG defines a background colour");
      png_set_background(png_ptr,backgndp,PNG_BACKGROUND_GAMMA_FILE,1,1.0);
     }
    else
     {
      if (DEBUG) ppl_log("PNG does not define a background colour");
      background.red = background.green = background.blue = 0xff;  // Define background colour to be white
      png_set_background(png_ptr,&background,PNG_BACKGROUND_GAMMA_FILE,0,1.0);
     }
   }

  if ( (png_colour_type == PNG_COLOR_TYPE_GRAY) || (png_colour_type == PNG_COLOR_TYPE_GRAY_ALPHA)) image->colour = BMP_COLOUR_GREY;
  if ( (png_colour_type == PNG_COLOR_TYPE_RGB ) || (png_colour_type == PNG_COLOR_TYPE_RGB_ALPHA )) image->colour = BMP_COLOUR_RGB;

  if (png_colour_type == PNG_COLOR_TYPE_PALETTE)
   {
    image->type   = BMP_COLOUR_PALETTE;
    image->colour = BMP_COLOUR_PALETTE;
    i = png_get_PLTE(png_ptr,info_ptr,&palette,&ncols);
    if (i==0) { ppl_error(ERR_FILE, "PNG image file claims to be paletted, but no palette was found"); return; }
    image->pal_len = ncols;
    image->palette = (unsigned char *)lt_malloc(ncols*3);
    if (image->palette == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    for (i=0; i<ncols; i++)
     {
      *(image->palette+3*i  ) = palette[i].red;
      *(image->palette+3*i+1) = palette[i].green;
      *(image->palette+3*i+2) = palette[i].blue;
     }
    if (DEBUG) { sprintf(temp_err_string, "PNG image file contains a palette of %d colours", ncols); ppl_log(temp_err_string); }
   }
  else
   { image->type = BMP_COLOUR_BMP; }

  // Update png info to reflect any requested conversions (e.g. 16 bit to 8 bit or Alpha to non-alpha
  png_read_update_info(png_ptr, info_ptr);

  // Now rowbytes will reflect what we will get, not what we had originally
  row_bytes = png_get_rowbytes(png_ptr, info_ptr);

  // Allocate block of memory for uncompressed image
  image->data = (unsigned char *)lt_malloc(row_bytes*height);
  if (image->data == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  // libpng requires a sepate pointer to each row of image
  row_ptrs = (png_bytep *)malloc(height*sizeof(png_bytep));
  if (row_ptrs == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); image->data = NULL; return; }

  for (i=0; i<height; i++) row_ptrs[i] = image->data + row_bytes*i;

  // Get uncompressed image
  png_read_image(png_ptr, row_ptrs);

  // Free everything we don't need
  png_read_end(png_ptr,NULL);

  // Deal with transparency (we can only support images with single transparent palette entries)
  if (png_get_valid(png_ptr,info_ptr,PNG_INFO_tRNS))
   {
    if (DEBUG) ppl_log("PNG has transparency");
    png_get_tRNS(png_ptr, info_ptr, &trans_colours, &ntrans, &trans_val);
    if (DEBUG) { sprintf(temp_err_string, "PNG has %d transparent entries in palette", ntrans); ppl_log(temp_err_string); }
    if (png_colour_type == PNG_COLOR_TYPE_PALETTE)
     {
      // We can cope with just one, fully transparent, entry in palette
      j=0;
      for (i=0; i<ntrans; i++)
        if      (trans_colours[i] ==   0) j++;
        else if (trans_colours[i] != 255) j+=10;
      if (j!=1) { ppl_warning(ERR_FILE, "PNG has transparency, but not in the form of a single fully colour in its palette. Such transparency is not supported by PyXPlot."); }
      else
       {
        for (i=0; (i<ntrans) && (trans_colours[i]==255); i++);
        image->trans  = index;
        *image->trans = i;
       }
     }
    else
     {
      image->trans  = index;
      *image->trans = trans_val->gray;
      if (image->colour == BMP_COLOUR_RGB)
       {
        image->trans[0] = trans_val->red;
        image->trans[1] = trans_val->green;
        image->trans[2] = trans_val->blue;
       }
     }
   }
  png_destroy_read_struct(&png_ptr, &info_ptr,(png_infopp)NULL);

  // Put all necessary information into the output data structure
  image->data_len = ((long)row_bytes)*height;
  image->height   = height;
  image->width    = width;
  image->depth    = depth;
  if (image->colour == BMP_COLOUR_RGB) image->depth*=3;
  return;
 }

