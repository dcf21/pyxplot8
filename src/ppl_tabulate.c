// ppl_tabulate.c
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
// Foundation; either version 2 of the License, -1, -1, or (at your option) any later
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

#include <gsl/gsl_math.h>

#include "MathsTools/dcfmath.h"

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_datafile_rasters.h"
#include "ppl_error.h"
#include "ppl_glob.h"
#include "ppl_passwd.h"
#include "ppl_settings.h"
#include "ppl_tabulate.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// Display data from a data block
static int DataGridDisplay(FILE *output, DataTable *data, int Ncolumns, value **min, value **max, char *format)
 {
  DataBlock *blk;
  char *cptr, tmpchr='\0';
  double tmpdbl, multiplier[USING_ITEMS_MAX];
  unsigned char InRange, split, AllInts[USING_ITEMS_MAX], AllSmall[USING_ITEMS_MAX];
  long int i,k;
  int j,l,pos;
  double val;

  // Check that the FirstEntries above have the same units as any supplied ranges
  for (j=0; j<Ncolumns; j++)
   if (min[j] != NULL)
    {
     if (!ppl_units_DimEqual(min[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %d in the tabulate command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ppl_units_GetUnitStr(min[j],NULL,NULL,0,1,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
    }
   else if (max[j] != NULL)
    {
     if (!ppl_units_DimEqual(max[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %d in the tabulate command have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ppl_units_GetUnitStr(max[j],NULL,NULL,0,1,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return 1; }
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
      cptr = ppl_units_GetUnitStr(data->FirstEntries+j, multiplier+j, &tmpdbl, 0, 1, SW_DISPLAY_T);
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
        val = blk->data_real[k + Ncolumns*i].d;
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
       }
      if (InRange)
       for (k=0; k<Ncolumns; k++)
        {
         val = blk->data_real[k + Ncolumns*i].d * multiplier[k];
         if ((fabs(val)>1000) || (!ppl_units_DblEqual(val,floor(val+0.5)))) AllInts [k]=0;
         if ((fabs(val)>1000) || (fabs(val)<0.0999999999999)              ) AllSmall[k]=0; // Columns containing only numbers in this range are fprintfed using %f, rather than %e
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
        val = blk->data_real[k + Ncolumns*i].d;
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
       }
      if (InRange)
       {
        if (split) { fprintf(output, "\n"); split=0; }
        if (format == NULL) // User has not supplied a format string, and so we just list the contents of each column in turn using best-fit format style
         {
          for (k=0; k<Ncolumns; k++)
           {
            val = blk->data_real[k + Ncolumns*i].d * multiplier[k];
            if      (AllInts [k]) fprintf(output, "%10d ", (int)val);
            else if (AllSmall[k]) fprintf(output, "%11f ",      val);
            else                  fprintf(output, "%15e ",      val);
           }
         } else { // The user has supplied a format string, which we now substitute column values into
          for(pos=l=0; format[pos]!='\0'; pos++)
           if (format[pos]!='%') fprintf(output, "%c", format[pos]); // Just copy text of format string until we hit a % character
           else
            {
             k=pos+1; // k looks ahead to see experimentally if syntax is right
             if (format[k]=='%') { fprintf(output, "%%"); continue; }
             if ((format[k]=='+')||(format[k]=='-')||(format[k]==' ')||(format[k]=='#')) k++; // optional flag can be <+- #>
             while ((format[k]>='0') && (format[k]<='9')) k++; // length can be some digits
             if (format[k]=='.') // precision starts with a . and is followed by more digits
              {
               k++; while ((format[k]>='0') && (format[k]<='9')) k++; // length can be some digits
              }
             // We do not allow user to specify optional length flag, which could potentially be <hlL>
             if (l>=Ncolumns) val = GSL_NAN;
             else             val = blk->data_real[l + Ncolumns*i].d * multiplier[l]; // Set val to equal data from data table that we're about to print.
             l++;
             if (format[k]!='\0') { tmpchr = format[k+1]; format[k+1] = '\0'; } // NULL terminate format token before passing it to fprintf
             if      (format[k]=='d') // %d -- print quantity as an integer, but take care to avoid overflows
              {
               if ((!gsl_finite(val))||(val>INT_MAX-1)||(val<INT_MIN+1)) fprintf(output, "nan");
               else                                                      fprintf(output, format+pos, (int)floor(val));
               pos = k;
              }
             else if ((format[k]=='e') || (format[k]=='f')) // %f or %e -- print quantity as floating point number
              {
               fprintf(output, format+pos, val);
               pos = k;
              }
             else if (format[k]=='s') // %s -- print quantity in our best-fit format style
              {
               if (l>Ncolumns)         sprintf(temp_err_string,"nan");
               else if (AllInts [l-1]) sprintf(temp_err_string, "%10d ", (int)val);
               else if (AllSmall[l-1]) sprintf(temp_err_string, "%11f ",      val);
               else                    sprintf(temp_err_string, "%15e ",      val);
               fprintf(output, format+pos, temp_err_string);
               pos = k;
              }
             else
              { fprintf(output, "%c", format[pos]); l--; } // l-- because this wasn't a valid format token, so we didn't print any data in the end
             if (format[k]!='\0') format[k+1] = tmpchr; // Remove temporary NULL termination at the end of the format token
            }
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
  char         *cptr, *cptr2, *filename, *format, FilenameTemp[FNAME_LENGTH];
  wordexp_t     WordExp;
  ppl_glob     *glob_handle;
  int           i, k, status, NUsingItems, ContextOutput, ContextLocalVec, ContextDataTab, index=-1, *indexptr, rowcol=DATAFILE_COL, ErrCount=DATAFILE_NERRS;
  int           NumberOfSamples;
  long int      j;
  value        *min[USING_ITEMS_MAX+1], *max[USING_ITEMS_MAX+1], *spacing;
  double       *ordinate_raster, raster_min, raster_max, SpacingDbl, NumberOfSamplesDbl;
  unsigned char raster_log, FlagParametric;
  value         raster_units, para_Tunit;
  DataTable    *data;
  List         *RangeList, *TabList, *ExprList;
  ListIterator *ListIter, *ExprListIter;
  Dict         *TempDict, *TempDict2, *TempExprDict;
  char          errtext[LSTR_LENGTH], *tempstr=NULL, *SelectCrit=NULL, *SortBy=NULL, *fnlist[USING_ITEMS_MAX];
  List         *UsingList=NULL, *EveryList=NULL;


  // Work out filename for output file
  filename = settings_term_current.output;
  if ((filename==NULL)||(filename[0]=='\0')) filename = "pyxplot.txt";

  // Perform expansion of shell filename shortcuts such as ~
  if ((wordexp(filename, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not find directory containing filename '%s'.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); return 1; }
  if  (WordExp.we_wordc > 1) { sprintf(temp_err_string, "Filename '%s' is ambiguous.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); return 1; }
  strcpy(FilenameTemp, WordExp.we_wordv[0]);
  wordfree(&WordExp);
  filename = FilenameTemp;

  // Open output file and write header at the top of it
  DataFile_CreateBackupIfRequired(filename);
  output = fopen(filename , "w");
  if (output == NULL) { sprintf(temp_err_string, "The tabulate command could not open output file '%s' for writing.", filename); ppl_error(ERR_FILE, -1, -1, temp_err_string); return 1; }
  fprintf(output, "# Datafile generated by PyXPlot %s\n# Timestamp: %s\n", VERSION, StrStrip(FriendlyTimestring(),temp_err_string));
  fprintf(output, "# User: %s\n# PyXPlot command: %s\n\n", UnixGetIRLName(), line);

  // Read in specified data ranges
  DictLookup(command, "range_list", NULL, (void **)&RangeList);
  ListIter = ListIterateInit(RangeList);
  for (j=0; j<USING_ITEMS_MAX+1; j++) // Can have up to USING_ITEMS_MAX ranges, plus first range which is for ordinate axis when tabulating functions
   if (ListIter == NULL) { min[j]=NULL; max[j]=NULL; }
   else
    {
     TempDict = (Dict *)ListIter->data;
     DictLookup(TempDict,"min",NULL,(void **)(min+j));
     DictLookup(TempDict,"max",NULL,(void **)(max+j));
     if ((min[j]!=NULL)&&(max[j]!=NULL)&&(!ppl_units_DimEqual(min[j],max[j]))) { sprintf(temp_err_string, "The minimum and maximum limits specified in range %ld in the tabulate command have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", j+1, ppl_units_GetUnitStr(min[j],NULL,NULL,0,1,0), ppl_units_GetUnitStr(max[j],NULL,NULL,1,1,0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); fclose(output); return 1; }
     ListIter = ListIterate(ListIter, NULL);
    }
   if (ListIter != NULL) { sprintf(temp_err_string, "Too many ranges supplied to the tabulate command. %d ranges were supplied, but only a maximum of %d are supported.", ListLen(RangeList), USING_ITEMS_MAX); ppl_error(ERR_SYNTAX, -1, -1, temp_err_string); fclose(output); return 1; }

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
    DictLookup(TempDict, "every_list:", NULL, (void **)&EveryList);
    DictLookup(TempDict, "select_criterion", NULL, (void **)&SelectCrit);
    DictLookup(TempDict, "sort_expression", NULL, (void **)&SortBy);
    DictLookup(TempDict, "format", NULL, (void **)(&format));

    // Case 1: Plotting a datafile
    DictLookup(TempDict,"filename",NULL,(void **)(&cptr));
    if (cptr!=NULL)
     {
      NUsingItems = ListLen(UsingList);
      if      (NUsingItems<1) NUsingItems = 2;
      else if (NUsingItems==1)
       {
        TempDict2 = (Dict *)(UsingList->first->data);
        if (TempDict2==NULL) { NUsingItems = 2; }
        else
         {
          DictLookup(TempDict2, "using_item", NULL, (void **)&cptr2);
          if (cptr2==NULL) {  NUsingItems = 2; }
         }
       }

      // Expand filename if it contains wildcards
      status=0;
      glob_handle = ppl_glob_allresults(cptr);
      if (glob_handle == NULL) return 1;
      while ((filename = ppl_glob_iterate(glob_handle)) != NULL)
       {
        fprintf(output, "\n# %s\n\n", filename); // Start new data block for each globbed filename

        // Allocate a new memory context for the data file we're about to read
        ContextOutput  = lt_GetMemContext();
        ContextLocalVec= lt_DescendIntoNewContext();
        ContextDataTab = lt_DescendIntoNewContext();

        // Read data from file
        DataFile_read(&data, &status, errtext, filename, *indexptr, rowcol, UsingList, 0, EveryList, NULL, NUsingItems, SelectCrit, DATAFILE_DISCONTINUOUS, SortBy, DATAFILE_CONTINUOUS, &ErrCount);
        if (status) { ppl_error(ERR_GENERAL, -1, -1, errtext); ppl_glob_close(glob_handle); fclose(output); return 1; }
        status = DataGridDisplay(output, data, NUsingItems, min, max, format);
        if (status) { ppl_glob_close(glob_handle); fclose(output); return 1; }

        // We're finished... can now free DataTable
        lt_AscendOutOfContext(ContextLocalVec);
       }
      ppl_glob_close(glob_handle);
     }
    else // Case 2: Plotting a function
     {
      // See whether we are plotting parametric or explicit function
      DictLookup(TempDict, "parametric", NULL, (void **)&cptr);
      FlagParametric = (cptr != NULL);

      // Read list of expressions supplied instead of filename
      DictLookup(TempDict, "expression_list:", NULL, (void **)&ExprList);
      NUsingItems = ListLen(UsingList);
      if      (NUsingItems<1) NUsingItems = ListLen(ExprList)+(!FlagParametric); // Only have x in column 1 for non-parametric function plotting
      else if (NUsingItems==1)
       {
        TempDict2 = (Dict *)(UsingList->first->data);
        if (TempDict2==NULL) { NUsingItems = ListLen(ExprList)+(!FlagParametric); }
        else
         {
          DictLookup(TempDict2, "using_item", NULL, (void **)&cptr2);
          if (cptr2==NULL) {  NUsingItems = ListLen(ExprList)+(!FlagParametric); }
         }
       }

      k=0;
      ExprListIter = ListIterateInit(ExprList);
      while (ExprListIter != NULL)
       {
        TempExprDict = (Dict *)ExprListIter->data;
        if (k>=USING_ITEMS_MAX) {sprintf(temp_err_string, "Colon-separated lists of expressions can only contain a maximum of %d expressions.",USING_ITEMS_MAX); ppl_error(ERR_SYNTAX, -1, -1,temp_err_string);}
        DictLookup(TempExprDict, "expression", NULL, (void **)(fnlist+k));
        k++;
        ExprListIter = ListIterate(ExprListIter, NULL);
       }

      // Allocate a new memory context for the data file we're about to read
      ContextOutput  = lt_GetMemContext();
      ContextLocalVec= lt_DescendIntoNewContext();
      ContextDataTab = lt_DescendIntoNewContext();

      // Work out the range we are going to use for the ordinate raster
      if (!FlagParametric)
       {
        raster_log = (XAxes[1].log == SW_BOOL_TRUE); // Read from axis x1
        if      (min[0] != NULL)                  { raster_min = min[0]->real;                                            raster_units = *(min[0]);     }
        else if (XAxes[1].MinSet == SW_BOOL_TRUE) { raster_min = XAxes[1].min;                                            raster_units = XAxes[1].unit; }
        else if (max[0] != NULL)                  { raster_min = raster_log ? (max[0]->real / 100) : (max[0]->real - 20); raster_units = *(max[0]);     }
        else if (XAxes[1].MaxSet == SW_BOOL_TRUE) { raster_min = raster_log ? (XAxes[1].max / 100) : (XAxes[1].max - 20); raster_units = XAxes[1].unit; }
        else                                      { raster_min = raster_log ?  1.0                 : -10.0;               ppl_units_zero(&raster_units);}
        if      ((max[0] != NULL)                  && (ppl_units_DimEqual(&raster_units,max[0]          ))) raster_max = max[0]->real;
        else if ((XAxes[1].MaxSet == SW_BOOL_TRUE) && (ppl_units_DimEqual(&raster_units,&(XAxes[1].unit)))) raster_max = XAxes[1].max;
        else                                                                                                raster_max = raster_log ? (raster_min * 100) : (raster_min + 20);
       } else {
        value *tempval1, *tempval2;
        raster_log = (settings_graph_current.Tlog == SW_BOOL_TRUE);
        raster_min = settings_graph_current.Tmin.real;
        raster_max = settings_graph_current.Tmax.real;
        para_Tunit = settings_graph_current.Tmin;
        DictLookup(TempDict, "tmin", NULL, (void **)&tempval1);
        DictLookup(TempDict, "tmax", NULL, (void **)&tempval2);
        if (tempval1 != NULL)
         {
          if      (!gsl_finite(tempval1->real)) { sprintf(temp_err_string, "Lower limit specified for parameter t is not finite."); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); }
          else if (!gsl_finite(tempval2->real)) { sprintf(temp_err_string, "Upper limit specified for parameter t is not finite."); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); }
          else if (!ppl_units_DimEqual(tempval1,tempval2)) { sprintf(temp_err_string, "Upper and lower limits specified for parameter t have conflicting physical units of <%s> and <%s>.", ppl_units_GetUnitStr(tempval1, NULL, NULL, 0, 1, 0), ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 1, 0)); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); }
          else
           {
            raster_min = tempval1->real;
            raster_max = tempval2->real;
            para_Tunit = *tempval1;
           }
         }
       }
      if (raster_log && ((raster_min<=0) || (raster_max<=0))) ppl_warning(ERR_NUMERIC,"Attempt to tabulate data using a logarithmic ordinate axis with negative or zero limits set. Reverting limits to finite positive values with well-defined logarithms.");

      // See if spacing has already been specified
      DictLookup(TempDict, "spacing", NULL, (void **)&spacing);
      if (spacing != NULL)
       {
        if (raster_log)
         {
          if (!spacing->dimensionless) { sprintf(temp_err_string, "The tabulate command has been passed a spacing with units of <%s> for a logarithmic ordinate axis. The spacing should be a dimensionless multiplicative factor.", ppl_units_GetUnitStr(spacing,NULL,NULL,0,1,0)); fclose(output); return 1; }
          if ((spacing->real < 1e-200)||(spacing->real > 1e100)) { sprintf(temp_err_string, "The spacing specified to the tabulate command must be a positive multiplicative factor for logarithmic ordinate axes."); fclose(output); return 1; }
          SpacingDbl = spacing->real;
          if ((raster_max > raster_min) && (SpacingDbl < 1.0)) SpacingDbl = 1.0/SpacingDbl;
          if ((raster_min > raster_max) && (SpacingDbl > 1.0)) SpacingDbl = 1.0/SpacingDbl;
          NumberOfSamplesDbl = 1.0 + floor(log(raster_max / raster_min) / log(SpacingDbl));
          if (NumberOfSamplesDbl<2  ) { sprintf(temp_err_string, "The spacing specified to the tabulate command produced fewer than two samples; this does not seem sensible."); fclose(output); return 1; }
          if (NumberOfSamplesDbl>1e7) { sprintf(temp_err_string, "The spacing specified to the tabulate command produced more than 1e7 samples. If you really want to do this, use 'set samples'."); fclose(output); return 1; }
          raster_max = raster_min * pow(SpacingDbl, NumberOfSamplesDbl-1);
          NumberOfSamples = (int)NumberOfSamplesDbl;
         } else {
          if (!ppl_units_DimEqual(&raster_units,spacing)) { sprintf(temp_err_string, "The tabulate command has been passed a spacing with units of <%s> for an ordinate axis which has units of <%s>,", ppl_units_GetUnitStr(spacing,NULL,NULL,0,1,0), ppl_units_GetUnitStr(&raster_units,NULL,NULL,1,1,0)); fclose(output); return 1; }
          SpacingDbl = spacing->real;
          if ((raster_max > raster_min) && (SpacingDbl < 0.0)) SpacingDbl = -SpacingDbl;
          if ((raster_min > raster_max) && (SpacingDbl > 0.0)) SpacingDbl = -SpacingDbl;
          NumberOfSamplesDbl = 1.0 + (raster_max - raster_min) / SpacingDbl;
          if (NumberOfSamplesDbl<2  ) { sprintf(temp_err_string, "The spacing specified to the tabulate command produced fewer than two samples; this does not seem sensible."); fclose(output); return 1; }
          if (NumberOfSamplesDbl>1e7) { sprintf(temp_err_string, "The spacing specified to the tabulate command produced more than 1e7 samples. If you really want to do this, use 'set samples'."); fclose(output); return 1; }
          raster_max = raster_min + SpacingDbl*(NumberOfSamplesDbl+1);
          NumberOfSamples = (int)NumberOfSamplesDbl;
         }
       } else {
        NumberOfSamples = settings_graph_current.samples;
       }

      // Generate the raster of ordinate values at which we will evaluate any functions
      ordinate_raster = lt_malloc(NumberOfSamples * sizeof(double));
      if (raster_log) LogarithmicRaster(ordinate_raster, raster_min, raster_max, NumberOfSamples);
      else            LinearRaster     (ordinate_raster, raster_min, raster_max, NumberOfSamples);

      // Read data from supplied functions
      if (spacing == NULL)
       {
        DataFile_FromFunctions_CheckSpecialRaster(fnlist, k, FlagParametric?"t":"x",
                          ((!FlagParametric) && (min[0]!=NULL)) ? &raster_min : NULL,
                          ((!FlagParametric) && (max[0]!=NULL)) ? &raster_max : NULL,
                          &ordinate_raster, &NumberOfSamples);
       }
      DataFile_FromFunctions(ordinate_raster, FlagParametric,
                             NumberOfSamples, (FlagParametric ? &para_Tunit : &raster_units),
                             NULL, 0, NULL,
                             &data, &status, errtext, fnlist, k, UsingList, 0, NULL, NUsingItems, SelectCrit, DATAFILE_DISCONTINUOUS, SortBy, DATAFILE_CONTINUOUS, &ErrCount);
      if (status) { ppl_error(ERR_GENERAL, -1, -1, errtext); fclose(output); return 1; }
      status = DataGridDisplay(output, data, NUsingItems, min+1, max+1, format); // First range is for ordinate axis
      if (status) { fclose(output); return 1; }

      // We're finished... can now free DataTable
      lt_AscendOutOfContext(ContextLocalVec);
     }
    j++;
    ListIter = ListIterate(ListIter, NULL);
   }

  fclose(output);
  return 0;
 }

