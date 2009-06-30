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

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

#include "ppl_datafile.h"
#include "ppl_input.h"

FILE *DataFile_LaunchCoProcess(char *filename, int *status, char *errout, int *ErrCounter)
 {
  FILE *infile;
  // Check whether we have a specified coprocessor to work on this filetype

  // ....

  // If not, then we just open the file and return a file-handle to it
  if ((infile = fopen(filename, "r")) == NULL) { if (*ErrCounter>0) { (*ErrCounter)--; sprintf(errout,"Could not open input file '%s'.",filename); } *status=1; return NULL; };
  return infile;
 }

void DataFile_read(DataTable *output, int *status, char *errout, char *filename, int index, int UsingRowCol, List *UsingList, List *EveryList, int Ncolumns, char *SelectCriterion, int continuity, int *ErrCounter)
 {
  unsigned char AutoUsingList=0, ReadFromCommandLine=0, discontinuity=0, hadwhitespace, hadcomma; // OneColumnInput=1;
  int           UsingLen;
  char         *UsingItems[USING_ITEMS_MAX];
  ListIterator *listiter;
  Dict         *tempdict;
  int           linestep, blockstep, linefirst, blockfirst, linelast, blocklast;
  long int      index_number, linenumber_count, linenumber_stepcnt, block_count, block_stepcnt, prev_blanklines, file_linenumber, ItemsOnLine;
  FILE         *filtered_input;
  char          linebuffer[LSTR_LENGTH], *lineptr;

  int i, *intptr;

  // Init
  lineptr = linebuffer;

  // Read items out of Using List into an array of strings
  UsingLen=0; listiter = ListIterateInit(UsingList);
  while (listiter != NULL)
   {
    if (UsingLen>=USING_ITEMS_MAX) { if (*ErrCounter>0) { (*ErrCounter)--; strcpy(errout,"Too many items in using list."); } *status=1; return; }
    tempdict = (Dict *)listiter->data;
    DictLookup(tempdict,"using_item",NULL,(void **)&(UsingItems[UsingLen++]));
    listiter = ListIterate(listiter, NULL);
   }

  // If using list was empty, generate an automatic list
  if (UsingLen==0)
   {
    for (i=0; i<Ncolumns; i++)
     {
      UsingItems[i] = (char *)lt_malloc(10);
      sprintf(UsingItems[i], "$%d", i+1);
     }
    UsingLen = Ncolumns;
    AutoUsingList = 1; // We have automatically generated this using list
   }
  else if (UsingLen==1) // Prepend data point number if only one number specified in using statement
   {
    UsingItems[1] = UsingItems[0];
    UsingItems[0] = "$0";
    UsingLen++;
   }

  // If our using list has the wrong number of elements for the plot style we're aiming to use, then reject it
  if (UsingLen != Ncolumns) { if (*ErrCounter>0) { (*ErrCounter)--; sprintf(errout,"The supplied using ... clause contains the wrong number of items. We need %d columns of data, but only %d have been supplied.", Ncolumns, UsingLen); } *status=1; return; }

  // Read items out of Every List
  listiter = ListIterateInit(EveryList);
  if   (listiter == NULL) { linestep   =  0; } // READ linestep
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if (intptr==NULL) { linestep  =  0; } else { linestep   = *intptr; } }
  if   (listiter == NULL) { blockstep  =  0; } // READ blockstep
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if (intptr==NULL) { blockstep =  0; } else { blockstep  = *intptr; } }
  if   (listiter == NULL) { linefirst  = -1; } // READ linefirst
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if (intptr==NULL) { linefirst = -1; } else { linefirst  = *intptr; } }
  if   (listiter == NULL) { blockfirst = -1; } // READ blockfirst
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if (intptr==NULL) { blockfirst= -1; } else { blockfirst = *intptr; } }
  if   (listiter == NULL) { linelast   = -1; } // READ linelast
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if (intptr==NULL) { linelast  = -1; } else { linelast   = *intptr; } }
  if   (listiter == NULL) { blocklast  = -1; } // READ blocklast
  else                    { tempdict = (Dict *)listiter->data; listiter = ListIterate(listiter, NULL); DictLookup(tempdict,"every_item",NULL,(void **)&intptr);
                            if (intptr==NULL) { blocklast = -1; } else { blocklast  = *intptr; } }
  if   (listiter != NULL)

  // Reset the counters which we use to monitor our position in the file
  index_number       = 0;
  linenumber_count   = 0;
  linenumber_stepcnt = 0;
  block_count        = 0;
  block_stepcnt      = 0;
  prev_blanklines    = 10; // The number of blank lines we've hit since the last datapoint
  file_linenumber    = 0;

  // Open the requested datafile
  if      (strcmp(filename,"-" )) { filtered_input = stdin;  } // Special filename '-'  means we read from stdin
  else if (strcmp(filename,"--")) { ReadFromCommandLine = 1; } // Special filename '--' means we read from the commandline
  else
   {
    filtered_input = DataFile_LaunchCoProcess(filename, status, errout, ErrCounter);
    if (*status>0) return; // There was a problem launching coprocess or opening file
   }

  // Read input file, line by line
  while ( (!ferror(filtered_input)) && (!feof(filtered_input)) )
   {
    if (!ReadFromCommandLine) file_readline(filtered_input, linebuffer);
    else                      lineptr = FetchInputStatement("data ..> ",".......> ");

    if (ReadFromCommandLine && (strcmp(StrStrip(lineptr, linebuffer),"END")==0)) break;

    file_linenumber++;
    StrStrip(lineptr, linebuffer);

    if (linebuffer[0]=='\0') // We have a blank line
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
        if ((index>=0) && (index_number>index)) break; // We'll never find any more data once we've gone past the specified index
       }
     }

   // Ignore comment lines
   if (linebuffer[0]=='#') continue;
   prev_blanklines=0;

   // If we're in an index we're ignoring, don't do anything with this line
   if ((index>=0) && (index_number != index)) continue;

   // If we're in a block that we're ignoring, don't do anything with this line
   if ((block_stepcnt!=0) || ((blockfirst>=0)&&(block_count<blockfirst)) || ((blocklast>=0)&&(block_count<blocklast))) continue;

   // Fetch this line if linenumber is within range, or if we are using rows and we need everything
   if ((UsingRowCol == DATAFILE_ROW) || ((linenumber_stepcnt==0) && ((linefirst<0)||(linenumber_count>=linefirst)) && ((linelast<0)||(linenumber_count<=linelast))))
    {
     // Count the number of data items on this line
     ItemsOnLine = 0; hadwhitespace = 1; hadcomma = 1;
     for (i=0; linebuffer[i]!='\0'; i++)
      {
       if      (linebuffer[i]< ' ') {  hadwhitespace = 1; }
       else if (linebuffer[i]==',') { ItemsOnLine++; hadwhitespace = hadcomma = 1; }
       else                         { if (hadwhitespace && !hadcomma) ItemsOnLine++; hadwhitespace = hadcomma = 0; }
      }

     // Convert these to numbers


     linenumber_count++;
     linenumber_stepcnt = ((linenumber_stepcnt-1) % blockstep);
    }
   }

  // If we are reading rows, go through all of the data that we've read and rotate it by 90 degrees
  if (UsingRowCol == DATAFILE_ROW)
   {
   }

  // Now process the data into its final form by applying 'using' and 'select' expressions to it

  // If requested, now sort the datagrid
  if ((continuity == DATAFILE_SORTED) || (continuity == DATAFILE_SORTEDLOGLOG))
   {
   }
 }

