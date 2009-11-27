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
// Michael Rutter. It reads in data from JPEG files without performing any
// decompression, since the DCT-compressed data can be rewritten straight out
// to postscript for decompression by the postscript rasterising engine.

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
  int i,j,ncols,debug,os2,rle;

  debug=image->flags;
  if (debug) fprintf(stderr,"BMP file detected\n");

  off2=3;
  rle=size=ncols=0;

  /* Read rest of first header */
  fread(buff,11,1,in);
  off2+=11;

  if ((buff[3])||(buff[4])||(buff[5])||(buff[6])){
    fprintf(stderr,"Not a valid BMP file\n");
    exit(1);
  }

  offset=buff[7]+(buff[8]<<8)+(buff[9]<<16)+(buff[10]<<24);

  /* Read info header */

  fread(buff,12,1,in);
  off2+=12;
  encode=0;

  if ((buff[0]==12)&&(buff[1]==0)){ /* OS/2 style BMP */
    os2=1;
    if ((buff[8]!=1)||(buff[9])){
      fprintf(stderr,"Not a valid OS/2 BMP file\n");
      exit(1);
    }

    width=buff[4]+((unsigned)buff[5]<<8);
    height=buff[6]+((unsigned)buff[7]<<8);
    depth=buff[10];
  }
  else{ /* Must be Windows style BMP */
    os2=0;
    if ((buff[0]!=40)||(buff[1])||(buff[2])||(buff[3])){
      fprintf(stderr,"Not a valid Windows BMP file\n");
      exit(1);
    }

    fread(buff+12,40-12,1,in);
    off2+=40-12;

    width=buff[4]+((unsigned)buff[5]<<8)+((unsigned)buff[6]<<16)
      +((unsigned)buff[6]<<24);
    height=buff[8]+((unsigned)buff[9]<<8)+((unsigned)buff[10]<<16)
      +((unsigned)buff[11]<<24);

    if ((buff[12]!=1)||(buff[13])||(buff[15])){
      fprintf(stderr,"Not a valid BMP file\n");
      exit(1);
    }

    depth=buff[14];
    encode=buff[16];
    size=buff[20]+(((int)(buff[21]))<<8)+(((int)(buff[22]))<<16)+
      (((int)(buff[22]))<<24);
  }

  if (debug) fprintf(stderr,"Size %dx%d depth %d bits\n",width,height,depth);

  if (encode!=0){
    if ((encode==1)&&(depth==8)) rle=8;
    else if ((encode==2)&&(depth==4)) rle=4;
    else if ((encode!=3)||(depth!=16)){
      fprintf(stderr,"Encoding not supported\n");
      exit(1);
    }
  }

  if ((depth!=1)&&(depth!=4)&&(depth!=8)&&(depth!=16)&&(depth!=24)){
    fprintf(stderr,"Depth of %d not supported\n",depth);
    exit(1);
  }

  if(depth<=8){ /* We have a palette to read */

    if (!os2) ncols=buff[32]+(buff[33]<<8)+(buff[34]<<16)+(buff[35]<<24);
    if (!ncols) ncols=1<<depth;
      
    if (ncols>(1<<depth)){
      fprintf(stderr,"Inconsistent palette length of %d.\n",ncols);
      exit(1);
    }

    image->pal_len=ncols;
    image->palette=malloc(3*image->pal_len);
    off2+=(4-os2)*image->pal_len;
    if (!image->palette){fprintf(stderr,"Malloc error\n");exit(1);}
    p=image->palette;
    for(i=0;i<image->pal_len;i+=2){
      fread(buff,8-2*os2,1,in);  /* MS Win uses BGR0, OS/2 uses BGR */
      *(p++)=buff[2];
      *(p++)=buff[1];
      *(p++)=buff[0];
      *(p++)=buff[6-os2];
      *(p++)=buff[5-os2];
      *(p++)=buff[4-os2];
    }
    image->type=PALETTE;
    image->colour=PALETTE;
  }else{
    image->type=BMP;
    image->colour=RGB;
  }

  if ((depth==16)&&(encode==3)){ /* Must read 12 byte pseudopalette */
    fread(buff+40,12,1,in);
    off2+=12;
  }

  if (offset<off2){
    fprintf(stderr,"Error in BMP format. %d %d\n",offset,off2);
    exit(1);
  }
  else if (offset>off2) {
    if (debug) fprintf(stderr,"%d bytes of extra data\n",offset-off2);
    for (;off2<offset;off2++) fgetc(in);
  }

  image->height=height;
  image->width=width;

  dw=(width*depth+7)/8;
  image->data_len=dw*height;
  image->depth=depth;

  if (depth==16){
    bmp16read(in, buff, image);
    return;
  }

  dw=(width*depth+7)/8;
  image->data_len=dw*height;
  image->depth=depth;

  if(!rle){
    image->data=malloc(dw*height);
    if (!image->data){fprintf(stderr,"Malloc error\n");exit(1);}
    for (i=1;i<=height;i++){
      fread(image->data+(height-i)*dw,dw,1,in);
      if (dw&3) fread(buff,4-(dw&3),1,in); /* Lines are dword aligned */
    }
    
    if(depth==24){  /* BMP is BGR not RGB ... */
      p=image->data;
      for(i=0;i<height;i++){
	for(j=0;j<width;j++){
	  c=*p;
	  *p=*(p+2);
	  *(p+2)=c;
	  p+=3;
	}
      }
    }
  }else{  /* if (rle) */
    p=malloc(size);
    if (!p){fprintf(stderr,"Malloc error\n");exit(1);}
    fread(p,size,1,in);
    de_msrle(image,p,size);
    free(p);
  }
  
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

  if (header[16]==3){ /* Read "palette" and check correct
		       * Palette can be 7c00 03e0 001f (15 bit)
		       * or             f800 07e0 001f (16 bit)
		       * as represents RGB bitmasks. However,
		       * Intel byte ordering afflicts the above,
                       * and the entries are stored as dwords,
                       * not words. */
    palette=header+40;
    if ((palette[0]!=0)||(palette[4]!=0xe0)|| /* could check more than this */
	(palette[8]!=0x1f)||(palette[9]!=0)){
      fprintf(stderr,"Unexpected palette in 16 bit bmp. Exiting.\n");
      exit(1);
    }
    if (palette[1]&0x80) is15=0;
  }else if (header[30]!=0){
    fprintf(stderr,"Unexpected compression type %d for 16 bit bmp."
	    " Exiting.\n",(int)header[30]);
    exit(1);
  }

  if (image->flags>1){
    if (is15) fprintf(stderr,"15 bit 555\n");
    else fprintf(stderr,"16 bit 565\n");
  }

  image->depth=24;
  image->data_len=3*width*height;
  image->data=malloc(3*width*height);
  if (!image->data){fprintf(stderr,"Malloc error\n");exit(1);}

  rowptr=malloc(2*width);
  if (!rowptr){fprintf(stderr,"Malloc error\n");exit(1);}

  for(i=1;i<=height;i++){
    fread(rowptr,2*width,1,in);
    ptr=image->data+(height-i)*3*width;
    for(j=0;j<width;j++){
      colour=rowptr[2*j]+(rowptr[2*j+1]<<8);
      *(ptr+2)=(colour&0x1f)<<3; /* blue */
      if (is15){                 /* green */
	*(ptr+1)=(colour&0x03e0)>>2;
      }else{
	*(ptr+1)=(colour&0x07e0)>>3;
	colour>>=1;
      }
      *ptr=(colour&0x7c00)>>7;  /* red */
      ptr+=3;
    }
    if (width&1) fread(rowptr,2,1,in); /* rows are dword aligned */
  }
}
