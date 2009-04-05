// ppl_settings_term.c
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

#define _PPL_SETTINGS_TERM 1

#include <stdio.h>
#include <string.h>

#include "asciidouble.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_colours.h"
#include "ppl_passwd.h"
#include "ppl_error.h"

settings_terminal settings_term_default;
settings_terminal settings_term_current;

settings_graph    settings_graph_default;
settings_graph    settings_graph_current;

settings_axis     settings_axis_default;

settings_session  settings_session_default;

int               settings_palette[PALETTE_LENGTH] = {COLOUR_BLACK, COLOUR_RED, COLOUR_BLUE, COLOUR_MAGENTA, COLOUR_CYAN, COLOUR_BROWN, COLOUR_SALMON, COLOUR_GRAY, COLOUR_GREEN, COLOUR_NAVYBLUE, COLOUR_PERIWINKLE, COLOUR_PINEGREEN, COLOUR_SEAGREEN, COLOUR_GREENYELLOW, COLOUR_ORANGE, COLOUR_CARNATIONPINK, COLOUR_PLUM, -1};

void ppl_settings_term_init()
 {
  FILE  *LocalePipe;
  int    Nchars;
  double PaperWidth, PaperHeight;
  char   ConfigFname[FNAME_LENGTH];

  // Default Terminal Settings, used when these values are not changed by any configuration files
  settings_term_default.backup    = SW_ONOFF_OFF;
  settings_term_default.colour    = SW_ONOFF_ON;
  settings_term_default.display   = SW_ONOFF_ON;
  settings_term_default.dpi       = 300.0;
  settings_term_default.landscape = SW_ONOFF_OFF;
  strcpy(settings_term_default.LatexPreamble, "");
  settings_term_default.multiplot = SW_ONOFF_OFF;
  strcpy(settings_term_default.output, "");
  settings_term_default.PaperHeight= 297.0;
  strcpy(settings_term_default.PaperName, "A4");
  settings_term_default.PaperWidth = 210.0;
  settings_term_default.TermAntiAlias = SW_ONOFF_ON;
  settings_term_default.TermType      = SW_TERMTYPE_X11S;
  settings_term_default.TermEnlarge   = SW_ONOFF_OFF;
  settings_term_default.TermInvert    = SW_ONOFF_OFF;
  settings_term_default.TermTransparent = SW_ONOFF_OFF;

  // Default Graph Settings, used when these values are not changed by any configuration files
  settings_graph_default.aspect     = 1.0;
  settings_graph_default.AutoAspect = SW_ONOFF_ON;
  settings_graph_default.AxesColour = COLOUR_BLACK;
  settings_graph_default.bar        = 1.0;
  settings_graph_default.BinOrigin  = 0.0;
  settings_graph_default.BinWidth   = 1.0;
  settings_graph_default.BoxFrom    = 0.0;
  settings_graph_default.BoxWidth   = 0.0;
  settings_graph_default.DataStyle  = SW_STYLE_POINTS;
  settings_graph_default.FontSize   = 0;
  settings_graph_default.FuncStyle  = SW_STYLE_LINES;
  settings_graph_default.grid       = SW_ONOFF_OFF;
  settings_graph_default.GridAxisX  = 1;
  settings_graph_default.GridAxisY  = 1;
  settings_graph_default.GridMajColour = COLOUR_GREY60;
  settings_graph_default.GridMinColour = COLOUR_GREY90;
  settings_graph_default.key        = SW_ONOFF_ON;
  settings_graph_default.KeyColumns = 1;
  settings_graph_default.KeyPos     = SW_KEYPOS_TR;
  settings_graph_default.KeyXOff    = 0.0;
  settings_graph_default.KeyYOff    = 0.0;
  settings_graph_default.LineWidth  = 1.0;
  settings_graph_default.OriginX    = 0.0;
  settings_graph_default.OriginY    = 0.0;
  settings_graph_default.PointSize  = 1.0;
  settings_graph_default.PointLineWidth = 1.0;
  settings_graph_default.samples    = 250;
  settings_graph_default.TextColour = COLOUR_BLACK;
  settings_graph_default.TextHAlign = SW_HALIGN_LEFT;
  settings_graph_default.TextVAlign = SW_VALIGN_BOT;
  strcpy(settings_graph_default.title, "");
  settings_graph_default.TitleXOff  = 0.0;
  settings_graph_default.TitleYOff  = 0.0;
  settings_graph_default.width      = 8.0;

  // Default Axis Settings, used whenever a new axis is created
  settings_axis_default.log         = SW_BOOL_FALSE;
  settings_axis_default.MaxSet      = SW_BOOL_FALSE;
  settings_axis_default.MinSet      = SW_BOOL_FALSE;
  settings_axis_default.TickDirection=SW_TICDIR_IN;
  settings_axis_default.MTickMaxSet = SW_BOOL_FALSE;
  settings_axis_default.MTickMinSet = SW_BOOL_FALSE;
  settings_axis_default.MTickStepSet= SW_BOOL_FALSE;
  settings_axis_default.TickMaxSet  = SW_BOOL_FALSE;
  settings_axis_default.TickMinSet  = SW_BOOL_FALSE;
  settings_axis_default.TickStepSet = SW_BOOL_FALSE;
  settings_axis_default.LogBase     = 10.0;
  settings_axis_default.max         =  0.0;
  settings_axis_default.min         =  0.0;
  settings_axis_default.MTickMax    =  0.0;
  settings_axis_default.MTickMin    =  0.0;
  settings_axis_default.MTickStep   =  0.0;
  settings_axis_default.TickMax     =  0.0;
  settings_axis_default.TickMin     =  0.0;
  settings_axis_default.TickStep    =  0.0;
  settings_axis_default.MTickList   = NULL;
  settings_axis_default.TickList    = NULL;
  strcpy(settings_axis_default.label, "");

  // Setting which affect how we talk to the current interactive session
  settings_session_default.splash    = SW_ONOFF_ON;
  settings_session_default.colour    = SW_ONOFF_ON;
  settings_session_default.colour_rep= SW_TERMCOL_GRN;
  settings_session_default.colour_wrn= SW_TERMCOL_BRN;
  settings_session_default.colour_err= SW_TERMCOL_RED;
  strcpy(settings_session_default.homedir, UnixGetHomeDir());

  // Try and find out the default papersize from the locale command
  // Do this using the popen() command rather than direct calls to nl_langinfo(_NL_PAPER_WIDTH), because the latter is gnu-specific
  if (DEBUG) ppl_log("Querying papersize from the locale command.");
  if ((LocalePipe = popen("locale -c LC_PAPER 2> /dev/null","r"))==NULL)
   {
    if (DEBUG) ppl_log("Failed to open a pipe to the locale command.");
   } else {
    file_readline(LocalePipe, ConfigFname); // Should read LC_PAPER
    file_readline(LocalePipe, ConfigFname); // Should quote the default paper width
    PaperHeight = GetFloat(ConfigFname, &Nchars);
    if (Nchars != strlen(ConfigFname)) goto PAPERSIZE_DONE;
    file_readline(LocalePipe, ConfigFname); // Should quote the default paper height
    PaperWidth  = GetFloat(ConfigFname, &Nchars);
    if (Nchars != strlen(ConfigFname)) goto PAPERSIZE_DONE;
    if (DEBUG) { sprintf(temp_err_string, "Read papersize %f x %f", PaperWidth, PaperHeight); ppl_log(temp_err_string); }
    settings_term_default.PaperHeight = PaperHeight;
    settings_term_default.PaperWidth  = PaperWidth;
    if (0) { PAPERSIZE_DONE: if (DEBUG) ppl_log("Failed to read papersize from the locale command."); }
    pclose(LocalePipe);
   }

  sprintf(ConfigFname, "%s%s%s", settings_session_default.homedir, PATHLINK, ".pyxplotrc");
  ReadConfigFile(ConfigFname);
  sprintf(ConfigFname, "%s", ".pyxplotrc");
  ReadConfigFile(ConfigFname);

  // Copy Default Settings to Current Settings
  settings_term_current  = settings_term_default;
  settings_graph_current = settings_graph_default;
  return;
 }

void *FetchSettingName(int id, int *id_list, void **name_list)
 {
  int first;
  static int latch=0;
  static char *dummyout = "";
  first = *id_list;
  while(1)
   {
    if (*id_list == id) return *name_list;
    if (*id_list == -1)
     {
      if (latch==1) return dummyout; // Prevent recursive calling
      latch=1;
      sprintf(temp_err_string, "Setting with illegal value %d; should have had a value of type %d.", id, first);
      ppl_fatal(__FILE__, __LINE__, temp_err_string);
     }
    id_list++; name_list++;
   }
  if (latch==1) return dummyout;
  latch=1;
  sprintf(temp_err_string, "Setting has illegal value %d.", id);
  ppl_fatal(__FILE__, __LINE__, temp_err_string);
  return NULL;
 }

int FetchSettingByName(char *name, int *id_list, char **name_list)
 {
  char UpperName[LSTR_LENGTH];
  StrUpper(name, UpperName);
  while(1)
   {
    if (*id_list == -1) return -1;
    if (strcmp(UpperName, *name_list) == 0) return *id_list;
    id_list++; name_list++;
   }
  return -1;
 }

