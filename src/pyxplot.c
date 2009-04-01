// pyxplot.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-8 Dominic Ford <coders@pyxplot.org.uk>
//               2008   Ross Church
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
#include <readline/readline.h>

#include "pyxplot.h"
#include "ppl_settings.h"

int main(int argc, char **argv)
 {
  int i,fail;
  int tempdirnumber = 1;
  char tempdirpath[FNAME_LENGTH];
  struct stat statinfo;

  ppl_text_init();
  ppl_settings_term_init();

  // Decide upon a path for a temporary directory for us to live in
  if (getcwd( settings_session_default.cwd , FNAME_LENGTH ) < 0) { ppl_fatal("Fatal Error: Could not read current working directory."); } // Store cwd
  while (1) { sprintf(tempdirpath, "/tmp/pyxplot_%d_%d", getpid(), tempdirnumber); if (access(tempdirpath, F_OK) != 0) break; tempdirnumber++; } // Find an unused dir path
  strcpy(settings_session_default.tempdir, tempdirpath); // Store our chosen temporary directory path

  // Launch child process


  // Wait for temporary directory to appear, and change directory into it
  for (i=0; i<5; i++) { if (access(tempdirpath, F_OK) == 0) break; sleep(0); } // Wait for temp dir to be created by child process
  if (access(tempdirpath, F_OK) != 0) { fail=1; } // If it never turns up, fail.
  else
   {
    stat(tempdirpath, &statinfo); // Otherwise stat it and make sure it's a directory we own
    if (!S_ISDIR(statinfo.st_mode)) fail=1;
    if (statinfo.st_uid != getuid()) fail=1;
   }
  if (fail==1)                { ppl_fatal("Fatal Error: Security error whilst trying to create temporary directory."); }
  if (chdir(tempdirpath) < 0) { ppl_fatal("Fatal Error: chdir into temporary directory failed."                     ); } // chdir into temporary directory

  ppl_report(txt_init); // Print welcome text
  return 0;
 }

