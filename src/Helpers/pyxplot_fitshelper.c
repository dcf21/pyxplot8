// pyxplot_fitshelper.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#ifdef HAVE_FITSIO
#include <fitsio.h>
#endif

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

int main(int argc, char **argv)
 {
  char  help_string[LSTR_LENGTH], version_string[FNAME_LENGTH], version_string_underline[FNAME_LENGTH], warntext[FNAME_LENGTH];
  char *filename;
  int   i, HaveFilename=0;

#ifndef HAVE_FITSIO
strcpy(warntext,"\n*** WARNING ***\nAs the cfitsio library was not installed on this computer when PyXPlot was\n\
installed, PyXPlot has been compiled with no support for fits files. This\n\
application is a dummy filter which does nothing.\n\n");
#else
strcpy(warntext,"");
#endif

  // Make help and version strings
  sprintf(version_string, "\nPyXPlot Fits Helper %s\n", VERSION);

  sprintf(help_string   , "%s\
%s\n\
\n\
%sUsage: pyxplot_fitshelper <filename>\n\
  -h, --help:       Display this help.\n\
  -v, --version:    Display version number.\n\
\n\
This application is used as in input filter to convert tables of data in FITS\n\
format into ASCII for reading into PyXPlot. It is the filter which PyXPlot uses\n\
when asked to plot a .fits file, but is also generally useful whenever it is\n\
desired to convert data from FITS format into plain text.\n\
\n\
A brief introduction to PyXPlot can be obtained by typing 'man pyxplot'; the\n\
full Users' Guide can be found in the file:\n\
%s%spyxplot.pdf\n\
\n\
For the latest information on PyXPlot development, see the project website:\n\
<http://www.pyxplot.org.uk>\n", version_string, StrUnderline(version_string, version_string_underline), warntext, DOCDIR, PATHLINK);

  // Scan commandline options for any switches
  HaveFilename=0;
  for (i=1; i<argc; i++)
   {
    if (strlen(argv[i])==0) continue;
    if (argv[i][0]!='-')
     {
      HaveFilename++; filename = argv[i];
      continue;
     }
    if      ((strcmp(argv[i], "-v")==0) || (strcmp(argv[i], "-version")==0) || (strcmp(argv[i], "--version")==0))
     {
      printf("%s",version_string);
      return 0;
     }
    else if ((strcmp(argv[i], "-h")==0) || (strcmp(argv[i], "-help")==0) || (strcmp(argv[i], "--help")==0))
     {
      printf("%s",help_string);
      return 0;
     }
    else
    {
     fprintf(stderr, "Received switch '%s' which was not recognised. Type 'pyxplot_fitshelper -help' for a list of available commandline options.", argv[i]);
     return 1;
    }
   }

  // Check that we have been provided with exactly one filename on the command line
  if (HaveFilename < 1)
   {
    fprintf(stderr, "pyxplot_fitshelper should be provided with a filename on the command line to act upon. Type 'pyxplot_fitshelper -help' for a list of available commandline options.");
    return 1;
   }
  else if (HaveFilename > 1)
   {
    fprintf(stderr, "pyxplot_fitshelper should be provided with only one filename on the command line to act upon. Multiple filenames appear to have been supplied. Type 'pyxplot_fitshelper -help' for a list of available commandline options.");
    return 1;
   }

#ifndef HAVE_FITSIO
  fprintf(stderr, "The cfitsio library was not installed on this computer when PyXPlot was\n\
installed, and consequently PyXPlot was compiled with no support for fits\n\
files. The fits helper application which you are using is a dummy which does\n\
nothing. To plot this datafile, you need to install libcfitsio and then\n\
recompile PyXPlot.");
  exit(1);
#else
#endif

  return 0;
 }
