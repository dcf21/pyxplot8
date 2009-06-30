// ppl_datafile.h
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

#ifndef _PPL_DATAFILE_H
#define _PPL_DATAFILE_H 1

#define DATAFILE_CONTINUOUS     24001
#define DATAFILE_DISCONTINUOUS  24002
#define DATAFILE_SORTED         24003
#define DATAFILE_SORTEDLOGLOG   24004

#define DATAFILE_ROW 24101
#define DATAFILE_COL 24102

#define USING_ITEMS_MAX 32

#include "ppl_units.h"

typedef struct DataBlock {
  double          **data;     // Array of Ncolumns x array of length BlockLength
  long int        **FileLine; // For each double above... store the line number in the data file that it came from
  long int        **FileCol;  // For each double above... store the column number that it came from
  unsigned char    *split;    // Array of length BlockLength; TRUE if we should break data before this next datapoint
  int               BlockLength;
  struct DataBlock *next;
 } DataBlock;

typedef struct DataTable {
  int    Ncolumns;
  int    Nrows;
  value *FirstEntries; // Array of size Ncolumns
  struct DataBlock *next;
 } DataTable;

void DataFile_read(DataTable *output, int *status, char *errout, char *filename, int index, int UsingRowCol, List *UsingList, List *EveryList, int Ncolumns, char *SelectCriterion, int continuity, int *ErrCounter);

#endif

