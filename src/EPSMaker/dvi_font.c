// dvi_font.c
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

// Functions for manupulating dvi files -- font section

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <kpathsea/kpathsea.h>

#include "ListTools/lt_memory.h"
#include "StringTools/str_constants.h"

#include "ppl_error.h"
#include "ppl_settings.h"

#include "dvi_read.h"
#include "dvi_font.h"

int dviGetTFM(dviFontDetails *font)
 {
  char *TFMpath, *PFApath, *PFBpath;
  char *s;
  char errStr[SSTR_LENGTH];
  int err=0;
  FILE *TFMfp;
  FILE *fpin, *fpout;

  // Get the TFM file
  s = (char *)lt_malloc((strlen(font->name)+5)*sizeof(char));
  if (s==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return DVIE_MEMORY; }
  sprintf(s, "%s.tfm", font->name);
  TFMpath = (char *)kpse_find_tfm(s);
  if (TFMpath==NULL) { ppl_error(ERR_INTERNAL,"Could not find TFM file"); return 1; }
  if (DEBUG) { sprintf(temp_err_string, "Font file %s: TFM path: %s", font->name, TFMpath); ppl_log(temp_err_string); }
  TFMfp = fopen(TFMpath, "r");
  if (TFMfp==NULL) { ppl_error(ERR_INTERNAL,"Could not open TFM file"); return 1; }
  font->tfm = dviReadTFM(TFMfp, &err);
  if (err) return err;

  // Find the maximum height and depth of the font
  err = dviFindMaxSize(font);
  if (err) return err;

  // Work out what type of font this is
  if      (strncmp(font->tfm->coding, "TeX text"          ,  8)==0) font->fontType = FONT_TEX_TEXT;
  else if (strncmp(font->tfm->coding, "TeX math italic"   , 15)==0) font->fontType = FONT_TEX_MATH;
  else if (strncmp(font->tfm->coding, "TeX math extension", 18)==0) font->fontType = FONT_TEX_MEXT;
  else                                                              font->fontType = FONT_UNKNOWN;

  if (DEBUG) {sprintf(temp_err_string, "TFM: font type %d", font->fontType);  ppl_log(temp_err_string);}

   
  // Additionally obtain the pfa file
  s = (char *)lt_malloc((strlen(font->name)+5)*sizeof(char));
  if (s==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return DVIE_MEMORY; }
  sprintf(s, "%s.pfa", font->name);
  PFApath = (char *)kpse_find_file(s, kpse_type1_format, true);
  if (PFApath != NULL)
   {
    font->pfaPath = PFApath;
   }
  else
   {
    // Make a PFA file from the PFB file (assuming that one exists)
    sprintf(s, "%s.pfb", font->name);
    PFBpath = (char *)kpse_find_file(s, kpse_type1_format, true);
    if (PFBpath == NULL)
     {
      snprintf(errStr, SSTR_LENGTH, "dviGetTfm: Cannot find pfa or pfb file for font %s", font->name);
      ppl_error(ERR_GENERAL,errStr);
      return DVIE_NOFONT;
     }
    fpin = fopen(PFBpath, "r");
    if (fpin==NULL)
     {
      snprintf(errStr, SSTR_LENGTH, "dviGetTfm: Cannot open pfb file %s", PFBpath);
      ppl_error(ERR_GENERAL,errStr);
      return DVIE_ACCESS;
     }
      
    // Make a filename for the destination pfa file
    // free(PFApath);
    PFApath = (char *)lt_malloc(SSTR_LENGTH*sizeof(char));
    if (PFApath==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return DVIE_MEMORY; }
    snprintf(PFApath, SSTR_LENGTH, "%s%s%s.pfa", settings_session_default.tempdir, PATHLINK, font->name);
    fpout = fopen(PFApath, "w");
    if (fpout == NULL)
     {
      snprintf(errStr, SSTR_LENGTH, "dviGetTfm: Cannot write to pfa file %s", PFApath);
      ppl_error(ERR_GENERAL,errStr);
      return DVIE_ACCESS;
     }
    if ((err=pfb2pfa(fpin, fpout))!=0) return err;
    fclose(fpin);
    fclose(fpout);
    font->pfaPath = PFApath;
   }

  // Obtain the font name from the PFA file
  font->psName = psNameFromPFA(PFApath);
  if (font->psName==NULL) return 1;
  return 0;
 }

// Extract the font name from a PFA file
// XXX This is an entirely empirical algorithm and may not be general!
// If the font name is longer than SSTR_LENGTH this will fail (safely)...
char *psNameFromPFA(char *PFApath)
 {
  FILE *fp;
  char *s;
  char buf[SSTR_LENGTH], c;
  int i;

  if ((fp = fopen(PFApath, "r"))==NULL)
   {
    ppl_error(ERR_INTERNAL, "Cannot open pfa file");
    return NULL;
   }

  // Forward file to the first space
  c = '\0';
  while (c!=' ') c = getc(fp);

  // Grab characters until the next space
  i=0;
  while ((c=getc(fp))!=' ' && i<SSTR_LENGTH-1)
   {
    buf[i]=c; i++;
   }
  buf[i]='\0';

  // Produce a malloced string with the name in and return it
  s = (char *)lt_malloc((i+1)*sizeof(char));
  if (s==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return NULL; }
  snprintf(s, i+1, "%s", buf);
  return s;
 }

// Convert pfb file to pfa file
int pfb2pfa(FILE *in, FILE *out)
 {
  int i, j;    // Loop variables
  int len;     // Record length
  char c;      // Input string

  while (!feof(in))
   {
    if (getc(in) != 128)
     {
      ppl_error(ERR_INTERNAL, "Error in pfb file format");
      return 1;
     }
    i = getc(in);
    if (i==1)
     {
      // Ascii text record
      len = getc(in) | getc(in)<<8 | getc(in)<<16 | getc(in)<<24;
      for (j=0; j<len ; j++)
       {
        if ((c=getc(in)) == '\r') putc('\n', out);
        else                      putc(c, out);
       }
     }
    else if (i==2)
     {
      // Binary data record
      len = getc(in) | getc(in)<<8 | getc(in)<<16 | getc(in)<<24;
      for (j=0; j<len; j++)
       {
        fprintf(out, "%02x", getc(in));
        if ((j+1) % 30 == 0) putc('\n', out);
       }
      putc('\n', out);
     }
    else if (i==3)
     {
      // EOF
      break;
     }
    else
     {
      ppl_error(ERR_INTERNAL, "Corrupt pfb file");
      return 1;
     }
   }
  return 0;
 }

// TFM-related routines
dviTFM *dviReadTFM(FILE *fp, int *err)
 {
  dviTFM *tfm;
  unsigned long int buff[12];
  int i, j;
  int lh;
  int Nchars;

  char *tit[12] = {"lf", "lh", "bc", "ec", "nw", "nh", "nd", "ni", "nl", "nk", "ne", "np"};

  tfm = (dviTFM *)lt_malloc(sizeof(dviTFM));
  if (tfm==NULL) { ppl_error(ERR_INTERNAL,"Out of memory"); *err=DVIE_MEMORY; return NULL; }

  // Read the file header
  for (i=0; i<12; i++) ReadLongInt(fp, buff+i, 2);
  tfm->lf = buff[0];
  tfm->lh = buff[1];
  tfm->bc = buff[2];
  tfm->ec = buff[3];
  tfm->nw = buff[4];
  tfm->nh = buff[5];
  tfm->nd = buff[6];
  tfm->ni = buff[7];
  tfm->nl = buff[8];
  tfm->nk = buff[9];
  tfm->ne = buff[10];
  tfm->np = buff[11];

  if (DEBUG)
   {
    sprintf(temp_err_string, "TFM: ");
    j = strlen(temp_err_string);
    for (i=0; i<12; i++) { sprintf(temp_err_string+j, "%s:%lu  ", tit[i], buff[i]); j+=strlen(temp_err_string+j); }
    ppl_log(temp_err_string);
   }
   
  // We should have lf=6+lh+(ec-bc+1)+nw+nh+nd+ni+nl+nk+ne+np
  if (tfm->lf != 6 + tfm->lh + tfm->ec - tfm->bc + 1 + tfm->nw + tfm->nh + tfm->nd + tfm->ni + tfm->nl + tfm->nk + tfm->ne + tfm->np)
   {
    ppl_error(ERR_INTERNAL,"TFM fail");
    *err=1; return NULL;
   }

  // Read the header (distinct from the file header...)
  lh = tfm->lh;
  ReadLongInt(fp, buff, 4); tfm->checksum = buff[0]; 
  lh--;
  tfm->ds = ReadFixWord(fp, err); if (*err) return NULL;
  lh--;
  if (DEBUG) { sprintf(temp_err_string, "TFM: lh now %d", lh); ppl_log(temp_err_string); }
  if (lh>10)
   {
    int len;
    if ((*err=ReadUChar(fp, &len))!=0) return NULL;
    if (DEBUG) { sprintf(temp_err_string, "TFM: Coding length: %d", len); ppl_log(temp_err_string); }
    if (len>39)
     {
      ppl_error(ERR_INTERNAL,"Malformed DVI header. coding len>40");
      len=39;
     }
    for (i=0; i<39; i++)
     {
      int t;
      if ((*err=ReadUChar(fp,&t))!=0) return NULL;
      tfm->coding[i] = t;
     }
    tfm->coding[len] = '\0';
    lh-=10;
   }
  if (lh>5)
   {
    int len;
    if ((*err=ReadUChar(fp, &len))!=0) return NULL;
    if (len>19)
     {
      ppl_error(ERR_INTERNAL,"Malformed DVI header. coding len>19");
      len=19;
     }
    for (i=0; i<19; i++)
     {
      int t;
      if ((*err=ReadUChar(fp,&t))!=0) return NULL;
      tfm->family[i] = t;
     }
    tfm->family[len] = '\0';
    lh-=5;
   }
  if (DEBUG) { sprintf(temp_err_string, "TFM: coding:%s: family:%s: lh now %d", tfm->coding, tfm->family, lh); ppl_log(temp_err_string); }
  if (lh>0)
   {
    int temp;
    if ((*err=ReadUChar(fp, &temp))!=0) return NULL;
    if ((*err=ReadUChar(fp, &temp))!=0) return NULL;
    tfm->face = temp;
    if ((*err=ReadUChar(fp, &temp))!=0) return NULL;
    if ((*err=ReadUChar(fp, &temp))!=0) return NULL;
    lh--;
   }
  while (lh>0)
   {
    unsigned long int i;
    ReadLongInt(fp, &i, 4);
    lh--;
   }

  // Read the char info tables
  Nchars = tfm->ec - tfm->bc + 1;
  tfm->charInfo = (TFMcharInfo *)lt_malloc(Nchars*sizeof(TFMcharInfo));
  if (tfm->charInfo==NULL) { ppl_error(ERR_INTERNAL,"Out of memory"); *err=DVIE_MEMORY; return NULL; }
  for (i=0; i<Nchars; i++)
   {
    int j;
    int t[4];
    for (j=0; j<4; j++) if ((*err=ReadUChar(fp, t+j))!=0) return NULL;
    (tfm->charInfo+i)->wi = t[0];
    (tfm->charInfo+i)->hi = t[1]>>4;
    (tfm->charInfo+i)->di = t[1]&0xf;
    (tfm->charInfo+i)->ii = t[2]>>2;
    (tfm->charInfo+i)->tag = t[2]&0x3;
    (tfm->charInfo+i)->rem = t[3];
   }

  // Read the width, height, depth & italic tables
  tfm->width  = (double *)lt_malloc(tfm->nw*sizeof(double));
  tfm->height = (double *)lt_malloc(tfm->nh*sizeof(double));
  tfm->depth  = (double *)lt_malloc(tfm->nd*sizeof(double));
  tfm->italic = (double *)lt_malloc(tfm->ni*sizeof(double));
  if ((tfm->width==NULL)||(tfm->height==NULL)||(tfm->depth==NULL)||(tfm->italic==NULL)) { ppl_error(ERR_INTERNAL,"Out of memory"); *err=DVIE_MEMORY; return NULL; }

  for (i=0; i<tfm->nw; i++) { tfm->width[i]  = ReadFixWord(fp,err); if (*err) return NULL; }
  for (i=0; i<tfm->nh; i++) { tfm->height[i] = ReadFixWord(fp,err); if (*err) return NULL; }
  for (i=0; i<tfm->nd; i++) { tfm->depth[i]  = ReadFixWord(fp,err); if (*err) return NULL; }
  for (i=0; i<tfm->ni; i++) { tfm->italic[i] = ReadFixWord(fp,err); if (*err) return NULL; }

  // Read the lig_kern table
  tfm->ligKern = (TFMligKern *)lt_malloc(tfm->nl*sizeof(TFMligKern));
  if (tfm->ligKern==NULL) { ppl_error(ERR_INTERNAL,"Out of memory"); *err=DVIE_MEMORY; return NULL; }
  for (i=0; i<tfm->nl; i++)
   {
    int j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->ligKern+i)->skip_byte = j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->ligKern+i)->next_char = j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->ligKern+i)->op_byte   = j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->ligKern+i)->remainder = j;
   }

  // Read the kern table
  tfm->kern = (double *)lt_malloc(tfm->nk*sizeof(double));
  if (tfm->kern==NULL) { ppl_error(ERR_INTERNAL,"Out of memory"); *err=DVIE_MEMORY; return NULL; }
  for (i=0; i<tfm->nk; i++) { tfm->kern[i] = ReadFixWord(fp,err); if (*err) return NULL; }

  // Read the extensible character recipies
  tfm->extensibleRecipe = (TFMextRec *)lt_malloc(tfm->ne*sizeof(TFMextRec));
  if (tfm->extensibleRecipe==NULL) { ppl_error(ERR_INTERNAL,"Out of memory"); *err=DVIE_MEMORY; return NULL; }
  for (i=0; i<tfm->ne; i++)
   {
    int j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->extensibleRecipe+i)->top = j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->extensibleRecipe+i)->mid = j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->extensibleRecipe+i)->bot = j;
    if ((*err=ReadUChar(fp, &j))!=0) return NULL;    (tfm->extensibleRecipe+i)->rep = j;
   }

  // We should read in the param aray at this point, but let's see if we can get away without doing so
  return tfm;
 }

// Read a TFM "fix_word", a signed four-byte int with the dp after 12 bits
double ReadFixWord(FILE *fp, int *err)
 {
  double fw;
  long signed int li;
  const double twoP20 = 1048576; // 2**20
  if ((*err=ReadSignedInt(fp, &li, 4))!=0) return 0.0;
  fw = (double) li / twoP20;
  return fw;
 }

// Find the maximum (unscaled) height and depth of (standard) glyphs within a font
int dviFindMaxSize(dviFontDetails *font)
 {
  dviTFM *tfm;         // Details of this font
  int hi, di;          // Indices
  int chnum;           // Character number in this font
  TFMcharInfo *chin;   // Character info for this character
  int i, loopMax, hmax, dmax;
  double height, depth;

  if (!font) { ppl_error(ERR_INTERNAL,"Internal font failure!"); return DVIE_INTERNAL; }
  tfm = font->tfm;
  font->maxHeight = 0.0;
  font->maxDepth = 0.0;

  // Loop over upper-case characters
  loopMax = tfm->ec > ASCII_CHAR_Z_UP ? ASCII_CHAR_Z_UP : tfm->ec;
  hmax=0; dmax=0;
  for (i=ASCII_CHAR_A_UP; i<=loopMax; i++)
   {
    chnum = i - tfm->bc;
    chin  = tfm->charInfo+chnum;
    hi = (int)chin->hi;
    di = (int)chin->di;
    height = tfm->height[hi];
    depth  = tfm->depth[di];
    if (font->maxHeight < height) { font->maxHeight = height; hmax=i; }
    if (font->maxDepth  < depth ) { font->maxDepth  = depth ; dmax=i; }
   }

  // Loop over lower-case characters
  loopMax = tfm->ec > ASCII_CHAR_Z_DN ? ASCII_CHAR_Z_DN : tfm->ec;
  for (i=ASCII_CHAR_A_DN; i<=loopMax; i++)
   {
    chnum = i - tfm->bc;
    chin  = tfm->charInfo+chnum;
    hi = (int)chin->hi;
    di = (int)chin->di;
    height = tfm->height[hi];
    depth  = tfm->depth[di];
    if (font->maxHeight < height) { font->maxHeight = height; hmax=i; }
    if (font->maxDepth  < depth ) { font->maxDepth  = depth ; dmax=i; }
   }
  font->maxHeight *= font->useSize;
  font->maxDepth  *= font->useSize;
  if (DEBUG) { sprintf(temp_err_string, "Maximum height %f depth %f from characters %d %d %s %s", font->maxHeight, font->maxDepth, hmax, dmax, (char *)(&hmax), (char *)(&dmax)); ppl_log(temp_err_string); }
  return 0;
 }

