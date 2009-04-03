// ppl_shell.c
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

#define _PPL_SHELL 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "pyxplot.h"
#include "asciidouble.h"
#include "ppl_children.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_constants.h"

int PPL_SHELL_EXITTING;
int PPL_SHELL_MULTILINE;

void InteractiveSession()
 {
  int linenumber = 1;
  char *line_ptr;
  char  linebuffer[LSTR_LENGTH];

  PPL_SHELL_EXITTING = PPL_SHELL_MULTILINE = 0;
  ppl_log("Starting an interactive session.");
  if ((isatty(STDIN_FILENO) == 1) && (settings_session_default.splash == SW_ONOFF_ON)) ppl_report(txt_init);

  while (PPL_SHELL_EXITTING == 0)
   {
    printf("...\n");
    CheckForGvOutput();
    printf("@@@\n");
    if (isatty(STDIN_FILENO) == 1)
     {
      if (PPL_SHELL_MULTILINE==0) line_ptr = readline("pyxplot> ");
      else                        line_ptr = readline(".......> ");
      strcpy(linebuffer, line_ptr) ; free(line_ptr);
      ProcessDirective(linebuffer);
     } else {
      file_readline(stdin, linebuffer);
      ProcessDirective(linebuffer);
      linenumber++;
     }
   }
  return;
 }

void ProcessPyXPlotScript(char *input)
 {
  int linenumber = 1;
  int status;
  int ProcessedALine = 0;
  char full_filename[FNAME_LENGTH];
  char linebuffer[LSTR_LENGTH];
  FILE *infile;

  PPL_SHELL_EXITTING = 0;
  sprintf(temp_err_string, "Processing input from the script file '%s'.", input); ppl_log(temp_err_string);
  sprintf(full_filename, "%s%s%s", settings_session_default.cwd, PATHLINK, input);
  if ((infile=fopen(full_filename,"r")) == NULL)
   {
    sprintf(temp_err_string, "PyXPlot Error: Could not find command file '%s'\nSkipping on to next command file", full_filename); ppl_error(temp_err_string);
    return;
   }

  while ((PPL_SHELL_EXITTING == 0) && (!feof(infile)))
   {
    file_readline(infile, linebuffer);
    if (strlen( StrStrip(linebuffer) ) > 0)
     {
      status = ProcessDirective(linebuffer);
      if ((ProcessedALine==0) && (status!=0)) // If an error occurs on the first line of a script, aborted processing it
       {
        ppl_error("Error on first line of commandfile: Is this is valid script?\nAborting");
        break;
       }
      ProcessedALine = 1;
     }
    linenumber++;
   }

  fclose(infile);
  CheckForGvOutput();
  return;
 }

int ProcessDirective(char *in)
 {
  return 0;
 }

