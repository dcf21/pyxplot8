// ppl_glob.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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

#ifndef _PPL_GLOB_H
#define _PPL_GLOB_H 1

#include <glob.h>
#include <wordexp.h>

typedef struct ppl_glob {
  glob_t         GlobData;
  wordexp_t      WordExp;
  int            ig,iw;
  char          *SpecialFilename;
  unsigned char  GlobDataSet, WordExpSet, SpecialCase;
 } ppl_glob;

char     *ppl_glob_oneresult (char *filename);
ppl_glob *ppl_glob_allresults(char *filename);
char     *ppl_glob_iterate   (ppl_glob *x);
void      ppl_glob_close     (ppl_glob *x);

#endif

