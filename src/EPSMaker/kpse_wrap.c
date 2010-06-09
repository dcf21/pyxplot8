// kpse_wrap.c
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

// Wrapper for libkpathsea which provides an alternative implementation of the
// library -- directly forking kpsewhich -- for use on machines such as Macs
// where libkpathsea is tricky to install.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "StringTools/str_constants.h"

#include "ppl_error.h"

#ifdef HAVE_KPATHSEA
#include <kpathsea/kpathsea.h>
#else
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "ppl_children.h"
#define MAX_PATHS 256
char  ppl_kpse_FilePaths[3][LSTR_LENGTH];
char *ppl_kpse_PathList[3][MAX_PATHS];
#endif

void ppl_kpse_wrap_init()
 {
  #ifdef HAVE_KPATHSEA
  kpse_set_program_name("dvips", "dvips");
  #else
  const char     *FileTypes[3] = {"tfm","pfa","pfb"};
  int             i, j, k, l, s, TrialNumber, fstdout;
  struct timespec waitperiod; // A time.h timespec specifier for a wait of zero seconds
  fd_set          readable;
  sigset_t        sigs;

  for (j=0; j<3; j++) ppl_kpse_PathList[j][0] = NULL;

  for (j=0; j<3; j++)
   {
    ForkKpseWhich(FileTypes[j] , &fstdout);

    // Wait for kpsewhich process's stdout to become readable. Get bored if this takes more than a second.
    TrialNumber = 1;
    while (1)
     {
      waitperiod.tv_sec  = 1; waitperiod.tv_nsec = 0;
      FD_ZERO(&readable); FD_SET(fstdout, &readable);
      if (pselect(fstdout+1, &readable, NULL, NULL, &waitperiod, NULL) == -1)
       {
        if ((errno==EINTR) && (TrialNumber<3)) { TrialNumber++; continue; }
        ppl_error(ERR_INTERNAL, -1, -1, "Failure of the pselect() function whilst waiting for kpsewhich to return data.");
        return;
       }
      break;
     }
    if (!FD_ISSET(fstdout , &readable)) { ppl_error(ERR_GENERAL, -1, -1, "Got bored waiting for kpsewhich to return data."); sigprocmask(SIG_UNBLOCK, &sigs, NULL); continue; }

    // Read data back from kpsewhich process
    if ((i = read(fstdout, ppl_kpse_FilePaths[j], LSTR_LENGTH)) < 0) { ppl_error(ERR_GENERAL, -1, -1, "Could not read from pipe to kpsewhich."); sigprocmask(SIG_UNBLOCK, &sigs, NULL); continue; }
    ppl_kpse_FilePaths[j][i] = '\0';
    close(fstdout);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);

    // Split up returned data into a list of paths
    for (i=s=k=0; ppl_kpse_FilePaths[j][i]!='\0'; i++)
     {
      if (ppl_kpse_FilePaths[j][i]=='!') continue;
      if ((ppl_kpse_FilePaths[j][i]==':') || (ppl_kpse_FilePaths[j][i]=='\n'))
       {
        for (l=i-1; ((l>=0)&&(ppl_kpse_FilePaths[j][l]==PATHLINK[0])); l--) ppl_kpse_FilePaths[j][l]='\0';
        s=0;
        ppl_kpse_FilePaths[j][i]='\0';
        continue;
       }
      if (!s)
       {
        s=1;
        ppl_kpse_PathList[j][k++] = ppl_kpse_FilePaths[j]+i;
        if (k==MAX_PATHS) { k--; ppl_error(ERR_GENERAL, -1, -1, "kpsewhich returned too many paths"); }
       }
     }
    ppl_kpse_PathList[j][k] = NULL;

    // If debugging, log a list of the paths that we've extracted
    if (DEBUG)
     {
      for (i=0; ppl_kpse_PathList[j][i]!=NULL; i++)
       {
        sprintf(temp_err_string, "Using path for %s files: <%s>", FileTypes[j], ppl_kpse_PathList[j][i]);
        ppl_log(temp_err_string);
       }
     }
   }
  #endif
  return;
 }

#ifndef HAVE_KPATHSEA
char *ppl_kpse_wrap_find_file(char *s, char **paths)
 {
  int i;
  static char buffer[FNAME_LENGTH];
  if (DEBUG) { sprintf(temp_err_string, "Searching for file <%s>", s); ppl_log(temp_err_string); }
  for (i=0; paths[i]!=NULL; i++)
   {
    snprintf(buffer, FNAME_LENGTH, "%s%s%s", paths[i], PATHLINK, s);
    buffer[FNAME_LENGTH-1]='\0';
    if (DEBUG) { sprintf(temp_err_string, "Trying file <%s>", buffer); ppl_log(temp_err_string); }
    if (access(buffer, R_OK) == 0)
     {
      if (DEBUG) { sprintf(temp_err_string, "KPSE found file <%s>", buffer); ppl_log(temp_err_string); }
      return buffer;
     }
   }
  return NULL;
 }
#endif

char *ppl_kpse_wrap_find_pfa(char *s)
 {
  #ifdef HAVE_KPATHSEA
  return (char *)kpse_find_file(s, kpse_type1_format, true);
  #else
  return ppl_kpse_wrap_find_file(s, ppl_kpse_PathList[1]);
  #endif
 }

char *ppl_kpse_wrap_find_pfb(char *s)
 {
  #ifdef HAVE_KPATHSEA
  return (char *)kpse_find_file(s, kpse_type1_format, true);
  #else
  return ppl_kpse_wrap_find_file(s, ppl_kpse_PathList[2]);
  #endif
 }

char *ppl_kpse_wrap_find_tfm(char *s)
 {
  #ifdef HAVE_KPATHSEA
  return (char *)kpse_find_tfm(s);
  #else
  return ppl_kpse_wrap_find_file(s, ppl_kpse_PathList[0]);
  #endif
 }

