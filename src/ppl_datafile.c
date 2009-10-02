// ppl_datafile.c
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

#define _PPL_DATAFILE_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

#include "ppl_children.h"
#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_input.h"
#include "ppl_settings.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

// -------------------------------------------------------------------------
// Routine which creates a backup of a datafile if 'set backup' is turned on
// -------------------------------------------------------------------------

void DataFile_CreateBackupIfRequired(char *filename)
 {
  char newname[FNAME_LENGTH];
  int i,j;

  if (settings_term_current.backup == SW_ONOFF_OFF) return; // Backup is switched off
  if (access(filename, F_OK) != 0) return; // File we're about to write to does not already exist

  strcpy(newname, filename);
  i = strlen(filename);
  for (j=0 ; j<65536 ; j++)
   {
    sprintf(newname+i, "~%d",j);
    if (access(newname, F_OK) != 0) break; // We've found a backup file which does not already exist
   }
  rename(filename, newname);
  return;
 }

// -----------------------------------------------------------------------------------------------
// Routine used to launch input filters as coprocesses with piped output returned as a file handle
// -----------------------------------------------------------------------------------------------

FILE *DataFile_LaunchCoProcess(char *filename, int *status, char *errout)
 {
  FILE         *infile;
  DictIterator *DictIter;
  char         *filter, *FilterArgs, **ArgList;
  int           i,j,k;
  sigset_t      sigs;

  sigemptyset(&sigs);
  sigaddset(&sigs,SIGCHLD);

  // Check whether we have a specified coprocessor to work on this filetype
  DictIter = DictIterateInit(settings_filters);
  while (DictIter != NULL)
   {
    if (StrWildcardTest(filename, DictIter->key))
     {
      filter = ((value *)DictIter->data)->string;
      if (DEBUG) { sprintf(temp_err_string, "Using input filter '%s'.", filter); ppl_log(temp_err_string); }
      FilterArgs = (char  *)lt_malloc(strlen(filter)+1);
      ArgList    = (char **)lt_malloc((strlen(filter)/2+1)*sizeof(char *));
      if ((FilterArgs==NULL)||(ArgList==NULL)) { sprintf(errout,"Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return NULL; };
      strcpy(FilterArgs, filter);
      for (i=j=k=0; FilterArgs[i]!='\0'; i++)
       {
        if      ((k==0) && (FilterArgs[i]> ' ')) { k=1; ArgList[j++] = FilterArgs+i; }
        else if ((k==1) && (FilterArgs[i]<=' ')) { k=0; FilterArgs[i] = '\0'; }
       }
      ArgList[j++] = filename;
      ArgList[j++] = NULL;
      ForkInputFilter(ArgList, &i); // Fork process for input filter, and runned piped output through the standard IO library using fdopen()
      sigprocmask(SIG_UNBLOCK, &sigs, NULL);
      if ((infile = fdopen(i, "r")) == NULL) { sprintf(errout,"Could not open connection to input filter '%s'.",ArgList[0]); *status=1; if (DEBUG) ppl_log(errout); return NULL; };
      return infile;
     }
    DictIter = DictIterate(DictIter, NULL, NULL);
   }

  // If not, then we just open the file and return a file-handle to it
  if ((infile = fopen(filename, "r")) == NULL) { sprintf(errout,"Could not open input file '%s'.",filename); *status=1; if (DEBUG) ppl_log(errout); return NULL; };
  return infile;
 }

// -------------------------------------------------------------------------------------------------
// The following routines are a part of the BODMAS machine, and evaluate "$3" in the current context
// -------------------------------------------------------------------------------------------------

static            int     UCFC_OutputContext   = -1;
static            char  **UCFC_columns         = NULL;
static            int     UCFC_Ncols           = -1;
static const      char   *UCFC_filename        = NULL;
static  long int          UCFC_file_linenumber = -1;
static const long int    *UCFC_file_linenumbers= NULL;
static       long int     UCFC_linenumber_count= -1;
static       long int     UCFC_block_count     = -1;
static       long int     UCFC_index_number    = -1;
static            int     UCFC_UsingRowCol     = -1;
static const      char   *UCFC_RowColWord      = NULL;
static const      char   *UCFC_UsingExpression = NULL;
static            char  **UCFC_ColumnHeads     = NULL;
static            int     UCFC_NColumnHeads    = -1;
static const      value  *UCFC_ColumnUnits     = NULL;
static            int     UCFC_NColumnUnits    = -1;

void __inline__ DataFile_UCFC_configure(const int OutputContext, char **columns, const int Ncols, const char *filename, const long int file_linenumber, const long int *file_linenumbers, const long int linenumber_count, const long int block_count, const long int index_number, const int UsingRowCol, const char *RowColWord, const char *UsingExpression, char **ColumnHeads, const int NColumnHeads, const value *ColumnUnits, const int NColumnUnits)
 {
  UCFC_OutputContext    = OutputContext;
  UCFC_columns          = columns;
  UCFC_Ncols            = Ncols;
  UCFC_filename         = filename;
  UCFC_file_linenumber  = file_linenumber;
  UCFC_file_linenumbers = file_linenumbers;
  UCFC_linenumber_count = linenumber_count;
  UCFC_block_count      = block_count;
  UCFC_index_number     = index_number;
  UCFC_UsingRowCol      = UsingRowCol;
  UCFC_RowColWord       = RowColWord;
  UCFC_UsingExpression  = UsingExpression;
  UCFC_ColumnHeads      = ColumnHeads;
  UCFC_NColumnHeads     = NColumnHeads;
  UCFC_ColumnUnits      = ColumnUnits;
  UCFC_NColumnUnits     = NColumnUnits;
  return;
 }

void __inline__ DataFile_UCFC_deconfigure()
 {
  UCFC_OutputContext    = -1;
  UCFC_columns          = NULL;
  UCFC_Ncols            = -1;
  UCFC_filename         = NULL;
  UCFC_file_linenumber  = -1;
  UCFC_file_linenumbers = NULL;
  UCFC_linenumber_count = -1;
  UCFC_block_count      = -1;
  UCFC_index_number     = -1;
  UCFC_UsingRowCol      = -1;
  UCFC_RowColWord       = NULL;
  UCFC_UsingExpression  = NULL;
  UCFC_ColumnHeads      = NULL;
  UCFC_NColumnHeads     = -1;
  UCFC_ColumnUnits      = NULL;
  UCFC_NColumnUnits     = -1;
  return;
 }

void __inline__ DataFile_UsingConvert_FetchColumnByNumber(double ColumnNo, value *output, const int NumericOut, const unsigned char MallocOut, int *status, char *errtext)
 {
  int          i,j=-1;
  long int     k;
  const  char *outstr=NULL;
  static char  buffer[32];

  if ((ColumnNo<-10)||(ColumnNo>MAX_DATACOLS)) { sprintf(errtext, "%s:%ld: In the expression <%s>, the requested %s number %f does not exist %son line %ld.", UCFC_filename, UCFC_file_linenumber, UCFC_UsingExpression, UCFC_RowColWord, ColumnNo, (UCFC_UsingRowCol==DATAFILE_COL)?"":"in the block commencing ", UCFC_file_linenumber); *status=1; return; }
  i=(int)floor(ColumnNo);
  if ((i<-3)||(i>UCFC_Ncols)) { sprintf(errtext, "%s:%ld: In the expression <%s>, the requested %s number %d does not exist %son line %ld.", UCFC_filename, UCFC_file_linenumber, UCFC_UsingExpression, UCFC_RowColWord, i, (UCFC_UsingRowCol==DATAFILE_COL)?"":"in the block commencing ", UCFC_file_linenumber); *status=1; return; }
  if (!NumericOut)
   {
    if      (i==-3) { sprintf(buffer,"%ld",UCFC_file_linenumber ); outstr=buffer; }
    else if (i==-2) { sprintf(buffer,"%ld",UCFC_index_number    ); outstr=buffer; }
    else if (i==-1) { sprintf(buffer,"%ld",UCFC_block_count     ); outstr=buffer; }
    else if (i== 0) { sprintf(buffer,"%ld",UCFC_linenumber_count); outstr=buffer; }
    else      { outstr=UCFC_columns[i-1]; }
    goto RETURN_STRING;
   }
  if      (i==-3) ColumnNo = UCFC_file_linenumber;
  else if (i==-2) ColumnNo = UCFC_index_number;
  else if (i==-1) ColumnNo = UCFC_block_count;
  else if (i== 0) ColumnNo = UCFC_linenumber_count;
  else
   {
    ColumnNo=GetFloat(UCFC_columns[i-1],&j);
    if (UCFC_file_linenumbers == NULL) k = UCFC_file_linenumber;
    else                               k = UCFC_file_linenumbers[i-1];
    if (((UCFC_columns[i-1][j]>' ')&&(UCFC_columns[i-1][j]!=','))||(j<=0)) { sprintf(errtext, "%s:%ld: In the expression <%s>, the requested %s number %d does not contain numeric data.", UCFC_filename, k, UCFC_UsingExpression, UCFC_RowColWord, i); *status=1; return; } // j==0 for blank columns in CSV, and data off the end of rows
   }
  ppl_units_zero(output);
  output->real = ColumnNo;
  if ((i>0)&&(i<UCFC_NColumnUnits)) ppl_units_mult(output,UCFC_ColumnUnits+i-1,output,status,errtext);
  return;

RETURN_STRING:
  ppl_units_zero(output); 
  if (MallocOut)
   {
    output->string = (char *)lt_malloc_incontext(strlen(outstr),UCFC_OutputContext);
    if (output->string == NULL) { sprintf(errtext, "Out of memory."); *status=1; if (DEBUG) ppl_log(errtext); return; }
    strcpy(output->string,outstr);
   }
  else
   {
    output->string = (char *)outstr;
   }
  return;
 }

void __inline__ DataFile_UsingConvert_FetchColumnByName(char *ColumnName, value *output, const int NumericOut, const unsigned char MallocOut, int *status, char *errtext)
 {
  int i;
  for (i=0;i<UCFC_NColumnHeads;i++)
   if (strcmp(UCFC_ColumnHeads[i],ColumnName)==0)
    {
     DataFile_UsingConvert_FetchColumnByNumber((double)i+1.1, output, NumericOut, MallocOut, status, errtext); // ColumnHead at position 0 corresponds to column 1
     return;
    }
  sprintf(errtext, "%s:%ld: In the expression <%s>, the requested %s named '%s' does not exist %son line %ld.", UCFC_filename, UCFC_file_linenumber, UCFC_UsingExpression, UCFC_RowColWord, ColumnName, (UCFC_UsingRowCol==DATAFILE_COL)?"":"in the block commencing ", UCFC_file_linenumber);
  *status=1;
  return;
 }

// DataFile_UsingConvert(): Top level entry point for evaluating expressions supplied to the 'using' modifier
//
//   input = string from a using :...: clause.
//   output = output of what this clause computes to.
//   OutputContext = the lt_memory context in which memory should be malloced if we're returning a string.
//   columns = text in each column (or row) of datafile.
//   Ncols = number of items in the array 'columns'.
//   filename = filename of the data source we are currently working on.
//   LineNo = the number of the line we should return as $0.
//   NumericOut = if true, we're returning a number. Otherwise, a string.
//   ColumnHeads = column names.
//   NColumnHeads = Length of above array.
//   ColumnUnits = column units.
//   NColumnUnits = Length of above array.
//   status = 0 on success. 1 on failure.
//   errtext = error text output.

void __inline__ DataFile_UsingConvert(char *input, value *output, const int OutputContext, char **columns, const int Ncols, const char *filename, const long int file_linenumber, const long int *file_linenumbers, const long int linenumber_count, const long int block_count, const long int index_number, const int UsingRowCol, const char *RowColWord, const int NumericOut, char **ColumnHeads, const int NColumnHeads, const value *ColumnUnits, const int NColumnUnits, int *status, char *errtext)
 {
  double dbl;
  int i,l=strlen(input);
  unsigned char NamedColumn=0;

  DataFile_UCFC_configure(OutputContext, columns, Ncols, filename, file_linenumber, file_linenumbers, linenumber_count, block_count, index_number, UsingRowCol, RowColWord, input, ColumnHeads, NColumnHeads, ColumnUnits, NColumnUnits); // Configure the Using Convert Fetch Column routines above

  if (ValidFloat(input,&l)) // using 1:2 -- number is column number -- "1" means "the contents of column 1", i.e. "$1".
   { dbl=GetFloat(input,NULL); NamedColumn=1; }
  else
   { for (i=0;i<NColumnHeads;i++) if (strcmp(ColumnHeads[i],input)==0) { dbl=i+1.1; NamedColumn=1; break; } } // using ColumnName

  if (NamedColumn) // Clean up these cases
   {
    DataFile_UsingConvert_FetchColumnByNumber(dbl, output, NumericOut, 1, status, errtext);
   }
  else
   {
    *status=-1;
    if (NumericOut) ppl_EvaluateAlgebra(input, output, 0, &l, 1, status, errtext, 1); // Supplied expression is not a column name or number; it is an expression we need to evaluate
    else
     {
      ppl_GetQuotedString(input, temp_err_string, 0, &l, 1, status, errtext, 1); // We are trying to return a string, so expect to be evaluating a quoted string
      if (*status<0)
       {
        ppl_units_zero(output);
        output->string = (char *)lt_malloc_incontext(strlen(temp_err_string),UCFC_OutputContext);
        if (output->string == NULL) { sprintf(errtext, "Out of memory."); *status=1; if (DEBUG) ppl_log(errtext); return; }
        strcpy(output->string, temp_err_string);
       }
     }
    if (*status>=0) { if (*status<20000) { sprintf(temp_err_string, "%s:%ld: Could not evaluate expression <%s>. The error, encountered at character position %d, was: '%s'", filename, file_linenumber, input, *status, errtext); strcpy(errtext, temp_err_string); } *status=1; } else { *status=0; }
   }

  DataFile_UCFC_deconfigure(); // Put the UCFC routines above into 'neutral'; this shouldn't be necessary but means they fail gracefully if called after this point
  return;
 }

// -------------------------------------------------------------------------------
// Routines for mallocing and adding data to DataTable and RawDataTable structures
// -------------------------------------------------------------------------------

// DataFile_NewDataBlock()
DataBlock *DataFile_NewDataBlock(const int Ncolumns, const int MemoryContext, const int Length)
 {
  DataBlock *output;
  int BlockLength = 1 + DATAFILE_DATABLOCK_BYTES / (sizeof(double) + sizeof(long int)) / Ncolumns; // automatic length

  if (Length>0) BlockLength=Length; // overriden when manually specified Length > 0

  output = (DataBlock *)lt_malloc_incontext(sizeof(DataBlock), MemoryContext);
  if (output==NULL) return NULL;
  output->data_real  = (double *)       lt_malloc_incontext(BlockLength * Ncolumns * sizeof(double       ), MemoryContext);
  output->text       = (char **)        lt_malloc_incontext(BlockLength *            sizeof(char *       ), MemoryContext);
  output->FileLine   = (long int *)     lt_malloc_incontext(BlockLength * Ncolumns * sizeof(long int     ), MemoryContext);
  output->split      = (unsigned char *)lt_malloc_incontext(BlockLength *            sizeof(unsigned char), MemoryContext);
  output->BlockLength   = BlockLength;
  output->BlockPosition = 0;
  output->next          = NULL;
  output->prev          = NULL;
  if ((output->data_real==NULL)||(output->text==NULL)||(output->FileLine==NULL)||(output->split==NULL)) return NULL;
  return output;
 }

// DataFile_NewDataTable()
DataTable *DataFile_NewDataTable(const int Ncolumns, const int MemoryContext, const int Length)
 {
  DataTable *output;
  int i;

  output = (DataTable *)lt_malloc_incontext(sizeof(DataTable), MemoryContext);
  if (output==NULL) return NULL;
  output->Ncolumns      = Ncolumns;
  output->Nrows         = 0;
  output->MemoryContext = MemoryContext;
  output->FirstEntries  = (value *)lt_malloc_incontext(Ncolumns*sizeof(value), MemoryContext);
  if (output->FirstEntries==NULL) return NULL;
  for (i=0;i<Ncolumns;i++) ppl_units_zero(output->FirstEntries + i);
  output->first         = DataFile_NewDataBlock(Ncolumns, MemoryContext, Length);
  output->current       = output->first;
  if (output->first==NULL) return NULL;
  return output;
 }

// DataFile_NewRawDataBlock()
RawDataBlock *DataFile_NewRawDataBlock(const int MemoryContext)
 {
  RawDataBlock *output;
  int BlockLength = 1 + DATAFILE_DATABLOCK_BYTES / (sizeof(char *) + sizeof(long int));

  output = (RawDataBlock *)lt_malloc_incontext(sizeof(RawDataBlock), MemoryContext);
  if (output==NULL) return NULL;
  output->text       = (char **)        lt_malloc_incontext(BlockLength * sizeof(char *       ), MemoryContext);
  output->FileLine   = (long int *)     lt_malloc_incontext(BlockLength * sizeof(long int     ), MemoryContext);
  output->BlockLength   = BlockLength;
  output->BlockPosition = 0;
  output->next          = NULL;
  output->prev          = NULL;
  if ((output->text==NULL)||(output->FileLine==NULL)) return NULL;
  return output;
 }

// DataFile_NewRawDataTable()
RawDataTable *DataFile_NewRawDataTable(const int MemoryContext)
 {
  RawDataTable *output;

  output = (RawDataTable *)lt_malloc_incontext(sizeof(RawDataTable), MemoryContext);
  if (output==NULL) return NULL;
  output->Nrows         = 0;
  output->MemoryContext = MemoryContext;
  output->first         = DataFile_NewRawDataBlock(MemoryContext);
  output->current       = output->first;
  if (output->first==NULL) return NULL;
  return output;
 }

// DataFile_DataTable_AddRow()
int DataFile_DataTable_AddRow(DataTable *i)
 {
  if (i==NULL) return 1;
  if (i->current==NULL) return 1;
  i->Nrows++;
  if (i->current->BlockPosition < (i->current->BlockLength-1)) { i->current->BlockPosition++; return 0; }
  i->current->next          = DataFile_NewDataBlock(i->Ncolumns, i->MemoryContext, -1);
  if (i->current==NULL) return 1;
  i->current->BlockPosition = i->current->BlockLength;
  i->current->next->prev    = i->current;
  i->current                = i->current->next;
  return 0;
 }

// DataFile_RawDataTable_AddRow()
int DataFile_RawDataTable_AddRow(RawDataTable *i)
 {
  if (i==NULL) return 1;
  if (i->current==NULL) return 1;
  i->Nrows++;
  if (i->current->BlockPosition < (i->current->BlockLength-1)) { i->current->BlockPosition++; return 0; }
  i->current->next          = DataFile_NewRawDataBlock(i->MemoryContext);
  if (i->current==NULL) return 1;
  i->current->BlockPosition = i->current->BlockLength;
  i->current->next->prev    = i->current;
  i->current                = i->current->next;
  if (i->current==NULL) return 1;
  return 0;
 }

// ------------------------------------------------------------------------
// Routine used for debugging purposes to list the contents of a data table
// ------------------------------------------------------------------------

void DataFile_DataTable_List(DataTable *i)
 {
  DataBlock *blk;
  value v;
  int j,k,Ncolumns;

  if (i==NULL) { printf("<NULL data table>\n"); return; }
  printf("Table size: %d x %ld\n", i->Ncolumns, i->Nrows);
  printf("Memory context: %d\n", i->MemoryContext);
  blk = i->first;
  Ncolumns = i->Ncolumns;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      if (blk->split[j]) printf("\n\n");
      for (k=0; k<Ncolumns; k++)
       {
        v             = i->FirstEntries[k];
        v.real        = blk->data_real[j*Ncolumns + k];
        v.imag        = 0.0;
        v.FlagComplex = 0;
        printf("%15s [line %6ld]  ",ppl_units_NumericDisplay(&v, 0, 0, 0), blk->FileLine[j*Ncolumns + k] );
       }
      if (blk->text[j] != NULL) printf("Label: <%s>",blk->text[j]);
      printf("\n");
     }
    blk=blk->next;
   }
 }

// ----------------------------------------------------------------------------------------------------------------------------------------------------
// DataFile_ApplyUsingList(): Convert an array of strings into a DataTable data point, using supplied using :...: expressions and select criterion, etc
// ----------------------------------------------------------------------------------------------------------------------------------------------------

#define COUNTEDERR1 if (*ErrCounter > 0) { (*ErrCounter)--;
#define COUNTEDERR2 if (*ErrCounter==0) { sprintf(temp_err_string, "%s:%ld: Too many errors: no more errors will be shown.",filename,file_linenumber); ppl_warning(ERR_STACKED, temp_err_string); } }

void DataFile_ApplyUsingList(DataTable *out, int ContextOutput, char **ColumnData, int ItemsOnLine, char **UsingItems, int Ncolumns, char *filename, long int file_linenumber, long int *file_linenumbers, long int linenumber_count, long int block_count, long int index_number, int UsingRowCol, char *RowColWord, char **ColumnHeadings, int NColumnHeadings, value *ColumnUnits, int NColumnUnits, char *LabelStr, char *SelectCriterion, int continuity, unsigned char *discontinuity, int *ErrCounter, int *status, char *errout)
 {
  int   i, LocalStatus;
  value tempval;

  // If select criterion has been supplied, make sure it is true before proceeding
  LocalStatus=0;
  if (SelectCriterion != NULL)
   {
    LocalStatus=0;
    DataFile_UsingConvert(SelectCriterion, &tempval, ContextOutput, ColumnData, ItemsOnLine, filename, file_linenumber, file_linenumbers, linenumber_count, block_count, index_number, UsingRowCol, RowColWord, 1, ColumnHeadings, NColumnHeadings, ColumnUnits, NColumnUnits, &LocalStatus, errout);
    if (LocalStatus) { COUNTEDERR1; ppl_warning(ERR_STACKED, errout); COUNTEDERR2; ppl_units_zero(&tempval); }
    if (!tempval.dimensionless) { COUNTEDERR1; sprintf(errout, "%s:%ld: Select criteria should return dimensionless quantities. The supplied select criterion <%s> returns a value with units of <%s>.", filename, file_linenumber, SelectCriterion, ppl_units_GetUnitStr(&tempval, NULL, NULL, 0, 0)); ppl_warning(ERR_STACKED, errout); COUNTEDERR2; ppl_units_zero(&tempval); }
    if (ppl_units_DblEqual(tempval.real,0)&&ppl_units_DblEqual(tempval.imag,0)) LocalStatus=1; // Do not proceed
   }

  // Only proceed if select criterion was TRUE or not present
  if (!LocalStatus)
   {
    // If a label expression has been specified, evaluate it now
    if (LabelStr != NULL)
     {
      DataFile_UsingConvert(LabelStr, &tempval, ContextOutput, ColumnData, ItemsOnLine, filename, file_linenumber, file_linenumbers, linenumber_count, block_count, index_number, UsingRowCol, RowColWord, 0, ColumnHeadings, NColumnHeadings, ColumnUnits, NColumnUnits, &LocalStatus, errout);
      if ((!LocalStatus) && (tempval.string==NULL)) { LocalStatus=1; }
      if (!LocalStatus)
       {
        out->current->text[out->current->BlockPosition] = (char *)lt_malloc_incontext(strlen(tempval.string)+1, ContextOutput);
        if (out->current->text[out->current->BlockPosition] == NULL)
         { sprintf(errout, "%s:%ld: Out of memory whilst generating text label for data point.",filename,file_linenumber); *status=1; if (DEBUG) ppl_log(errout); return; }
        else
         strcpy(out->current->text[out->current->BlockPosition], tempval.string);
       }
      if (LocalStatus)
       {
        COUNTEDERR1; ppl_warning(ERR_STACKED, errout); COUNTEDERR2;
        out->current->text[out->current->BlockPosition] = NULL;
       }
     }
    else
     out->current->text[out->current->BlockPosition] = NULL;

    // Evaluate using expression on data in columns
    for (i=0; i<Ncolumns; i++)
     {
      DataFile_UsingConvert(UsingItems[i], &tempval, ContextOutput, ColumnData, ItemsOnLine, filename, file_linenumber, file_linenumbers, linenumber_count, block_count, index_number, UsingRowCol, RowColWord, 1, ColumnHeadings, NColumnHeadings, ColumnUnits, NColumnUnits, &LocalStatus, errout);
      if (LocalStatus) break;
      if (tempval.FlagComplex) { COUNTEDERR1; sprintf(errout, "%s:%ld: Data item calculated from expression <%s> is complex.", filename, file_linenumber, UsingItems[i]); COUNTEDERR2; LocalStatus=1; break; }
      if (out->Nrows==0)
       out->FirstEntries[i] = tempval;
      else
       if (!ppl_units_DimEqual(&tempval, out->FirstEntries+i)) { COUNTEDERR1; sprintf(errout, "%s:%ld: The expression <%s> produces data with inconsistent units. On this line, a datum with units of <%s> is produced, but previous data have had units of <%s>.", filename, file_linenumber, UsingItems[i], ppl_units_GetUnitStr(&tempval, NULL, NULL, 0, 0), ppl_units_GetUnitStr(out->FirstEntries+i, NULL, NULL, 1, 0)); COUNTEDERR2; LocalStatus=1; break; }
      out->current->data_real[i + out->current->BlockPosition * Ncolumns] = tempval.real;
      out->current->FileLine [i + out->current->BlockPosition * Ncolumns] = file_linenumber;
     }

    out->current->split[out->current->BlockPosition] = *discontinuity;

    if (LocalStatus)
     {
      COUNTEDERR1; ppl_warning(ERR_STACKED, errout); COUNTEDERR2;
      *discontinuity=(continuity == DATAFILE_DISCONTINUOUS);
     }
    else // If we have evaluated all USING expressions successfully, commit this row to the DataTable
     {
      LocalStatus = DataFile_DataTable_AddRow(out);
      if (LocalStatus) { strcpy(errout, "Out of memory whilst reading data file."); *status=1; if (DEBUG) ppl_log(errout); return; }
      *discontinuity=0;
     }
   }
  return;
 }

// -----------------------------------------------------------------------------------------------
// DataFile_RotateRawData(): Routine used to rotate raw data by 90 degrees when plotting with rows
// -----------------------------------------------------------------------------------------------

void DataFile_RotateRawData(RawDataTable **in, DataTable *out, char **UsingItems, int Ncolumns, char *filename, long int block_count, long int index_number, char **RowHeadings, int NRowHeadings, value *RowUnits, int NRowUnits, char *LabelStr, char *SelectCriterion, int continuity, int *ErrCounter, int *status, char *errout)
 {
  RawDataBlock *blk;
  int           i,GotData;
  long          linenumber_count=0;

  char         *RowData[MAX_DATACOLS];
  int           ItemsOnLine;
  unsigned char discontinuity = 1;
  int           ContextRaw = (*in )->MemoryContext;
  int           ContextOut = ( out)->MemoryContext;

  long int      file_linenumber[MAX_DATACOLS];

  while (1)
   {
    GotData = ItemsOnLine = 0;
    blk = (*in)->first;
    while ((blk!=NULL)&&(ItemsOnLine<MAX_DATACOLS))
     {
      for (i=0; ((i<blk->BlockPosition)&&(ItemsOnLine<MAX_DATACOLS)); i++)
       {
        if (blk->text[i]==NULL)
         { RowData[ItemsOnLine++]=" "; }
        else
         {
          while ((blk->text[i][0]!='\0') && (blk->text[i][0]<=' ')) blk->text[i]++;
          if (blk->text[i][0]=='\0') RowData[ItemsOnLine++]=" ";
          else
           {
            RowData        [ItemsOnLine] = blk->text    [i];
            file_linenumber[ItemsOnLine] = blk->FileLine[i];
            ItemsOnLine++;
            GotData = 1;
            while  (blk->text[i][0]> ' ' )                            blk->text[i]++;
            while ((blk->text[i][0]!='\0') && (blk->text[i][0]<=' ')) blk->text[i]++;
           }
         }
       }
      blk = blk->next;
     }
    if (!GotData) break;
    DataFile_ApplyUsingList(out, ContextOut, RowData, ItemsOnLine, UsingItems, Ncolumns, filename, file_linenumber[0], file_linenumber, linenumber_count, block_count, index_number, DATAFILE_ROW, "row", RowHeadings, NRowHeadings, RowUnits, NRowUnits, LabelStr, SelectCriterion, continuity, &discontinuity, ErrCounter, status, errout);
    linenumber_count++;
   }

  lt_Free(ContextRaw); // Free input table
  (*in) = DataFile_NewRawDataTable(ContextRaw);
  return;
 }

// ----------------------------------------------------------------------------------
// DataFile_read is the main entry point for reading a table of data from a data file
// ----------------------------------------------------------------------------------

void DataFile_read(DataTable **output, int *status, char *errout, char *filename, int index, int UsingRowCol, List *UsingList, List *EveryList, char *LabelStr, int Ncolumns, char *SelectCriterion, int continuity, int *ErrCounter)
 {
  unsigned char AutoUsingList=0, ReadFromCommandLine=0, discontinuity=0, hadwhitespace, hadcomma, OneColumnInput=1;
  int           UsingLen, logi, logj, ContextOutput, ContextRough, ContextRaw;
  char         *UsingItems[USING_ITEMS_MAX], LineNumberStr[32];
  ListIterator *listiter;
  Dict         *tempdict;
  int           linestep=1, blockstep=1, linefirst=-1, blockfirst=-1, linelast=-1, blocklast=-1;
  long int      index_number, linenumber_count, linenumber_stepcnt, block_count, block_stepcnt, prev_blanklines, file_linenumber, ItemsOnLine;
  FILE         *filtered_input;
  char          linebuffer[LSTR_LENGTH], *lineptr, *cptr;

  int i, j, k, l, m, *intptr;

  char        **ColumnHeadings  = NULL;
  int           NColumnHeadings = 0;
  char        **RowHeadings     = NULL;
  int           NRowHeadings    = 0;
  value        *ColumnUnits     = NULL;
  int           NColumnUnits    = 0;
  value        *RowUnits        = NULL;
  int           NRowUnits       = 0;

  char         *ColumnData[MAX_DATACOLS];
  RawDataTable *RawDataTab = NULL;

  // Init
  if (DEBUG) { sprintf(temp_err_string, "Opening datafile '%s'.", filename); ppl_log(temp_err_string); }
  lineptr = linebuffer;
  if (Ncolumns != 2) OneColumnInput=0; // Only have special handling for one-column datafiles when looking for two columns

  // Read items out of Using List into an array of strings
  UsingLen=0; listiter = ListIterateInit(UsingList);
  while (listiter != NULL)
   {
    if (UsingLen>=USING_ITEMS_MAX) { strcpy(errout,"Too many items in using list."); *status=1; if (DEBUG) ppl_log(errout); return; }
    tempdict = (Dict *)listiter->data;
    DictLookup(tempdict,"using_item",NULL,(void **)&(UsingItems[UsingLen]));
    if ((UsingItems[UsingLen]==NULL)||(strlen(StrStrip(UsingItems[UsingLen], UsingItems[UsingLen]))==0)) // Using item is blank; replace with item number
     {
      if ((UsingItems[UsingLen] = (char *)lt_malloc(10))==NULL) { sprintf(errout,"Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return; };
      sprintf(UsingItems[UsingLen], "%d", UsingLen+1);
     }
    UsingLen++;
    listiter = ListIterate(listiter, NULL);
   }

  // If using list was empty, generate an automatic list
  if (UsingLen==0)
   {
    for (i=0; i<Ncolumns; i++)
     {
      if ((UsingItems[i] = (char *)lt_malloc(10))==NULL) { sprintf(errout,"Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return; };
      sprintf(UsingItems[i], "%d", i+1);
     }
    UsingLen = Ncolumns;
    AutoUsingList = 1; // We have automatically generated this using list
   }
  else if (UsingLen==1) // Prepend data point number if only one number specified in using statement
   {
    UsingItems[1] = UsingItems[0];
    UsingItems[0] = "0";
    UsingLen++;
   }

  // Output using list to log file if required
  if (DEBUG)
   {
    logi=0; sprintf(temp_err_string+logi, "Using "); logi+=strlen(temp_err_string+logi);
    for (logj=0; logj<UsingLen; logj++)
     {
      if (logj!=0) temp_err_string[logi++]=':';
      strcpy(temp_err_string+logi, UsingItems[logj]);
      logi+=strlen(temp_err_string+logi);
     }
    ppl_log(temp_err_string);
   }

  // If our using list has the wrong number of elements for the plot style we're aiming to use, then reject it
  if (UsingLen != Ncolumns) { sprintf(errout,"The supplied using ... clause contains the wrong number of items. We need %d columns of data, but %d have been supplied.", Ncolumns, UsingLen); *status=1; if (DEBUG) ppl_log(errout); return; }

  // Read items out of Every List
  listiter = ListIterateInit(EveryList);
  if   (listiter == NULL) { linestep   =  1; } // READ linestep
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if ((intptr==NULL) || (*intptr<1)) { linestep  =  1; } else { linestep   = *intptr; } }
  if   (listiter == NULL) { blockstep  =  1; } // READ blockstep
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if ((intptr==NULL) || (*intptr<1)) { blockstep =  1; } else { blockstep  = *intptr; } }
  if   (listiter == NULL) { linefirst  = -1; } // READ linefirst
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if ((intptr==NULL) || (*intptr<0)) { linefirst = -1; } else { linefirst  = *intptr; } }
  if   (listiter == NULL) { blockfirst = -1; } // READ blockfirst
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if ((intptr==NULL) || (*intptr<0)) { blockfirst= -1; } else { blockfirst = *intptr; } }
  if   (listiter == NULL) { linelast   = -1; } // READ linelast
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if ((intptr==NULL) || (*intptr<0)) { linelast  = -1; } else { linelast   = *intptr; } }
  if   (listiter == NULL) { blocklast  = -1; } // READ blocklast
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if ((intptr==NULL) || (*intptr<0)) { blocklast = -1; } else { blocklast  = *intptr; } }
  if   (listiter != NULL) { strcpy(errout, "More than six items specified in every modifier -- final items are not valid syntax."); *status=1; if (DEBUG) ppl_log(errout); return; }

  // Output every list to log file if required
  if (DEBUG) { sprintf(temp_err_string, "Every %d:%d:%d:%d:%d:%d", linestep, blockstep, linefirst, blockfirst, linelast, blocklast); ppl_log(temp_err_string); }

  // Reset the counters which we use to monitor our position in the file
  index_number       = 0;
  linenumber_count   = 0;
  linenumber_stepcnt = 0;
  block_count        = 0;
  block_stepcnt      = 0;
  prev_blanklines    = 10; // The number of blank lines we've hit since the last datapoint
  file_linenumber    = 0;

  // Open the requested datafile
  if      (strcmp(filename,"-" )==0) { filtered_input = stdin;  if (DEBUG) ppl_log("Reading from stdin.");       } // Special filename '-'  means we read from stdin
  else if (strcmp(filename,"--")==0) { ReadFromCommandLine = 1; if (DEBUG) ppl_log("Reading from commandline."); } // Special filename '--' means we read from the commandline
  else
   {
    filtered_input = DataFile_LaunchCoProcess(filename, status, errout);
    if (*status>0) return; // There was a problem launching coprocess or opening file
   }

  // Keep a record of the memory context we're going to output into, and then make a scratchpad context
  ContextOutput = lt_GetMemContext();
  ContextRough  = lt_DescendIntoNewContext(); // Rough mallocs inside this subroutine happen in this context
  ContextRaw    = lt_DescendIntoNewContext(); // Raw data goes into here when plotting with rows

  *output = DataFile_NewDataTable(Ncolumns, ContextOutput, -1);
  if (*output == NULL) { strcpy(errout, "Out of memory whilst trying to allocate data table to read data from file."); *status=1; if (DEBUG) ppl_log(errout); return; }
  if (UsingRowCol == DATAFILE_ROW)
   {
    RawDataTab = DataFile_NewRawDataTable(ContextRaw);
    if (RawDataTab == NULL) { strcpy(errout, "Out of memory whilst trying to allocate data table to read data from file."); *status=1; if (DEBUG) ppl_log(errout); return; }
   }

  // Read input file, line by line
  while ( (!ferror(filtered_input)) && (!feof(filtered_input)) )
   {
    if (!ReadFromCommandLine) file_readline(filtered_input, linebuffer, LSTR_LENGTH);
    else                      lineptr = FetchInputStatement("data ..> ",".......> ");

    if (ReadFromCommandLine && (strcmp(StrStrip(lineptr, linebuffer),"END")==0)) break;

    file_linenumber++;
    StrStrip(lineptr, linebuffer);

    for (j=0; ((linebuffer[j]!='\0')&&(linebuffer[j]<=' ')); j++);
    if (linebuffer[j]=='\0') // We have a blank line
     {
      if (prev_blanklines>1) continue; // A very long gap in the datafile
      prev_blanklines++;
      if (prev_blanklines==1)
       {
        block_count++; // First newline gives us a new block
        block_stepcnt = ((block_stepcnt-1) % blockstep);
        discontinuity=1; // We want to break the line here
        linenumber_count=0; // Zero linecounter within block
        linenumber_stepcnt=0;
       } else {
        index_number++; // Second newline gives us a new index
        block_count=0; // Zero block counter
        block_stepcnt=0;
        if (DEBUG) { sprintf(temp_err_string, "Index %ld begins on line %ld of datafile.", index_number, file_linenumber); ppl_log(temp_err_string); }
        if ((index>=0) && (index_number>index)) break; // We'll never find any more data once we've gone past the specified index
       }
      continue;
     }

    // Ignore comment lines
    if (linebuffer[j]=='#')
     {
      for (i=j+1; ((linebuffer[i]!='\0')&&(linebuffer[i]<=' ')); i++);
      if ((strncmp(linebuffer+i, "Columns:", 8)==0) && (UsingRowCol == DATAFILE_COL)) // '# Columns:' means we have a list of column headings
       {
        i+=8;
        if (DEBUG) { sprintf(temp_err_string,"Reading column headings as specified on line %ld of datafile",file_linenumber); ppl_log(temp_err_string); }
        while ((linebuffer[i]!='\0')&&(linebuffer[i]<=' ')) i++;
        ItemsOnLine = 0; hadwhitespace = 1;
        for (j=i; linebuffer[j]!='\0'; j++)
         {
          if (linebuffer[j]<=' ') { hadwhitespace = 1; }
          else if (hadwhitespace) { ItemsOnLine++; hadwhitespace = 0; }
         }
        cptr            = (char  *)lt_malloc_incontext(j-i+1                     , ContextRough); strcpy(cptr, linebuffer+i);
        ColumnHeadings  = (char **)lt_malloc_incontext(ItemsOnLine*sizeof(char *), ContextRough);
        NColumnHeadings = ItemsOnLine;
        if ((cptr==NULL)||(ColumnHeadings==NULL)) { strcpy(errout, "Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return; }
        ItemsOnLine = 0; hadwhitespace = 1;
        for (j=0; cptr[j]!='\0'; j++)
         {
          if      (cptr[j]<=' ')  { cptr[j]='\0'; hadwhitespace = 1; }
          else if (hadwhitespace) { ColumnHeadings[ItemsOnLine] = cptr+j; ItemsOnLine++; hadwhitespace = 0; }
         }
        if (DEBUG)
         {
          sprintf(temp_err_string,"Total of %ld column headings read.",ItemsOnLine); ppl_log(temp_err_string);
          for (k=0; k<ItemsOnLine; k++) { sprintf(temp_err_string,"Column heading %d: %s",k,ColumnHeadings[k]); ppl_log(temp_err_string); }
         }
       }
      else if ((strncmp(linebuffer+i, "Rows:", 5)==0) && (UsingRowCol == DATAFILE_ROW)) // '# Rows:' means we have a list of row headings
       {
        i+=5;
        if (DEBUG) { sprintf(temp_err_string,"Reading row headings as specified on line %ld of datafile",file_linenumber); ppl_log(temp_err_string); }
        while ((linebuffer[i]!='\0')&&(linebuffer[i]<=' ')) i++;
        ItemsOnLine = 0; hadwhitespace = 1;
        for (j=i; linebuffer[j]!='\0'; j++)
         {
          if (linebuffer[j]<=' ') { hadwhitespace = 1; }
          else if (hadwhitespace) { ItemsOnLine++; hadwhitespace = 0; }
         }
        cptr            = (char  *)lt_malloc_incontext(j-i+1                     , ContextRough); strcpy(cptr, linebuffer+i);
        RowHeadings     = (char **)lt_malloc_incontext(ItemsOnLine*sizeof(char *), ContextRough);
        NRowHeadings    = ItemsOnLine;
        if ((cptr==NULL)||(RowHeadings==NULL)) { strcpy(errout, "Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return; }
        ItemsOnLine = 0; hadwhitespace = 1;
        for (j=0; cptr[j]!='\0'; j++)
         {
          if      (cptr[j]<=' ')  { cptr[j]='\0'; hadwhitespace = 1; }
          else if (hadwhitespace) { RowHeadings[ItemsOnLine] = cptr+j; ItemsOnLine++; hadwhitespace = 0; }
         }
        if (DEBUG)
         {
          sprintf(temp_err_string,"Total of %ld row headings read.",ItemsOnLine); ppl_log(temp_err_string);
          for (k=0; k<ItemsOnLine; k++) { sprintf(temp_err_string,"Row heading %d: %s",k,RowHeadings[k]); ppl_log(temp_err_string); }
         }
       }
      else if (strncmp(linebuffer+i, "ColumnUnits:", 12)==0) // '# ColumnUnits:' means we have a list of column units
       {
        i+=12;
        if (DEBUG) { sprintf(temp_err_string,"Reading column units as specified on line %ld of datafile",file_linenumber); ppl_log(temp_err_string); }
        while ((linebuffer[i]!='\0')&&(linebuffer[i]<=' ')) i++;
        ItemsOnLine = 0; hadwhitespace = 1;
        for (j=i; linebuffer[j]!='\0'; j++)
         {
          if (linebuffer[j]<=' ') { hadwhitespace = 1; }
          else if (hadwhitespace) { ItemsOnLine++; hadwhitespace = 0; }
         }
        cptr         = (char  *)lt_malloc_incontext(j-i+2                    , ContextRough); strncpy(cptr, linebuffer+i, j-i+2);
        ColumnUnits  = (value *)lt_malloc_incontext(ItemsOnLine*sizeof(value), ContextRough);
        NColumnUnits = ItemsOnLine;
        if ((cptr==NULL)||(ColumnUnits==NULL)) { strcpy(errout, "Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return; }
        ItemsOnLine = 0; hadwhitespace = 1;
        for (k=0; cptr[k]!='\0'; k++)
         {
          if (cptr[k]<=' ') { continue; }
          else
           {
            for (l=0; cptr[k+l]>' '; l++);
            cptr[k+l]='\0'; m=-1;
            ppl_units_StringEvaluate(cptr+k, ColumnUnits+ItemsOnLine, &l, &m, errout);
            if (m>=0)
             {
              ppl_units_zero(ColumnUnits+ItemsOnLine);
              ColumnUnits[ItemsOnLine].real = 1.0;
              COUNTEDERR1; sprintf(temp_err_string,"%s:%ld:%d: %s",filename,file_linenumber,i+k,errout); ppl_warning(ERR_STACKED, temp_err_string); COUNTEDERR2;
             }
            ItemsOnLine++; k+=l;
           }
         }
        if (DEBUG)
         {
          sprintf(temp_err_string,"Total of %ld column units read.",ItemsOnLine); ppl_log(temp_err_string);
          for (k=0; k<ItemsOnLine; k++) { sprintf(temp_err_string,"Column unit %d: %s",k,ppl_units_NumericDisplay(ColumnUnits+k,0,0,0)); ppl_log(temp_err_string); }
         }
       }
      else if (strncmp(linebuffer+i, "RowUnits:", 9)==0) // '# RowUnits:' means we have a list of row units
       {
        i+=9;
        if (DEBUG) { sprintf(temp_err_string,"Reading row units as specified on line %ld of datafile",file_linenumber); ppl_log(temp_err_string); }
        while ((linebuffer[i]!='\0')&&(linebuffer[i]<=' ')) i++;
        ItemsOnLine = 0; hadwhitespace = 1;
        for (j=i; linebuffer[j]!='\0'; j++)
         {
          if (linebuffer[j]<=' ') { hadwhitespace = 1; }
          else if (hadwhitespace) { ItemsOnLine++; hadwhitespace = 0; }
         }
        cptr         = (char  *)lt_malloc_incontext(j-i+2                    , ContextRough); strncpy(cptr, linebuffer+i, j-i+2);
        RowUnits     = (value *)lt_malloc_incontext(ItemsOnLine*sizeof(value), ContextRough);
        NRowUnits    = ItemsOnLine;
        if ((cptr==NULL)||(RowUnits==NULL)) { strcpy(errout, "Out of memory."); *status=1; if (DEBUG) ppl_log(errout); return; }
        ItemsOnLine = 0; hadwhitespace = 1;
        for (k=0; cptr[k]!='\0'; k++)
         {
          if (cptr[k]<=' ') { continue; }
          else
           {
            for (l=0; cptr[k+l]>' '; l++);
            cptr[k+l]='\0'; m=-1;
            ppl_units_StringEvaluate(cptr+k, RowUnits+ItemsOnLine, &l, &m, errout);
            if (m>=0)
             {
              ppl_units_zero(RowUnits+ItemsOnLine);
              RowUnits[ItemsOnLine].real = 1.0;
              COUNTEDERR1; sprintf(temp_err_string,"%s:%ld:%d: %s",filename,file_linenumber,i+k,errout); ppl_warning(ERR_STACKED, temp_err_string); COUNTEDERR2;
             }
            ItemsOnLine++; k+=l;
           }
         }
        if (DEBUG)
         {
          sprintf(temp_err_string,"Total of %ld row units read.",ItemsOnLine); ppl_log(temp_err_string);
          for (k=0; k<ItemsOnLine; k++) { sprintf(temp_err_string,"Row unit %d: %s",k,ppl_units_NumericDisplay(RowUnits+k,0,0,0)); ppl_log(temp_err_string); }
         }
       }
      continue; // Ignore comment lines
     }
    prev_blanklines=0;

    // If we're in an index we're ignoring, don't do anything with this line
    if ((index>=0) && (index_number != index)) continue;

    // If we're in a block that we're ignoring, don't do anything with this line
    if ((block_stepcnt!=0) || ((blockfirst>=0)&&(block_count<blockfirst)) || ((blocklast>=0)&&(block_count>blocklast))) continue;

    // Fetch this line if linenumber is within range, or if we are using rows and we need everything
    if (UsingRowCol == DATAFILE_ROW) // Store the whole line into a raw text spool
     {
      if (discontinuity) DataFile_RotateRawData(&RawDataTab, *output, UsingItems, Ncolumns, filename, block_count, index_number, RowHeadings, NRowHeadings, RowUnits, NRowUnits, LabelStr, SelectCriterion, continuity, ErrCounter, status, errout);
      if (*status) return;
      cptr = RawDataTab->current->text[RawDataTab->current->BlockPosition] = (char *)lt_malloc_incontext(strlen(linebuffer)+1, ContextRaw);
      if (cptr==NULL) { strcpy(errout, "Out of memory whilst placing data into text spool."); *status=1; if (DEBUG) ppl_log(errout); return; }
      strcpy(cptr, linebuffer);
      RawDataTab->current->FileLine[RawDataTab->current->BlockPosition] = file_linenumber;
      if (DataFile_RawDataTable_AddRow(RawDataTab)) { strcpy(errout, "Out of memory whilst placing data into text spool."); *status=1; if (DEBUG) ppl_log(errout); return; }
      discontinuity = 0;
     }
    else if ((linenumber_stepcnt==0) && ((linefirst<0)||(linenumber_count>=linefirst)) && ((linelast<0)||(linenumber_count<=linelast)))
     {
      // Count the number of data items on this line
      ItemsOnLine = 0; hadwhitespace = 1; hadcomma = 0;
      for (i=0; linebuffer[i]!='\0'; i++)
       {
        if      (linebuffer[i]<=' ') {  hadwhitespace = 1; }
        else if (linebuffer[i]==',') { ColumnData[ItemsOnLine++]=linebuffer+i+1; hadwhitespace = hadcomma = 1; }
        else                         { if (hadwhitespace && !hadcomma) { ColumnData[ItemsOnLine++]=linebuffer+i; } hadwhitespace = hadcomma = 0; }
        if (ItemsOnLine==MAX_DATACOLS) break; // Don't allow ColumnData array to overflow
       }

      // Add line numbers as first column to one-column datafiles
      if  (ItemsOnLine >  1) OneColumnInput=0;
      if ((ItemsOnLine == 1) && (OneColumnInput))
       {
        ColumnData[ItemsOnLine++]=ColumnData[0];
        ColumnData[0]=LineNumberStr;
        sprintf(LineNumberStr,"%ld",file_linenumber);
       }

      DataFile_ApplyUsingList(*output, ContextOutput, ColumnData, ItemsOnLine, UsingItems, Ncolumns, filename, file_linenumber, NULL, linenumber_count, block_count, index_number, DATAFILE_COL, "column", ColumnHeadings, NColumnHeadings, ColumnUnits, NColumnUnits, LabelStr, SelectCriterion, continuity, &discontinuity, ErrCounter, status, errout);
      if (*status) return;
     }
    linenumber_count++;
    linenumber_stepcnt = ((linenumber_stepcnt-1) % linestep);
   }

  // If we are reading rows, go through all of the data that we've read and rotate it by 90 degrees
  if (UsingRowCol == DATAFILE_ROW) { DataFile_RotateRawData(&RawDataTab, *output, UsingItems, Ncolumns, filename, block_count, index_number, RowHeadings, NRowHeadings, RowUnits, NRowUnits, LabelStr, SelectCriterion, continuity, ErrCounter, status, errout); if (*status) return; }

  // Debugging line
  // DataFile_DataTable_List(*output);

  // Delete rough workspace
  lt_AscendOutOfContext(ContextRough);
  return;
 }

