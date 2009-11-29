// eps_image.c
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

#define _PPL_EPS_IMAGE 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <zlib.h>

#include "ListTools/lt_memory.h"

#include "ppl_error.h"
#include "ppl_setting_types.h"

#include "bmp_a85.h"
#include "bmp_bmpread.h"
#include "bmp_jpegread.h"
#include "bmp_optimise.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_image.h"
#include "eps_settings.h"

void eps_image_RenderEPS(EPSComm *x)
 {
  bitmap_data   data;
  FILE         *infile;
  int           ImageType, i, j;
  double        xscale, yscale, r;
  unsigned char buff[10], *imagez;
  uLongf        zlen; // Length of buffer passed to zlib

  data.data = data.palette = data.trans = NULL;
  data.type = 0;
  data.XDPI = data.YDPI = 180;
  data.TargetCompression = BMP_ENCODING_FLATE;

  fprintf(x->epsbuffer, "%% Canvas item %d [bitmap image]\n", x->current->id);

  // Open input data file
  infile = fopen(x->current->text, "r");
  if (infile==NULL) { sprintf(temp_err_string, "Could not open input file '%s'", x->current->text); ppl_error(ERR_FILE, temp_err_string); *(x->status) = 1; return; }

  // Use magic to determine file type
  for (i=0; i<3; i++)
   {
    j = fgetc(infile);
    if (j==EOF) { sprintf(temp_err_string, "Could not read any image data from the input file '%s'", x->current->text); ppl_error(ERR_FILE, temp_err_string); *(x->status) = 1; return; }
    buff[i] = (unsigned char)j;
   }
  if      ((buff[0]=='G' )&&(buff[1]=='I' )&&(buff[2]=='F' )) ImageType = SW_BITMAP_GIF;
  else if ((buff[0]==0xff)&&(buff[1]==0xd8)&&(buff[2]==0xff)) ImageType = SW_BITMAP_JPG;
  else if ((buff[0]=='B' )&&(buff[1]=='M' )                 ) ImageType = SW_BITMAP_BMP;
  else if ((buff[0]==137 )&&(buff[1]=='P' )&&(buff[2]=='N' )) ImageType = SW_BITMAP_PNG;
  else
   {
    sprintf(temp_err_string, "Could not determine the file type of input file '%s'. The image command only supports bmp, gif, jpeg and png images. The supplied image does not appear to be in any of these formats.", x->current->text);
    ppl_error(ERR_FILE, temp_err_string);
    *(x->status) = 1;
    return;
   }

  // Read data from file using appropriate input filter
  switch (ImageType)
   {
    case SW_BITMAP_BMP: bmp_bmpread (infile , &data); break;
    case SW_BITMAP_GIF: return; break;
    case SW_BITMAP_JPG: bmp_jpegread(infile , &data); break;
    case SW_BITMAP_PNG: return; break;
    default: ppl_error(ERR_INTERNAL, "Unrecognised image type"); *(x->status) = 1; return;
   }

  // Check to see whether reading of data failed
  if (data.data == NULL) return;

  // Apply palette optimisations to images if possible
  if ((data.depth ==  8) && (data.type==BMP_COLOUR_PALETTE)) bmp_palette_check(&data); // If we did not construct palette, check for trailing unused entries
  if ((data.depth == 24) && (data.type==BMP_COLOUR_BMP    )) bmp_colour_count(&data);  // Check full colour image to ensure more than 256 colours
  if ((data.depth ==  8) && (data.type==BMP_COLOUR_PALETTE)) bmp_grey_check(&data);    // Check paletted images for greyscale conversion
  if ((data.type == BMP_COLOUR_PALETTE) && (data.pal_len <= 16) && (data.depth == 8)) bmp_compact(&data); // Compact images with few palette entries

  // Apply compression to image data
  switch (data.TargetCompression)
   {
    case BMP_ENCODING_NULL: break; // No image compression
    case BMP_ENCODING_DCT: break; // Special case: JPEG data is encoded in DCT, but is already supplied from bmp_jpegread() in encoded form
    case BMP_ENCODING_FLATE:
      zlen   = data.data_len*1.01+12; // Nasty guess at size of buffer needed.
      imagez = (unsigned char *)lt_malloc(zlen);
      if (imagez == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
      if (DEBUG) { ppl_log("Calling zlib to compress image data"); }
      j = compress2(imagez,&zlen,data.data,data.data_len,9); // Call zlib to do deflation

      if (j!=0)
       {
        if (DEBUG) { sprintf(temp_err_string, "zlib returned error code %d\n",j); ppl_log(temp_err_string); }
        data.TargetCompression = BMP_ENCODING_NULL; // Give up trying to compress data
        break;
       }
      if (DEBUG) { sprintf(temp_err_string, "zlib has completed compression. Before flate: %ld bytes. After flate: %ld bytes", data.data_len, (long)zlen); ppl_log(temp_err_string); }
      if (zlen >= data.data_len)
       {
        data.TargetCompression = BMP_ENCODING_NULL; // Give up trying to compress data; result was larger than original data size
        break;
       }
      data.data = imagez; // Replace old data with new compressed data
      data.data_len = zlen;
      break;
    default:
      ppl_error(ERR_INTERNAL, "Unrecognised image compression type requested"); *(x->status) = 1; return;
   }

  // Work out dimensions of image
  if ((x->current->xpos2set) && (x->current->ypos2set)) // Both width and height have been specified
   {
    xscale = x->current->xpos2 * M_TO_PS;
    yscale = x->current->ypos2 * M_TO_PS;
   }
  else if (x->current->xpos2set) // Only width has been set
   {
    xscale = x->current->xpos2 * M_TO_PS;
    yscale = x->current->xpos2 * M_TO_PS * ((double)data.height / (double)data.width);
   }
  else if (x->current->ypos2set) // Only height has been set
   {
    xscale = x->current->ypos2 * M_TO_PS * ((double)data.width / (double)data.height);
    yscale = x->current->ypos2 * M_TO_PS;
   }
  else // Neither height nor width has been set; use DPI information to work out how big image should be
   {
    xscale = ((double)data.width ) / data.XDPI * 72;
    yscale = ((double)data.height) / data.YDPI * 72;
   }

  // Make it into postscript
  fprintf(x->epsbuffer, "gsave\n");
  fprintf(x->epsbuffer, "%.2f %.2f translate\n", x->current->xpos * M_TO_PS, x->current->ypos * M_TO_PS);
  fprintf(x->epsbuffer, "%.2f rotate\n", x->current->rotation * 180 / M_PI);
  fprintf(x->epsbuffer, "%.2f %.2f scale\n", xscale, yscale);

  if      (data.colour == BMP_COLOUR_RGB ) fprintf(x->epsbuffer, "/DeviceRGB setcolorspace\n");  // RGB palette
  else if (data.colour == BMP_COLOUR_GREY) fprintf(x->epsbuffer, "/DeviceGray setcolorspace\n"); // Greyscale palette
  else if (data.colour == BMP_COLOUR_PALETTE) // Indexed palette
   {
    fprintf(x->epsbuffer, "[/Indexed /DeviceRGB %d <~\n", data.pal_len-1);
    bmp_A85(x->epsbuffer, data.palette, 3*data.pal_len);
    fprintf(x->epsbuffer, "] setcolorspace\n\n");
   }

  fprintf(x->epsbuffer, "<<\n /ImageType %d\n /Width %d\n /Height %d\n /ImageMatrix [%d 0 0 %d 0 %d]\n", (data.trans!=NULL)?4:1, data.width, data.height, data.width, -data.height, data.height);
  fprintf(x->epsbuffer, " /DataSource currentfile /ASCII85Decode filter");
  if      (data.TargetCompression == BMP_ENCODING_DCT  ) fprintf(x->epsbuffer, " /DCTDecode filter");
  else if (data.TargetCompression == BMP_ENCODING_FLATE) fprintf(x->epsbuffer, " /FlateDecode filter");
  fprintf(x->epsbuffer, "\n /BitsPerComponent %d\n /Decode [0 %d%s]\n", (data.colour==BMP_COLOUR_RGB)?(data.depth/3):(data.depth),
                                                                        (data.type==BMP_COLOUR_PALETTE)?((1<<data.depth)-1):1,
                                                                        (data.colour==BMP_COLOUR_RGB)?" 0 1 0 1":"");
  if (x->current->smooth) fprintf(x->epsbuffer, " /Interpolate true\n");
  fprintf(x->epsbuffer, ">> image\n");
  bmp_A85(x->epsbuffer, data.data, data.data_len);
  fprintf(x->epsbuffer, "grestore\n");

  // Update postscript bounding box
  r = x->current->rotation;
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS                                  , x->current->ypos*M_TO_PS                                 , 0);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS + xscale*cos(r)                  , x->current->ypos*M_TO_PS + xscale*sin(r)                 , 0);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS                 + yscale*-sin(r) , x->current->ypos*M_TO_PS                 + yscale*cos(r) , 0);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS + xscale*cos(r) + yscale*-sin(r) , x->current->ypos*M_TO_PS + xscale*sin(r) + yscale*cos(r) , 0);

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

