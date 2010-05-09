// ppl_glob.c
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

#define _PPL_GLOB_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <glob.h>

#include "ListTools/lt_memory.h"

#include "StringTools/str_constants.h"

#include "ppl_error.h"
#include "ppl_glob.h"

static char LastFilename[FNAME_LENGTH]="";

static int ppl_glob_SpecialFilename(char *fn)
 {
  int i;
  if (fn[0]=='\0') return 1;
  for (i=0; fn[i]!='\0'; i++) if ((fn[i]==':')&&(fn[i+1]=='/')&&(fn[i+2]=='/')) return 1;
  if (strcmp(fn,"-" )==0) return 1;
  if (strcmp(fn,"--")==0) return 1;
  return 0;
 }

char *ppl_glob_oneresult(char *filename)
 {
  char      *output, *outtemp;
  wordexp_t  WordExp;
  glob_t     GlobData;
  unsigned char GlobSet=0;

  if (filename[0]!='\0')
   {
    strncpy(LastFilename, filename, FNAME_LENGTH);
    LastFilename[FNAME_LENGTH-1]='\0';
   } else {
    filename = LastFilename;
   }

  if (ppl_glob_SpecialFilename(filename))
   {
    outtemp=filename;
   }
  else
   {
    if (wordexp(filename, &WordExp, 0) != 0) { sprintf(temp_err_string, "No file with filename '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); return NULL; }
    if (WordExp.we_wordc <= 0) { sprintf(temp_err_string, "No file with filename '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&WordExp); return NULL; }
    if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&WordExp); return NULL; }

    if (glob(WordExp.we_wordv[0], 0, NULL, &GlobData) != 0) { sprintf(temp_err_string, "No file with filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&WordExp); return NULL; }
    if (GlobData.gl_pathc <= 0) { sprintf(temp_err_string, "No file with filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return NULL; }
    if (GlobData.gl_pathc  > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return NULL; }
    outtemp=GlobData.gl_pathv[0];
    GlobSet=1;
   }

  output = (char *)lt_malloc(strlen(outtemp)+1);
  if (output==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory."); if (GlobSet) { wordfree(&WordExp); globfree(&GlobData); } return NULL; }
  strcpy(output, outtemp);
  if (GlobSet) { wordfree(&WordExp); globfree(&GlobData); }
  return output;
 }

ppl_glob *ppl_glob_allresults(char *filename)
 {
  ppl_glob  *output;
  char      *outtemp, *SpecialFilename=NULL;
  unsigned char GlobSet=0;

  if (filename[0]!='\0')
   {
    strncpy(LastFilename, filename, FNAME_LENGTH);
    LastFilename[FNAME_LENGTH-1]='\0';
   } else {
    filename = LastFilename;
   }

  output = (ppl_glob *)lt_malloc(sizeof(ppl_glob));
  if (output==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory."); return NULL; }

  if (ppl_glob_SpecialFilename(filename))
   {
    outtemp=filename;
    SpecialFilename = (char *)lt_malloc(strlen(outtemp)+1);
    if (SpecialFilename==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory."); return NULL; }
    strcpy(SpecialFilename, outtemp);
   }
  else
   {
    if (wordexp(filename, &output->WordExp, 0) != 0) { sprintf(temp_err_string, "No file with filename '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); return NULL; }
    if (output->WordExp.we_wordc <= 0) { sprintf(temp_err_string, "No file with filename '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&output->WordExp); return NULL; }

    if (glob(output->WordExp.we_wordv[0], 0, NULL, &output->GlobData) != 0) { sprintf(temp_err_string, "No file with filename '%s'.", output->WordExp.we_wordv[0]); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&output->WordExp); return NULL; }
    if (output->GlobData.gl_pathc <= 0) { sprintf(temp_err_string, "No file with filename '%s'.", output->WordExp.we_wordv[0]); ppl_error(ERR_FILE, -1, -1, temp_err_string); wordfree(&output->WordExp); globfree(&output->GlobData); return NULL; }
    GlobSet=1;
   }

  output->GlobDataSet     = GlobSet;
  output->WordExpSet      = GlobSet;
  output->SpecialFilename = SpecialFilename;
  output->SpecialCase     = (SpecialFilename!=NULL);
  output->ig              = 0;
  output->iw              = 0;
  return output;
 }

char *ppl_glob_iterate(ppl_glob *x)
 {
  char *output;

  if (x->SpecialCase)
   {
    output = x->SpecialFilename;
    x->SpecialFilename = NULL;
    return output;
   }

  if (x->GlobDataSet && (x->ig < x->GlobData.gl_pathc))
   {
    output = x->GlobData.gl_pathv[x->ig];
    x->ig++;
    return output;
   }
  x->ig=0;
  x->iw++;
  if (x->WordExpSet && (x->iw < x->WordExp.we_wordc))
   {
    if (glob(x->WordExp.we_wordv[x->iw], 0, NULL, &x->GlobData) != 0) { sprintf(temp_err_string, "No file with filename '%s'.", x->WordExp.we_wordv[x->iw]); ppl_error(ERR_FILE, -1, -1, temp_err_string); x->GlobDataSet = 0; return NULL; }
    x->GlobDataSet = 1;
    if (x->GlobData.gl_pathc <= 0) { sprintf(temp_err_string, "No file with filename '%s'.", x->WordExp.we_wordv[x->iw]); ppl_error(ERR_FILE, -1, -1, temp_err_string); return NULL; }
    output = x->GlobData.gl_pathv[x->ig];
    x->ig++;
    return output;
   }
  return NULL;
 }

void ppl_glob_close(ppl_glob *x)
 {
  if (x==NULL) return;
  if (x->GlobDataSet) globfree(&x->GlobData);
  if (x->WordExpSet ) wordfree(&x->WordExp);
  x->GlobDataSet = x->WordExpSet = 0;
  return;
 }

