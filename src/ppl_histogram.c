// ppl_histogram.c
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

#define _PPL_HISTOGRAM_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wordexp.h>
#include <glob.h>

#include <gsl/gsl_math.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_histogram.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Main entry point for the implementation of the histogram command
int directive_histogram(Dict *command)
 {
  DataTable *data;
  //DataBlock *blk;
  //long int   i, j, k, Nrows;
  int        ContextOutput, ContextLocalVec, ContextDataTab, status=0, index=-1, *indexptr, rowcol=DATAFILE_COL, continuity, ErrCount=DATAFILE_NERRS;
  char       errtext[LSTR_LENGTH], *cptr, *filename=NULL, *histfunc=NULL, *tempstr=NULL, *SelectCrit=NULL;
  wordexp_t  WordExp;
  glob_t     GlobData;
  //double    *xdata, *ydata;
  List      *UsingList=NULL, *EveryList=NULL;
  FunctionDescriptor *FuncPtr /*, *FuncPtrNext, *FuncPtr2 */ ;
  //value      v, FirstEntries[2];
  value        *xmin=NULL, *xmax=NULL;

  // Expand filename if it contains wildcards
  DictLookup(command,"filename",NULL,(void **)(&cptr));
  if (cptr==NULL) ppl_error(ERR_INTERNAL, "File attribute not found in histogram command.");
  status=0;
  if ((wordexp(cptr, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
  if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
  if ((glob(WordExp.we_wordv[0], 0, NULL, &GlobData) != 0) || (GlobData.gl_pathc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); return 1; }
  if  (GlobData.gl_pathc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", WordExp.we_wordv[0]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); globfree(&GlobData); return 1; }
  filename = lt_malloc(strlen(GlobData.gl_pathv[0])+1);
  if (filename==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); wordfree(&WordExp); globfree(&GlobData); return 1; }
  strcpy(filename, GlobData.gl_pathv[0]);
  wordfree(&WordExp);
  globfree(&GlobData);

  DictLookup(command, "hist_function", NULL, (void **)&histfunc);   if (histfunc == NULL) { ppl_error(ERR_INTERNAL, "ppl_histogram could not read name of function for output."); return 1; }
  DictLookup(command, "index"        , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_rows"     , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_cols"     , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:"  , NULL, (void **)&UsingList);
  DictLookup(command, "every_list:"  , NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  DictLookup(command,"min",NULL,(void **)&xmin);
  DictLookup(command,"max",NULL,(void **)&xmax);
  if ((xmin!=NULL)&&(xmax!=NULL)&&(!ppl_units_DimEqual(xmin,xmax))) { sprintf(temp_err_string, "The minimum and maximum limits specified in the histogram command for the x axis have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", ppl_units_GetUnitStr(xmin,NULL,NULL,0,0), ppl_units_GetUnitStr(xmax,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }

  continuity = DATAFILE_CONTINUOUS;

  // Check that the function we're about to replace isn't a system function
  DictLookup(_ppl_UserSpace_Funcs, histfunc, NULL, (void *)&FuncPtr); // Check whether we are going to overwrite an existing function
  if ((FuncPtr!=NULL)&&((FuncPtr->FunctionType==PPL_USERSPACE_SYSTEM)||(FuncPtr->FunctionType==PPL_USERSPACE_STRFUNC)||(FuncPtr->FunctionType==PPL_USERSPACE_UNIT)))
   { sprintf(temp_err_string, "Attempt to redefine a core system function %s()", histfunc); ppl_error(ERR_GENERAL, temp_err_string); return 1; }

  ContextOutput  = lt_GetMemContext();
  ContextLocalVec= lt_DescendIntoNewContext();
  ContextDataTab = lt_DescendIntoNewContext();

  DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, 2, SelectCrit, continuity, &ErrCount);
  if (status) { ppl_error(ERR_GENERAL, errtext); return 1; }

  return 0;
 }

