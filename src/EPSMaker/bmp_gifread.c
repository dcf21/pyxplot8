// bmp_gifread.c
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

#define _PPL_BMP_GIFREAD_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ListTools/lt_memory.h"

#include "ppl_error.h"

#include "eps_image.h"
#include "bmp_gifread.h"

void bmp_gifread(FILE *in, bitmap_data *image)
 {
  unsigned char buff[8],flags,len,*rawz;
  int gcm,ncols,interlaced;
  long lxoff,lyoff,lw,lh,lzwcs;
  unsigned long datalen,width,height;
  static unsigned char trans;

  if (DEBUG) ppl_log("Beginning to decode GIF image file");

  fread(buff,3,1,in);
  buff[3]=0;

  if      (!strcmp((char *)buff,"87a")) { if (DEBUG) ppl_log("GIF image in format GIF87a"); }
  else if (!strcmp((char *)buff,"89a")) { if (DEBUG) ppl_log("GIF image in format GIF89a"); }
  else                                  { ppl_error(ERR_FILE,"GIF image file is in an unrecognised format"); return; }

  fread(buff,4,1,in);
  width  = ((unsigned)buff[1]<<8) + buff[0];
  height = ((unsigned)buff[3]<<8) + buff[2];

  if (DEBUG) { sprintf(temp_err_string, "Size %ldx%ld",width,height); ppl_log(temp_err_string); }

  fread(&flags,1,1,in);

  if (DEBUG) { sprintf(temp_err_string, "Global colour map=%d",(flags&0x80)>>7); ppl_log(temp_err_string); }
  if (DEBUG) { sprintf(temp_err_string, "Colour resolution=%d",(flags&0x70)>>4); ppl_log(temp_err_string); }
  if (DEBUG) { sprintf(temp_err_string, "Colour depth     =%d",(flags&0x7)); ppl_log(temp_err_string); }

  gcm=((flags&0x80)>>7);
  ncols=2<<(flags&0x7);

  if (DEBUG) { sprintf(temp_err_string, "Global number of colours=%d",ncols); ppl_log(temp_err_string); }

  fread(buff,2,1,in);

  if (gcm)
   {
    image->palette = (unsigned char *)lt_malloc(ncols*3);
    if (image->palette == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    fread(image->palette,ncols*3,1,in);
    image->pal_len = ncols;
   }

  fread(&flags,1,1,in);

  while (flags==0x21)
   {
    fread(&flags,1,1,in);
    if (DEBUG) { sprintf(temp_err_string, "Extension block %d",(int)flags); ppl_log(temp_err_string); }
    if ((int)flags==249){
      fread(&flags,1,1,in);
      if (flags!=4) { ppl_error(ERR_FILE, "GIF image file has an unexpected extension length"); return; }
      fread(&flags,1,1,in);
      if (flags&1)
       {
        image->trans = &trans;
        fseek(in,2,SEEK_CUR);
        fread(&flags,1,1,in);
        *image->trans = flags;
        if (DEBUG) { sprintf(temp_err_string, "Transparent colour index at %d", (int)flags); ppl_log(temp_err_string); }
       }
      else
       { fseek(in,3,SEEK_CUR); }
      fread(&flags,1,1,in);
      if (flags) { ppl_error(ERR_FILE, "GIF image file has unexpected extension data"); return; }
     }
    else
     {
      do
       {
        if (!fread(&flags,1,1,in)) { ppl_error(ERR_FILE, "GIF image file ends prematurely"); return; }
        fseek(in,(long)flags,SEEK_CUR);
       }
      while(flags);
     }
    fread(&flags,1,1,in);
   }

  if ((flags==0x2c) && DEBUG) { sprintf(temp_err_string, "Local descriptor"); ppl_log(temp_err_string); }

  fread(buff,4,1,in);
  lxoff = ((unsigned)buff[1]<<8) + buff[0];
  lyoff = ((unsigned)buff[3]<<8) + buff[2];

  if (DEBUG) { sprintf(temp_err_string, "Local offset %ldx%ld",lxoff,lyoff); ppl_log(temp_err_string); }

  fread(buff,4,1,in);
  lw = ((unsigned)buff[1]<<8) + buff[0];
  lh = ((unsigned)buff[3]<<8) + buff[2];

  if (DEBUG) { sprintf(temp_err_string, "Local size %ldx%ld",lw,lh); ppl_log(temp_err_string); }

  fread(&flags,1,1,in);

  if (flags&0x80)
   {
    if (DEBUG) { sprintf(temp_err_string, "Local colour map"); ppl_log(temp_err_string); }
    ncols=2<<(flags&0x7);
    if (DEBUG) { sprintf(temp_err_string, "Local number of colours=%d",ncols); ppl_log(temp_err_string); }
    if (gcm) free (image->palette);
    image->palette=malloc(ncols*3);
    if (image->palette == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    fread(image->palette,ncols*3,1,in);
    image->pal_len=ncols;
   }

  interlaced=flags&0x40;

  if (interlaced && DEBUG) { sprintf(temp_err_string, "Interlaced"); ppl_log(temp_err_string); }

  if (lxoff || lyoff || (lw!=width) || (lh!=height)) { ppl_error(ERR_FILE, "GIF image file cannot be processed because local and global sizes differ; file appears to be corrupt"); return; }

  fread(&flags,1,1,in);
  lzwcs = flags+1;
  if (DEBUG) { sprintf(temp_err_string, "Initial code size=%ld",lzwcs); ppl_log(temp_err_string); }

  rawz = (unsigned char *)lt_malloc((width*height*3)/2);
  if (rawz == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  datalen = 0;
  fread(&len,1,1,in);
  while(len)
   {
    fread(rawz+datalen,len,1,in);
    datalen += len;
    fread(&len,1,1,in);
   }
  if (DEBUG) { sprintf(temp_err_string, "Total GIF data length=%ld",datalen); ppl_log(temp_err_string); }

  image->data = (unsigned char *)lt_malloc(width*height);
  if (image->data == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }

  datalen = bmp_de_lzw(rawz,image->data,width*height,lzwcs);

  if (datalen == 0) { image->data = NULL; return; } // Subroutine failed
  if (datalen != width*height) { sprintf(temp_err_string, "Decoding error whilst processing GIF image file. Expecting %ld bytes of decoded data, but received %ld.",width*height,datalen); ppl_error(ERR_FILE, temp_err_string); return; }

  image->data_len = width*height;
  image->height   = height;
  image->width    = width;
  image->type     = BMP_COLOUR_PALETTE;
  image->colour   = BMP_COLOUR_PALETTE;
  image->depth    = 8;

  if (interlaced)
   {
    if (bmp_de_gifinterlace(image)) { image->data = NULL; return; } // Subroutine failed
   }
  return;
 }

int bmp_de_gifinterlace(bitmap_data *image)
 {
  int i,j;
  unsigned char *out,*in,*outp;

  out = (unsigned char *)lt_malloc(image->height*image->width);
  if (out == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return 1; }

  in = image->data;

  // First pass is eights
  outp = out;
  for (i=0; i<image->height; i+=8)
   {
    for (j=0; j<image->width; j++)
     *outp++ = *in++;
    outp += 7*image->width;
   }

  // Next is fours
  outp = out + 4*image->width;
  for (i=4; i<image->height; i+=8)
   {
    for (j=0; j<image->width; j++)
     *outp++ = *in++;
    outp += 7*image->width;
   }

  // Next is twos
  outp = out+2*image->width;
  for (i=2; i<image->height; i+=4)
   {
    for (j=0; j<image->width; j++)
     *outp++ = *in++;
    outp += 3*image->width;
   }

  // Next is ones
  outp=out+image->width;
  for (i=1; i<image->height; i+=2)
   {
    for (j=0; j<image->width; j++)
     *outp++ = *in++;
    outp += image->width;
   }

  image->data=out;
  return 0;
 }

#define MAXCS 12

unsigned long bmp_de_lzw(unsigned char *buff, unsigned char *out, unsigned long len, int cs)
 {
  unsigned char store[256], *start, *end;
  unsigned int off,tpos,eoi,clr;
  struct str {unsigned char *s; unsigned len;} table[1<<MAXCS];
  int tmax=(1<<MAXCS)-1,ccs,i,j;

  start = out;
  end   = out+len;

  if (cs>MAXCS) { sprintf(temp_err_string, "Whilst decoding GIF image file, encountered de_lzw error: initial token size of %d too large",cs); ppl_error(ERR_FILE, temp_err_string); return 0; }

  // Init table
  off  = 0;
  ccs  = cs;
  clr  = (1<<(cs-1));
  eoi  = clr+1;
  tpos = clr+2;

  for (i=0; i<clr; i++)
   {
    table[i].len = 1;
    table[i].s   = store+i;
    store[i]     = (unsigned char)i;
   }

  i = bmp_de_lzw_bits(buff,0,ccs);
  if (i!=clr) { sprintf(temp_err_string, "Whilst decoding GIF image file, encountered de_lzw error: ClearCode not first code, but instead got %x",i); ppl_error(ERR_FILE, temp_err_string); return 0; }

  while(1)
   {
    i = bmp_de_lzw_bits(buff+(off>>3),off&7,ccs);
    off += ccs;

    if (i==clr)
     {
      ccs  = cs;
      tpos = clr+2;
      continue;
     }

    if (i==eoi) return(out-start);

    if (i>=tpos) { sprintf(temp_err_string, "Whilst decoding GIF image file, encountered de_lzw error: token erroneously large"); ppl_error(ERR_FILE, temp_err_string); return 0; }
    if (out>end) { sprintf(temp_err_string, "Whilst decoding GIF image file, encountered de_lzw error: output buffer full"); ppl_error(ERR_FILE, temp_err_string); return 0; }

    if (tpos<=tmax)
     {
      table[tpos].len = table[i].len + 1;
      table[tpos].s   = out;
      tpos++;
     }

    for (j=0; (j<table[i].len) && (out<=end); j++) *(out++) = *(table[i].s+j);

    if ((tpos==(1<<ccs)+1) && (ccs<MAXCS)) ccs++;
   }
  // Should never get here
  sprintf(temp_err_string, "Whilst decoding GIF image file, encountered unidentified de_lzw error"); ppl_error(ERR_FILE, temp_err_string);
  return 0;
 }

unsigned int bmp_de_lzw_bits(unsigned char *c,int st, int len)
 {
  unsigned tmp,mask;

  tmp  = ((unsigned)(*(c+2))<<16) + ((unsigned)(*(c+1))<<8) + *(c);
  mask = ((1<<len)-1)<<st;
  tmp &= mask;
  return(tmp>>st);
 }

