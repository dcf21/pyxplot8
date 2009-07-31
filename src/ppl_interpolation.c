// ppl_interpolation.c
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

#define _PPL_INTERPOLATION_C 1

#include <stdlib.h>
#include <stdio.h>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_memory.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_interpolation.h"

int directive_interpolate(Dict *command, int mode)
 {
  DataTable *data;
  DataBlock *blk;
  int        i, j, k, ContextOutput, ContextDataTab, status=0, index=0, *indexptr, rowcol=DATAFILE_COL, continuity, ErrCount=5;
  char       errtext[LSTR_LENGTH], *filename=NULL, *tempstr=NULL, *SelectCrit=NULL;
  double    *xdata, *ydata;
  List      *UsingList=NULL, *EveryList=NULL;

  DictLookup(command, "filename"   , NULL, (void **)&filename);   if (filename == NULL) { ppl_error("Internal error: ppl_interpolation could not read filename."); return 1; }
  DictLookup(command, "index"      , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_row"    , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_col"    , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:", NULL, (void **)&UsingList);
  DictLookup(command, "every_list:", NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  if   (mode == INTERP_LOGLIN) continuity = DATAFILE_SORTEDLOGLOG;
  else                         continuity = DATAFILE_SORTED;

  ContextOutput  = lt_GetMemContext();
  ContextDataTab = lt_DescendIntoNewContext();

  DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, 2, SelectCrit, continuity, &ErrCount);
  if (status) { ppl_error(errtext); return 1; }

  xdata = (double *)lt_malloc_incontext(data->Nrows * sizeof(double), ContextOutput); // Transfer data from multiple data tables into single vectors
  ydata = (double *)lt_malloc_incontext(data->Nrows * sizeof(double), ContextOutput);

  if ((xdata==NULL)||(ydata==NULL)) { ppl_error("Error: Out of memory whilst reading data from input file."); return 1; }

  blk = data->first; i=0;
  while (blk != NULL)
   {
    k=i;  
    for (j=0; j<blk->BlockPosition; j++) xdata[i++] = blk->data_real[0 + 2*j];
    i=k;
    for (j=0; j<blk->BlockPosition; j++) ydata[i++] = blk->data_real[1 + 2*j];
    blk=blk->next;
   }

  // Free original data table which is no longer needed
  lt_AscendOutOfContext(ContextDataTab);

  return 0;
 }

