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
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "pyxplot.h"
#include "asciidouble.h"
#include "ppl_children.h"
#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_parser.h"
#include "ppl_passwd.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

int PPL_SHELL_EXITING;
int PPL_SHELL_MULTILINE;

void InteractiveSession()
 {
  int linenumber = 1;
  char *line_ptr;
  char  linebuffer[LSTR_LENGTH];

  PPL_SHELL_EXITING = PPL_SHELL_MULTILINE = 0;
  ppl_log("Starting an interactive session.");

  // Set up SIGINT handler
  if (sigsetjmp(sigjmp_ToInteractive, 1) == 0)
   {
    sigjmp_FromSigInt = &sigjmp_ToInteractive;

    if ((isatty(STDIN_FILENO) == 1) && (settings_session_default.splash == SW_ONOFF_ON)) ppl_report(txt_init);

    while (PPL_SHELL_EXITING == 0)
     {
      CheckForGvOutput();
      if (isatty(STDIN_FILENO) == 1)
       {
        if (PPL_SHELL_MULTILINE==0) line_ptr = readline("pyxplot> ");
        else                        line_ptr = readline(".......> ");
        if (line_ptr == NULL) { PPL_SHELL_EXITING = 1; continue; }
        strcpy(linebuffer, line_ptr) ; free(line_ptr);
        ProcessDirective(linebuffer, 1);
       } else {
        ppl_error_setstreaminfo(linenumber, "piped input");
        file_readline(stdin, linebuffer);
        ProcessDirective(linebuffer, 0);
        ppl_error_setstreaminfo(0, "");
        linenumber++;
        if (feof(stdin) || ferror(stdin)) PPL_SHELL_EXITING = 1;
       }
     }

    if ((isatty(STDIN_FILENO) == 1) && (settings_session_default.splash == SW_ONOFF_ON)) ppl_report("\nGoodbye. Have a nice day.");
   } else {
    ppl_error("\nReceived CTRL-C. Terminating session."); // SIGINT longjmps return here
   }
  sigjmp_FromSigInt = &sigjmp_ToMain; // SIGINT now drops back through to main().
  return;
 }

void ProcessPyXPlotScript(char *input)
 {
  int linenumber = 1;
  int status;
  int ProcessedALine = 0;
  char full_filename[FNAME_LENGTH];
  char filename_description[FNAME_LENGTH];
  char linebuffer[LSTR_LENGTH];
  FILE *infile;

  PPL_SHELL_EXITING = 0;
  sprintf(temp_err_string, "Processing input from the script file '%s'.", input); ppl_log(temp_err_string);
  UnixExpandUserHomeDir(input , settings_session_default.cwd, full_filename);
  sprintf(filename_description, "file '%s'", input);
  if ((infile=fopen(full_filename,"r")) == NULL)
   {
    sprintf(temp_err_string, "PyXPlot Error: Could not find command file '%s'\nSkipping on to next command file", full_filename); ppl_error(temp_err_string);
    return;
   }

  while ((PPL_SHELL_EXITING == 0) && (!feof(infile)) && (!ferror(infile)))
   {
    file_readline(infile, linebuffer);
    if (StrStrip(linebuffer,linebuffer)[0] != '\0')
     {
      ppl_error_setstreaminfo(linenumber, filename_description); 
      status = ProcessDirective(linebuffer, 0);
      ppl_error_setstreaminfo(0, "");
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

int ProcessDirective(char *in, int interactive)
 {
  int status;

  if ((interactive==0) || (sigsetjmp(sigjmp_ToDirective, 1) == 0))  // Set up SIGINT handler, but only if this is an interactive session
   {
    if (interactive!=0) sigjmp_FromSigInt = &sigjmp_ToDirective;
    status = ProcessDirective2(in);
   } else {
    ppl_error("\nReceived CTRL-C. Terminating command."); // SIGINT longjmps return here
    status = 1;
   }
  sigjmp_FromSigInt = &sigjmp_ToMain; // SIGINT now drops back through to main().
  return status;
 }

int ProcessDirective2(char *in)
 {
  if (DEBUG) { sprintf(temp_err_string, "Received command:\n%s", in); ppl_log(temp_err_string); }
  //SendCommandToCSP("2/home/dcf21/pyxplot/pyxplot/doc/figures/pyx_colours.eps\n");
  return 0;
 }
