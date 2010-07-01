// eps_plot_colourmap.c
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

#define _PPL_EPS_PLOT_COLOURMAP_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>

#include "ListTools/lt_memory.h"

#include "ppl_canvasdraw.h"
#include "ppl_canvasitems.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "bmp_a85.h"
#include "bmp_optimise.h"

#include "eps_comm.h"
#include "eps_core.h"
#include "eps_colours.h"
#include "eps_image.h"
#include "eps_plot.h"
#include "eps_plot_canvas.h"
#include "eps_settings.h"
#include "eps_style.h"

// Random shade of purple to use as mask colour
#define TRANS_R 35
#define TRANS_G 2
#define TRANS_B 20

// Yield up text items which label colour scale of a colourmap
void eps_plot_colourmap_YieldText(EPSComm *x, DataTable *data, settings_graph *sg, canvas_plotdesc *pd)
 {
  DataBlock     *blk;
  int            XSize = (x->current->settings.SamplesXAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesX;
  int            YSize = (x->current->settings.SamplesYAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesY;
  int            i, j, Ncol;
  double         CMin, CMax;
  unsigned char  CMinAuto, CMinSet, CMaxAuto, CMaxSet, CLog;

  // Check that we have some data
  if ((data==NULL) || (data->Nrows<1)) return; // No data present
  Ncol = data->Ncolumns;
  blk = data->first;

  // Work out normalisation of variable c1
  CMinAuto = (sg->Cminauto[0]==SW_BOOL_TRUE);
  CMinSet  = !CMinAuto;
  CMin     = sg->Cmin[0].real;
  CMaxAuto = (sg->Cmaxauto[0]==SW_BOOL_TRUE);
  CMaxSet  = !CMaxAuto;
  CMax     = sg->Cmax[0].real;
  CLog     = (sg->Clog[0]==SW_BOOL_TRUE);

  // Find extremal values
  if (CMinAuto || CMaxAuto)
   for (j=0; j<YSize; j++)
    for (i=0; i<XSize; i++)
     {
      double val = blk->data_real[2 + Ncol*(i+XSize*j)].d;
      if (!gsl_finite(val)) continue;
      if ((CMinAuto) && ((!CMinSet) || (CMin>val)) && ((!CLog)||(val>0.0))) { CMin=val; CMinSet=1; }
      if ((CMaxAuto) && ((!CMaxSet) || (CMax<val)) && ((!CLog)||(val>0.0))) { CMax=val; CMaxSet=1; }
     }

  // If no data present, stop now
  if ((!CMinSet)||(!CMaxSet)) return;

  // If log spacing, make sure range is strictly positive
  if (CLog && (CMin<1e-200)) CMin=1e-200;
  if (CLog && (CMax<1e-200)) CMax=1e-200;

  // DITTO THE ABOVE BELOW

  // Work out units in which colour key will be labelled

  // Estimate length of colour key

  // Submit axis labels for colour key

  // If we have three columns of data, consider drawing a colour scale bar
  if ((Ncol==3)&&(sg->ColKey==SW_ONOFF_ON))
   {
    pd->CRangeDisplay = 1;
    pd->CMinFinal     = CMin;
    pd->CMaxFinal     = CMax;
    pd->CRangeUnit    = data->FirstEntries[2];
   }

  return;
 }
// Render a colourmap to postscript
int  eps_plot_colourmap(EPSComm *x, DataTable *data, unsigned char ThreeDim, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, double origin_x, double origin_y, double width, double height, double zdepth)
 {
  double         scale_x, scale_y, scale_z;
  DataBlock     *blk;
  int            XSize = (x->current->settings.SamplesXAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesX;
  int            YSize = (x->current->settings.SamplesYAuto==SW_BOOL_TRUE) ? x->current->settings.samples : x->current->settings.SamplesY;
  int            i, j, c, cmax, errpos, Ncol, NcolsData;
  long           p;
  double         xo, yo, Lx, Ly, ThetaX, ThetaY, comp[4], CMin[4], CMax[4];
  value         *CVar[4], CDummy[4], outval;
  uLongf         zlen; // Length of buffer passed to zlib
  unsigned char *imagez, CMinAuto[4], CMinSet[4], CMaxAuto[4], CMaxSet[4], CLog[4];
  char          *ColExpr[4] = {sg->ColMapExpr1 , sg->ColMapExpr2 , sg->ColMapExpr3 , sg->ColMapExpr4 }, *errtext;
  unsigned char  component_r, component_g, component_b, transparent[3] = {TRANS_R, TRANS_G, TRANS_B};
  bitmap_data    img;

  if ((data==NULL) || (data->Nrows<1)) return 0; // No data present
  Ncol = data->Ncolumns;
  if (eps_plot_WithWordsCheckUsingItemsDimLess(&pd->ww_final, data->FirstEntries, Ncol, &NcolsData)) return 1;
  if (!ThreeDim) { scale_x=width; scale_y=height; scale_z=1.0;    }
  else           { scale_x=width; scale_y=height; scale_z=zdepth; }
  blk = data->first;

  errtext = lt_malloc(LSTR_LENGTH);
  if (errtext==NULL) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); return 1; }

  // Work out orientation of colourmap
  if (!ThreeDim)
   {
    xo     = origin_x;
    yo     = origin_y;
    Lx     = scale_x;
    Ly     = scale_y;
    ThetaX = M_PI/2;
    ThetaY = 0.0;
    if (xn!=0) { double t1=Lx, t2=ThetaX; Lx=Ly; ThetaX=ThetaY; Ly=t1; ThetaY=t2; }
   }
  else // 3D case: put colour map on back face of cuboid
   {
    double ap[3]={0.5,0.5,0.0}, xtmp, ytmp, z0tmp, z1tmp;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z0tmp);
    ap[2]=1.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z1tmp);
    ap[2]=(z1tmp>z0tmp)?1.0:0.0; // Determine whether zap=0 or zap=1 represents back of cuboid

    ap[0]=ap[1]=0.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xo, &yo, &z0tmp);

    ap[0]=1.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z0tmp);
    Lx = hypot(xtmp-xo, ytmp-yo);
    ThetaX = atan2(xtmp-xo, ytmp-yo);
    if (!gsl_finite(ThetaX)) ThetaX = 0.0;

    ap[0]=0.0;
    ap[1]=1.0;
    eps_plot_ThreeDimProject(ap[xn], ap[yn], ap[zn], sg, origin_x, origin_y, scale_x, scale_y, scale_z, &xtmp, &ytmp, &z0tmp);
    Ly = hypot(xtmp-xo, ytmp-yo);
    ThetaY = atan2(xtmp-xo, ytmp-yo);
    if (!gsl_finite(ThetaY)) ThetaY = 0.0;
   }

  // Populate bitmap image decriptor
  img.type     = BMP_COLOUR_BMP;
  img.colour   = BMP_COLOUR_RGB;
  img.pal_len  = 0;
  img.palette  = NULL;
  img.trans    = transparent;
  img.height   = YSize;
  img.width    = XSize;
  img.depth    = 24;
  img.data_len = 3*XSize*YSize;
  img.data     = lt_malloc(3*XSize*YSize);
  img.TargetCompression = BMP_ENCODING_FLATE;
  if (img.data==NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory."); return 1; }

  // Get pointer to variable c in the user's variable space
  for (i=0; i<4; i++)
   {
    char v[3]={'c','1'+i,'\0'};
    DictLookup(_ppl_UserSpace_Vars, v, NULL, (void **)&CVar[i]);
    if (CVar[i]!=NULL)
     {
      CDummy[i] = *CVar[i];
     }
    else // If variable is not defined, create it now
     {
      ppl_units_zero(&CDummy[i]);
      DictAppendValue(_ppl_UserSpace_Vars, v, CDummy[i]);
      DictLookup(_ppl_UserSpace_Vars, v, NULL, (void **)&CVar[i]);
      CDummy[i].modified = 2;
     }
    if ((i<NcolsData-2)&&(sg->Crenorm[i]==SW_BOOL_FALSE)) { *CVar[i] = data->FirstEntries[i+2]; CVar[i]->FlagComplex=0; CVar[i]->imag=0.0; }
    else ppl_units_zero(CVar[i]); // c1...c4 are dimensionless numbers in range 0-1, regardless of units of input data
   }

  // Work out normalisation of variables c1...c4
  for (c=0; ((c<4)&&(c<NcolsData-2)); c++)
   {
    CMinAuto[c] = (sg->Cminauto[c]==SW_BOOL_TRUE);
    CMinSet [c] = !CMinAuto[c];
    CMin    [c] = sg->Cmin[c].real;
    CMaxAuto[c] = (sg->Cmaxauto[c]==SW_BOOL_TRUE);
    CMaxSet [c] = !CMaxAuto[c];
    CMax    [c] = sg->Cmax[c].real;
    CLog    [c] = (sg->Clog[c]==SW_BOOL_TRUE);

    // Find extremal values
    if (CMinAuto[c] || CMaxAuto[c])
     for (j=0; j<YSize; j++)
      for (i=0; i<XSize; i++)
       {
        double val = blk->data_real[c+2 + Ncol*(i+XSize*j)].d;
        if (!gsl_finite(val)) continue;
        if ((CMinAuto[c]) && ((!CMinSet[c]) || (CMin[c]>val)) && ((!CLog[c])||(val>0.0))) { CMin[c]=val; CMinSet[c]=1; }
        if ((CMaxAuto[c]) && ((!CMaxSet[c]) || (CMax[c]<val)) && ((!CLog[c])||(val>0.0))) { CMax[c]=val; CMaxSet[c]=1; }
       }

    // Reverse range of colour scale if requested
    if (sg->Creverse[c]==SW_BOOL_TRUE)
     {
      double td=CMin[c]; unsigned char tc=CMinSet[c], tc2=CMinAuto[c];
      CMinAuto[c] = CMaxAuto[c];
      CMinSet [c] = CMaxSet [c];
      CMin    [c] = CMax    [c];
      CMaxAuto[c] = tc2;
      CMaxSet [c] = tc;
      CMax    [c] = td;
     }

    // If no data present, stop now
    if ((!CMinSet[c])||(!CMaxSet[c]))
     {
      sprintf(temp_err_string, "No data supplied to determine range for variable c%d", c+1);
      for (i=0; i<4; i++) *CVar[i] = CDummy[i];
      return 0;
     }

    // Output result to debugging output
    if (DEBUG)
     {
      int SF = settings_term_current.SignificantFigures;
      sprintf(temp_err_string, "Range for variable c%d is [%s:%s]", c+1, NumericDisplay(CMin[c],0,SF,0), NumericDisplay(CMax[c],1,SF,0));
      ppl_log(temp_err_string);
     }
   }
  cmax = c-1;

  // Check that variables c1...c4 has appropriate units
  for (c=0; c<=cmax; c++)
   if ( ((!CMinAuto[c])||(!CMaxAuto[c])) && (!ppl_units_DimEqual(CVar[c] , (sg->Cminauto[c]==SW_BOOL_TRUE)?(&sg->Cmax[c]):(&sg->Cmin[c]))) )
    {
     sprintf(temp_err_string, "Column %d of data supplied to the colourmap plot style has conflicting units with those set in the 'set crange' command. The former has units of <%s> whilst the latter has units of <%s>.", c+3, ppl_units_GetUnitStr(CVar[c], NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr((sg->Cminauto[c]==SW_BOOL_TRUE)?(&sg->Cmax[c]):(&sg->Cmin[c]), NULL, NULL, 1, 1, 0));
     ppl_error(ERR_NUMERIC,-1,-1,temp_err_string);
     return 1;
    }

  // Populate bitmap data array
  for (p=0, j=YSize-1; j>=0; j--) // Postscript images are top-first. Data block is bottom-first.
   for (i=0; i<XSize; i++)
    {
     // Set values of c1...c4
     for (c=0;c<4; c++)
      if      (c>cmax)  /* No c<c> */         { *CVar[c]       = CDummy[c]; }
      else if (sg->Crenorm[c]==SW_BOOL_FALSE) {  CVar[c]->real = blk->data_real[c+2 + Ncol*(i+XSize*j)].d; } // No renormalisation
      else if (CMax[c]==CMin[c])  /* Ooops */ {  CVar[c]->real = 0.5; }
      else if (!CLog[c]) /* Linear */         {  CVar[c]->real = (blk->data_real[c+2 + Ncol*(i+XSize*j)].d - CMin[c]) / (CMax[c] - CMin[c]); }
      else               /* Logarithmic */    {  CVar[c]->real = log(blk->data_real[c+2 + Ncol*(i+XSize*j)].d / CMin[c]) / log(CMax[c] / CMin[c]); }

     // Check if mask criterion is satisfied
     if (sg->MaskExpr[0]!='\0')
      {
       errpos=-1;
       ppl_EvaluateAlgebra(sg->MaskExpr, &outval, 0, NULL, 0, &errpos, errtext, 0);
       if (errpos>=0) { sprintf(temp_err_string, "Could not evaluate mask expression <%s>. The error, encountered at character position %d, was: '%s'", sg->MaskExpr, errpos, errtext); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); return 1; }
       if (outval.real==0) { component_r = TRANS_R; component_g = TRANS_G; component_b = TRANS_B; goto write_rgb; }
      }

     // Compute RGB, HSB or CMYK components
     for (c=0; c<3+(sg->ColMapColSpace==SW_COLSPACE_CMYK); c++)
      {
       errpos=-1;
       ppl_EvaluateAlgebra(ColExpr[c], &outval, 0, NULL, 0, &errpos, errtext, 0);
       if (errpos>=0) { sprintf(temp_err_string, "Could not evaluate colour expression <%s>. The error, encountered at character position %d, was: '%s'", ColExpr[c], errpos, errtext); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); return 1; }
       if (!outval.dimensionless) { sprintf(temp_err_string, "Expression <%s> for colour component %d returns result with units of <%s>; this should be a dimensionless number in the range 0-1.",ColExpr[c], c+1, ppl_units_GetUnitStr(&outval, NULL, NULL, 0, 1, 0)); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); return 1; }
       if (outval.FlagComplex) { sprintf(temp_err_string, "Expression <%s> for colour component %d returns a complex result.", ColExpr[c], c+1); ppl_error(ERR_NUMERIC,-1,-1,temp_err_string); return 1; }
       if (!gsl_finite(outval.real)) { component_r = TRANS_R; component_g = TRANS_G; component_b = TRANS_B; goto write_rgb; }
       comp[c]=outval.real;
      }

     // Convert to RGB
#define CLIP_COMPS \
  comp[0] = (comp[0] < 0.0) ? 0.0 : ((comp[0]>1.0) ? 1.0 : comp[0] ); \
  comp[1] = (comp[1] < 0.0) ? 0.0 : ((comp[1]>1.0) ? 1.0 : comp[1] ); \
  comp[2] = (comp[2] < 0.0) ? 0.0 : ((comp[2]>1.0) ? 1.0 : comp[2] ); \
  comp[3] = (comp[3] < 0.0) ? 0.0 : ((comp[3]>1.0) ? 1.0 : comp[3] );
     switch (sg->ColMapColSpace)
      {
       case SW_COLSPACE_RGB: // Convert RGB --> RGB
        break;
       case SW_COLSPACE_HSB: // Convert HSB --> RGB
        {
         double h2, ch, x, m; int h2i;
         CLIP_COMPS;
         ch  = comp[1]*comp[2];
         h2i = (int)(h2 = comp[0] * 6);
         x   = ch*(1.0-fabs(fmod(h2,2)-1.0));
         m   = comp[2] - ch;
         switch (h2i)
          {
           case 0 : comp[0]=ch; comp[1]=x ; comp[2]=0 ; break;
           case 1 : comp[0]=x ; comp[1]=ch; comp[2]=0 ; break;
           case 2 : comp[0]=0 ; comp[1]=ch; comp[2]=x ; break;
           case 3 : comp[0]=0 ; comp[1]=x ; comp[2]=ch; break;
           case 4 : comp[0]=x ; comp[1]=0 ; comp[2]=ch; break;
           case 5 :
           case 6 : comp[0]=ch; comp[1]=0 ; comp[2]=x ; break; // case 6 is for hue=1.0 only
           default: comp[0]=0 ; comp[1]=0 ; comp[2]=0 ; break;
          }
         comp[0]+=m; comp[1]+=m; comp[2]+=m;
         break;
        }
       case SW_COLSPACE_CMYK: // Convert CMYK --> RGB
        comp[0] = 1.0 - (comp[0]+comp[3]);
        comp[1] = 1.0 - (comp[1]+comp[3]);
        comp[2] = 1.0 - (comp[2]+comp[3]);
        break;
       default: // Unknown colour space
        comp[0] = comp[1] = comp[2] = 0.0;
        break;
      }
     CLIP_COMPS;

     // Store RGB components
     component_r = (unsigned char)floor(comp[0] * 255.99);
     component_g = (unsigned char)floor(comp[1] * 255.99);
     component_b = (unsigned char)floor(comp[2] * 255.99);
     if ((component_r==TRANS_R)&&(component_g==TRANS_G)&&(component_b==TRANS_B)) component_b++;

write_rgb:
     img.data[p++] = component_r;
     img.data[p++] = component_g;
     img.data[p++] = component_b;
    }

  // Restore variables c1...c4 in the user's variable space
  for (i=0; i<4; i++) *CVar[i] = CDummy[i];

  // Consider converting RGB data into a paletted image
  if ((img.depth == 24) && (img.type==BMP_COLOUR_BMP    )) bmp_colour_count(&img);  // Check full colour image to ensure more than 256 colours
  if ((img.depth ==  8) && (img.type==BMP_COLOUR_PALETTE)) bmp_grey_check(&img);    // Check paletted images for greyscale conversion
  if ((img.type == BMP_COLOUR_PALETTE) && (img.pal_len <= 16) && (img.depth == 8)) bmp_compact(&img); // Compact images with few palette entries

  // Apply compression to image data
  switch (img.TargetCompression)
   {
    case BMP_ENCODING_FLATE:
     zlen   = img.data_len*1.01+12; // Nasty guess at size of buffer needed.
     imagez = (unsigned char *)lt_malloc(zlen);
     if (imagez == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory."); img.TargetCompression = BMP_ENCODING_NULL; break; }
     if (DEBUG) { ppl_log("Calling zlib to compress image data."); }
     j = compress2(imagez,&zlen,img.data,img.data_len,9); // Call zlib to do deflation

     if (j!=0)
      {
       if (DEBUG) { sprintf(temp_err_string, "zlib returned error code %d\n",j); ppl_log(temp_err_string); }
       img.TargetCompression = BMP_ENCODING_NULL; // Give up trying to compress data
       break;
      }
     if (DEBUG) { sprintf(temp_err_string, "zlib has completed compression. Before flate: %ld bytes. After flate: %ld bytes.", img.data_len, (long)zlen); ppl_log(temp_err_string); }
     if (zlen >= img.data_len)
      {
       if (DEBUG) { ppl_log("Using original uncompressed data since zlib made it bigger than it was to start with."); }
       img.TargetCompression = BMP_ENCODING_NULL; // Give up trying to compress data; result was larger than original data size
       break;
      }
     img.data = imagez; // Replace old data with new compressed data
     img.data_len = zlen;
     break;
   }

  // Write out postscript image
  fprintf(x->epsbuffer, "gsave\n");
  fprintf(x->epsbuffer, "[ %.2f %.2f %.2f %.2f %.2f %.2f ] concat\n", Lx*sin(ThetaX), Lx*cos(ThetaX), Ly*sin(ThetaY), Ly*cos(ThetaY), xo, yo);

  if      (img.colour == BMP_COLOUR_RGB ) fprintf(x->epsbuffer, "/DeviceRGB setcolorspace\n");  // RGB palette
  else if (img.colour == BMP_COLOUR_GREY) fprintf(x->epsbuffer, "/DeviceGray setcolorspace\n"); // Greyscale palette
  else if (img.colour == BMP_COLOUR_PALETTE) // Indexed palette
   {
    fprintf(x->epsbuffer, "[/Indexed /DeviceRGB %d <~\n", img.pal_len-1);
    bmp_A85(x->epsbuffer, img.palette, 3*img.pal_len);
    fprintf(x->epsbuffer, "] setcolorspace\n\n");
   }

  fprintf(x->epsbuffer, "<<\n /ImageType %d\n /Width %d\n /Height %d\n /ImageMatrix [%d 0 0 %d 0 %d]\n", (img.trans==NULL)?1:4, img.width, img.height, img.width, -img.height, img.height);
  fprintf(x->epsbuffer, " /DataSource currentfile /ASCII85Decode filter"); // Image data is stored in currentfile, but need to apply filters to decode it
  if (img.TargetCompression == BMP_ENCODING_FLATE) fprintf(x->epsbuffer, " /FlateDecode filter");
  fprintf(x->epsbuffer, "\n /BitsPerComponent %d\n /Decode [0 %d%s]\n", (img.colour==BMP_COLOUR_RGB)?(img.depth/3):(img.depth),
                                                                        (img.type==BMP_COLOUR_PALETTE)?((1<<img.depth)-1):1,
                                                                        (img.colour==BMP_COLOUR_RGB)?" 0 1 0 1":"");
  if (img.trans != NULL)
   {
    fprintf(x->epsbuffer," /MaskColor [");
    if (img.colour == BMP_COLOUR_RGB) fprintf(x->epsbuffer, "%d %d %d]\n",(int)img.trans[0], (int)img.trans[1], (int)img.trans[2]);
    else                              fprintf(x->epsbuffer, "%d]\n"      ,(int)img.trans[0]);
   }
  fprintf(x->epsbuffer, ">> image\n");
  bmp_A85(x->epsbuffer, img.data, img.data_len);
  fprintf(x->epsbuffer, "grestore\n");
  return 0;
 }

