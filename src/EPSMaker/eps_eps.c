// eps_eps.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
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

#define _PPL_EPS_EPS 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "ListTools/lt_memory.h"

#include "StringTools/asciidouble.h"

#include "ppl_error.h"
#include "ppl_glob.h"
#include "ppl_settings.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_eps.h"
#include "eps_settings.h"

void eps_eps_RenderEPS(EPSComm *x)
 {
  FILE *inf;
  char  tmpdata[FNAME_LENGTH], command[LSTR_LENGTH], *filename;
  double bb_left=0.0, bb_right=0.0, bb_top=0.0, bb_bottom=0.0;
  double xscale, yscale, r;
  unsigned char GotBBox;

  fprintf(x->epsbuffer, "%% Canvas item %d [eps image]\n", x->current->id);

  // Expand filename if it contains wildcards
  filename = ppl_glob_oneresult(x->current->text);
  if (filename == NULL) { *(x->status) = 1; return; }

  // Work out bounding box of EPS image
  GotBBox = 0;
  if (!x->current->calcbbox) // Read bounding box from headers of EPS file
   {
    inf = fopen(filename, "r");
    if (inf==NULL) { sprintf(temp_err_string, "Could not open EPS file '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); *(x->status) = 1; return; }
    eps_eps_ExtractBBox(inf, &bb_left, &bb_bottom, &bb_right, &bb_top, &GotBBox);
    fclose(inf);
    if (!GotBBox) { sprintf(temp_err_string, "Could not extract bounding box from EPS file '%s'. Will therefore process file in calcbbox mode, and attempt to determine its bounding box using ghostview.", filename); ppl_warning(ERR_GENERAL, temp_err_string); }
   }
  if ((x->current->calcbbox) || (!GotBBox)) // Calculate bounding box for EPS file using ghostview
   {
    sprintf(tmpdata, "%s%s%s", settings_session_default.tempdir, PATHLINK, "bbox_in"); // Temporary file for gs to output bounding box into
    sprintf(command, "%s -dQUIET -dSAFER -dBATCH -dNOPAUSE -sDEVICE=bbox %s > %s 2> %s", GHOSTSCRIPT_COMMAND, filename, tmpdata, tmpdata);
    system(command);
    inf = fopen(tmpdata, "r");
    if (inf==NULL) { sprintf(temp_err_string, "Could not open temporary file '%s'.", tmpdata); ppl_error(ERR_FILE, -1, -1, temp_err_string); *(x->status) = 1; return; }
    eps_eps_ExtractBBox(inf, &bb_left, &bb_bottom, &bb_right, &bb_top, &GotBBox);
    fclose(inf);
    if (!GotBBox) { sprintf(temp_err_string, "Could not calculate bounding box for EPS file '%s'.", filename); ppl_warning(ERR_GENERAL, temp_err_string); }
   }

  // Work out scaling factor to apply to EPS image
  if (GotBBox && (x->current->xpos2set) && (x->current->ypos2set)) // Both width and height have been specified
   {
    xscale = x->current->xpos2 * M_TO_PS / (bb_right - bb_left  );
    yscale = x->current->ypos2 * M_TO_PS / (bb_top   - bb_bottom);
   }
  else if (GotBBox && x->current->xpos2set) // Only width has been set
   {
    xscale = x->current->xpos2 * M_TO_PS / (bb_right - bb_left  );
    yscale = x->current->xpos2 * M_TO_PS / (bb_right - bb_left  );
   }
  else if (GotBBox && x->current->ypos2set) // Only height has been set
   {
    xscale = x->current->ypos2 * M_TO_PS / (bb_top   - bb_bottom);
    yscale = x->current->ypos2 * M_TO_PS / (bb_top   - bb_bottom);
   }
  else // Neither height nor width has been set; EPS file should be drawn at its natural size
   {
    xscale = 1.0;
    yscale = 1.0;
   }

  // Begin encapsulation of EPS file
  fprintf(x->epsbuffer, "BeginEPSF\n");
  fprintf(x->epsbuffer, "%.2f %.2f translate %% Position the EPS file\n", x->current->xpos * M_TO_PS, x->current->ypos * M_TO_PS);
  fprintf(x->epsbuffer, "%.2f rotate         %% Rotation of EPS graphic\n", x->current->rotation * 180 / M_PI);
  fprintf(x->epsbuffer, "%f %f scale         %% Scale to desired size\n", xscale, yscale);
  fprintf(x->epsbuffer, "%f %f translate     %% Move to lower left of the EPS\n", -bb_left, -bb_bottom);

  // Clip EPS to bounding box if requested
  if ((x->current->clip) && (GotBBox))
   {
    fprintf(x->epsbuffer, "newpath\n%f %f moveto %f %f lineto %f %f lineto %f %f lineto closepath\n", bb_left, bb_bottom, bb_right, bb_bottom, bb_right, bb_top, bb_left, bb_top);
    fprintf(x->epsbuffer, "clip newpath %% Set clipping path around bounding box\n");
   }

  // Copy contents of EPS into output postscript file
  fprintf(x->epsbuffer, "%% ---- Beginning of included EPS graphic ----\n");
  inf = fopen(filename, "r");
  if (inf==NULL) { sprintf(temp_err_string, "Could not open EPS file '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); *(x->status) = 1; return; }
  while (fgets(temp_err_string, FNAME_LENGTH, inf) != NULL)
   if (fputs(temp_err_string, x->epsbuffer) == EOF)
    {
     sprintf(temp_err_string, "Error while reading EPS file '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string);
     *(x->status)=1;
     fclose(inf);
     return;
    }
  fclose(inf);

  // Finish off encapsulation of EPS file
  fprintf(x->epsbuffer, "\n%% ---- End of included EPS graphic ----\nEndEPSF\n");

  // Update postscript bounding box
  r = x->current->rotation;
  xscale *= (bb_right - bb_left); // Multiply scaling factors by width and height to give total size of resulting image in points
  yscale *= (bb_top - bb_bottom);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS                                  , x->current->ypos*M_TO_PS                                 , 0);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS + xscale*cos(r)                  , x->current->ypos*M_TO_PS + xscale*sin(r)                 , 0);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS                 + yscale*-sin(r) , x->current->ypos*M_TO_PS                 + yscale*cos(r) , 0);
  eps_core_BoundingBox(x, x->current->xpos*M_TO_PS + xscale*cos(r) + yscale*-sin(r) , x->current->ypos*M_TO_PS + xscale*sin(r) + yscale*cos(r) , 0);

  // Final newline at end of canvas item
  fprintf(x->epsbuffer, "\n");
  return;
 }

void eps_eps_ExtractBBox(FILE *in, double *bl, double *bb, double *br, double *bt, unsigned char *GotBox)
 {
  const char BBoxStr  [] = "%%BoundingBox:";
  const char HRBBoxStr[] = "%%HiResBoundingBox:";
  int bbox_i=0, hrbbox_i=0, j, bbox_len, hrbbox_len, i;
  double blt, bbt, brt, btt; // Temporary bounding box values

  bbox_len   = strlen(BBoxStr  );
  hrbbox_len = strlen(HRBBoxStr);

  *GotBox = 0;

  while ((j=fgetc(in))!=EOF) // Search input file for bounding box strings
   {
    if (BBoxStr  [bbox_i  ] == j) { bbox_i  ++; } else { bbox_i   = 0; }
    if (HRBBoxStr[hrbbox_i] == j) { hrbbox_i++; } else { hrbbox_i = 0; }

    if (bbox_i   == bbox_len)
     {
      bbox_i = 0;
      if (fgets(temp_err_string, FNAME_LENGTH, in) != NULL) // Read %%BoundingBox: . . . .
       {
        i=0;
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        blt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        bbt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        brt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        btt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        *bl = blt; *bb = bbt; *br = brt; *bt = btt; // Carry on looping to see if we can find a high resolution bounding box
        *GotBox = 1;
       }
     }

    if (hrbbox_i == hrbbox_len)
     {
      hrbbox_i = 0;
      if (fgets(temp_err_string, FNAME_LENGTH, in) != NULL) // Read %%HiResBoundingBox: . . . .
       {
        i=0;
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        blt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        bbt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        brt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        while ((temp_err_string[i]<=' ') && (temp_err_string[i]!='\0')) i++;
        btt = GetFloat(temp_err_string+i, &j); if (j<=0) continue; else i+=j; // If j<=0, bad bounding box
        *bl = blt; *bb = bbt; *br = brt; *bt = btt;
        *GotBox = 1;
        return; // A high resolution bounding box is better than a low resolution bounding box, so we've got all we need
       }
     }
   }
  return;
 }

