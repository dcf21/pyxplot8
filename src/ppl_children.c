// ppl_children.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "asciidouble.h"
#include "pyxplot.h"
#include "ppl_children.h"
#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_list.h"
#include "ppl_memory.h"

// Pipes for communication between the main PyXPlot process and the CSP

int PipeCSP2MAIN[2];
int PipeMAIN2CSP[2];

char PipeOutputBuffer[LSTR_LENGTH] = "";

static char SIGTERM_NAME[16]; // The name of SIGTERM, which we filter out from GV's output, as it tends to whinge about being killed.

// Flags used to keep track of all of the open GhostView processes 

static List *GhostViews;                    // A list of X11_multiwindow and X11_singlewindow sessions which we kill on PyXPlot exit
static List *GhostView_Persists;            // A list of X11_persist sessions for which we leave our temporary directory until they quit
static int   GhostView_pid = 0;             // pid of any running gv process launched under X11_singlewindow
char         GhostView_Fname[FNAME_LENGTH]; // Filename of the eps file currently being viewed in X11_singlewindow
static int   PyXPlotRunning = 1;            // Flag which we drop in the CSP when the main process stops running

// Functions to be called from main PyXPlot process

void  InitialiseCSP()
 {
  int pid, fail;
  struct stat statinfo;

  // Create empty lists for storing lists of GhostView processes
  GhostViews         = ListInit();
  GhostView_Persists = ListInit();
  
  // The string "signal 15" we filter out of GhostView's output
  sprintf(SIGTERM_NAME, "signal %d", SIGTERM);

  // Create pipes for communication between main process and the CSP
  if (pipe(PipeCSP2MAIN) < 0) ppl_fatal(__FILE__,__LINE__,"Could not create a pipe.");
  if (pipe(PipeMAIN2CSP) < 0) ppl_fatal(__FILE__,__LINE__,"Could not create a pipe.");

  if      ((pid=fork()) < 0) ppl_fatal(__FILE__,__LINE__,"Could not fork a child process for the CSP.");
  else if ( pid        != 0)
   {
    close(PipeCSP2MAIN[1]); // Parent process; close CSP's ends of pipes
    close(PipeMAIN2CSP[0]);
    return; // Parent process returns
   }

  // Close main process's ends of the pipes
  close(PipeCSP2MAIN[0]);
  close(PipeMAIN2CSP[1]);

  // Make all log messages appear to come from the CSP
  sprintf(ppl_error_source, "CSP%6d", getpid());
  setpgid( getpid() , getpid() ); // Make into a process group leader so that we won't catch SIGINT

  // Make temporary working directory
  fail=0;
  mkdir(settings_session_default.tempdir , 0700); // Create temporary working directory
  if (access(settings_session_default.tempdir, F_OK) != 0) { fail=1; } // If temporary directory does not exist, fail.
  else
   {
    stat(settings_session_default.tempdir, &statinfo); // Otherwise stat it and make sure it's a directory we own
    if (!S_ISDIR(statinfo.st_mode)) fail=1;
    if (statinfo.st_uid != getuid()) fail=1;
   }
  if (fail==1)                                     { ppl_fatal(__FILE__,__LINE__,"Failed to create temporary directory." ); }
  if (chdir(settings_session_default.tempdir) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into temporary directory failed."); } // chdir into temporary directory

  // Enter CSP execution loop
  CSPmain();
  close(PipeCSP2MAIN[1]); // Close pipes
  close(PipeMAIN2CSP[0]);
  if (chdir("/") < 0) ppl_fatal(__FILE__,__LINE__,"chdir out of temporary directory failed."); // chdir out of temporary directory
  sprintf(PipeOutputBuffer, "rm -Rf %s", settings_session_default.tempdir);
  if (system(PipeOutputBuffer) < 0) ppl_fatal(__FILE__,__LINE__,"Removal of temporary directory failed."); // Remove temporary directory
  ppl_FreeAll(MEMORY_GLOBAL);
  if (DEBUG) ppl_log("CSP terminating normally.");
  exit(0);
 }

void CheckForGvOutput()
 {
  int   pos;
  char *line;
  char  linebuffer[SSTR_LENGTH];
  pos = strlen(PipeOutputBuffer);
  // Todo: Call select() here to make sure there is data to read
  read(PipeCSP2MAIN[0], PipeOutputBuffer+pos, LSTR_LENGTH-pos-5);
  while (1)
   {
    line = StrRemoveCompleteLine(PipeOutputBuffer);
    if (strlen(line)==0) break;
    if (strstr(line, SIGTERM_NAME)!=NULL) continue;
    strcpy(linebuffer,line);
    ppl_error(linebuffer);
   }
  return;
 }

void SendCommandToCSP(char *cmd)
 {
  if (write(PipeMAIN2CSP[1], cmd, strlen(cmd)) != strlen(cmd)) ppl_error("Attempt to send a message to the CSP failed.");
  return;
 }

// Functions to be called from the Child Support Process

void  CSPmain()
 {
  while ((PyXPlotRunning==1) || (ListLen(GhostView_Persists)>0))
   {
    sleep(1); // Wake up every second
    CSPCheckForChildExits();
    CSPCheckForNewCommands(); // Check for orders from PyXPlot
    if (getppid()==1) // We've been orphaned and adopted by init
     {
      PyXPlotRunning=0;
      if (DEBUG) ppl_log("CSP has detected that it has been orphaned.");
     }
   }
  return;
 }

void CSPCheckForChildExits()
 {
  ListIterator *iter;
  int          *gv_pid;
  iter = ListIterateInit(GhostViews);
  while (iter != NULL)
   {
    iter = ListIterate(iter, (void **)&gv_pid);
    if (waitpid(*gv_pid,NULL,WNOHANG) != 0)
     {
      if (DEBUG) { sprintf(temp_err_string, "A GhostView process with pid %d has terminated.", *gv_pid); ppl_log(temp_err_string); }
      ListRemovePtr(GhostViews, (void *)gv_pid); // Stabat mater dolorosa
     }
    if (GhostView_pid == *gv_pid) GhostView_pid = 0;
   }
  iter = ListIterateInit(GhostView_Persists);
  while (iter != NULL)
   {
    iter = ListIterate(iter, (void **)&gv_pid);
    if (waitpid(*gv_pid,NULL,WNOHANG) != 0)
     {
      if (DEBUG) { sprintf(temp_err_string, "A persistent GhostView process with pid %d has terminated.", *gv_pid); ppl_log(temp_err_string); }
      ListRemovePtr(GhostView_Persists, (void *)gv_pid); // Stabat mater dolorosa
     }
    if (GhostView_pid == *gv_pid) GhostView_pid = 0;
   }
  return;
 }

void CSPCheckForNewCommands()
 {
  int pos;
  char *line;
  char linebuffer[SSTR_LENGTH];
  pos = strlen(PipeOutputBuffer);
  read(PipeMAIN2CSP[0], PipeOutputBuffer+pos, LSTR_LENGTH-pos-5);
  while (1)
   {
    line = StrRemoveCompleteLine(PipeOutputBuffer);
    if (strlen(line)==0) break;
    strcpy(linebuffer, line);
    CSPProcessCommand(linebuffer);
   }
  return;
 }

void CSPProcessCommand(char *in)
 {
  char cmd[FNAME_LENGTH];

  if      (strlen(in)==0) return;
  else if (in[0]=='A')                                 // clear command executed
   {
    if (DEBUG) ppl_log("Received request to clear display.");
    CSPKillLatestSinglewindow();
   }
  else if (in[0]=='B')                                 // PyXPlot quit
   {
    if (DEBUG) ppl_log("Received notice that PyXPlot is quitting.");
    CSPKillAllGvs();
    PyXPlotRunning=0;
   }
  else if (in[0]=='0')                                // gv_singlewindow
   {
    if (GhostView_pid > 1)
     {
      if (DEBUG) { sprintf(temp_err_string, "Received gv_singlewindow request. Putting into existing window with pid %d.", GhostView_pid); ppl_log(temp_err_string); }
      sprintf(cmd, "cp -f %s %s", in+1, GhostView_Fname);
      system(cmd);
     }
    else
     {
      if (DEBUG) ppl_log("Received gv_singlewindow request. Making a new window for it.");
      strcpy(GhostView_Fname, in+1);
      GhostView_pid = CSPForkNewGv(in+1, GhostViews);
     }
   }
  else if (in[0]=='1')                                // gv_multiwindow
   {
    if (DEBUG) ppl_log("Received gv_multiwindow request.");
    CSPForkNewGv(in+1, GhostViews);
   }
  else if (in[0]=='2')                                // gv_persist
   {
    if (DEBUG) ppl_log("Received gv_persist request.");
    CSPForkNewGv(in+1, GhostView_Persists);
   }
  return;
 }

int CSPForkNewGv(char *fname, List *gv_list)
 {
  int pid;
  char WatchText[16];

  if      ((pid=fork()) < 0) ppl_fatal(__FILE__,__LINE__,"Could not fork a child process for the CSP.");
  else if ( pid        != 0)
   {
    // Parent process (i.e. the CSP)
    ListAppendInt(gv_list, pid);
    return pid;
   }
  else
   {
    // Child process; about to run GhostView.
    sprintf(ppl_error_source, "GV%7d", getpid());
    if (DEBUG) { sprintf(temp_err_string, "New GhostView process alive; going to view %s.", fname); ppl_log(temp_err_string); }
    if (PipeCSP2MAIN[1] != STDERR_FILENO) // Redirect stderr to pipe, so that GhostView doesn't spam terminal
     {
      if (dup2(PipeCSP2MAIN[1], STDERR_FILENO) != STDERR_FILENO) ppl_fatal(__FILE__,__LINE__,"Could not redirect stderr to pipe.");
      close(PipeCSP2MAIN[1]);
     }
    setpgid( getpid() , getpid() ); // Make into a process group leader so that we won't catch SIGINT
    sprintf(WatchText, "%s%s", GHOSTVIEW_OPT, "watch");
    execlp(GHOSTVIEW, GHOSTVIEW, WatchText, fname, NULL); // Execute GhostView
    ppl_fatal(__FILE__,__LINE__,"Execution of GhostView failed."); // execlp call should not return
    exit(1); // ppl_fatal shouldn't either, so something's gone really wrong...
   }
  return 0;
 }

void CSPKillAllGvs()
 {
  ListIterator *iter;
  int          *gv_pid;
  if (DEBUG) ppl_log("Killing all GhostView processes.");
  iter = ListIterateInit(GhostViews);
  while (iter != NULL)
   {
    iter = ListIterate(iter, (void **)&gv_pid);
    kill(*gv_pid, SIGTERM); // Dulce et decorum est pro patria mori
    ListRemovePtr(GhostViews, (void *)gv_pid);
    if (GhostView_pid == *gv_pid) GhostView_pid = 0;
   }
  return;
 }

void CSPKillLatestSinglewindow()
 {
  if (DEBUG) { sprintf(temp_err_string, "Killing latest GhostView singlewindow process with pid %d.", GhostView_pid); ppl_log(temp_err_string); }
  if (GhostView_pid > 1) kill(GhostView_pid, SIGTERM);
  GhostView_pid = 0;
  CSPCheckForChildExits();
  return;
 }

