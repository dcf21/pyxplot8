// bmp_bmpread.c
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
// Michael Rutter. It reads in data from Windows and OS/2 bitmap files.

#define _PPL_BMP_BMPREAD_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ListTools/lt_memory.h"

#include "ppl_error.h"

#include "eps_image.h"
#include "bmp_bmpread.h"

void bmpread(FILE *in, bitmap_data *image)
 {
  unsigned char buff[60],encode,*p,c;
  unsigned width,height,depth,dw,offset,off2,size;
  int i,j,ncols,os2,rle;

  if (DEBUG) ppl_log("Beginning to decode BMP image file");

  off2=3;

  // Read rest of first header
  fread(buff,11,1,in);
  off2 += 11;

  if ((buff[3])||(buff[4])||(buff[5])||(buff[6])) { ppl_error(ERR_FILE,"This bitmap file appears to be corrupted"); return; }

  offset = buff[7] + (buff[8]<<8) + (buff[9]<<16) + (buff[10]<<24);

  // Read info header
  fread(buff,12,1,in);
  off2 += 12;

  if ((buff[0]==12)&&(buff[1]==0)) // OS/2 bitmap
   {
    os2=1;
    if ((buff[8]!=1)||(buff[9])) { ppl_error(ERR_FILE,"This OS/2 bitmap file appears to be corrupted"); return; }
    width  = buff[4] + ((unsigned)buff[5]<<8);
    height = buff[6] + ((unsigned)buff[7]<<8);
    depth  = buff[10];
    encode = 0;
    size   = 0;
   }
  else // Windows bitmap
   {
    os2=0;
    if ((buff[0]!=40)||(buff[1])||(buff[2])||(buff[3])) { ppl_error(ERR_FILE,"This Windows bitmap file appears to be corrupted"); return; }

    fread(buff+12,40-12,1,in);
    off2 += 40-12;

    width  = buff[4] + ((unsigned)buff[5]<<8) + ((unsigned)buff[ 6]<<16) + ((unsigned)buff[ 7]<<24);
    height = buff[8] + ((unsigned)buff[9]<<8) + ((unsigned)buff[10]<<16) + ((unsigned)buff[11]<<24);

    if ((buff[12]!=1)||(buff[13])||(buff[15])) { ppl_error(ERR_FILE,"This Windows bitmap file appears to be corrupted"); return; }

    depth  = buff[14];
    encode = buff[16];
    size   = buff[20] + (((int)(buff[21]))<<8) + (((int)(buff[22]))<<16) + (((int)(buff[23]))<<24);
  }

  if (DEBUG) sprintf(temp_err_string, "Size %dx%d depth %d bits",width,height,depth); ppl_log(temp_err_string); }

  if (encode!=0)
   {
    rle=0;
    if      ((encode==1)&&(depth== 8)) rle=8;
    else if ((encode==2)&&(depth== 4)) rle=4;
    else if ((encode!=3)||(depth!=16)) { ppl_error(ERR_FILE,"This Windows bitmap file has an invalid encoding"); return; }
   }

  if ((depth!=1)&&(depth!=4)&&(depth!=8)&&(depth!=16)&&(depth!=24)) { sprintf(temp_err_string, "Bitmap colour depth of %d not supported\n",depth); ppl_error(ERR_FILE, temp_err_string); return; }

  if (depth<=8) // We have a palette to read
   {
    ncols = 0;
    if (!os2  ) ncols = buff[32] + (buff[33]<<8) + (buff[34]<<16) + (buff[35]<<24);
    if (!ncols) ncols = 1<<depth;

    if (ncols > (1<<depth)) { sprintf(temp_err_string, "Bitmap image has a palette length of %d, which is not possible with a colour depth of %d", ncols, depth); ppl_error(ERR_FILE, temp_err_string); return; }

    image->pal_len = ncols;
    image->palette = lt_malloc(3*image->pal_len);
    off 2+= (4-os2)*image->pal_len;
    if (image->palette == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    p = image->palette;
    for (i=0; i<image->pal_len; i+=2)
     {
      fread(buff,8-2*os2,1,in);  // MS Windows uses BGR0, OS/2 uses BGR
      *(p++)=buff[2];
      *(p++)=buff[1];
      *(p++)=buff[0];
      *(p++)=buff[6-os2];
      *(p++)=buff[5-os2];
      *(p++)=buff[4-os2];
     }
    image->type   = BMP_COLOUR_PALETTE;
    image->colour = BMP_COLOUR_PALETTE;
   } else {
    image->type   = BMP_COLOUR_BMP;
    image->colour = BMP_COLOUR_RGB;
   }

  if ((depth==16)&&(encode==3)) // Must read 12 byte pseudopalette
   {
    fread(buff+40,12,1,in);
    off2 += 12;
   }

  if      (offset<off2) { ppl_error(ERR_FILE, "This bitmap file appears to be corrupted"); return; }
  else if (offset>off2)
   {
    if (DEBUG) { sprintf(temp_err_string, "%d bytes of extra data", offset-off2); ppl_log(temp_err_string); }
    for ( ; off2<offset ; off2++) fgetc(in);
   }

  image->height = height;
  image->width  = width;

  dw              = (width*depth+7)/8;
  image->data_len = dw*height;
  image->depth    = depth;

  if (depth==16)
   {
    bmp16read(in, buff, image);
    return;
  }

  if (!rle)
   {
    image->data = lt_malloc(dw*height);
    if (image->data==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    for (i=1 ; i<=height ; i++)
     {
      fread(image->data+(height-i)*dw,dw,1,in);
      if (dw&3) fread(buff,4-(dw&3),1,in); // Lines are dword aligned
    }

    if (depth==24) // BMP is BGR not RGB
     {
      p=image->data;
      for(i=0; i<height; i++)
       {
        for(j=0; j<width; j++)
         {
          c=*p;
          *p=*(p+2);
          *(p+2)=c;
          p+=3;
         }
       }
     }
   } else {  // if (rle)
    p = lt_malloc(size);
    if (p==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    fread(p,size,1,in);
    de_msrle(image,p,size);
   }
  return;
 }

void bmp16read(FILE *in, unsigned char *header, bitmap_data *image)
 {
  unsigned char *palette;
  unsigned char *rowptr,*ptr;
  unsigned int width,height,colour;
  int is15,i,j;

  is15=1;
  width=image->width;
  height=image->height;

  if (header[16]==3)  // Read "palette" and check correct
   {                  // Palette can be 7c00 03e0 001f (15 bit)
                      // or             f800 07e0 001f (16 bit)
                      // as represents RGB bitmasks. However,
                      // Intel byte ordering afflicts the above,
                      // and the entries are stored as dwords,
                      // not words.
    palette = header+40;
    if ((palette[0]!=0) || (palette[4]!=0xe0) || (palette[8]!=0x1f) || (palette[9]!=0)) { ppl_error(ERR_FILE, "This 16-bit bitmap file appears to be corrupted"); return; }

    if (palette[1]&0x80) is15=0;
   } else if (header[30]!=0) { ppl_error(ERR_FILE, "This 16-bit bitmap file has invalid compression type"); return; }

  if (DEBUG)
   {
    if (is15) ppl_log("15 bit 555");
    else      ppl_log("16 bit 565");
   }

  image->depth    = 24;
  image->data_len = 3*width*height;
  image->data     = lt_malloc(3*width*height);
  if (image->data==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  rowptr = lt_malloc(2*width);
  if (rowptr==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); image->data=NULL; return; }

  for (i=1; i<=height; i++)
   {
    fread(rowptr,2*width,1,in);
    ptr = image->data+(height-i)*3*width;
    for (j=0; j<width; j++)
     {
      colour   = rowptr[2*j] + (rowptr[2*j+1]<<8);
      *(ptr+2) = (colour&0x1f)<<3; // blue
      if (is15)                    // green
       {
        *(ptr+1) = (colour&0x03e0)>>2;
       } else {
        *(ptr+1) = (colour&0x07e0)>>3;
        colour>>=1;
       }
      *ptr = (colour&0x7c00)>>7;  // red
      ptr += 3;
     }
    if (width&1) fread(rowptr,2,1,in); // rows are dword aligned
   }
  return;
 }

