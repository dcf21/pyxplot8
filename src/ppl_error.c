// ppl_error.c
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

#include "ppl_constants.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

static int  ppl_error_input_linenumber             = -1;
static char ppl_error_input_filename[FNAME_LENGTH] = "";
static int  ppl_error_last_linenumber              = -1; // Only inform user one of each line number
static char ppl_error_last_filename[FNAME_LENGTH]  = "";

static char temp_stringA[FNAME_LENGTH], temp_stringB[FNAME_LENGTH];

void ppl_error_setstreaminfo(int linenumber,char *filename)
 {
  ppl_error_input_linenumber = linenumber;
  strcpy(ppl_error_input_filename, filename);
  return;
 }

void ppl_error(char *msg)
 {
  if ( ((       ppl_error_input_linenumber    != -1) && (       ppl_error_input_linenumber                         != ppl_error_last_linenumber)) ||
       ((strcmp(ppl_error_input_filename, "") !=  0) && (strcmp(ppl_error_input_filename, ppl_error_last_filename) !=                         0))   )
   {
    strcpy(ppl_error_last_filename, ppl_error_input_filename);
    ppl_error_last_linenumber = ppl_error_input_linenumber;
    sprintf(temp_stringA, "Error encountered in %s at line %d:", ppl_error_input_filename, ppl_error_input_linenumber);
    ppl_error(temp_stringA);
   }
  if (isatty(STDERR_FILENO) == 1)
   sprintf(temp_stringB, "%s%s%s\n", (char *)FetchSettingName( settings_session_default.colour_err , SW_TERMCOL_INT , SW_TERMCOL_TXT),
                                     msg,
                                     (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , SW_TERMCOL_TXT) );
  else
   sprintf(temp_stringB, "%s\n", msg);
  fputs(temp_stringB, stderr);
  return; 
 }

void ppl_fatal(char *msg)
 {
  ppl_error(msg);
  exit(1);
 }

void ppl_warning(char *msg)
 {
  if (isatty(STDERR_FILENO) == 1)
   sprintf(temp_stringB, "%s%s%s\n", (char *)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , SW_TERMCOL_TXT),
                                     msg,
                                     (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , SW_TERMCOL_TXT) );
  else
   sprintf(temp_stringB, "%s\n", msg);
  fputs(temp_stringB, stderr);
  return;
 }

void ppl_report(char *msg)
 {
  if (isatty(STDOUT_FILENO) == 1)
   sprintf(temp_stringB, "%s%s%s\n", (char *)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , SW_TERMCOL_TXT),
                                     msg,
                                     (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , SW_TERMCOL_TXT) );
  else
   sprintf(temp_stringB, "%s\n", msg);
  fputs(temp_stringB, stdout);
  return;
 }

