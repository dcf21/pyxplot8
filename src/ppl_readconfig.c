// ppl_readconfig.c
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

#define _PPL_READCONFIG 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "asciidouble.h"
#include "ppl_colours.h"
#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

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
  char  linebuffer[LSTR_LENGTH], setkey[LSTR_LENGTH], setvalue[LSTR_LENGTH];
  FILE *infile;
  int   state=-1;
  int   linecounter=1;
  int   i;
  double fl;

  if (DEBUG) { sprintf(temp_err_string, "Scanning configuration file %s.", ConfigFname); ppl_log(temp_err_string); }

  if ((infile=fopen(ConfigFname,"r")) == NULL)
   {
    if (DEBUG) { ppl_log("File does not exist."); }
    return;
   }

  while ((!feof(infile)) && (!ferror(infile)))
   {
    file_readline(infile, linebuffer);
    StrStrip(linebuffer, linebuffer);
    if             (linebuffer[0] == '\0')                   continue;
    else if (strcmp(linebuffer, "[settings]" )==0) {state=1; continue;}
    else if (strcmp(linebuffer, "[terminal]" )==0) {state=2; continue;}
    else if (strcmp(linebuffer, "[colours]"  )==0) {state=3; continue;}
    else if (strcmp(linebuffer, "[latex]"    )==0) {state=4; continue;}
    else if (strcmp(linebuffer, "[variables]")==0) {state=5; continue;}
    else if (strcmp(linebuffer, "[functions]")==0) {state=6; continue;}

    _ReadConfig_FetchKey  (linebuffer, setkey  );
    _ReadConfig_FetchValue(linebuffer, setvalue);

    if (state == 1)
     {
      StrUpper(setkey, setkey);
      if      (strcmp(setkey, "ASPECT"       )==0)
        if ((fl=GetFloat(setvalue, &i) || 1) && (i==strlen(setvalue)))     settings_graph_default.aspect          = fl;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s:\nIllegal value for setting ASPECT."       , linecounter, ConfigFname); ppl_warning(temp_err_string); break; }
      else if (strcmp(setkey, "AUTOASPECT"   )==0)
        if ((i=FetchSettingByName(setvalue,SW_ONOFF_INT, SW_ONOFF_STR ))>0) settings_graph_default.AutoAspect    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s:\nIllegal value for setting AUTOASPECT."   , linecounter, ConfigFname); ppl_warning(temp_err_string); break; }
      else if (strcmp(setkey, "COLOUR"       )==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.AxesColour    = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s:\nIllegal value for setting COLOUR."       , linecounter, ConfigFname); ppl_warning(temp_err_string); break; }
      else if (strcmp(setkey, "GRIDMAJCOLOUR")==0)
        if ((i=FetchSettingByName(setvalue,SW_COLOUR_INT,SW_COLOUR_STR))>0) settings_graph_default.GridMajColour = i;
        else {sprintf(temp_err_string, "Error in line %d of configuration file %s:\nIllegal value for setting GRIDMAJCOLOUR.", linecounter, ConfigFname); ppl_warning(temp_err_string); break; }
      else
       { sprintf(temp_err_string, "Error in line %d of configuration file %s:\nUnrecognised setting name '%s'.", linecounter, ConfigFname, setkey); ppl_warning(temp_err_string); break; }
     }
    else if (state == 2)
     {
      StrUpper(setkey, setkey);
     }
    else if (state == 3)
     {
      StrUpper(setkey, setkey);
     }
    else if (state == 4)
     {
      StrUpper(setkey, setkey);
     }
    else if (state == 5)
     {
     }
    else if (state == 6)
     {
     }
    else
     {
      sprintf(temp_err_string, "Error in line %d of configuration file %s:\nSettings need to be preceded by a block name such as [settings].", linecounter, ConfigFname);
      ppl_warning(temp_err_string);
      break;
     }
    linecounter++;
   }
  fclose(infile);
  return;
 }

