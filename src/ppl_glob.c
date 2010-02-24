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

#include "ppl_error.h"
#include "ppl_glob.h"

char *ppl_glob_oneresult(char *filename)
 {
  char      *output;
  wordexp_t  WordExp;
  glob_t     GlobData;

  if (wordexp(filename, &WordExp, 0) != 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", filename); ppl_error(ERR_FILE, temp_err_string); return NULL; }
  if (WordExp.we_wordc <= 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", filename); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return NULL; }
  if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", filename); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return NULL; }

  if (glob(WordExp.we_wordv[0], 0, NULL, &GlobData) != 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return NULL; }
  if (GlobData.gl_pathc <= 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return NULL; }
  if  (GlobData.gl_pathc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return NULL; }

  output = (char *)lt_malloc(strlen(GlobData.gl_pathv[0])+1);
  if (output==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); wordfree(&WordExp); globfree(&GlobData); return NULL; }
  strcpy(output, GlobData.gl_pathv[0]);
  wordfree(&WordExp);
  globfree(&GlobData);
  return output;
 }

ppl_glob *ppl_glob_allresults(char *filename)
 {
  ppl_glob  *output;
  wordexp_t  WordExp;
  glob_t     GlobData;

  if (wordexp(filename, &WordExp, 0) != 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", filename); ppl_error(ERR_FILE, temp_err_string); return NULL; }
  if (WordExp.we_wordc <= 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", filename); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return NULL; }

  if (glob(WordExp.we_wordv[0], 0, NULL, &GlobData) != 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return NULL; }
  if (GlobData.gl_pathc <= 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return NULL; }

  output = (ppl_glob *)lt_malloc(sizeof(ppl_glob));
  if (output==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); wordfree(&WordExp); globfree(&GlobData); return NULL; }
  output->GlobData    = GlobData;
  output->WordExp     = WordExp;
  output->GlobDataSet = 1;
  output->WordExpSet  = 1;
  output->ig          = 0;
  output->iw          = 0;
  return output;
 }

char *ppl_glob_iterate(ppl_glob *x)
 {
  char *output;

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
    if (glob(x->WordExp.we_wordv[x->iw], 0, NULL, &x->GlobData) != 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", x->WordExp.we_wordv[x->iw]); ppl_error(ERR_FILE, temp_err_string); x->GlobDataSet = 0; return NULL; }
    x->GlobDataSet = 1;
    if (x->GlobData.gl_pathc <= 0) { sprintf(temp_err_string, "Could not glob filename '%s'.", x->WordExp.we_wordv[x->iw]); ppl_error(ERR_FILE, temp_err_string); return NULL; }
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

