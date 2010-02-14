// ppl_readconfig.c
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

#define _PPL_READCONFIG_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#include "StringTools/asciidouble.h"
#include "EPSMaker/eps_colours.h"
#include "EPSMaker/eps_settings.h"
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
    if             (linebuffer[0] == '\0')                    continue;
    else if (strcmp(linebuffer, "[settings]" )==0) {state= 1; continue;}
    else if (strcmp(linebuffer, "[terminal]" )==0) {state= 2; continue;}
    else if (strcmp(linebuffer, "[colours]"  )==0) {state= 3; continue;}
    else if (strcmp(linebuffer, "[latex]"    )==0) {state= 4; continue;}
    else if (strcmp(linebuffer, "[variables]")==0) {state= 5; continue;}
    else if (strcmp(linebuffer, "[functions]")==0) {state= 6; continue;}
    else if (strcmp(linebuffer, "[units]"    )==0) {state= 7; continue;}
    else if (strcmp(linebuffer, "[filters]"  )==0) {state= 8; continue;}
    else if (strcmp(linebuffer, "[script]"   )==0) {state= 9; continue;}
    else if (strcmp(linebuffer, "[styling]"  )==0) {state=10; continue;}

    _ReadConfig_FetchKey  (linebuffer, setkey  );
    _ReadConfig_FetchValue(linebuffer, setvalue);

    if (state == 1) // [settings] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "ASPECT"       )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.aspect        = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Aspect."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AUTOASPECT"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.AutoAspect    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting AutoAspect."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXESCOLOUR"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.AxesColour    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Colour."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXISUNITSTYLE")==0)
        if ((i=FetchSettingByName(setvalue,SW_AXISUNITSTY_INT,SW_AXISUNITSTY_STR))>0) settings_graph_default.AxisUnitStyle = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting AxisUnitStyle.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BACKUP"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .backup        = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Backup."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BAR"          )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.bar           = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Bar."          , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BINORIGIN"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_term_default.BinOrigin.real = fl;
                                                                              settings_term_default.BinOriginAuto  = 0;  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BinOrigin."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BINWIDTH"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_term_default.BinWidth.real  = fl;
                                                                              settings_term_default.BinWidthAuto   = (fl>0.0);  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BinWidth."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BOXFROM"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_graph_default.BoxFrom.real  = fl;
                                                                              settings_graph_default.BoxWidthAuto  = 0;  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BoxFrom."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BOXWIDTH"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_graph_default.BoxWidth.real = fl;
                                                                              settings_graph_default.BoxWidthAuto  = (fl>0.0);  }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting BoxWidth."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "CALENDARIN"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_CALENDAR_INT, SW_CALENDAR_STR ))>0) settings_term_default.CalendarIn = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting CalendarIn."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "CALENDAROUT"  )==0)
        if ((i=FetchSettingByName(setvalue,SW_CALENDAR_INT, SW_CALENDAR_STR ))>0) settings_term_default.CalendarOut= fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting CalendarOut."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .colour        = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Colour."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "DATASTYLE"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_STYLE_INT, SW_STYLE_STR ))>0) settings_graph_default.DataStyle.style = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting DataStyle."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "DISPLAY"      )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .display       = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Display."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "DPI"          )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_term_default .dpi           = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting DPI."          , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "FONTSIZE"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.FontSize      = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting FontSize."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "FUNCSTYLE"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_STYLE_INT, SW_STYLE_STR ))>0) settings_graph_default.FuncStyle.style = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting FuncStyle."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRID"         )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.grid          = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Grid."         , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDAXISX"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue) && (fl>=0) && (fl<=MAX_AXES)))
         {
          settings_graph_default.GridAxisX[1]       = 0;
          settings_graph_default.GridAxisX[(int)fl] = 1;
         }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GridAxisX."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDAXISY"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue) && (fl>=0) && (fl<=MAX_AXES)))               
         {               
          settings_graph_default.GridAxisY[1]       = 0;
          settings_graph_default.GridAxisY[(int)fl] = 1;
         }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GridAxisY."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDAXISZ"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue) && (fl>=0) && (fl<=MAX_AXES)))               
         {               
          settings_graph_default.GridAxisZ[1]       = 0;
          settings_graph_default.GridAxisZ[(int)fl] = 1;
         }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GridAxisZ."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDMAJCOLOUR")==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.GridMajColour = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GridMajColour.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "GRIDMINCOLOUR")==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.GridMinColour = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting GridMinColour.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEY"          )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.key           = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Key."          , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEYCOLUMNS"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.KeyColumns    = max((int)fl, 0);
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KeyColumns."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEYPOS"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_KEYPOS_INT,SW_KEYPOS_STR))>0) settings_graph_default.KeyPos        = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting KeyPos."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEY_XOFF"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.KeyXOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Key_XOff."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "KEY_YOFF"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.KeyYOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Key_YOff."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "LANDSCAPE"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .landscape     = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Landscape."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "LINEWIDTH"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.LineWidth     = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting LineWidth."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "MULTIPLOT"    )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .multiplot     = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting MultiPlot."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMCOMPLEX"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .ComplexNumbers= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NumComplex."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMDISPLAY"  )==0)
        if ((i=FetchSettingByName(setvalue,SW_DISPLAY_INT, SW_DISPLAY_STR ))>0) settings_term_default.NumDisplay = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NumDisplay."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMERR"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .ExplicitErrors= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NumErr."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "NUMSF"        )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_term_default .SignificantFigures = min(max((int)fl, 1), 30);
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting NumSF."        , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ORIGINX"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.OriginX.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting OriginX."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ORIGINY"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.OriginY.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting OriginY."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "OUTPUT"       )==0)
        strcpy(settings_term_default.output , setvalue);
      else if (strcmp(setkey, "PAPERHEIGHT" )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_term_default .PaperHeight.real  = fl/1000;
                                                                              PaperHeight = settings_term_default.PaperHeight.real * 1000;
                                                                              PaperWidth  = settings_term_default.PaperWidth .real * 1000;
                                                                              ppl_GetPaperName(settings_term_default.PaperName, &PaperHeight, &PaperWidth);
                                                                            }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PaperHeight." , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "PAPERNAME"   )==0)
        {
         ppl_PaperSizeByName(setvalue, &PaperHeight, &PaperWidth);
         if (PaperHeight <= 0) {sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised papersize specified for setting PaperName."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
         settings_term_default.PaperHeight.real = PaperHeight/1000;
         settings_term_default.PaperWidth.real  = PaperWidth/1000;
         ppl_GetPaperName(settings_term_default.PaperName, &PaperHeight, &PaperWidth);
        }
      else if (strcmp(setkey, "PAPERWIDTH"  )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               { settings_term_default .PaperWidth.real  = fl/1000;
                                                                              PaperHeight = settings_term_default.PaperHeight.real * 1000;
                                                                              PaperWidth  = settings_term_default.PaperWidth .real * 1000;
                                                                              ppl_GetPaperName(settings_term_default.PaperName, &PaperHeight, &PaperWidth);
                                                                            }
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PaperWidth."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "POINTLINEWIDTH")==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.PointLineWidth= fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PointLineWidth.",linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "POINTSIZE"    )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.PointSize     = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting PointSize."    , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "PROJECTION"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_PROJ_INT, SW_PROJ_STR ))>0)   settings_graph_default.projection    = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Projection."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "SAMPLES"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.samples       = max((int)fl, 2);
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Samples."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMANTIALIAS")==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermAntiAlias = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TermAntiAlias.", linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMENLARGE"  )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermEnlarge   = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TermEnlarge."  , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ENLARGE"      )==0) // ENLARGE, as opposed to TERMENLARGE is supported for back-compatibility with PyXPlot 0.7
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermEnlarge   = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Enlarge."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMINVERT"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermInvert = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TermInvert."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMTRANSPARENT")==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .TermTransparent= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TermTransparent.",linecounter,ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TERMTYPE"     )==0)
        if ((i=FetchSettingByName(setvalue,SW_TERMTYPE_INT,SW_TERMTYPE_STR))>0) settings_term_default.TermType  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TermType."     , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TEXTCOLOUR"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.TextColour    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TextColour."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TEXTHALIGN"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_HALIGN_INT,SW_HALIGN_STR))>0) settings_graph_default.TextHAlign    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TextHAlign."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TEXTVALIGN"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_VALIGN_INT,SW_VALIGN_STR))>0) settings_graph_default.TextVAlign    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TextVAlign."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TITLE"        )==0)
        strcpy(settings_graph_default.title  , setvalue);
      else if (strcmp(setkey, "TITLE_XOFF"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.TitleXOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Title_XOff."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TITLE_YOFF"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.TitleYOff.real  = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Title_YOff."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TRANGE_LOG"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_BOOL_INT,SW_BOOL_STR))>0)     settings_graph_default.Tlog            = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TRange_Log."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TRANGE_MIN"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.Tmin.real       = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TRange_Min."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "TRANGE_MAX"   )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.Tmax.real       = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting TRange_Max."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "UNITABBREV"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .UnitDisplayAbbrev= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting UnitAbbrev."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "UNITPREFIX"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_term_default .UnitDisplayPrefix= i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting UnitPrefix."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "UNITSCHEME"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_UNITSCH_INT,SW_UNITSCH_STR))>0) settings_term_default.UnitScheme = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting UnitScheme."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "WIDTH"        )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.width.real      = fl/100;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Width."        , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "VIEW_XY"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.XYview.real     = fl/180*M_PI;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting View_XY."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "VIEW_YZ"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue)))               settings_graph_default.YZview.real     = fl/180*M_PI;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting View_YZ."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
     }
    else if (state == 2) // [terminal] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "COLOUR"       )==0)
        if ((i=FetchSettingByName(setvalue, SW_ONOFF_INT,   SW_ONOFF_STR  ))>0) settings_session_default.colour      = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Colour."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR_ERR"   )==0)
        if ((i=FetchSettingByName(setvalue, SW_TERMCOL_INT, SW_TERMCOL_STR))>0) settings_session_default.colour_err  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Colour_Err."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR_REP"   )==0)
        if ((i=FetchSettingByName(setvalue, SW_TERMCOL_INT, SW_TERMCOL_STR))>0) settings_session_default.colour_rep  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Colour_Rep."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "COLOUR_WRN"   )==0)
        if ((i=FetchSettingByName(setvalue, SW_TERMCOL_INT, SW_TERMCOL_STR))>0) settings_session_default.colour_wrn  = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Colour_Wrn."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "SPLASH"       )==0)
        if ((i=FetchSettingByName(setvalue, SW_ONOFF_INT,   SW_ONOFF_STR  ))>0) settings_session_default.splash      = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Splash."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
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
              settings_palette_default [PalettePos  ] = ColourNumber;
              settings_paletteR_default[PalettePos  ] = 0;
              settings_paletteG_default[PalettePos  ] = 0;
              settings_paletteB_default[PalettePos++] = 0;
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
      for (i=0; i<PALETTE_LENGTH; i++)
       {
        settings_palette_current [i] = settings_palette_default [i];
        settings_paletteR_current[i] = settings_paletteR_default[i];
        settings_paletteG_current[i] = settings_paletteG_default[i];
        settings_paletteB_current[i] = settings_paletteB_default[i];
       }
      for (i=0; i<MAX_AXES; i++) { DestroyAxis( &(XAxes[i]) ); CopyAxis(&(XAxes[i]), &(XAxesDefault[i]));
                                   DestroyAxis( &(YAxes[i]) ); CopyAxis(&(YAxes[i]), &(YAxesDefault[i]));
                                   DestroyAxis( &(ZAxes[i]) ); CopyAxis(&(ZAxes[i]), &(ZAxesDefault[i]));
                                 }
      for (i=0; i<MAX_PLOTSTYLES; i++) { with_words_destroy(&(settings_plot_styles[i])); with_words_copy(&(settings_plot_styles[i]) , &(settings_plot_styles_default[i])); }
      arrow_list_destroy(&arrow_list);
      arrow_list_copy(&arrow_list, &arrow_list_default);
      label_list_destroy(&label_list);
      label_list_copy(&label_list, &label_list_default);
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
      for (i=0; i<PALETTE_LENGTH; i++)
       {
        settings_palette_default [i] = settings_palette_current [i];
        settings_paletteR_default[i] = settings_paletteR_current[i];
        settings_paletteG_default[i] = settings_paletteG_current[i];
        settings_paletteB_default[i] = settings_paletteB_current[i];
       }
      for (i=0; i<MAX_AXES; i++) { DestroyAxis( &(XAxesDefault[i]) ); CopyAxis(&(XAxesDefault[i]), &(XAxes[i]));
                                   DestroyAxis( &(YAxesDefault[i]) ); CopyAxis(&(YAxesDefault[i]), &(YAxes[i]));
                                   DestroyAxis( &(ZAxesDefault[i]) ); CopyAxis(&(ZAxesDefault[i]), &(ZAxes[i]));
                                 }
      for (i=0; i<MAX_PLOTSTYLES; i++) { with_words_destroy(&(settings_plot_styles_default[i])); with_words_copy(&(settings_plot_styles_default[i]) , &(settings_plot_styles[i])); }
      arrow_list_destroy(&arrow_list_default);
      arrow_list_copy(&arrow_list_default, &arrow_list);
      label_list_destroy(&label_list_default);
      label_list_copy(&label_list_default, &label_list);
     }
    else if (state == 10) // [styling] section
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "BASELINE_LINEWIDTH"  )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_DEFAULT_LINEWIDTH = fl * EPS_BASE_DEFAULT_LINEWIDTH;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Baseline_LineWidth."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "BASELINE_POINTSIZE"  )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_DEFAULT_PS        = fl * EPS_BASE_DEFAULT_PS;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Baseline_PointSize."   , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ARROW_HEADANGLE"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_ARROW_ANGLE       = fl * M_PI / 180;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Arrow_HeadAngle."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ARROW_HEADSIZE"      )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_ARROW_HEADSIZE    = fl * EPS_BASE_ARROW_HEADSIZE;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Arrow_HeadSize."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "ARROW_HEADBACKINDENT")==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_ARROW_CONSTRICT   = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Arrow_HeadBackIndent." , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXES_SEPARATION"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_AXES_SEPARATION   = fl * EPS_BASE_AXES_SEPARATION;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Axes_Separation."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXES_TEXTGAP"        )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_AXES_TEXTGAP      = fl * EPS_BASE_AXES_TEXTGAP;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Axes_TextGap."         , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXES_LINEWIDTH"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_AXES_LINEWIDTH    = fl * EPS_BASE_AXES_LINEWIDTH;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Axes_LineWidth."       , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXES_MAJTICKLEN"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_AXES_MAJTICKLEN   = fl * EPS_BASE_AXES_MAJTICKLEN;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Axes_MajTickLen."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else if (strcmp(setkey, "AXES_MINTICKLEN"     )==0)
        if ((fl=GetFloat(setvalue, &i), i==strlen(setvalue))) EPS_AXES_MINTICKLEN   = fl * EPS_BASE_AXES_MINTICKLEN;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s: Illegal value for setting Axes_MinTickLen."      , linecounter, ConfigFname); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s: Unrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(ERR_PREFORMED, temp_err_string); continue; }
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

