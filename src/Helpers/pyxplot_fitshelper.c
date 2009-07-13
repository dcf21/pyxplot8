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
  int   i, j, HaveFilename=0;
#ifdef HAVE_FITSIO
  fitsfile *fptr=NULL;
  int       status=0, hdupos, hdutype, nkeys, bitpix, naxis, ncols, anynull=0, dispwidth[16384];
  long      naxes[256], nrows;
  char      card[FLEN_CARD], keyword[FLEN_KEYWORD], colname[FLEN_VALUE], nullstr[]="nan", tabval[16384]="\0", *valptr;
#endif

#ifndef HAVE_FITSIO
strcpy(warntext,"*** WARNING ***\nAs the cfitsio library was not installed on this computer when PyXPlot was\n\
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
     fprintf(stderr, "\nReceived switch '%s' which was not recognised.\nType 'pyxplot_fitshelper -help' for a list of available commandline options.\n\n", argv[i]);
     return 1;
    }
   }

  // Check that we have been provided with exactly one filename on the command line
  if (HaveFilename < 1)
   {
    fprintf(stderr, "\npyxplot_fitshelper should be provided with a filename on the command line to\nact upon. Type 'pyxplot_fitshelper -help' for a list of available commandline\noptions.\n\n");
    return 1;
   }
  else if (HaveFilename > 1)
   {
    fprintf(stderr, "\npyxplot_fitshelper should be provided with only one filename on the command\nline to act upon. Multiple filenames appear to have been supplied. Type\n'pyxplot_fitshelper -help' for a list of available commandline options.\n\n");
    return 1;
   }

#ifndef HAVE_FITSIO
  fprintf(stderr, "\nThe cfitsio library was not installed on this computer when PyXPlot was\n\
installed, and consequently PyXPlot was compiled with no support for fits\n\
files. The fits helper application which you are using is a dummy which does\n\
nothing. To plot this datafile, you need to install libcfitsio and then\n\
recompile PyXPlot.\n\n");
  exit(1);
#else
  if (fits_open_file(&fptr, filename, READONLY, &status) != 0) { fits_report_error(stderr, status); exit(1); }
  fits_get_hdu_num(fptr, &hdupos); // Get the current HDU number

  fprintf(stdout, "# Listing the contents of file <%s>\n", filename);
  fprintf(stdout, "# This ASCII representation generated by %s.\n", StrStrip(version_string,warntext));
  fprintf(stdout, "# Conversion performed at %s.\n", StrStrip(FriendlyTimestring(),warntext));

  for (; !status; hdupos++) // Loop through extensions in this fits file
   {
    fits_get_hdrspace(fptr, &nkeys, NULL, &status); // Get number of keywords in this extension
    fprintf(stdout, "\n\n\n\n# HDU %d\n#\n", hdupos);

    for (i=0; i<=nkeys; i++) // List keys from this header
     {
      if (fits_read_record(fptr, i, card, &status)) break;
      fprintf(stdout, "# %s\n", card);
     }

    fits_get_hdu_type(fptr, &hdutype, &status); // Get HDU type

    if (hdutype == IMAGE_HDU)
     {
      fits_get_img_param(fptr, 256, &bitpix, &naxis, naxes, &status);
      fprintf(stdout, "\n# Image array, with %d bits per pixel, and %d axes", abs(bitpix), naxis);
      if (naxis>0) fprintf(stdout, ". Size: ");
      for (i=0; i<naxis; i++)
       {
        if (i!=0) fprintf(stdout, " x ");
        fprintf(stdout, "%ld", naxes[i]);
       }
      fprintf(stdout, ".\n\nnull\n");
     } else {
      fits_get_num_rows(fptr, &nrows, &status);
      fits_get_num_cols(fptr, &ncols, &status);

      if (hdutype == ASCII_TBL) fprintf(stdout, "\n# ASCII table. Size: %d x %ld.\n\n", ncols, nrows);
      else                      fprintf(stdout, "\n# Binary table. Size: %d x %ld.\n\n", ncols, nrows);

      // Print a list of column names
      fprintf(stdout, "# Columns: ");
      for (i=1; i<=ncols; i++)
       {
        fits_get_col_display_width(fptr, i, &dispwidth[i], &status);
        fits_make_keyn("TTYPE", i, keyword, &status);
        fits_read_key(fptr, TSTRING, keyword, colname, NULL, &status);
        colname[dispwidth[i]] = '\0'; // Truncate long names
        printf("%*s ", dispwidth[i], colname);
       }

      // Print a list of column units
      fprintf(stdout, "\n# Units:   ");
      for (i=1; i<=ncols; i++)
       {
        fits_get_col_display_width(fptr, i, &dispwidth[i], &status);
        fits_make_keyn("TUNIT", i, keyword, &status);
        fits_read_key(fptr, TSTRING, keyword, colname, NULL, &status);
        colname[dispwidth[i]] = '\0'; // Truncate long names
        if (status == KEY_NO_EXIST) { status=0; strcpy(colname, "1"); } // fits file may not specify unit
        printf("%*s ", dispwidth[i], colname);
       }
      fprintf(stdout, "\n\n");

      // Print data from table
      valptr = tabval;
      for (j=1; (j<=nrows) && (!status); j++)
       {
        fprintf(stdout, "           "); // Indent to meet column headings
        for (i=1; (i<=ncols) && (!status); i++)
         {
          fits_read_col_str(fptr, i, j, 1, 1, nullstr, &valptr, &anynull, &status);
          if (!status) fprintf(stdout, "%-*s ", dispwidth[i], tabval);
         }
        fprintf(stdout, "\n");
       }
     }
    fits_movrel_hdu(fptr, 1, NULL, &status); // Move to the next HDU
   }

  if (status == END_OF_FILE) status=0; // Reset status after reading an EOF
  fits_close_file(fptr, &status);
  if (status) fits_report_error(stderr, status); // Print any error message which has caused premature halt
  return status;
#endif

  return 0; // Should never get here
 }
