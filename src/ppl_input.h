// ppl_input.h
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

#ifndef _PPL_INPUT_H
#define _PPL_INPUT_H 1

#include "ListTools/lt_dict.h"

void SetInputSourcePipe(int *linenumber_, char *filename_description_);
void SetInputSourceReadline(int *linenumber_, char *filename_description_);
void SetInputSourceFile(FILE *infile_, int *linenumber_, char *filename_description_);
void SetInputSourceLoop(cmd_chain *inchain_);
void SetInputSourceString(char *instr, int *inpos_);
void GetInputSource(int **lineno, char **descr);
char *FetchInputLine(char *output, char *prompt);

#endif

