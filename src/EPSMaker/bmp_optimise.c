// bmp_optimise.c
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
// Michael Rutter. It checks paletted images for possible optimisations of the
// palette size if there are unused entries, and checks RGB images to see if
// they can efficiently be reduced to paletted images.

#define _PPL_BMP_OPTIMISE_C 1

#include <stdlib.h>

#include "ListTools/lt_memory.h"

#include "ppl_error.h"

#include "bmp_optimise.h"
#include "eps_image.h"

void bmp_colour_count(bitmap_data *image)
 {
  unsigned long size, i;
  int ncols,j,colour;
  int palette[257];
  unsigned char *p;

  size  = (long)image->height * (long)image->width;
  ncols = 0;
  p     = image->data;

  // Count the number of colours in the image
  for (i=0; i<size; i++)
   {
    colour = (((int)*p)<<16) + (((int)*(p+1))<<8) + *(p+2);
    p += 3;
    for (j=0; j<ncols; j++) if (colour == palette[j]) break;
    if (j==ncols)
     {
      palette[ncols++] = colour;
      if (ncols==257) break;
     }
   }

  if (ncols > 256) { ppl_log("Image contains more than 256 colours"); return; }

  if (DEBUG) { sprintf(temp_err_string, "Image contains only %d colours: reducing to paletted image",ncols); ppl_log(temp_err_string); }

  // Reduce to paletted image if possible
  image->palette = (unsigned char *)lt_malloc(3*ncols);
  if (image->palette == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  // Copy colours we found into palette
  for (i=0; i<ncols; i++)
   {
    image->palette[3*i  ]=(palette[i]&0xff0000)>>16;
    image->palette[3*i+1]=(palette[i]&0xff00)>>8;
    image->palette[3*i+2]=(palette[i]&0xff);
   }

  // Replace RGB data with paletted colours
  p = image->data;
  for (i=0; i<size; i++)
   {
    colour= (((int)p[3*i])<<16) + (((int)p[3*i+1])<<8) + p[3*i+2];
    for (j=0; j<ncols; j++) if (colour == palette[j]) break;
    p[i]=j;
   }

  // Replace transparent colour with paletted colour
  if (image->trans != NULL)
   {
    p = image->trans;
    colour= (((int)p[3*i])<<16) + (((int)p[3*i+1])<<8) + p[3*i+2];
    for (j=0; j<ncols; j++) if (colour == palette[j]) break;
    if (j==ncols) image->trans=NULL; // Transparent colour not present in image
    else          p[0]=j;
   }

  // Set up image headers to show that it is now a paletted image
  image->data_len = size;
  image->depth    = 8;
  image->colour   = BMP_COLOUR_PALETTE;
  image->type     = BMP_COLOUR_PALETTE;
  image->pal_len  = ncols;

  return;
 }

void bmp_palette_check(bitmap_data *image)
 {
  int i;
  unsigned long size;
  unsigned char ncols;

  if (image->type  != BMP_COLOUR_PALETTE) return;
  if (image->depth !=                  8) return;

  // Find highest used palette entry
  ncols = 0;
  size  = image->data_len;
  for (i=0; i<size; i++) ncols = (ncols>image->data[i] ? ncols : image->data[i]);

  if (ncols < (image->pal_len-1))
   {
    if (DEBUG) { sprintf(temp_err_string, "Palette length reduced from %d to %d", image->pal_len, 1+ncols); ppl_log(temp_err_string); }
    image->pal_len = 1+ncols;
    if ((image->trans!=NULL)&&(image->trans[0]>ncols)) image->trans=NULL; // Transparent colour is not used in image
   }
  return;
 }

void bmp_grey_check(bitmap_data *image)
 {
  int           i,j,grey,depth=8,magic,test;
  unsigned int  ncols;
  unsigned long size;

  if (image->type != BMP_COLOUR_PALETTE) return;
  if (image->depth!=                  8) return;

  ncols = image->pal_len;
  size  = image->data_len;

  // See if palette is greyscale. If not, we cannot reduce it to greyscale.
  grey=1;
  for (i=0; i<image->pal_len; i++)
   if ( (image->palette[3*i]!=image->palette[3*i+1]) || (image->palette[3*i]!=image->palette[3*i+2]) ) { grey=0; break; }
  if (!grey) return;

  if (DEBUG) { sprintf(temp_err_string, "Image is greyscale"); ppl_log(temp_err_string); }

  if (ncols<=16) // Investigate whether we can reduce colour depth below 8
   {
    if (ncols==2) // We have only two colours
     {
      magic=255;
      depth=1;
     }
    else if (ncols<=4) // We have only four colours
     {
      magic=255/3;
      depth=2;
     }
    else // We have sixteen colours
     {
      magic=255/15;
      depth=4;
     }
    test=1; // Test whether the colours we have are all evenly spaced between 0 and 255
    for (i=0; i<ncols; i++)
     {
      j = (image->palette[3*i]*magic)&255;
      if ((j!=0)&&(j!=1)&&(j!=255)) { test=0; break; }
     }
    if (test) // If so, we can reduce colour depth
     {
      for (i=0; i<ncols; i++) image->palette[3*i] /= magic;
      if (DEBUG) { sprintf(temp_err_string,"Greyscale depth is %d bit",depth); ppl_log(temp_err_string); }
     } else { // We have a <=4 bit image, but cannot use a 4-bit greyscale representation. Give up and use palette!
      if (DEBUG) { sprintf(temp_err_string, "ncols=%d, but not %d bit greyscale\n", ncols, depth); ppl_log(temp_err_string); }
      return;
     }
   }

  // See if palette is already ordered
  for (i=0; i<image->pal_len; i++) if (image->palette[3*i] != i) { grey=0; break; }

  if (!grey) // Need to order things
   {
    if (DEBUG) ppl_log("Greyscale palette being reordered");
    for (i=0; i<size; i++) image->data[i] = image->palette[3*image->data[i]];
    if (image->trans != NULL) image->trans[0] = image->palette[3*image->trans[0]]; // Reorder transparent colour as well
   }

  // Now we can ditch palette etc.
  image->palette = NULL;
  if (depth <= 4)
   {
    image->pal_len = 1<<depth; // Fudge settings so that bmp_compact is happy
    image->type    = BMP_COLOUR_PALETTE;
    bmp_compact(image);
   }
  image->type    = BMP_COLOUR_BMP;
  image->colour  = BMP_COLOUR_GREY;
  image->pal_len = 0;
  return;
 }

void bmp_compact(bitmap_data *image)
 {
  int ncols,i,j,height,width;
  unsigned char *p,*p2,ctmp;

  if (image->type    != BMP_COLOUR_PALETTE) return;
  if (image->depth   !=                  8) return;
  if (image->pal_len >                  16) return;

  ncols  = image->pal_len;
  height = image->height;
  width  = image->width;

  if (ncols==2)
   {
    if (DEBUG) ppl_log("Compacting to 1 bit per pixel");
    image->depth = 1;
    p = p2 = image->data;
    for (i=0; i<height; i++)
     {
      for (j=0; j<width-7; j+=8, p2+=8) *p++ = (*p2<<7) + (*(p2+1)<<6) + (*(p2+2)<<5) + (*(p2+3)<<4) + (*(p2+4)<<3) + (*(p2+5)<<2) + (*(p2+6)<<1) + *(p2+7);
      if (j!=width)
       {
        *p=0;
        for ( ; j<width; j++) *p = (*p<<1) + *p2++;
        *p   = *p<<(8-(width&7));
        ctmp = *(p-1) & ((1<<(8-(width&7)))-1); // this might help the rle
        *p  |= ctmp;                            // ditto
        p++;
       }
     }
    image->data_len = ((long)height)*((width+7)>>3);
   }
  else if (ncols<=4)
   {
    if (DEBUG) ppl_log("Compacting to 2 bits per pixel");
    image->depth = 2;
    p = p2 = image->data;
    for (i=0; i<height; i++)
     {
      for (j=0; j<width-3; j+=4, p2+=4) *p++ = (*p2<<6) + (*(p2+1)<<4) + (*(p2+2)<<2) + *(p2+3);
      if (j!=width)
       {
        *p=0;
        for ( ; j<width; j++) *p = (*p<<2) + *p2++;
        *p   = *p<<(2*(4-(width&3)));
        ctmp = *(p-1) & ((1<<(2*(4-(width&3))))-1); // this might help the rle
        *p  |= ctmp;                                // ditto
        p++;
       }
     }
    image->data_len = ((long)height)*((width+3)>>2);
   }
  else if (ncols<=16)
   {
    if (DEBUG) ppl_log("Compacting to 4 bits per pixel");
    image->depth = 4;
    p = p2 = image->data;
    for (i=0; i<height; i++)
     {
      for(j=0;j<(width-1);j+=2,p2+=2) *(p++) = (*p2<<4) + *(p2+1);
      if (j == (width-1))
       {
        *p  = (*(p2++))<<4;
        *p |= *(p-1)&0xf;   // to help rle
        p++;
       }
     }
    image->data_len = ((long)height)*((width+1)>>1);
   }

  return;
 }

