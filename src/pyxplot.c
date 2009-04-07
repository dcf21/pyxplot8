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

#define _PYXPLOT_C 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "pyxplot.h"
#include "asciidouble.h"
#include "ppl_children.h"
#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_list.h"
#include "ppl_memory.h"
#include "ppl_papersize.h"
#include "ppl_parser.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_userspace.h"

int WillBeInteractive;

// SIGINT Handling information

sigjmp_buf sigjmp_ToMain;
sigjmp_buf sigjmp_ToInteractive;
sigjmp_buf sigjmp_ToDirective;
sigjmp_buf *sigjmp_FromSigInt = NULL;

int main(int argc, char **argv)
 {
  int i,fail;
  int tempdirnumber = 1;
  char tempdirpath[FNAME_LENGTH];
  char *EnvDisplay;
  struct stat statinfo;

  struct timespec waitperiod, waitedperiod; // A time.h timespec specifier for a 100ms nanosleep wait
  waitperiod.tv_sec  = 0;
  waitperiod.tv_nsec = 50000000;
  WillBeInteractive  = 1;

  // Initialise sub-modules
  if (DEBUG) ppl_log("Initialising PyXPlot.");
  ppl_MemoryInit();
  ppl_PaperSizeInit();
  ppl_UserSpaceInit();
  ppl_text_init();
  if (DEBUG) ppl_log("Initialising settings.");
  ppl_settings_term_init();

  // Scan commandline options for any switches
  for (i=1; i<argc; i++)
   {
    if (strlen(argv[i])==0) continue;
    if (argv[i][0]!='-')
     {
      if (WillBeInteractive==1) WillBeInteractive=0;
      continue;
     }
    if      (strcmp(argv[i], "-q"         )==0) settings_session_default.splash = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-quiet"     )==0) settings_session_default.splash = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-V"         )==0) settings_session_default.splash = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-verbose"   )==0) settings_session_default.splash = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-c"         )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-colour"    )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-color"     )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-m"         )==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-mono"      )==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-monochrome")==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-"          )==0) WillBeInteractive=2;
    else if ((strcmp(argv[i], "-v")==0) || (strcmp(argv[i], "-version")==0))
     {
      ppl_report(txt_version);
      ppl_FreeAll(0);
      if (DEBUG) ppl_log("Reported version number as requested.");
      return 0;
     }
    else if ((strcmp(argv[i], "-h")==0) || (strcmp(argv[i], "-help")==0))
     {
      ppl_report(txt_help);
      ppl_FreeAll(0);
      if (DEBUG) ppl_log("Reported help text as requested.");
      return 0;
     }
    else
    {
     sprintf(temp_err_string, "Received switch '%s' which was not recognised. Type 'pyxplot -help' for a list of available commandline options.", argv[i]);
     ppl_error(temp_err_string);
     ppl_FreeAll(0);
     if (DEBUG) ppl_log("Received unexpected commandline switch.");
     return 1;
    }
   }

  // Decide upon a path for a temporary directory for us to live in
  if (DEBUG) ppl_log("Finding a filepath for a temporary directory.");
  if (getcwd( settings_session_default.cwd , FNAME_LENGTH ) < 0) { ppl_fatal(__FILE__,__LINE__,"Could not read current working directory."); } // Store cwd
  while (1) { sprintf(tempdirpath, "/tmp/pyxplot_%d_%d", getpid(), tempdirnumber); if (access(tempdirpath, F_OK) != 0) break; tempdirnumber++; } // Find an unused dir path
  strcpy(settings_session_default.tempdir, tempdirpath); // Store our chosen temporary directory path

  // Launch child process
  if (DEBUG) ppl_log("Launching the Child Support Process.");
  InitialiseCSP();

  // Set up commandline parser
  if (DEBUG) ppl_log("Setting up commandline parser from RE++ definitions.");
  ppl_commands_read();

  // Set up SIGINT handler
  if (sigsetjmp(sigjmp_ToMain, 1) == 0)
   {
    sigjmp_FromSigInt = &sigjmp_ToMain;
    signal(SIGINT, SigIntHandler);

    // Wait for temporary directory to appear, and change directory into it
    if (DEBUG) ppl_log("Waiting for temporary directory to appear.");
    for (i=0; i<100; i++) { if (access(tempdirpath, F_OK) == 0) break; nanosleep(&waitperiod,&waitedperiod); } // Wait for temp dir to be created by child process
    if (access(tempdirpath, F_OK) != 0) { fail=1; } // If it never turns up, fail.
    else
     {
      stat(tempdirpath, &statinfo); // Otherwise stat it and make sure it's a directory we own
      if (!S_ISDIR(statinfo.st_mode)) fail=1;
      if (statinfo.st_uid != getuid()) fail=1;
     }
    if (fail==1)                { ppl_fatal(__FILE__,__LINE__,"Failed to create temporary directory." ); }
    if (chdir(tempdirpath) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into temporary directory failed."); } // chdir into temporary directory

    // Read GNU Readline history
    if (DEBUG) ppl_log("Reading GNU Readline history.");
    sprintf(tempdirpath, "%s%s%s", settings_session_default.homedir, PATHLINK, ".pyxplot_history");
    read_history(tempdirpath);
    stifle_history(1000);

    // Set default terminal
    EnvDisplay = getenv("DISPLAY"); // Check whether the environment variable DISPLAY is set
    if ((ppl_termtype_set_in_configfile == 0) && ((WillBeInteractive==0) ||
                                                  (EnvDisplay==NULL) ||
                                                  (EnvDisplay[0]=='\0') ||
                                                  (strcmp(GHOSTVIEW, "/bin/false")==0) ||
                                                  (isatty(STDIN_FILENO) != 1)))
     {
      ppl_log("Detected that we are running a non-interactive session; defaulting to the EPS terminal.");
      settings_term_default.TermType = SW_TERMTYPE_EPS;
     }

    // Scan commandline and process all script files we have been given
    for (i=1; i<argc; i++)
     {
      if (strlen(argv[i])==0) continue;
      if (argv[i][0]=='-')
       {
        if (argv[i][1]=='\0') InteractiveSession();
        continue;
       }
      ProcessPyXPlotScript(argv[i]);
     }
    if (WillBeInteractive==1) InteractiveSession();

   // SIGINT longjmps to main return here
   } else {
    ppl_error("\nReceived SIGINT. Terminating.");
   }

  // Notify the CSP that we are about to quit
  SendCommandToCSP("B\n");

  // Save GNU Readline history
  if (WillBeInteractive>0)
   {
    if (DEBUG) ppl_log("Saving GNU Readline history.");
    sprintf(tempdirpath, "%s%s%s", settings_session_default.homedir, PATHLINK, ".pyxplot_history");
    write_history(tempdirpath);
   }

  // Terminate
  ppl_FreeAll(0);
  ppl_MemoryStop();
  if (DEBUG) ppl_log("Terminating normally.");
  return 0;
 }

void SigIntHandler(int signo)
 {
  sigjmp_buf *destination = sigjmp_FromSigInt;
  sigjmp_FromSigInt = NULL; // DO NOT recursively go back to the same sigint handler over and over again if it doesn't seem to work.
  if (destination != NULL) siglongjmp(*destination, 1);
  raise(SIGTERM);
  raise(SIGKILL);
 }
