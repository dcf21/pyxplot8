// ppl_readconfig.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-9 Dominic Ford <coders@pyxplot.org.uk>
//               2008-9 Ross Church
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

#define _PPL_READCONFIG_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "StringTools/asciidouble.h"
#include "EPSMaker/eps_colours.h"
#include "ListTools/lt_memory.h"
#include "MathsTools/dcfmath.h"

#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_papersize.h"
#include "ppl_parser.h"
#include "ppl_setshow.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

int ppl_termtype_set_in_configfile = 0;

void _ReadConfig_FetchKey(char *line, char *out)
 {
  char *scan = out;
  while ((*line != '\0') && ((*(scan) = *(line++)) != '=')) scan++;
  *scan = '\0';
  StrStrip(out, out);
  return;
 }

void _ReadConfig_FetchValue(char *line, char *out)
 {
  char *scan = out;
  while ((*line != '\0') && (*(line++) != '='));
  while  (*line != '\0') *(scan++) = *(line++);
  *scan = '\0';
  StrStrip(out, out);
  return;
 }

void ReadConfigFile(char *ConfigFname)
 {
  char  linebuffer[LSTR_LENGTH], setkey[LSTR_LENGTH], setvalue[LSTR_LENGTH], ColourName[SSTR_LENGTH], *StringScan;
  char  errtext[LSTR_LENGTH], setstring[LSTR_LENGTH];
  value setnumeric;
  Dict *scriptcmd;
  FILE *infile;
  int   state=-1;
  int   linecounter=0;
  int   i, j, k, PalettePos, ColourNumber;
  int   errpos, end;
  double fl, PaperHeight, PaperWidth;

  if (DEBUG) { sprintf(temp_err_string, "Scanning configuration file %s.", ConfigFname); ppl_log(temp_err_string); }

  if ((infile=fopen(ConfigFname,"r")) == NULL)
   {
    if (DEBUG) { ppl_log("File does not exist."); }
    return;
   }

  while ((!feof(infile)) && (!ferror(infile)))
   {
    linecounter++;
    file_readline(infile, linebuffer, LSTR_LENGTH);
    StrStrip(linebuffer, linebuffer);
    if             (linebuffer[0] == '\0')                   continue;
    else if (strcmp(linebuffer, "[settings]" )==0) {state=1; continue;}
    else if (strcmp(linebuffer, "[terminal]" )==0) {state=2; continue;}
    else if (strcmp(linebuffer, "[colours]"  )==0) {state=3; continue;}
    else if (strcmp(linebuffer, "[latex]"    )==0) {state=4; continue;}
    else if (strcmp(linebuffer, "[variables]")==0) {state=5; continue;}
    else if (strcmp(linebuffer, "[functions]")==0) {state=6; continue;}
    else if (strcmp(linebuffer, "[units]"    )==0) {state=7; continue;}
    else if (strcmp(linebuffer, "[filters]"  )==0) {state=8; continue;}
    else if (strcmp(linebuffer, "[script]"   )==0) {state=9; continue;}

    _ReadConfig_FetchKey  (linebuffer, setkey  );
    _ReadConfig_FetchValue(linebuffer, setvalue);

    if (state == 1) // [settings] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "ASPECT"       )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.aspect        = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting ASPECT."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AUTOASPECT"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.AutoAspect    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting AUTOASPECT."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXESCOLOUR"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.AxesColour    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting COLOUR."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BACKUP"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .backup        = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BACKUP."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BAR"          )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.bar           = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BAR."          , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BINORIGIN"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_term_default.BinOrigin.real = fl;
                                                                              settings_term_default.BinOriginAuto  = 0;  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BINORIGIN."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BINWIDTH"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_term_default.BinWidth.real  = fl;
                                                                              settings_term_default.BinWidthAuto   = (fl>0.0);  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BINWIDTH."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BOXFROM"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_graph_default.BoxFrom.real  = fl;
                                                                              settings_graph_default.BoxWidthAuto  = 0;  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BOXFROM."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BOXWIDTH"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_graph_default.BoxWidth.real = fl;
                                                                              settings_graph_default.BoxWidthAuto  = (fl>0.0);  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BOXWIDTH."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "CALENDARIN"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_CALENDAR_INT, SW_CALENDAR_STR ))>0) settings_term_default.CalendarIn = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting CALENDARIN."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "CALENDAROUT"  )==0)
        if ((i=FetchSettingByName(setvalue,SW_CALENDAR_INT, SW_CALENDAR_STR ))>0) settings_term_default.CalendarOut= fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting CALENDAROUT."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .colour        = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting COLOUR."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "DATASTYLE"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_STYLE_INT, SW_STYLE_STR ))>0) settings_graph_default.DataStyle.style = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting DATASTYLE."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "DISPLAY"      )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .display       = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting DISPLAY."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "DPI"          )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_term_default .dpi           = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting DPI."          , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "FONTSIZE"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.FontSize      = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting FONTSIZE."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "FUNCSTYLE"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_STYLE_INT, SW_STYLE_STR ))>0) settings_graph_default.FuncStyle.style = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting FUNCSTYLE."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRID"         )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.grid          = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GRID."         , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRISAXISX"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue) && (fl>=0) && (fl<=MAX_AXES)))
         {
          settings_graph_default.GridAxisX[1]       = 0;
          settings_graph_default.GridAxisX[(int)fl] = 1;
         }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GRIDAXISX."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRISAXISY"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue) && (fl>=0) && (fl<=MAX_AXES)))               
         {               
          settings_graph_default.GridAxisY[1]       = 0;
          settings_graph_default.GridAxisY[(int)fl] = 1;
         }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GRIDAXISY."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRISAXISZ"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue) && (fl>=0) && (fl<=MAX_AXES)))               
         {               
          settings_graph_default.GridAxisZ[1]       = 0;
          settings_graph_default.GridAxisZ[(int)fl] = 1;
         }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GRIDAXISZ."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDMAJCOLOUR")==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.GridMajColour = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GRIDMAJCOLOUR.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDMINCOLOUR")==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.GridMinColour = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GRIDMINCOLOUR.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEY"          )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.key           = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KEY."          , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEYCOLUMNS"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.KeyColumns    = max((int)fl, 1);
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KEYCOLUMNS."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEYPOS"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_KEYPOS_INT,SW_KEYPOS_STR))>0) settings_graph_default.KeyPos        = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KEYPOS."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEY_XOFF"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.KeyXOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KEY_XOFF."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEY_YOFF"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.KeyYOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KEY_YOFF."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "LANDSCAPE"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .landscape     = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting LANDSCAPE."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "LINEWIDTH"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.LineWidth     = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting LINEWIDTH."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "MULTIPLOT"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .multiplot     = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting MULTIPLOT."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMCOMPLEX"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .ComplexNumbers= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NUMCOMPLEX."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMERR"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .ExplicitErrors= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NUMERR."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMSF"        )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_term_default .SignificantFigures = min(max((int)fl, 1), 30);
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NUMSF."        , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMDISPLAY"  )==0)
        if ((i=FetchSettingByName(setvalue,SW_DISPLAY_INT, SW_DISPLAY_STR ))>0) settings_term_default.NumDisplay = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NUMDISPLAY."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ORIGINX"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.OriginX.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting ORIGINX."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ORIGINY"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.OriginY.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting ORIGINY."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "OUTPUT"       )==0)
        strcpy(settings_term_default.output , setvalue);
      else if (strcmp(setkey, "PAPER_HEIGHT" )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_term_default .PaperHeight.real  = fl/1000;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PAPER_HEIGHT." , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "PAPER_NAME"   )==0)
        {
         ppl_PaperSizeByName(setvalue, &PaperHeight, &PaperWidth);
         if (PaperHeight <= 0) {sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised papersize specified for setting PAPER_NAME."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
         settings_term_default.PaperHeight.real = PaperHeight/1000;
         settings_term_default.PaperWidth.real  = PaperWidth/1000;
        }
      else if (strcmp(setkey, "PAPER_WIDTH"  )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_term_default .PaperWidth.real  = fl/1000;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PAPER_WIDTH."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "POINTLINEWIDTH")==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.PointLineWidth= fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting POINTLINEWIDTH.",linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "POINTSIZE"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.PointSize     = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting POINTSIZE."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "PROJECTION"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_PROJ_INT, SW_PROJ_STR ))>0)   settings_graph_default.projection    = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PROJECTION."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "SAMPLES"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.samples       = max((int)fl, 2);
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting SAMPLES."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMANTIALIAS")==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermAntiAlias = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TERMANTIALIAS.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMENLARGE"  )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermEnlarge   = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TERMENLARGE."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ENLARGE"      )==0) // ENLARGE, as opposed to TERMENLARGE is supported for back-compatibility with PyXPlot 0.7
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermEnlarge   = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting ENLARGE."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMINVERT"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermInvert = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TERMINVERT."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMTRANSPARENT")==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermTransparent= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TERMTRANSPARENT.",linecounter,ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMTYPE"     )==0)
        if ((i=FetchSettingByName(setvalue,SW_TERMTYPE_INT,SW_TERMTYPE_STR))>0) settings_term_default.TermType  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TERMTYPE."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TEXTCOLOUR"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.TextColour    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TEXTCOLOUR."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TEXTHALIGN"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_HALIGN_INT,SW_HALIGN_STR))>0) settings_graph_default.TextHAlign    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TEXTHALIGN."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TEXTVALIGN"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_VALIGN_INT,SW_VALIGN_STR))>0) settings_graph_default.TextVAlign    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TEXTVALIGN."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TITLE"        )==0)
        strcpy(settings_graph_default.title  , setvalue);
      else if (strcmp(setkey, "TITLE_XOFF"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.TitleXOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TITLE_XOFF."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TITLE_YOFF"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.TitleYOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TITLE_YOFF."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TRANGE_MIN"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.Tmin.real       = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TRANGE_MIN."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TRANGE_MAX"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.Tmax.real       = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TRANGE_MAX."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "UNITABBREV"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .UnitDisplayAbbrev= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting UNITABBREV."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "UNITPREFIX"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .UnitDisplayPrefix= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting UNITPREFIX."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "UNITSCHEME"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_UNITSCH_INT,SW_UNITSCH_STR))>0) settings_term_default.UnitScheme = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting UNITSCHEME."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "WIDTH"        )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.width.real      = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting WIDTH."        , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
     }
    else if (state == 2) // [terminal] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "COLOUR"       )==0)
        if ((i=FetchSettingByName(setvalue, SW_ONOFF_INT,   SW_ONOFF_STR  ))>0) settings_session_default.colour      = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting COLOUR."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR_ERR"   )==0)
        if ((i=FetchSettingByName(setvalue, SW_TERMCOL_INT, SW_TERMCOL_STR))>0) settings_session_default.colour_err  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting COLOUR_ERR."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR_REP"   )==0)
        if ((i=FetchSettingByName(setvalue, SW_TERMCOL_INT, SW_TERMCOL_STR))>0) settings_session_default.colour_rep  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting COLOUR_REP."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR_WRN"   )==0)
        if ((i=FetchSettingByName(setvalue, SW_TERMCOL_INT, SW_TERMCOL_STR))>0) settings_session_default.colour_wrn  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting COLOUR_WRN."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "SPLASH"       )==0)
        if ((i=FetchSettingByName(setvalue, SW_ONOFF_INT,   SW_ONOFF_STR  ))>0) settings_session_default.splash      = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting SPLASH."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
     }
    else if (state == 3) // [colours] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "PALETTE"      )==0)
       {
        PalettePos = 0;
        StringScan = setvalue;
        while (strlen(StrCommaSeparatedListScan(&StringScan, ColourName)) != 0)
         {
          if (PalettePos == PALETTE_LENGTH-1)
           {
            sprintf(temp_err_string, "Error in line %d of configuration file %s: Specified palette is too long.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string);
            settings_palette_default[PalettePos] = -1;
            continue;
           } else {
            StrUpper(ColourName,ColourName);
            ColourNumber = FetchSettingByName(ColourName, SW_COLOUR_INT, SW_COLOUR_STR);
            if (ColourNumber<=0)
             {
              sprintf(temp_err_string, "Error in line %d of configuration file %s: Colour '%s' not recognised.", linecounter, ConfigFname, ColourName); ppl_warning(ERR_PREFORMED, temp_err_string);
             } else {
              settings_palette_default[PalettePos++] = ColourNumber;
             }
           }
         }
        if (PalettePos > 0)
         {
          settings_palette_default[PalettePos] = -1;
         } else {
          sprintf(temp_err_string, "Error in line %d of configuration file %s: The specified palette does not contain any colours.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string);
         }
       }
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
     }
    else if (state == 4) // [latex] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "PREAMBLE"     )==0)
        strcpy(settings_term_default.LatexPreamble, setvalue);
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
     }
    else if (state == 5) // [variables] section
     {
      i=0;
      if (isalpha(setkey[0])) for (i=1;isalnum(setkey[i]);i++);
      if ((i==0)||(setkey[i]!='\0')) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal variable name\n", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      errpos = -1; end = strlen(setvalue);
      if ((setvalue[0]=='\"') || (setvalue[0]=='\'')) ppl_GetQuotedString(setvalue, setstring , 0, &end, 0, &errpos, errtext, 0);
      else                                            ppl_EvaluateAlgebra(setvalue,&setnumeric, 0, &end, 0, &errpos, errtext, 0);
      if (errpos >= 0)
       {
        sprintf(temp_err_string, "Error in line %d of configuration file %s: %s", linecounter, ConfigFname, errtext);
        ppl_warning(ERR_PREFORMED, temp_err_string); continue;
       }
      if ((setvalue[0]=='\"') || (setvalue[0]=='\'')) ppl_UserSpace_SetVarStr    (setkey, setstring , 0);
      else                                            ppl_UserSpace_SetVarNumeric(setkey,&setnumeric, 0);
     }
    else if (state == 6) // [functions] section
     {
      i=0;
      if (isalpha(setkey[0])) for (i=1;isalnum(setkey[i]);i++);
      if ((i==0)||(setkey[i]!='(')) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal function name\n", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      errpos = -1;
      ppl_UserSpace_SetFunc(linebuffer, 0, &errpos, errtext);
      if (errpos >= 0)
       {
        sprintf(temp_err_string, "Error in line %d of configuration file %s: %s", linecounter, ConfigFname, errtext);
        ppl_warning(ERR_PREFORMED, temp_err_string); continue;
       }
     }
    else if (state == 7) // [units] section
     {

#define GET_UNITNAME(output, last, type, sep) \
      if (isalpha(setkey[i])) do { i++; } while ((isalnum(setkey[i])) || (setkey[i]=='_')); \
      if (i==j) \
       { \
        if (&last==&output) \
         { sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal %s name.", linecounter, ConfigFname, type); ppl_warning(ERR_PREFORMED, temp_err_string); continue; } \
        else \
         { \
          output = (char *)lt_malloc(strlen(last)+1); \
          if (output==NULL) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Out of memory error whilst generating new unit.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; } \
          strcpy(output, last); \
         } \
       } else { \
        output = (char *)lt_malloc(i-j+1); \
        if (output==NULL) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Out of memory error whilst generating new unit.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; } \
        strncpy(output, setkey+j, i-j); output[i-j]='\0'; \
       } \
      while ((setkey[i]<=' ')&&(setkey[i]!='\0')) i++; \
      if (setkey[i]==sep) i++; \
      while ((setkey[i]<=' ')&&(setkey[i]!='\0')) i++; \
      j=i;

      i=j=0;
      if (ppl_unit_pos == UNITS_MAX) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unit definition list full.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }

      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].nameFs  , ppl_unit_database[ppl_unit_pos].nameFs  , "unit"    , '/' );
      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].nameAs  , ppl_unit_database[ppl_unit_pos].nameFs  , "unit"    , '/' );
      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].nameLs  , ppl_unit_database[ppl_unit_pos].nameAs  , "unit"    , '/' );
      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].nameFp  , ppl_unit_database[ppl_unit_pos].nameFs  , "unit"    , '/' );
      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].nameAp  , ppl_unit_database[ppl_unit_pos].nameAs  , "unit"    , ':' );
      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].nameLp  , ppl_unit_database[ppl_unit_pos].nameAp  , "unit"    , ':' );
      GET_UNITNAME( ppl_unit_database[ppl_unit_pos].quantity, ppl_unit_database[ppl_unit_pos].quantity, "quantity", ' ' );

      if (setvalue[0]=='\0')
       {
        if (ppl_baseunit_pos == UNITS_MAX_BASEUNITS) { sprintf(temp_err_string, "Error in line %d of configuration file %s:\nBase unit definition list full.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
        ppl_unit_database[ppl_unit_pos++].exponent[ppl_baseunit_pos++] = 1;
       }
      else 
       {
        j = k = -1;
        ppl_units_StringEvaluate(setvalue, &setnumeric, &k, &j, errtext);
        if (j >= 0) { sprintf(temp_err_string, "Error in line %d of configuration file %s: %s", linecounter, ConfigFname, errtext); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
        if (setvalue[k]!='\0') { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unexpected trailing matter in definition", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
        if (setnumeric.FlagComplex) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Multiplier in units definition cannot be complex", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
        for (j=0; j<UNITS_MAX_BASEUNITS; j++) ppl_unit_database[ppl_unit_pos].exponent[j] = setnumeric.exponent[j];
        ppl_unit_database[ppl_unit_pos].multiplier = setnumeric.real;
        ppl_unit_pos++;
       }
     }
    else if (state == 8) // [filters] section
     {
      ppl_units_zero(&setnumeric);
      setnumeric.string = setvalue;
      DictAppendValue(settings_filters,setkey,setnumeric);
     }
    else if (state == 9) // [script] section
     {
      settings_term_current  = settings_term_default; // Copy settings for directive_set()
      settings_graph_current = settings_graph_default;
      for (i=0; i<PALETTE_LENGTH; i++) settings_palette_current[i] = settings_palette_default[i];
      for (i=0; i<MAX_AXES; i++) { XAxes[i] = XAxesDefault[i]; YAxes[i] = YAxesDefault[i]; ZAxes[i] = ZAxesDefault[i]; }
      ppl_error_setstreaminfo(linecounter, "configuration file");
      scriptcmd = parse(linebuffer);
      if (scriptcmd != NULL)
       {
        DictLookup(scriptcmd,"directive",NULL,(void **)(&StringScan));
        if (strcmp(StringScan,"set_error")==0) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised set command.", linecounter, ConfigFname); ppl_error(ERR_PREFORMED, temp_err_string); continue; }
        if (strcmp(StringScan,"set")!=0) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Only set commands are allowed in scripts in configuration files.", linecounter, ConfigFname); ppl_error(ERR_PREFORMED, temp_err_string); continue; }
        DictLookup(scriptcmd,"editno",NULL,(void **)(&StringScan));
        if (StringScan != NULL) { sprintf(temp_err_string, "Error in line %d of configuration file %s: Item specifiers are not allowed in set commands in scripts in configuration files.", linecounter, ConfigFname); ppl_error(ERR_PREFORMED, temp_err_string); continue; }
        directive_set(scriptcmd);
       }
      settings_term_default  = settings_term_current; // Copy changed settings into defaults
      settings_graph_default = settings_graph_current;
      for (i=0; i<PALETTE_LENGTH; i++) settings_palette_default[i] = settings_palette_current[i];
      for (i=0; i<MAX_AXES; i++) { XAxesDefault[i] = XAxes[i]; YAxesDefault[i] = YAxes[i]; ZAxesDefault[i] = ZAxes[i]; }
     }
    else
     {
      sprintf(temp_err_string, "Error in line %d of configuration file %s: Settings need to be preceded by a block name such as [settings].", linecounter, ConfigFname);
      ppl_warning(ERR_PREFORMED, temp_err_string);
      break;
     }
   }
  fclose(infile);
  return;
 }

