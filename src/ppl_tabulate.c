// ppl_tabulate.c
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

#define _PPL_TABULATE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <wordexp.h>
#include <glob.h>

#include <gsl/gsl_math.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_passwd.h"
#include "ppl_settings.h"
#include "ppl_tabulate.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Display data from a data block
static int DataGridDisplay(FILE *output, DataTable *data, int Ncolumns, value **min, value **max)
 {
  DataBlock *blk;
  char *cptr;
  double tmpdbl, multiplier[USING_ITEMS_MAX];
  unsigned char InRange, split, AllInts[USING_ITEMS_MAX], AllSmall[USING_ITEMS_MAX];
  long int i,k;
  int j;
  double val;

  // Check that the FirstEntries above have the same units as any supplied ranges
  for (j=0; j<Ncolumns; j++)
   if (min[j] != NULL)
    {
     if (!ppl_units_DimEqual(min[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %d in the tabulate command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ppl_units_GetUnitStr(min[j],NULL,NULL,0,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    }
   else if (max[j] != NULL)
    {
     if (!ppl_units_DimEqual(max[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %d in the tabulate command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ppl_units_GetUnitStr(max[j],NULL,NULL,0,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
    }

  // Output a column units line
  fprintf(output, "# ColumnUnits: ");
  for (j=0; j<Ncolumns; j++)
   {
    if (data->FirstEntries[j].dimensionless)
     {
      fprintf(output, "1 "); // This column contains dimensionless data
      multiplier[j] = 1.0;
     }
    else
     {
      data->FirstEntries[j].real = 1.0;
      data->FirstEntries[j].imag = 0.0;
      data->FirstEntries[j].FlagComplex = 0;
      cptr = ppl_units_GetUnitStr(data->FirstEntries+j, multiplier+j, &tmpdbl, 0, SW_DISPLAY_T);
      for (i=0; ((cptr[i]!='\0')&&(cptr[i]!='(')); i++);
      i++; // Fastforward over opening bracket
      for (   ; ((cptr[i]!='\0')&&(cptr[i]!=')')); i++) fprintf(output, "%c", cptr[i]);
      fprintf(output, " ");
     }
   }
  fprintf(output, "\n");

  // Iterate over columns of data in this data grid, working out which columns of data are all ints, and which are all %f-able data
  for (j=0; j<Ncolumns; j++) { AllInts[j] = AllSmall[j] = 1; }
  blk = data->first;
  while (blk != NULL)
   {
    for (i=0; i<blk->BlockPosition; i++)
     {
      InRange=1;
      for (k=0; k<Ncolumns; k++)
       {
        val = blk->data_real[k + Ncolumns*i];
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
       }
      if (InRange)
       for (k=0; k<Ncolumns; k++)
        {
         val = blk->data_real[k + Ncolumns*i] * multiplier[k];
         if ((fabs(val)>1000) || (!ppl_units_DblEqual(val,floor(val+0.5)))) AllInts [k]=0;
         if ((fabs(val)>1000) || (fabs(val)<0.0999999999999)              ) AllSmall[k]=0;
        }
     }
    blk=blk->next;
   }

  // Iterate over columns of data in this data grid, working out which columns of data are all ints, and which are all %f-able data
  blk = data->first;
  split = 0;
  while (blk != NULL)
   {
    for (i=0; i<blk->BlockPosition; i++)
     {
      InRange=1;
      if (blk->split[i]) split=1;
      for (k=0; k<Ncolumns; k++)
       {
        val = blk->data_real[k + Ncolumns*i];
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
       }
      if (InRange)
       {
        if (split) { fprintf(output, "\n"); split=0; }
        for (k=0; k<Ncolumns; k++)
         {
          val = blk->data_real[k + Ncolumns*i] * multiplier[k];
          if      (AllInts [k]) fprintf(output, "%10d ", (int)val);
          else if (AllSmall[k]) fprintf(output, "%11f ",      val);
          else                  fprintf(output, "%15e ",      val);
         }
        fprintf(output, "\n");
       }
      else split=1;
     }
    blk=blk->next;
   }

  return 0;
 }

// Main entry point for the implementation of the tabulate command
int directive_tabulate(Dict *command, char *line)
 {
  FILE         *output;
  char         *cptr, *filename;
  wordexp_t     WordExp;
  glob_t        GlobData;
  int           i, status, iwe, igl, NUsingItems, ContextOutput, ContextLocalVec, ContextDataTab, index=-1, *indexptr, rowcol=DATAFILE_COL, ErrCount=DATAFILE_NERRS;;
  long int      j;
  value        *min[USING_ITEMS_MAX], *max[USING_ITEMS_MAX];
  DataTable    *data;
  List         *RangeList, *TabList;
  ListIterator *ListIter;
  Dict         *TempDict;
  char          errtext[LSTR_LENGTH], *tempstr=NULL, *SelectCrit=NULL;
  List         *UsingList=NULL, *EveryList=NULL;


  // Open output file and write header at the top of it
  filename = settings_term_current.output;
  if ((filename==NULL)||(filename[0]=='\0')) filename = "pyxplot.txt";
  output = fopen(filename , "w");
  if (output == NULL) { sprintf(temp_err_string, "The tabulate command could not open output file '%s' for writing.", filename); ppl_error(ERR_FILE, temp_err_string); return 1; }
  fprintf(output, "# Datafile generated by PyXPlot %s\n# Timestamp: %s\n", VERSION, StrStrip(FriendlyTimestring(),temp_err_string));
  fprintf(output, "# User: %s\n# PyXPlot command: %s\n\n", UnixGetIRLName(), line);

  // Read in specified data ranges
  DictLookup(command, "range_list", NULL, (void **)&RangeList);
  ListIter = ListIterateInit(RangeList);
  for (j=0; j<USING_ITEMS_MAX; j++) // Can have up to USING_ITEMS_MAX ranges
   if (ListIter == NULL) { min[j]=NULL; max[j]=NULL; }
   else
    {
     TempDict = (Dict *)ListIter->data;
     DictLookup(TempDict,"min",NULL,(void **)(min+j));
     DictLookup(TempDict,"max",NULL,(void **)(max+j));
     if ((min[j]!=NULL)&&(max[j]!=NULL)&&(!ppl_units_DimEqual(min[j],max[j]))) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %ld in the tabulate command have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", j+1, ppl_units_GetUnitStr(min[j],NULL,NULL,0,0), ppl_units_GetUnitStr(max[j],NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); fclose(output); return 1; }
     ListIter = ListIterate(ListIter, NULL);
    }
   if (ListIter != NULL) { sprintf(temp_err_string, "Too many ranges supplied to the tabulate command. %d ranges were supplied, but only a maximum of %d are supported.", ListLen(RangeList), USING_ITEMS_MAX); ppl_error(ERR_SYNTAX, temp_err_string); fclose(output); return 1; }

  // Iterate over all of the items we are about to tabulate
  DictLookup(command, "tabulate_list,", NULL, (void **)&TabList);
  i = ListLen(TabList);
  j=1;
  ListIter = ListIterateInit(TabList);
  while (ListIter != NULL)
   {
    TempDict = (Dict *)ListIter->data;
    fprintf(output, "\n\n\n# Index %ld\n", j); // Put a heading at the top of the new data index

    // Look up index , using , every modifiers to datafile reading
    DictLookup(TempDict, "index"      , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
    DictLookup(TempDict, "use_rows"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
    DictLookup(TempDict, "use_cols"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
    DictLookup(TempDict, "using_list:", NULL, (void **)&UsingList);
    NUsingItems = ListLen(UsingList);
    if (NUsingItems<2) NUsingItems = 2;
    DictLookup(TempDict, "every_list:", NULL, (void **)&EveryList);
    DictLookup(TempDict, "select_criterion", NULL, (void **)&SelectCrit);

    // Case 1: Plotting a datafile
    DictLookup(TempDict,"filename",NULL,(void **)(&cptr));
    if (cptr!=NULL)
     {
      // Expand filename if it contains wildcards
      status=0;
      if ((wordexp(cptr, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", cptr); ppl_error(ERR_FILE, temp_err_string); return 1; }
      for (iwe=0; iwe<WordExp.we_wordc; iwe++)
       {
        if ((glob(WordExp.we_wordv[iwe], 0, NULL, &GlobData) != 0) || (GlobData.gl_pathc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[j]); ppl_error(ERR_FILE, temp_err_string); wordfree(&WordExp); fclose(output); return 1; }
        for (igl=0; igl<GlobData.gl_pathc; igl++)
         {
          filename = GlobData.gl_pathv[igl];
          fprintf(output, "\n# %s\n\n", filename); // Start new data block for each globbed filename

          // Allocate a new memory context for the data file we're about to read
          ContextOutput  = lt_GetMemContext();
          ContextLocalVec= lt_DescendIntoNewContext();
          ContextDataTab = lt_DescendIntoNewContext();

          // Read data from file
          DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, NUsingItems, SelectCrit, DATAFILE_DISCONTINUOUS, &ErrCount);
          if (status) { ppl_error(ERR_GENERAL, errtext); globfree(&GlobData); wordfree(&WordExp); fclose(output); return 1; }
          status = DataGridDisplay(output, data, NUsingItems, min, max);
          if (status) { globfree(&GlobData); wordfree(&WordExp); fclose(output); return 1; }

          // We're finished... can now free DataTable
          lt_AscendOutOfContext(ContextLocalVec);
         }
        globfree(&GlobData);
       }
      wordfree(&WordExp);
     }
    else // Case 2: Plotting a function
     {
     }
    j++;
    ListIter = ListIterate(ListIter, NULL);
   }

  fclose(output);
  return 0;
 }

