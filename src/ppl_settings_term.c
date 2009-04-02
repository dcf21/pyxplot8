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

#define PPL_SETTINGS_TERM 1

#include <stdio.h>
#include <string.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_colours.h"
#include "ppl_passwd.h"
#include "ppl_error.h"

settings_terminal settings_term_default;
settings_terminal settings_term_current;

settings_graph settings_graph_default;
settings_graph settings_graph_current;

settings_session settings_session_default;

void ppl_settings_term_init()
 {
  char ConfigFname[FNAME_LENGTH];

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
  settings_graph_default.AxesColour = COLOUR_BLACK;
  settings_graph_default.aspect     = 1.0;
  settings_graph_default.AutoAspect = SW_ONOFF_ON;
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

  // Setting which affect how we talk to the current interactive session
  settings_session_default.splash    = SW_ONOFF_ON;
  settings_session_default.colour    = SW_ONOFF_ON;
  settings_session_default.colour_rep= SW_TERMCOL_GRN;
  settings_session_default.colour_wrn= SW_TERMCOL_BRN;
  settings_session_default.colour_err= SW_TERMCOL_RED;
  strcpy(settings_session_default.homedir, UnixGetHomeDir());

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
  first = *id_list;
  while(1)
   {
    if (*id_list == id) return *name_list;
    if (*id_list == -1)
     {
      sprintf(temp_err_string, "Setting with illegal value %d; should have had a value of type %d.", id, first);
      ppl_fatal(__FILE__, __LINE__, temp_err_string);
     }
    id_list++; name_list++;
   }
  sprintf(temp_err_string, "Setting has illegal value %d.", id);
  ppl_fatal(__FILE__, __LINE__, temp_err_string);
  return NULL;
 }

