// pyxplot.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
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

#include <gsl/gsl_errno.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_list.h"
#include "ListTools/lt_memory.h"

#include "pyxplot.h"
#include "ppl_canvasitems.h"
#include "ppl_children.h"
#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_input.h"
#include "ppl_papersize.h"
#include "ppl_parser.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_userspace_init.h"

int WillBeInteractive;

// SIGINT Handling information

sigjmp_buf sigjmp_ToMain;
sigjmp_buf sigjmp_ToInteractive;
sigjmp_buf sigjmp_ToDirective;
sigjmp_buf *sigjmp_FromSigInt = NULL;

int main(int argc, char **argv)
 {
  int i,fail=0;
  int tempdirnumber = 1;
  char tempdirpath[FNAME_LENGTH];
  char *EnvDisplay;
  struct stat statinfo;

  sigset_t sigs;

  struct timespec waitperiod, waitedperiod; // A time.h timespec specifier for a 100ms nanosleep wait
  waitperiod.tv_sec  = 0;
  waitperiod.tv_nsec = 50000000;
  WillBeInteractive  = 1;

  // Initialise sub-modules
  if (DEBUG) ppl_log("Initialising PyXPlot.");
  lt_MemoryInit(&ppl_error, &ppl_log);
  ClearInputSource(NULL,NULL,NULL,NULL,NULL,NULL);
  ppl_units_init();
  ppl_PaperSizeInit();
  ppl_settings_makedefault();
  ppl_text_init();

  // Turn off GSL's automatic error handler
  gsl_set_error_handler_off();

  // Initialise GNU Readline
  rl_readline_name = "PyXPlot";                          /* Allow conditional parsing of the ~/.inputrc file. */
  rl_attempted_completion_function = ppl_rl_completion;  /* Tell the completer that we want a crack first. */

  // Initialise user variables and functions
  ppl_UserSpaceInit();

  // Set up commandline parser; do this BEFORE reading config files, which may contain a [script] section which needs parsing
  if (DEBUG) ppl_log("Setting up commandline parser from RE++ definitions.");
  ppl_commands_read();

  // Initialise settings and read configuration file; do this BEFORE processing command line arguments which take precedence
  if (DEBUG) ppl_log("Reading configuration file.");
  ppl_settings_readconfig();

  // Scan commandline options for any switches
  for (i=1; i<argc; i++)
   {
    if (strlen(argv[i])==0) continue;
    if (argv[i][0]!='-')
     {
      if (WillBeInteractive==1) WillBeInteractive=0;
      continue;
     }
    if      (strcmp(argv[i], "-q"          )==0) settings_session_default.splash = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-quiet"      )==0) settings_session_default.splash = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "--quiet"     )==0) settings_session_default.splash = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-V"          )==0) settings_session_default.splash = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-verbose"    )==0) settings_session_default.splash = SW_ONOFF_ON;
    else if (strcmp(argv[i], "--verbose"   )==0) settings_session_default.splash = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-c"          )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-colour"     )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "--colour"    )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-color"      )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "--color"     )==0) settings_session_default.colour = SW_ONOFF_ON;
    else if (strcmp(argv[i], "-m"          )==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-mono"       )==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "--mono"      )==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-monochrome" )==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "--monochrome")==0) settings_session_default.colour = SW_ONOFF_OFF;
    else if (strcmp(argv[i], "-"           )==0) WillBeInteractive=2;
    else if ((strcmp(argv[i], "-v")==0) || (strcmp(argv[i], "-version")==0) || (strcmp(argv[i], "--version")==0))
     {
      printf("%s\n",txt_version);
      if (DEBUG) ppl_log("Reported version number as requested.");
      lt_FreeAll(0); lt_MemoryStop();
      return 0;
     }
    else if ((strcmp(argv[i], "-h")==0) || (strcmp(argv[i], "-help")==0) || (strcmp(argv[i], "--help")==0))
     {
      printf("%s\n",txt_help);
      if (DEBUG) ppl_log("Reported help text as requested.");
      lt_FreeAll(0); lt_MemoryStop();
      return 0;
     }
    else
    {
     sprintf(temp_err_string, "Received switch '%s' which was not recognised. Type 'pyxplot -help' for a list of available commandline options.", argv[i]);
     ppl_error(ERR_PREFORMED, temp_err_string);
     if (DEBUG) ppl_log("Received unexpected commandline switch.");
     lt_FreeAll(0); lt_MemoryStop();
     return 1;
    }
   }

  // Decide upon a path for a temporary directory for us to live in
  if (DEBUG) ppl_log("Finding a filepath for a temporary directory.");
  if (getcwd( settings_session_default.cwd , FNAME_LENGTH ) == NULL) { ppl_fatal(__FILE__,__LINE__,"Could not read current working directory."); } // Store cwd
  while (1) { sprintf(tempdirpath, "/tmp/pyxplot_%d_%d", getpid(), tempdirnumber); if (access(tempdirpath, F_OK) != 0) break; tempdirnumber++; } // Find an unused dir path
  strcpy(settings_session_default.tempdir, tempdirpath); // Store our chosen temporary directory path

  // Launch child process
  if (DEBUG) ppl_log("Launching the Child Support Process.");
  InitialiseCSP();

  // Set up SIGINT handler
  if (sigsetjmp(sigjmp_ToMain, 1) == 0)
   {
    sigjmp_FromSigInt = &sigjmp_ToMain;
    if (signal(SIGINT, SIG_IGN)!=SIG_IGN) signal(SIGINT, SigIntHandler);
    signal(SIGPIPE, SIG_IGN);

    // Wait for temporary directory to appear, and change directory into it
    if (DEBUG) ppl_log("Waiting for temporary directory to appear.");
    strcpy(tempdirpath, settings_session_default.tempdir);
    for (i=0; i<100; i++) { if (access(tempdirpath, F_OK) == 0) break; nanosleep(&waitperiod,&waitedperiod); } // Wait for temp dir to be created by child process
    if (access(tempdirpath, F_OK) != 0) { fail=1; } // If it never turns up, fail.
    else
     {
      stat(tempdirpath, &statinfo); // Otherwise stat it and make sure it's a directory we own
      if (!S_ISDIR(statinfo.st_mode)) fail=1;
      if (statinfo.st_uid != getuid()) fail=1;
     }
    if (fail==1)                { ppl_fatal(__FILE__,__LINE__,"Failed to create temporary directory." ); }

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
                                                  (strcmp(GHOSTVIEW_COMMAND, "/bin/false")==0) ||
                                                  (isatty(STDIN_FILENO) != 1)))
     {
      if (DEBUG) ppl_log("Detected that we are running a non-interactive session; defaulting to the EPS terminal.");
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
      ProcessPyXPlotScript(argv[i], 0);
     }
    if (WillBeInteractive==1) InteractiveSession();

   // SIGINT longjmps to main return here
   } else {
    sigemptyset(&sigs);
    sigaddset(&sigs,SIGCHLD);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
    ppl_error(ERR_PREFORMED, "\nReceived SIGINT. Terminating.");
   }

  // Notify the CSP that we are about to quit
  SendCommandToCSP("B\n");
  directive_clear();

  // Free all of the axes
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(XAxes[i])       );
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(YAxes[i])       );
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(ZAxes[i])       );
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(XAxesDefault[i]));
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(YAxesDefault[i]));
  for (i=0; i<MAX_AXES; i++) DestroyAxis(&(ZAxesDefault[i]));

  // Free all of the plot styles which are set
  for (i=0; i<MAX_PLOTSTYLES; i++) with_words_destroy(&(settings_plot_styles        [i]));
  for (i=0; i<MAX_PLOTSTYLES; i++) with_words_destroy(&(settings_plot_styles_default[i]));

  // Free arrows and text labels which are set
  arrow_list_destroy(&arrow_list); arrow_list_destroy(&arrow_list_default);
  label_list_destroy(&label_list); label_list_destroy(&label_list_default);

  // Save GNU Readline history
  if (WillBeInteractive>0)
   {
    if (DEBUG) ppl_log("Saving GNU Readline history.");
    sprintf(tempdirpath, "%s%s%s", settings_session_default.homedir, PATHLINK, ".pyxplot_history");
    write_history(tempdirpath);
   }

  // Terminate
  lt_FreeAll(0);
  lt_MemoryStop();
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
