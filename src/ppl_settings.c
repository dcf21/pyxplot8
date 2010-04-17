// ppl_settings.c
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

#define _PPL_SETTINGS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "MathsTools/dcfmath.h"

#include "EPSMaker/eps_colours.h"

#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_papersize.h"
#include "ppl_passwd.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"

settings_terminal settings_term_default;
settings_terminal settings_term_current;

settings_graph    settings_graph_default;
settings_graph    settings_graph_current;

with_words        settings_plot_styles        [MAX_PLOTSTYLES];
with_words        settings_plot_styles_default[MAX_PLOTSTYLES];

settings_axis     settings_axis_default;

settings_axis     XAxes [MAX_AXES], XAxesDefault[MAX_AXES];
settings_axis     YAxes [MAX_AXES], YAxesDefault[MAX_AXES];
settings_axis     ZAxes [MAX_AXES], ZAxesDefault[MAX_AXES];

settings_session  settings_session_default;

int               settings_palette_default [PALETTE_LENGTH] = {COLOUR_BLACK, COLOUR_RED, COLOUR_BLUE, COLOUR_MAGENTA, COLOUR_CYAN, COLOUR_BROWN, COLOUR_SALMON, COLOUR_GRAY, COLOUR_GREEN, COLOUR_NAVYBLUE, COLOUR_PERIWINKLE, COLOUR_PINEGREEN, COLOUR_SEAGREEN, COLOUR_GREENYELLOW, COLOUR_ORANGE, COLOUR_CARNATIONPINK, COLOUR_PLUM, -1};
int               settings_paletteR_default[PALETTE_LENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int               settings_paletteG_default[PALETTE_LENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int               settings_paletteB_default[PALETTE_LENGTH] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int               settings_palette_current [PALETTE_LENGTH];
int               settings_paletteR_current[PALETTE_LENGTH];
int               settings_paletteG_current[PALETTE_LENGTH];
int               settings_paletteB_current[PALETTE_LENGTH];

Dict             *settings_filters;

arrow_object     *arrow_list, *arrow_list_default;
label_object     *label_list, *label_list_default;

#include "ppl_units_fns.h"

void ppl_settings_makedefault()
 {
  FILE  *LocalePipe;
  int    Nchars,i;
  double PaperWidth, PaperHeight;
  value  tempval;
  char   ConfigFname[FNAME_LENGTH];
  char  *PaperSizePtr;

  // Default Terminal Settings, used when these values are not changed by any configuration files
  settings_term_default.backup              = SW_ONOFF_OFF;
  ppl_units_zero(&(settings_term_default.BinOrigin));
  settings_term_default.BinOriginAuto       = 1;
  ppl_units_zero(&(settings_term_default.BinWidth));
  settings_term_default.BinWidth.real       = 1.0;
  settings_term_default.BinWidthAuto        = 1;
  settings_term_default.CalendarIn          = SW_CALENDAR_BRITISH;
  settings_term_default.CalendarOut         = SW_CALENDAR_BRITISH;
  settings_term_default.colour              = SW_ONOFF_ON;
  settings_term_default.ComplexNumbers      = SW_ONOFF_OFF;
  settings_term_default.display             = SW_ONOFF_ON;
  settings_term_default.dpi                 = 300.0;
  settings_term_default.ExplicitErrors      = SW_ONOFF_ON;
  settings_term_default.landscape           = SW_ONOFF_OFF;
  strcpy(settings_term_default.LatexPreamble, "");
  settings_term_default.multiplot           = SW_ONOFF_OFF;
  settings_term_default.NumDisplay          = SW_DISPLAY_N;
  strcpy(settings_term_default.output, "");
  ppl_units_zero(&(settings_term_default.PaperHeight));
  settings_term_default.PaperHeight.real    = 297.30178 / 1000;
  settings_term_default.PaperHeight.dimensionless = 0; settings_term_default.PaperHeight.exponent[UNIT_LENGTH] = 1;
  strcpy(settings_term_default.PaperName, "a4");
  ppl_units_zero(&(settings_term_default.PaperWidth));
  settings_term_default.PaperWidth.real     = 210.2241 / 1000;
  settings_term_default.PaperWidth.dimensionless = 0; settings_term_default.PaperWidth.exponent[UNIT_LENGTH] = 1;
  settings_term_default.SignificantFigures  = 8;
  settings_term_default.TermAntiAlias       = SW_ONOFF_ON;
  settings_term_default.TermType            = SW_TERMTYPE_X11S;
  settings_term_default.TermEnlarge         = SW_ONOFF_OFF;
  settings_term_default.TermInvert          = SW_ONOFF_OFF;
  settings_term_default.TermTransparent     = SW_ONOFF_OFF;
  settings_term_default.UnitScheme          = SW_UNITSCH_SI;
  settings_term_default.UnitDisplayPrefix   = SW_ONOFF_ON;
  settings_term_default.UnitDisplayAbbrev   = SW_ONOFF_ON;
  settings_term_default.viewer              = SW_VIEWER_GV;

  // Default Graph Settings, used when these values are not changed by any configuration files
  settings_graph_default.aspect                = 1.0;
  settings_graph_default.AutoAspect            = SW_ONOFF_ON;
  settings_graph_default.AxesColour            = COLOUR_BLACK;
  settings_graph_default.AxesColourR           = 0;
  settings_graph_default.AxesColourG           = 0;
  settings_graph_default.AxesColourB           = 0;
  settings_graph_default.AxisUnitStyle         = SW_AXISUNITSTY_RATIO;
  settings_graph_default.bar                   = 1.0;
  ppl_units_zero(&(settings_graph_default.BoxFrom));
  settings_graph_default.BoxFromAuto           = 1;
  ppl_units_zero(&(settings_graph_default.BoxWidth));
  settings_graph_default.BoxWidthAuto          = 1;
  settings_graph_default.clip                  = SW_ONOFF_OFF;
  with_words_zero(&(settings_graph_default.DataStyle),1);
  settings_graph_default.DataStyle.linespoints = SW_STYLE_POINTS;
  settings_graph_default.FontSize              = 1.0;
  with_words_zero(&(settings_graph_default.FuncStyle),1);
  settings_graph_default.FuncStyle.linespoints = SW_STYLE_LINES;
  settings_graph_default.grid                  = SW_ONOFF_OFF;
  for (i=0; i<MAX_AXES; i++) settings_graph_default.GridAxisX[i] = 0;
  for (i=0; i<MAX_AXES; i++) settings_graph_default.GridAxisY[i] = 0;
  for (i=0; i<MAX_AXES; i++) settings_graph_default.GridAxisZ[i] = 0;
  settings_graph_default.GridAxisX[1]  = 1;
  settings_graph_default.GridAxisY[1]  = 1;
  settings_graph_default.GridAxisZ[1]  = 1;
  settings_graph_default.GridMajColour = COLOUR_GREY60;
  settings_graph_default.GridMajColourR= 0;
  settings_graph_default.GridMajColourG= 0;
  settings_graph_default.GridMajColourB= 0;
  settings_graph_default.GridMinColour = COLOUR_GREY85;
  settings_graph_default.GridMinColourR= 0;
  settings_graph_default.GridMinColourG= 0;
  settings_graph_default.GridMinColourB= 0;
  settings_graph_default.key           = SW_ONOFF_ON;
  settings_graph_default.KeyColumns    = 0;
  settings_graph_default.KeyPos        = SW_KEYPOS_TR;
  ppl_units_zero(&(settings_graph_default.KeyXOff));
  settings_graph_default.KeyXOff.real  = 0.0;
  settings_graph_default.KeyXOff.dimensionless = 0; settings_graph_default.KeyXOff.exponent[UNIT_LENGTH] = 1;
  ppl_units_zero(&(settings_graph_default.KeyYOff));
  settings_graph_default.KeyYOff.real  = 0.0;
  settings_graph_default.KeyYOff.dimensionless = 0; settings_graph_default.KeyYOff.exponent[UNIT_LENGTH] = 1;
  settings_graph_default.LineWidth     = 1.0;
  ppl_units_zero(&(settings_graph_default.OriginX));
  settings_graph_default.OriginX.real  = 0.0;
  settings_graph_default.OriginX.dimensionless = 0; settings_graph_default.OriginX.exponent[UNIT_LENGTH] = 1;
  ppl_units_zero(&(settings_graph_default.OriginY));
  settings_graph_default.OriginY.real  = 0.0;
  settings_graph_default.OriginY.dimensionless = 0; settings_graph_default.OriginY.exponent[UNIT_LENGTH] = 1;
  settings_graph_default.PointSize     = 1.0;
  settings_graph_default.PointLineWidth= 1.0;
  settings_graph_default.projection    = SW_PROJ_FLAT;
  settings_graph_default.samples       = 250;
  settings_graph_default.TextColour    = COLOUR_BLACK;
  settings_graph_default.TextColourR   = 0;
  settings_graph_default.TextColourG   = 0;
  settings_graph_default.TextColourB   = 0;
  settings_graph_default.TextHAlign    = SW_HALIGN_LEFT;
  settings_graph_default.TextVAlign = SW_VALIGN_BOT;
  strcpy(settings_graph_default.title, "");
  ppl_units_zero(&(settings_graph_default.TitleXOff));
  settings_graph_default.TitleXOff.real= 0.0;
  settings_graph_default.TitleXOff.dimensionless = 0; settings_graph_default.TitleXOff.exponent[UNIT_LENGTH] = 1;
  ppl_units_zero(&(settings_graph_default.TitleYOff));
  settings_graph_default.TitleYOff.real= 0.0;
  settings_graph_default.TitleYOff.dimensionless = 0; settings_graph_default.TitleYOff.exponent[UNIT_LENGTH] = 1;
  settings_graph_default.Tlog          = SW_BOOL_FALSE;
  ppl_units_zero(&(settings_graph_default.Tmin));
  ppl_units_zero(&(settings_graph_default.Tmax));
  settings_graph_default.Tmin.real     = 0.0;
  settings_graph_default.Tmax.real     = 1.0;
  ppl_units_zero(&(settings_graph_default.width));
  settings_graph_default.width.real    = 0.08; // 8cm
  settings_graph_default.width.dimensionless = 0; settings_graph_default.width.exponent[UNIT_LENGTH] = 1;
  ppl_units_zero(&(settings_graph_default.XYview));
  settings_graph_default.XYview.real   = 60.0 * M_PI / 180; // 60 degrees
  settings_graph_default.XYview.dimensionless = 0; settings_graph_default.XYview.exponent[UNIT_ANGLE] = 1;
  ppl_units_zero(&(settings_graph_default.YZview));
  settings_graph_default.YZview.real   = 30.0 * M_PI / 180; // 30 degrees
  settings_graph_default.YZview.dimensionless = 0; settings_graph_default.YZview.exponent[UNIT_ANGLE] = 1;

  // Default Axis Settings, used whenever a new axis is created
  settings_axis_default.atzero      = 0;
  settings_axis_default.enabled     = 0;
  settings_axis_default.invisible   = 0;
  settings_axis_default.linked      = 0;
  settings_axis_default.RangeReversed=0;
  settings_axis_default.topbottom   = 0;
  settings_axis_default.ArrowType   = SW_AXISDISP_NOARR;
  settings_axis_default.LinkedAxisCanvasID = 0;
  settings_axis_default.LinkedAxisToXYZ    = 0;
  settings_axis_default.LinkedAxisToNum    = 0;
  settings_axis_default.log         = SW_BOOL_FALSE;
  settings_axis_default.MaxSet      = SW_BOOL_FALSE;
  settings_axis_default.MinSet      = SW_BOOL_FALSE;
  settings_axis_default.MirrorType  = SW_AXISMIRROR_AUTO;
  settings_axis_default.MTickDir    = SW_TICDIR_IN;
  settings_axis_default.MTickMaxSet = 0;
  settings_axis_default.MTickMinSet = 0;
  settings_axis_default.MTickStepSet= 0;
  settings_axis_default.TickDir     = SW_TICDIR_IN;
  settings_axis_default.TickLabelRotation  = SW_TICLABDIR_HORI;
  settings_axis_default.TickMaxSet  = 0;
  settings_axis_default.TickMinSet  = 0;
  settings_axis_default.TickStepSet = 0;
  settings_axis_default.LabelRotate =  0.0;
  settings_axis_default.LogBase     = 10.0;
  settings_axis_default.max         =  0.0;
  settings_axis_default.min         =  0.0;
  settings_axis_default.MTickMax    =  0.0;
  settings_axis_default.MTickMin    =  0.0;
  settings_axis_default.MTickStep   =  0.0;
  settings_axis_default.TickLabelRotate = 0.0;
  settings_axis_default.TickMax     =  0.0;
  settings_axis_default.TickMin     =  0.0;
  settings_axis_default.TickStep    =  0.0;
  settings_axis_default.format      = NULL;
  settings_axis_default.label       = NULL;
  settings_axis_default.linkusing   = NULL;
  settings_axis_default.MTickList   = NULL;
  settings_axis_default.TickList    = NULL;
  settings_axis_default.MTickStrs   = NULL;
  settings_axis_default.TickStrs    = NULL;
  ppl_units_zero(&(settings_axis_default.unit));

  // Set up list of input filters
  settings_filters = DictInit();
  ppl_units_zero(&tempval);
  #ifdef HAVE_FITSIO
  tempval.string = FITSHELPER;
  DictAppendValue(settings_filters, "*.fits", tempval);
  #endif
  #ifdef TIMEHELPER
  tempval.string = TIMEHELPER;
  DictAppendValue(settings_filters, "*.log", tempval);
  #endif
  #ifdef GUNZIP_COMMAND
  tempval.string = GUNZIP_COMMAND;
  DictAppendValue(settings_filters, "*.gz", tempval);
  #endif
  #ifdef WGET_COMMAND
  tempval.string = WGET_COMMAND;
  DictAppendValue(settings_filters, "http://*", tempval);
  DictAppendValue(settings_filters, "ftp://*", tempval);
  #endif

  // Set up empty lists of arrows and labels
  arrow_list = arrow_list_default = NULL;
  label_list = label_list_default = NULL;

  // Set up array of plot styles
  for (i=0; i<MAX_PLOTSTYLES; i++) with_words_zero(&(settings_plot_styles        [i]),1);
  for (i=0; i<MAX_PLOTSTYLES; i++) with_words_zero(&(settings_plot_styles_default[i]),1);

  // Set up current axes. Because default axis contains no malloced strings, we don't need to use AxisCopy() here.
  for (i=0; i<MAX_AXES; i++) XAxes[i] = YAxes[i] = ZAxes[i] = settings_axis_default;
  XAxes[1].enabled   = YAxes[1].enabled   = ZAxes[1].enabled   = 1; // By default, only axes 1 are enabled
  XAxes[2].topbottom = YAxes[2].topbottom = ZAxes[2].topbottom = 1; // By default, axes 2 are opposite axes 1
  for (i=0; i<MAX_AXES; i++) { XAxesDefault[i] = XAxes[i]; YAxesDefault[i] = YAxes[i]; ZAxesDefault[i] = ZAxes[i]; }

  // Setting which affect how we talk to the current interactive session
  settings_session_default.splash    = SW_ONOFF_ON;
  settings_session_default.colour    = SW_ONOFF_ON;
  settings_session_default.colour_rep= SW_TERMCOL_GRN;
  settings_session_default.colour_wrn= SW_TERMCOL_BRN;
  settings_session_default.colour_err= SW_TERMCOL_RED;
  strcpy(settings_session_default.homedir, UnixGetHomeDir());

  // Estimate the machine precision of the floating point unit we are using
  machine_epsilon = 1.0; // Variable defined in dcfmath.c
  do { machine_epsilon /= 2.0; } while ((1.0 + (machine_epsilon/2.0)) != 1.0);

  // Try and find out the default papersize from the locale command
  // Do this using the popen() command rather than direct calls to nl_langinfo(_NL_PAPER_WIDTH), because the latter is gnu-specific
  if (DEBUG) ppl_log("Querying papersize from the locale command.");
  if ((LocalePipe = popen("locale -c LC_PAPER 2> /dev/null","r"))==NULL)
   {
    if (DEBUG) ppl_log("Failed to open a pipe to the locale command.");
   } else {
    file_readline(LocalePipe, ConfigFname, FNAME_LENGTH); // Should read LC_PAPER
    file_readline(LocalePipe, ConfigFname, FNAME_LENGTH); // Should quote the default paper width
    PaperHeight = GetFloat(ConfigFname, &Nchars);
    if (Nchars != strlen(ConfigFname)) goto LC_PAPERSIZE_DONE;
    file_readline(LocalePipe, ConfigFname, FNAME_LENGTH); // Should quote the default paper height
    PaperWidth  = GetFloat(ConfigFname, &Nchars);
    if (Nchars != strlen(ConfigFname)) goto LC_PAPERSIZE_DONE;
    if (DEBUG) { sprintf(temp_err_string, "Read papersize %f x %f", PaperWidth, PaperHeight); ppl_log(temp_err_string); }
    settings_term_default.PaperHeight.real   = PaperHeight/1000;
    settings_term_default.PaperWidth.real    = PaperWidth /1000;
    if (0) { LC_PAPERSIZE_DONE: if (DEBUG) ppl_log("Failed to read papersize from the locale command."); }
    pclose(LocalePipe);
    ppl_GetPaperName(settings_term_default.PaperName, &PaperHeight, &PaperWidth);
   }

  // Try and find out the default papersize from /etc/papersize
  if (DEBUG) ppl_log("Querying papersize from /etc/papersize.");
  if ((LocalePipe = fopen("/etc/papersize","r"))==NULL)
   {
    if (DEBUG) ppl_log("Failed to open /etc/papersize.");
   } else {
    file_readline(LocalePipe, ConfigFname, FNAME_LENGTH); // Should a papersize name
    ppl_PaperSizeByName(ConfigFname, &PaperHeight, &PaperWidth);
    if (PaperHeight > 0)
     {
      if (DEBUG) { sprintf(temp_err_string, "Read papersize %s, with dimensions %f x %f", ConfigFname, PaperWidth, PaperHeight); ppl_log(temp_err_string); }
      settings_term_default.PaperHeight.real   = PaperHeight/1000;
      settings_term_default.PaperWidth.real    = PaperWidth /1000;
      ppl_GetPaperName(settings_term_default.PaperName, &PaperHeight, &PaperWidth);
     } else {
      if (DEBUG) ppl_log("/etc/papersize returned an unrecognised papersize.");
     }
    fclose(LocalePipe);
   }

  // Try and find out the default papersize from PAPERSIZE environment variable
  if (DEBUG) ppl_log("Querying papersize from $PAPERSIZE");
  PaperSizePtr = getenv("PAPERSIZE");
  if (PaperSizePtr == NULL)
   {
    if (DEBUG) ppl_log("Environment variable $PAPERSIZE not set.");
   } else {
    ppl_PaperSizeByName(PaperSizePtr, &PaperHeight, &PaperWidth);
    if (PaperHeight > 0)
     {
      if (DEBUG) { sprintf(temp_err_string, "Read papersize %s, with dimensions %f x %f", PaperSizePtr, PaperWidth, PaperHeight); ppl_log(temp_err_string); }
      settings_term_default.PaperHeight.real   = PaperHeight/1000;
      settings_term_default.PaperWidth.real    = PaperWidth /1000;
      ppl_GetPaperName(settings_term_default.PaperName, &PaperHeight, &PaperWidth);
     } else {
      if (DEBUG) ppl_log("$PAPERSIZE returned an unrecognised paper size."); 
     }
   }

  // Copy Default Settings to Current Settings
  settings_term_current  = settings_term_default;
  settings_graph_current = settings_graph_default;
  for (i=0; i<PALETTE_LENGTH; i++)
   { 
    settings_palette_current [i] = settings_palette_default [i];
    settings_paletteR_current[i] = settings_paletteR_default[i];
    settings_paletteG_current[i] = settings_paletteG_default[i];
    settings_paletteB_current[i] = settings_paletteB_default[i];
   }
  return;
 }

void ppl_settings_readconfig()
 {
  int    i;
  char   ConfigFname[FNAME_LENGTH];

  sprintf(ConfigFname, "%s%s%s", settings_session_default.homedir, PATHLINK, ".pyxplotrc");
  ReadConfigFile(ConfigFname);
  sprintf(ConfigFname, "%s", ".pyxplotrc");
  ReadConfigFile(ConfigFname);

  // Copy Default Settings to Current Settings
  settings_term_current  = settings_term_default;
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

  // Copy List of Preferred Units
  {
   int temp;
   ListIterator *listiter;
   listiter = ListIterateInit(ppl_unit_PreferredUnits);
   while (listiter != NULL)
    {
     ((PreferredUnit *)listiter->data)->modified = 0;
     listiter = ListIterate(listiter, NULL);
    }
   temp = lt_GetMemContext();
   _lt_SetMemContext(0);
   ppl_unit_PreferredUnits_default = ListCopy(ppl_unit_PreferredUnits, 1);
   _lt_SetMemContext(temp);
  }

  return;
 }

// ----------------------------------------------
// ROUTINES FOR READING COLOURS FROM DICTIONARIES
// ----------------------------------------------

#include "ppl_userspace.h"
#include "ppl_units.h"
#define COLMALLOC(X) (tmp = malloc(X)); if (tmp==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *outcolRS=*outcolGS=*outcolBS=NULL; return 1; }

int colour_fromdict(Dict *in, char *prefix, int *outcol, int *outcolR, int *outcolG, int *outcolB, char **outcolRS, char **outcolGS, char **outcolBS,
                    unsigned char *USEcol, unsigned char *USEcolRGB, int *errpos, unsigned char malloced)
 {
  char *tempstr, *tempstr2, DictName[32];
  int   cindex, i, j, palette_index;
  void *tmp;
  value valobj, *tempval;

  sprintf(DictName, "%scolour", prefix);
  DictLookup(in,DictName,NULL,(void **)&tempstr);
  sprintf(DictName, "%scolourR", prefix);
  DictLookup(in,DictName,NULL,(void **)&tempval);
  sprintf(DictName, "%scolourRexpr", prefix);
  DictLookup(in,DictName,NULL,(void **)&tempstr2);
  if (tempstr != NULL) // Colour is specified by name or by palette index
   {
    StrStrip(tempstr,tempstr);
    i = FetchSettingByName(tempstr, SW_COLOUR_INT, SW_COLOUR_STR);
    if (i >= 0)
     {
      cindex = i;
      *outcolR = *outcolG = *outcolB = 0;
     }
    else
     {
      j = strlen(tempstr);
      *errpos = -1;
      ppl_EvaluateAlgebra(tempstr, &valobj, 0, &j, 0, errpos, temp_err_string, 0);
      if (*errpos>=0) { ppl_error(ERR_GENERAL, temp_err_string); return 1; }
      if (!valobj.dimensionless) { sprintf(temp_err_string, "Colour indices should be dimensionless quantities; the specified quantity has units of <%s>.", ppl_units_GetUnitStr(&valobj, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, temp_err_string); return 1; }
      if ((valobj.real <= INT_MIN+5) || (valobj.real >= INT_MAX-5)) { sprintf(temp_err_string, "Colour indices should be in the range %d to %d.", INT_MIN, INT_MAX); ppl_error(ERR_GENERAL, temp_err_string); return 1; }
      for (j=1; j<PALETTE_LENGTH; j++) if (settings_palette_current[j]==-1) break;
      palette_index = ((int)valobj.real-1)%j;
      while (palette_index < 0) palette_index+=j;
      cindex   = settings_palette_current [palette_index];
      *outcolR = settings_paletteR_current[palette_index];
      *outcolG = settings_paletteG_current[palette_index];
      *outcolB = settings_paletteB_current[palette_index];
     }
    *outcol  = cindex;
    if (outcolRS !=NULL)
     {
      if (malloced && (*outcolRS!=NULL)) free(*outcolRS);
      if (malloced && (*outcolGS!=NULL)) free(*outcolGS);
      if (malloced && (*outcolBS!=NULL)) free(*outcolBS);
      *outcolRS=*outcolGS=*outcolBS=NULL;
     }
    if (USEcol   !=NULL) *USEcol    = (cindex> 0);
    if (USEcolRGB!=NULL) *USEcolRGB = (cindex==0);
   } else if (tempval != NULL) { // Colour is specified by RGB components

#define CHECK_REAL_DIMLESS \
    if (!tempval->dimensionless) { sprintf(temp_err_string, "Colour RGB components should be dimensionless quantities; the specified quantity has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, temp_err_string); return 1; }\
    if (tempval->imag>1e-6) { sprintf(temp_err_string, "Colour RGB components should be real numbers; the specified quantity is complex."); ppl_error(ERR_GENERAL, temp_err_string); return 1; }\

    CHECK_REAL_DIMLESS;
    *outcol  = 0;
    *outcolR = (tempval->real <= 0) ? 0 : ((tempval->real >= 255) ? 255 : tempval->real); // Make sure that colour component is in the range 0-255
    sprintf(DictName, "%scolourG", prefix);
    DictLookup(in,DictName,NULL,(void **)&tempval);
    CHECK_REAL_DIMLESS;
    *outcolG = (tempval->real <= 0) ? 0 : ((tempval->real >= 255) ? 255 : tempval->real); // Make sure that colour component is in the range 0-255
    sprintf(DictName, "%scolourB", prefix);
    DictLookup(in,DictName,NULL,(void **)&tempval);
    CHECK_REAL_DIMLESS;
    *outcolB = (tempval->real <= 0) ? 0 : ((tempval->real >= 255) ? 255 : tempval->real); // Make sure that colour component is in the range 0-255
    if (USEcol   !=NULL) *USEcol    = 0;
    if (USEcolRGB!=NULL) *USEcolRGB = 1;
    if (outcolRS !=NULL)
     {
      if (malloced && (*outcolRS!=NULL)) free(*outcolRS);
      if (malloced && (*outcolGS!=NULL)) free(*outcolGS);
      if (malloced && (*outcolBS!=NULL)) free(*outcolBS);
      *outcolRS=*outcolGS=*outcolBS=NULL;
     }
   } else if (tempstr2 != NULL) { // Colour is specified by RGB expressions
    if (USEcol   !=NULL) *USEcol    = 0;
    if (USEcolRGB!=NULL) *USEcolRGB = 0;
    if (outcolRS ==NULL) { ppl_error(ERR_INTERNAL, "Received RGB colour expressions, but have not received strings to put them into"); return 1; }
    if (malloced)
     {
      if (*outcolRS!=NULL) free(*outcolRS);
      if (*outcolGS!=NULL) free(*outcolGS);
      if (*outcolBS!=NULL) free(*outcolBS);
      *outcolRS = (char *)COLMALLOC(strlen(tempstr2)+1); strcpy(*outcolRS , tempstr2);
      sprintf(DictName, "%scolourGexpr", prefix);
      DictLookup(in,DictName,NULL,(void **)&tempstr2);
      *outcolGS = (char *)COLMALLOC(strlen(tempstr2)+1); strcpy(*outcolGS , tempstr2);
      sprintf(DictName, "%scolourBexpr", prefix);
      DictLookup(in,DictName,NULL,(void **)&tempstr2);
      *outcolBS = (char *)COLMALLOC(strlen(tempstr2)+1); strcpy(*outcolBS , tempstr2);
     }
    else
     {
      *outcolRS = tempstr2;
      sprintf(DictName, "%scolourGexpr", prefix);
      DictLookup(in,DictName,NULL,(void **)&tempstr2);
      *outcolGS = tempstr2;
      sprintf(DictName, "%scolourBexpr", prefix);
      DictLookup(in,DictName,NULL,(void **)&tempstr2);
      *outcolBS = tempstr2;
     }
   }
  return 0;
 }

// -------------------------------------------
// ROUTINES FOR MANIPULATING ARROWS STRUCTURES
// -------------------------------------------

#define arrow_add_get_system(X,Y) \
 { \
  DictLookup(in,X "_system",NULL,(void **)&tempstr); \
  if (tempstr == NULL) Y = SW_SYSTEM_FIRST; \
  else                 Y = FetchSettingByName(tempstr, SW_SYSTEM_INT, SW_SYSTEM_STR); \
 }

#define arrow_add_get_axis(X,Y) \
 { \
  DictLookup(in,X "_axis",NULL,(void **)&tempint); \
  if   (tempint == NULL) Y = 0; \
  else                   Y = *tempint; \
 }

#define arrow_add_check_axis(X) \
 { \
  DictLookup(in,X "_axis",NULL,(void **)&tempint); \
  if (tempint != NULL) \
   { \
    if ((*tempint<0)||(*tempint>MAX_AXES)) \
     { \
      sprintf(temp_err_string, "Axis number %d is out of range; axis numbers must be in the range 0 - %d", *tempint, MAX_AXES-1); \
      ppl_error(ERR_GENERAL,temp_err_string); \
      return; \
     } \
   } \
 }

#define arrow_add_check_dimensions(X,Y) \
 { \
  DictLookup(in,X,NULL,(void **)&tempval); \
  if (tempval == NULL) { tempval = &tempvalobj; ppl_units_zero(tempval); } \
  if ((Y == SW_SYSTEM_GRAPH) || (Y == SW_SYSTEM_PAGE)) \
   if (!tempval->dimensionless) \
    for (i=0; i<UNITS_MAX_BASEUNITS; i++) if (tempval->exponent[i] != (i==UNIT_LENGTH)) \
     { \
      sprintf(temp_err_string, "Coordinates specified in the graph and page systems must have dimensions of length. Received coordinate with dimensions of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 0, 1, 0)); \
      ppl_error(ERR_GENERAL, temp_err_string); return; \
     } \
 }

#define arrow_add_copy_coordinate(X,Y,Z) \
 { \
  DictLookup(in,X,NULL,(void **)&tempval); \
  if (tempval == NULL) { tempval = &tempvalobj; ppl_units_zero(tempval); } \
  if ((Y == SW_SYSTEM_GRAPH) || (Y == SW_SYSTEM_PAGE)) \
   if (tempval->dimensionless) { tempval->dimensionless=0; tempval->exponent[UNIT_LENGTH]=1; tempval->real /= 100; } \
  Z = *tempval; \
 }

void arrow_add(arrow_object **list, Dict *in)
 {
  int   *tempint, i, system_x0, system_y0, system_z0, system_x1, system_y1, system_z1;
  char  *tempstr;
  value *tempval, tempvalobj;
  with_words ww_temp1;
  arrow_object *out;

  arrow_add_get_system("x0",system_x0); arrow_add_get_system("y0",system_y0); arrow_add_get_system("z0",system_z0);
  arrow_add_get_system("x1",system_x1); arrow_add_get_system("y1",system_y1); arrow_add_get_system("z1",system_z1);

  arrow_add_check_dimensions("x0",system_x0); arrow_add_check_dimensions("y0",system_y0); arrow_add_check_dimensions("z0",system_z0);
  arrow_add_check_dimensions("x1",system_x1); arrow_add_check_dimensions("y1",system_y1); arrow_add_check_dimensions("z1",system_z1);

  arrow_add_check_axis("x0"); arrow_add_check_axis("y0"); arrow_add_check_axis("z0");
  arrow_add_check_axis("x1"); arrow_add_check_axis("y1"); arrow_add_check_axis("z1");

  DictLookup(in,"arrow_id",NULL,(void **)&tempint);
  while ((*list != NULL) && ((*list)->id < *tempint)) list = &((*list)->next);
  if ((*list != NULL) && ((*list)->id == *tempint))
   {
    out = *list;
    with_words_destroy(&out->style);
   } else {
    out = (arrow_object *)malloc(sizeof(arrow_object));
    if (out == NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return; }
    out->id   = *tempint;
    out->next = *list;
    *list     = out;
   }

  // Check whether arrow head type has been specified
  DictLookup(in,"arrow_style",NULL,(void **)&tempstr);
  if (tempstr==NULL) out->arrow_style = SW_ARROWTYPE_HEAD;
  else               out->arrow_style = FetchSettingByName(tempstr, SW_ARROWTYPE_INT, SW_ARROWTYPE_STR);

  // Check what style keywords have been specified in the 'with' clause
  with_words_fromdict(in, &ww_temp1, 1);
  with_words_copy(&out->style, &ww_temp1);

  out->system_x0 = system_x0; out->system_y0 = system_y0; out->system_z0 = system_z0;
  out->system_x1 = system_x1; out->system_y1 = system_y1; out->system_z1 = system_z1;

  arrow_add_get_axis("x0", out->axis_x0); arrow_add_get_axis("y0", out->axis_y0); arrow_add_get_axis("z0", out->axis_z0);
  arrow_add_get_axis("x1", out->axis_x1); arrow_add_get_axis("y1", out->axis_y1); arrow_add_get_axis("z1", out->axis_z1);

  arrow_add_copy_coordinate("x0",system_x0,out->x0); arrow_add_copy_coordinate("y0",system_y0,out->y0); arrow_add_copy_coordinate("z0",system_z0,out->z0);
  arrow_add_copy_coordinate("x1",system_x1,out->x1); arrow_add_copy_coordinate("y1",system_y1,out->y1); arrow_add_copy_coordinate("z1",system_z1,out->z1);

  return;
 }

void arrow_remove(arrow_object **list, Dict *in)
 {
  int          *tempint;
  arrow_object *obj, **first;
  List         *templist;
  Dict         *tempdict;
  ListIterator *listiter;

  first = list;
  DictLookup(in,"arrow_list,",NULL,(void **)&templist);
  listiter = ListIterateInit(templist);
  if (listiter == NULL) arrow_list_destroy(list); // set noarrow with no number specified means all arrows deleted
  while (listiter != NULL)
   {
    tempdict = (Dict *)listiter->data;
    DictLookup(tempdict,"arrow_id",NULL,(void **)&tempint);
    list = first;
    while ((*list != NULL) && ((*list)->id < *tempint)) list = &((*list)->next);
    if ((*list != NULL) && ((*list)->id == *tempint))
     {
      obj   = *list;
      *list = (*list)->next;
      with_words_destroy(&obj->style);
      free(obj);
     } else {
      //sprintf(temp_err_string,"Arrow number %d is not defined", *tempint);
      //ppl_error(ERR_GENERAL, temp_err_string);
     }
    listiter = ListIterate(listiter, NULL);
   }
  return;
 }

void arrow_unset(arrow_object **list, Dict *in)
 {
  int          *tempint;
  arrow_object *obj, *new, **first;
  List         *templist;
  Dict         *tempdict;
  ListIterator *listiter;

  arrow_remove(list, in); // First of all, remove any arrows which we are unsetting
  first = list;
  DictLookup(in,"arrow_list,",NULL,(void **)&templist);
  listiter = ListIterateInit(templist);

  if (listiter == NULL) arrow_list_copy(list, &arrow_list_default); // Check to see whether we are unsetting ALL arrows, and if so, use the copy command
  while (listiter != NULL)
   {
    tempdict = (Dict *)listiter->data;
    DictLookup(tempdict,"arrow_id",NULL,(void **)&tempint); // Go through each arrow_id in supplied list and copy items from default list into current settings
    obj  = arrow_list_default;
    while ((obj != NULL) && (obj->id < *tempint)) obj = (obj->next);
    if ((obj != NULL) && (obj->id == *tempint))
     {
      list = first;
      while ((*list != NULL) && ((*list)->id < *tempint)) list = &((*list)->next);
      new = (arrow_object *)malloc(sizeof(arrow_object));
      if (new == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
      *new = *obj;
      new->next = *list;
      with_words_copy(&new->style, &obj->style);
      *list = new;
     }
    listiter = ListIterate(listiter, NULL);
   }
  return;
 }

#define CMPVAL(X,Y) (ppl_units_DimEqual(&X,&Y) && ppl_units_DblEqual(X.real , Y.real))

unsigned char arrow_compare(arrow_object *a, arrow_object *b)
 {
  if (a->id!=b->id) return 0;
  if ( (!CMPVAL(a->x0 , b->x0)) || (!CMPVAL(a->y0 , b->y0)) || (!CMPVAL(a->z0 , b->z0)) ) return 0;
  if ( (!CMPVAL(a->x1 , b->x1)) || (!CMPVAL(a->y1 , b->y1)) || (!CMPVAL(a->z1 , b->z1)) ) return 0;
  if ( (a->system_x0!=b->system_x0) || (a->system_y0!=b->system_y0) || (a->system_z0!=b->system_z0) ) return 0;
  if ( (a->system_x1!=b->system_x1) || (a->system_y1!=b->system_y1) || (a->system_z1!=b->system_z1) ) return 0;
  if ( (a->axis_x0!=b->axis_x0) || (a->axis_y0!=b->axis_y0) || (a->axis_z0!=b->axis_z0) ) return 0;
  if ( (a->axis_x1!=b->axis_x1) || (a->axis_y1!=b->axis_y1) || (a->axis_z1!=b->axis_z1) ) return 0;
  if (!with_words_compare(&a->style , &b->style)) return 0;
  return 1;
 }

void arrow_list_copy(arrow_object **out, arrow_object **in)
 {
  *out = NULL;
  while (*in != NULL)
   {
    *out = (arrow_object *)malloc(sizeof(arrow_object));
    if (*out == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    **out = **in;
    (*out)->next = NULL;
    with_words_copy(&(*out)->style, &(*in)->style);
    in  = &((*in )->next);
    out = &((*out)->next);
   }
  return;
 }

void arrow_list_destroy(arrow_object **list)
 {
  arrow_object *obj, **first;

  first = list;
  while (*list != NULL)
   {
    obj = *list;
    *list = (*list)->next;
    with_words_destroy(&obj->style);
    free(obj);
   }
  *first = NULL;
  return;
 }

void arrow_print(arrow_object *in, char *out)
 {
  int i;
  sprintf(out, "from %s", *(char **)FetchSettingName(in->system_x0, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *)));
  i = strlen(out);
  if (in->system_x0==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_x0); i+=strlen(out+i); }
  sprintf(out+i, " %s,", ppl_units_NumericDisplay(&(in->x0),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " %s", *(char **)FetchSettingName(in->system_y0, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_y0==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_y0); i+=strlen(out+i); }
  sprintf(out+i, " %s,", ppl_units_NumericDisplay(&(in->y0),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " %s", *(char **)FetchSettingName(in->system_z0, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_z0==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_z0); i+=strlen(out+i); }
  sprintf(out+i, " %s ", ppl_units_NumericDisplay(&(in->z0),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, "to %s", *(char **)FetchSettingName(in->system_x1, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_x1==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_x1); i+=strlen(out+i); }
  sprintf(out+i, " %s,", ppl_units_NumericDisplay(&(in->x1),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " %s", *(char **)FetchSettingName(in->system_y1, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_y1==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_y1); i+=strlen(out+i); }
  sprintf(out+i, " %s,", ppl_units_NumericDisplay(&(in->y1),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " %s", *(char **)FetchSettingName(in->system_z1, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_z1==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_z1); i+=strlen(out+i); }
  sprintf(out+i, " %s", ppl_units_NumericDisplay(&(in->z1),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " with %s ", *(char **)FetchSettingName(in->arrow_style, SW_ARROWTYPE_INT, (void *)SW_ARROWTYPE_STR, sizeof(char *))); i+=strlen(out+i);
  with_words_print(&in->style, out+i);
  return;
 }

// -------------------------------------------
// ROUTINES FOR MANIPULATING LABELS STRUCTURES
// -------------------------------------------

#define ASSERT_LENGTH(VAR) \
  if (!(VAR->dimensionless)) \
   { \
    for (i=0; i<UNITS_MAX_BASEUNITS; i++) \
     if (VAR->exponent[i] != (i==UNIT_LENGTH)) \
      { \
       sprintf(temp_err_string,"The gap size supplied to the 'set label' command must have dimensions of length. Supplied gap size input has units of <%s>.",ppl_units_GetUnitStr(VAR,NULL,NULL,1,1,0)); \
       ppl_error(ERR_NUMERIC, temp_err_string); \
       return; \
      } \
   } \
  else { VAR->real /= 100; } // By default, dimensionless positions are in centimetres

#define ASSERT_ANGLE(VAR) \
  if (!(VAR->dimensionless)) \
   { \
    for (i=0; i<UNITS_MAX_BASEUNITS; i++) \
     if (VAR->exponent[i] != (i==UNIT_ANGLE)) \
      { \
       sprintf(temp_err_string,"The rotation angle supplied to the 'set label' command must have dimensions of angle. Supplied input has units of <%s>.",ppl_units_GetUnitStr(VAR,NULL,NULL,1,1,0)); \
       ppl_error(ERR_NUMERIC, temp_err_string); \
       return; \
      } \
   } \
  else { VAR->real *= M_PI/180.0; } // By default, dimensionless angles are in degrees

void label_add(label_object **list, Dict *in)
 {
  int   *tempint, i, system_x, system_y, system_z;
  char  *tempstr, *label;
  value *tempval, tempvalobj, *gap, *ang;
  with_words ww_temp1;
  label_object *out;

  arrow_add_get_system("x",system_x); arrow_add_get_system("y",system_y); arrow_add_get_system("z",system_z);

  arrow_add_check_dimensions("x",system_x); arrow_add_check_dimensions("y",system_y); arrow_add_check_dimensions("z",system_z);

  arrow_add_check_axis("x"); arrow_add_check_axis("y"); arrow_add_check_axis("z");

  DictLookup(in,"label_text",NULL,(void **)&tempstr);
  label = (char *)malloc(strlen(tempstr)+1);
  if (label == NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return; }
  strcpy(label, tempstr);

  // Check for rotation modifier
  DictLookup(in, "rotation", NULL, (void *)&ang);
  if (ang != NULL) { ASSERT_ANGLE(ang); }

  // Check for gap modifier
  DictLookup(in, "gap", NULL, (void *)&gap);
  if (gap != NULL) { ASSERT_LENGTH(gap); }

  // Look up ID number of the label we are adding and find appropriate place for it in label list
  DictLookup(in,"label_id",NULL,(void **)&tempint);
  while ((*list != NULL) && ((*list)->id < *tempint)) list = &((*list)->next);
  if ((*list != NULL) && ((*list)->id == *tempint))
   {
    out = *list;
    with_words_destroy(&out->style);
   } else {
    out = (label_object *)malloc(sizeof(label_object));
    if (out == NULL) { ppl_error(ERR_MEMORY, "Out of memory"); return; }
    out->id   = *tempint;
    out->next = *list;
    *list     = out;
   }

  // Check for halign or valign modifiers
  DictLookup(in,"halign",NULL,(void **)&tempstr);
  if (tempstr != NULL) out->HAlign = FetchSettingByName(tempstr, SW_HALIGN_INT, SW_HALIGN_STR);
  else                 out->HAlign = 0;
  DictLookup(in,"valign",NULL,(void **)&tempstr);
  if (tempstr != NULL) out->VAlign = FetchSettingByName(tempstr, SW_VALIGN_INT, SW_VALIGN_STR);
  else                 out->VAlign = 0;

  if (ang != NULL) out->rotation = ang->real;
  else             out->rotation = 0.0;
  if (gap != NULL) out->gap      = gap->real;
  else             out->gap      = 0.0;

  with_words_fromdict(in, &ww_temp1, 1);
  with_words_copy(&out->style, &ww_temp1);
  out->text  = label;
  out->system_x = system_x; out->system_y = system_y; out->system_z = system_z;
  arrow_add_get_axis("x", out->axis_x); arrow_add_get_axis("y", out->axis_y); arrow_add_get_axis("z", out->axis_z);
  arrow_add_copy_coordinate("x",system_x,out->x); arrow_add_copy_coordinate("y",system_y,out->y); arrow_add_copy_coordinate("z",system_z,out->z);
  return;
 }

void label_remove(label_object **list, Dict *in)
 {
  int          *tempint;
  label_object *obj, **first;
  List         *templist;
  Dict         *tempdict;
  ListIterator *listiter;

  first = list;
  DictLookup(in,"label_list,",NULL,(void **)&templist);
  listiter = ListIterateInit(templist);
  if (listiter == NULL) label_list_destroy(list); // set nolabel with no number specified means all labels deleted
  while (listiter != NULL)
   {
    tempdict = (Dict *)listiter->data;
    DictLookup(tempdict,"label_id",NULL,(void **)&tempint);
    list = first;
    while ((*list != NULL) && ((*list)->id < *tempint)) list = &((*list)->next);
    if ((*list != NULL) && ((*list)->id == *tempint))
     {
      obj   = *list;
      *list = (*list)->next;
      with_words_destroy(&obj->style);
      free(obj->text);
      free(obj);
     } else {
      //sprintf(temp_err_string,"Label number %d is not defined", *tempint);
      //ppl_error(ERR_GENERAL, temp_err_string);
     }
    listiter = ListIterate(listiter, NULL);
   }
  return;
 }

void label_unset(label_object **list, Dict *in)
 {
  int          *tempint;
  label_object *obj, *new, **first;
  List         *templist;
  Dict         *tempdict;
  ListIterator *listiter;

  label_remove(list, in); // First of all, remove any labels which we are unsetting
  first = list;
  DictLookup(in,"label_list,",NULL,(void **)&templist);
  listiter = ListIterateInit(templist);

  if (listiter == NULL) label_list_copy(list, &label_list_default); // Check to see whether we are unsetting ALL labels, and if so, use the copy command
  while (listiter != NULL)
   {
    tempdict = (Dict *)listiter->data;
    DictLookup(tempdict,"label_id",NULL,(void **)&tempint); // Go through each label_id in supplied list and copy items from default list into current settings
    obj  = label_list_default;
    while ((obj != NULL) && (obj->id < *tempint)) obj = (obj->next);
    if ((obj != NULL) && (obj->id == *tempint))
     {
      list = first;
      while ((*list != NULL) && ((*list)->id < *tempint)) list = &((*list)->next);
      new = (label_object *)malloc(sizeof(label_object));
      if (new == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
      *new = *obj;
      new->next = *list;
      new->text = (char *)malloc(strlen(obj->text)+1);
      if (new->text == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); free(new); return; }
      strcpy(new->text, obj->text);
      with_words_copy(&new->style, &obj->style);
      *list = new;
     }
    listiter = ListIterate(listiter, NULL);
   }
  return;
 }

unsigned char label_compare(label_object *a, label_object *b)
 {
  if (a->id!=b->id) return 0;
  if ( (!CMPVAL(a->x , b->x)) || (!CMPVAL(a->y , b->y)) || (!CMPVAL(a->z , b->z)) ) return 0;
  if ( (a->system_x!=b->system_x) || (a->system_y!=b->system_y) || (a->system_z!=b->system_z) ) return 0;
  if ( (a->axis_x!=b->axis_x) || (a->axis_y!=b->axis_y) || (a->axis_z!=b->axis_z) ) return 0;
  if (!with_words_compare(&a->style , &b->style)) return 0;
  if (strcmp(a->text , b->text)!=0) return 0;
  return 1;
 }

void label_list_copy(label_object **out, label_object **in)
 {
  *out = NULL;
  while (*in != NULL)
   {
    *out = (label_object *)malloc(sizeof(label_object));
    if (*out == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); return; }
    **out = **in;
    (*out)->next = NULL;
    (*out)->text = (char *)malloc(strlen((*in)->text)+1);
    if ((*out)->text == NULL) { ppl_error(ERR_MEMORY,"Out of memory"); free(*out); *out=NULL; return; }
    strcpy((*out)->text, (*in)->text);
    with_words_copy(&(*out)->style, &(*in)->style);
    in  = &((*in )->next);
    out = &((*out)->next);
   }
  return;
 }

void label_list_destroy(label_object **list)
 {
  label_object *obj, **first;

  first = list;
  while (*list != NULL)
   {
    obj = *list;
    *list = (*list)->next;
    with_words_destroy(&obj->style);
    free(obj->text);
    free(obj);
   }
  *first = NULL;
  return;
 }

void label_print(label_object *in, char *out)
 {
  int i;
  StrEscapify(in->text, out);
  i = strlen(out);
  sprintf(out+i, " at %s", *(char **)FetchSettingName(in->system_x, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_x==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_x); i+=strlen(out+i); }
  sprintf(out+i, " %s,", ppl_units_NumericDisplay(&(in->x),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " %s", *(char **)FetchSettingName(in->system_y, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_y==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_y); i+=strlen(out+i); }
  sprintf(out+i, " %s,", ppl_units_NumericDisplay(&(in->y),0,0,0)); i+=strlen(out+i);
  sprintf(out+i, " %s", *(char **)FetchSettingName(in->system_z, SW_SYSTEM_INT, (void *)SW_SYSTEM_STR, sizeof(char *))); i+=strlen(out+i);
  if (in->system_z==SW_SYSTEM_AXISN) { sprintf(out+i, " %d",in->axis_z); i+=strlen(out+i); }
  sprintf(out+i, " %s", ppl_units_NumericDisplay(&(in->z),0,0,0)); i+=strlen(out+i);
  if (in->rotation!=0.0) { sprintf(out+i, " rotate %s",
             NumericDisplay( in->rotation *180/M_PI , 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           ); i+=strlen(out+i); }
  if (in->HAlign>0) { sprintf(out+i, " halign %s", *(char **)FetchSettingName(in->HAlign, SW_HALIGN_INT, (void *)SW_HALIGN_STR, sizeof(char *))); i+=strlen(out+i); }
  if (in->VAlign>0) { sprintf(out+i, " valign %s", *(char **)FetchSettingName(in->VAlign, SW_VALIGN_INT, (void *)SW_VALIGN_STR, sizeof(char *))); i+=strlen(out+i); }
  if (in->gap!=0.0) { sprintf(out+i, " gap %s",
             NumericDisplay( in->gap * 100          , 0, settings_term_current.SignificantFigures, (settings_term_current.NumDisplay==SW_DISPLAY_L))
           ); i+=strlen(out+i); }
  with_words_print(&in->style, out+i+6);
  if (strlen(out+i+6)>0) { sprintf(out+i, " with"); out[i+5]=' '; }
  else                   { out[i]='\0'; }
  return;
 }

// -----------------------------------------------
// ROUTINES FOR MANIPULATING WITH_WORDS STRUCTURES
// -----------------------------------------------

void with_words_zero(with_words *a, const unsigned char malloced)
 {
  a->colour = a->fillcolour = a->linespoints = a->linetype = a->pointtype = a->style = 0;
  a->linewidth = a->pointlinewidth = a->pointsize = 1.0;
  a->colourR = a->colourG = a->colourB = a->fillcolourR = a->fillcolourG = a->fillcolourB = 0;
  a->STRlinetype = a->STRlinewidth = a->STRpointlinewidth = a->STRpointsize = a->STRpointtype = NULL;
  a->STRcolourR = a->STRcolourG = a->STRcolourB = a->STRfillcolourR = a->STRfillcolourG = a->STRfillcolourB = NULL;
  a->USEcolour = a->USEfillcolour = a->USElinespoints = a->USElinetype = a->USElinewidth = a->USEpointlinewidth = a->USEpointsize = a->USEpointtype = a->USEstyle = a->USEcolourRGB = a->USEfillcolourRGB = 0;
  a->malloced = malloced;
  return;
 }

#define XWWMALLOC(X) (tmp = malloc(X)); if (tmp==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); with_words_zero(out,1); return; }

void with_words_fromdict(Dict *in, with_words *out, const unsigned char MallocNew)
 {
  int    *tempint, i; // TO DO: Need to be able to read colours.
  double *tempdbl;
  char   *tempstr;
  void   *tmp;
  with_words_zero(out, MallocNew);

  // read colour names
  colour_fromdict(in,""    ,&out->colour    ,&out->colourR    ,&out->colourG    ,&out->colourB    ,&out->STRcolourR    ,&out->STRcolourG    ,&out->STRcolourB    ,
                  &out->USEcolour    ,&out->USEcolourRGB    ,&i,MallocNew);
  colour_fromdict(in,"fill",&out->fillcolour,&out->fillcolourR,&out->fillcolourG,&out->fillcolourB,&out->STRfillcolourR,&out->STRfillcolourG,&out->STRfillcolourB,
                  &out->USEfillcolour,&out->USEfillcolourRGB,&i,MallocNew);

  // Other settings
  DictLookup(in,"linetype",NULL,(void **)&tempint);
  if (tempint != NULL) { out->linetype = *tempint; out->USElinetype = 1; }
  DictLookup(in,"linetype_string",NULL,(void **)&tempstr);
  if (tempstr != NULL) { if (!MallocNew) { out->STRlinetype       = tempstr; }
                         else            { out->STRlinetype       = (char   *)XWWMALLOC(strlen(tempstr)+1); strcpy(out->STRlinetype      , tempstr); }
                       }
  DictLookup(in,"linewidth",NULL,(void **)&tempdbl);
  if (tempdbl != NULL) { out->linewidth = *tempdbl; out->USElinewidth = 1; }
  DictLookup(in,"linewidth_string",NULL,(void **)&tempstr);
  if (tempstr != NULL) { if (!MallocNew) { out->STRlinewidth      = tempstr; }
                         else            { out->STRlinewidth      = (char   *)XWWMALLOC(strlen(tempstr)+1); strcpy(out->STRlinewidth     , tempstr); }
                       }
  DictLookup(in,"pointsize",NULL,(void **)&tempdbl);
  if (tempdbl != NULL) { out->pointsize = *tempdbl; out->USEpointsize = 1; }
  DictLookup(in,"pointsize_string",NULL,(void **)&tempstr);
  if (tempstr != NULL) { if (!MallocNew) { out->STRpointsize      = tempstr; }
                         else            { out->STRpointsize      = (char   *)XWWMALLOC(strlen(tempstr)+1); strcpy(out->STRpointsize     , tempstr); }
                       }
  DictLookup(in,"pointtype",NULL,(void **)&tempint);
  if (tempint != NULL) { out->pointtype = *tempint; out->USEpointtype = 1; }
  DictLookup(in,"pointtype_string",NULL,(void **)&tempstr);
  if (tempstr != NULL) { if (!MallocNew) { out->STRpointtype      = tempstr; }
                         else            { out->STRpointtype      = (char   *)XWWMALLOC(strlen(tempstr)+1); strcpy(out->STRpointtype     , tempstr); }
                       }
  DictLookup(in,"style_number",NULL,(void **)&tempint);
  if (tempint != NULL) { out->style = *tempint; out->USEstyle = 1; }
  DictLookup(in,"pointlinewidth",NULL,(void **)&tempdbl);
  if (tempdbl != NULL) { out->pointlinewidth = *tempdbl; out->USEpointlinewidth = 1; }
  DictLookup(in,"pointlinewidth_string",NULL,(void **)&tempstr);
  if (tempstr != NULL) { if (!MallocNew) { out->STRpointlinewidth = tempstr; }
                         else            { out->STRpointlinewidth = (char   *)XWWMALLOC(strlen(tempstr)+1); strcpy(out->STRpointlinewidth, tempstr); }
                       }
  DictLookup(in,"style",NULL,(void **)&tempstr);
  if (tempstr != NULL)
   {
    i = FetchSettingByName(tempstr, SW_STYLE_INT, SW_STYLE_STR);
    out->linespoints = i;
    out->USElinespoints = 1;
   }
  return;
 }

int with_words_compare_zero(const with_words *a)
 {
  if (a->STRlinetype != NULL) return 0;
  if (a->STRlinewidth != NULL) return 0;
  if (a->STRpointlinewidth != NULL) return 0;
  if (a->STRpointsize != NULL) return 0;
  if (a->STRpointtype != NULL) return 0;
  if (a->STRcolourR != NULL) return 0;
  if (a->STRcolourG != NULL) return 0;
  if (a->STRcolourB != NULL) return 0;
  if (a->STRfillcolourR != NULL) return 0;
  if (a->STRfillcolourG != NULL) return 0;
  if (a->STRfillcolourB !=NULL) return 0;
  if (a->USEcolour) return 0;
  if (a->USEfillcolour) return 0;
  if (a->USElinespoints) return 0;
  if (a->USElinetype) return 0;
  if (a->USElinewidth) return 0;
  if (a->USEpointlinewidth) return 0;
  if (a->USEpointsize) return 0;
  if (a->USEpointtype) return 0;
  if (a->USEstyle) return 0;
  if (a->USEcolourRGB) return 0;
  if (a->USEfillcolourRGB) return 0;
  return 1;
 }

int with_words_compare(const with_words *a, const with_words *b)
 {
  // Check that the range of items which are defined in both structures are the same
  if ((a->STRcolourR       ==NULL) != (b->STRcolourR       ==NULL)                                                                            ) return 0;
  if ((a->STRcolourR       ==NULL)                                 &&                           (a->USEcolourRGB      != b->USEcolourRGB     )) return 0;
  if ((a->STRcolourR       ==NULL)                                 && (!a->USEcolourRGB    ) && (a->USEcolour         != b->USEcolour        )) return 0;
  if ((a->STRfillcolourR   ==NULL) != (b->STRfillcolourR   ==NULL)                                                                            ) return 0;
  if ((a->STRfillcolourR   ==NULL)                                 &&                           (a->USEfillcolourRGB  != b->USEfillcolourRGB )) return 0;
  if ((a->STRfillcolourR   ==NULL)                                 && (!a->USEfillcolourRGB) && (a->USEfillcolour     != b->USEfillcolour    )) return 0;
  if (                                                                                          (a->USElinespoints    != b->USElinespoints   )) return 0;
  if ((a->STRlinetype      ==NULL) != (b->STRlinetype      ==NULL)                                                                            ) return 0;
  if ((a->STRlinetype      ==NULL)                                                           && (a->USElinetype       != b->USElinetype      )) return 0;
  if ((a->STRlinewidth     ==NULL) != (b->STRlinewidth     ==NULL)                                                                            ) return 0;
  if ((a->STRlinewidth     ==NULL)                                                           && (a->USElinewidth      != b->USElinewidth     )) return 0;
  if ((a->STRpointlinewidth==NULL) != (b->STRpointlinewidth==NULL)                                                                            ) return 0;
  if ((a->STRpointlinewidth==NULL)                                                           && (a->USEpointlinewidth != b->USEpointlinewidth)) return 0;
  if ((a->STRpointsize     ==NULL) != (b->STRpointsize     ==NULL)                                                                            ) return 0;
  if ((a->STRpointsize     ==NULL)                                                           && (a->USEpointsize      != b->USEpointsize     )) return 0;
  if ((a->STRpointtype     ==NULL) != (b->STRpointtype     ==NULL)                                                                            ) return 0;
  if ((a->STRpointtype     ==NULL)                                                           && (a->USEpointtype      != b->USEpointtype     )) return 0;
  if (                                                                                          (a->USEstyle          != b->USEstyle         )) return 0;

  // Check that the actual values are the same in both structures
  if      ((a->STRcolourR       !=NULL) && ((strcmp(a->STRcolourR,b->STRcolourR)!=0)||(strcmp(a->STRcolourG,b->STRcolourG)!=0)||(strcmp(a->STRcolourB,b->STRcolourB)!=0))) return 0;
  else if ((a->USEcolourRGB           ) && ((a->colourR!=b->colourR)||(a->colourG!=b->colourG)||(a->colourB!=b->colourB))) return 0;
  else if ((a->USEcolour              ) && ((a->colour !=b->colour ))) return 0;
  if      ((a->STRfillcolourR   !=NULL) && ((strcmp(a->STRfillcolourR,b->STRfillcolourR)!=0)||(strcmp(a->STRfillcolourG,b->STRfillcolourG)!=0)||(strcmp(a->STRfillcolourB,b->STRfillcolourB)!=0))) return 0;
  else if ((a->USEfillcolourRGB       ) && ((a->fillcolourR   !=b->fillcolourR   )||(a->fillcolourG!=b->fillcolourG)||(a->fillcolourB!=b->fillcolourB))) return 0;
  else if ((a->USEfillcolour          ) && ((a->fillcolour    !=b->fillcolour    ))) return 0;
  if      ((a->USElinespoints         ) && ((a->linespoints   !=b->linespoints   ))) return 0;
  if      ((a->STRlinetype      !=NULL) && ((strcmp(a->STRlinetype      ,b->STRlinetype      )!=0))) return 0;
  else if ((a->USElinetype            ) && ((a->linetype      !=b->linetype      ))) return 0;
  if      ((a->STRlinewidth     !=NULL) && ((strcmp(a->STRlinewidth     ,b->STRlinewidth     )!=0))) return 0;
  else if ((a->USElinewidth           ) && ((a->linewidth     !=b->linewidth     ))) return 0;
  if      ((a->STRpointlinewidth!=NULL) && ((strcmp(a->STRpointlinewidth,b->STRpointlinewidth)!=0))) return 0;
  else if ((a->USEpointlinewidth      ) && ((a->pointlinewidth!=b->pointlinewidth))) return 0;
  if      ((a->STRpointsize     !=NULL) && ((strcmp(a->STRpointsize     ,b->STRpointsize     )!=0))) return 0;
  else if ((a->USEpointsize           ) && ((a->pointsize     !=b->pointsize     ))) return 0;
  if      ((a->STRpointtype     !=NULL) && ((strcmp(a->STRpointtype     ,b->STRpointtype     )!=0))) return 0;
  else if ((a->USEpointtype           ) && ((a->pointtype     !=b->pointtype     ))) return 0;
  if      ((a->USEstyle               ) && ((a->style         !=b->style         ))) return 0;

  return 1; // We have not found any differences
 }

// a has highest priority; e has lowest priority
void with_words_merge(with_words *out, const with_words *a, const with_words *b, const with_words *c, const with_words *d, const with_words *e, const unsigned char ExpandStyles)
 {
  int i;
  const with_words *InputArray[25] = {a,b,c,d,e};
  unsigned char BlockStyleSubstitution[25] = {0,0,0,0,0};
  const with_words *x;

  with_words_zero(out,0);

  for (i=4; i>=0; i--)
   {
    if (i>24) { ppl_error(ERR_GENERAL, "Iteration depth exceeded whilst substituting plot styles. Infinite plot style loop suspected."); return; } // i can reach 24 when recursion happens
    x = InputArray[i];
    if (x == NULL) continue;
    if ((x->USEstyle) && (!BlockStyleSubstitution[i])) // Substitute for numbered plot styles
     {
      if (ExpandStyles)
       {
        BlockStyleSubstitution[i  ] = 1; // Only do this once, to avoid infinite loop
        BlockStyleSubstitution[i+1] = 0; // Allow recursive substitutions
        InputArray[i+1] = &(settings_plot_styles[ x->style ]);
        i+=2; // Recurse
        continue;
       } else {
        out->style = x->style; out->USEstyle = 1;
       }
     }
    if (x->STRcolourR       !=NULL) { out->STRcolourR = x->STRcolourR; out->STRcolourG = x->STRcolourG; out->STRcolourB = x->STRcolourB; out->USEcolourRGB = 0; out->USEcolour = 0; }
    if (x->USEcolourRGB           ) { out->colourR = x->colourR; out->colourG = x->colourG; out->colourB = x->colourB; out->USEcolourRGB = 1; out->USEcolour = 0; out->STRcolourR = out->STRcolourG = out->STRcolourB = NULL; }
    if (x->USEcolour              ) { out->colour = x->colour; out->USEcolour = 1; out->USEcolourRGB = 0; out->STRcolourR = out->STRcolourG = out->STRcolourB = NULL; }
    if (x->STRfillcolourR   !=NULL) { out->STRfillcolourR = x->STRfillcolourR; out->STRfillcolourG = x->STRfillcolourG; out->STRfillcolourB = x->STRfillcolourB; out->USEfillcolourRGB = 0; out->USEfillcolour = 0; }
    if (x->USEfillcolourRGB       ) { out->fillcolourR = x->fillcolourR; out->fillcolourG = x->fillcolourG; out->fillcolourB = x->fillcolourB; out->USEfillcolourRGB = 1; out->USEfillcolour = 0; out->STRfillcolourR = out->STRfillcolourG = out->STRfillcolourB = NULL; }
    if (x->USEfillcolour          ) { out->fillcolour = x->fillcolour; out->USEfillcolour = 1; out->USEfillcolourRGB = 0; out->STRfillcolourR = out->STRfillcolourG = out->STRfillcolourB = NULL; }
    if (x->USElinespoints         ) { out->linespoints = x->linespoints; out->USElinespoints = 1; }
    if (x->STRlinetype      !=NULL) { out->STRlinetype = x->STRlinetype; out->USElinetype = 0; }
    if (x->USElinetype            ) { out->linetype = x->linetype; out->USElinetype = 1; out->STRlinetype = NULL; }
    if (x->STRlinewidth     !=NULL) { out->STRlinewidth = x->STRlinewidth; out->USElinewidth = 0; }
    if (x->USElinewidth           ) { out->linewidth = x->linewidth; out->USElinewidth = 1; out->STRlinewidth = NULL; }
    if (x->STRpointlinewidth!=NULL) { out->STRpointlinewidth = x->STRpointlinewidth; out->USEpointlinewidth = 0; }
    if (x->USEpointlinewidth      ) { out->pointlinewidth = x->pointlinewidth; out->USEpointlinewidth = 1; out->STRpointlinewidth = NULL; }
    if (x->STRpointsize     !=NULL) { out->STRpointsize = x->STRpointsize; out->USEpointsize = 0; }
    if (x->USEpointsize           ) { out->pointsize = x->pointsize; out->USEpointsize = 1; out->STRpointsize = NULL; }
    if (x->STRpointtype     !=NULL) { out->STRpointtype = x->STRpointtype; out->USEpointtype = 0; }
    if (x->USEpointtype           ) { out->pointtype = x->pointtype; out->USEpointtype = 1; out->STRpointtype = NULL; }
   }
  return;
 }

#define NUMDISP(X) NumericDisplay(X,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L))

void with_words_print(const with_words *defn, char *out)
 {
  int i=0;
  if      (defn->USElinespoints)          { sprintf(out+i, "%s "            , *(char **)FetchSettingName(defn->linespoints, SW_STYLE_INT , (void *)SW_STYLE_STR , sizeof(char *))); i += strlen(out+i); }
  if      (defn->STRcolourR!=NULL)        { sprintf(out+i, "colour rgb%s:%s:%s "     , defn->STRcolourR, defn->STRcolourG, defn->STRcolourB);                                       i += strlen(out+i); }
  else if (defn->USEcolourRGB)            { sprintf(out+i, "colour rgb%d:%d:%d "     , defn->colourR, defn->colourG, defn->colourB);                                                i += strlen(out+i); }
  else if (defn->USEcolour)               { sprintf(out+i, "colour %s "     , *(char **)FetchSettingName(defn->colour     , SW_COLOUR_INT, (void *)SW_COLOUR_STR, sizeof(char *))); i += strlen(out+i); }
  if      (defn->STRfillcolourR!=NULL)    { sprintf(out+i, "fillcolour rgb%s:%s:%s " , defn->STRfillcolourR, defn->STRfillcolourG, defn->STRfillcolourB);                           i += strlen(out+i); }
  else if (defn->USEfillcolourRGB)        { sprintf(out+i, "fillcolour rgb%d:%d:%d " , defn->fillcolourR, defn->fillcolourG, defn->fillcolourB);                                    i += strlen(out+i); }
  else if (defn->USEfillcolour)           { sprintf(out+i, "fillcolour %s " , *(char **)FetchSettingName(defn->fillcolour , SW_COLOUR_INT, (void *)SW_COLOUR_STR, sizeof(char *))); i += strlen(out+i); }
  if      (defn->STRlinetype!=NULL)       { sprintf(out+i, "linetype %s "            , defn->STRlinetype);                                                                          i += strlen(out+i); }
  else if (defn->USElinetype)             { sprintf(out+i, "linetype %d "            , defn->linetype);                                                                             i += strlen(out+i); }
  if      (defn->STRlinewidth!=NULL)      { sprintf(out+i, "linewidth %s "           , defn->STRlinewidth);                                                                         i += strlen(out+i); }
  else if (defn->USElinewidth)            { sprintf(out+i, "linewidth %s "           , NUMDISP(defn->linewidth));                                                                   i += strlen(out+i); }
  if      (defn->STRpointlinewidth!=NULL) { sprintf(out+i, "pointlinewidth %s "      , defn->STRpointlinewidth);                                                                    i += strlen(out+i); }
  else if (defn->USEpointlinewidth)       { sprintf(out+i, "pointlinewidth %s "      , NUMDISP(defn->pointlinewidth));                                                              i += strlen(out+i); }
  if      (defn->STRpointsize!=NULL)      { sprintf(out+i, "pointsize %s "           , defn->STRpointsize);                                                                         i += strlen(out+i); }
  else if (defn->USEpointsize)            { sprintf(out+i, "pointsize %s "           , NUMDISP(defn->pointsize));                                                                   i += strlen(out+i); }
  if      (defn->STRpointtype!=NULL)      { sprintf(out+i, "pointtype %s "           , defn->STRpointtype);                                                                         i += strlen(out+i); }
  else if (defn->USEpointtype)            { sprintf(out+i, "pointtype %d "           , defn->pointtype);                                                                            i += strlen(out+i); }
  if      (defn->USEstyle)                { sprintf(out+i, "style %d "               , defn->style);                                                                                i += strlen(out+i); }
  out[i]='\0';
  return;
 }

void with_words_destroy(with_words *a)
 {
  if (!a->malloced) return;
  if (a->STRlinetype       != NULL) { free(a->STRlinetype      ); a->STRlinetype       = NULL; }
  if (a->STRlinewidth      != NULL) { free(a->STRlinewidth     ); a->STRlinewidth      = NULL; }
  if (a->STRpointlinewidth != NULL) { free(a->STRpointlinewidth); a->STRpointlinewidth = NULL; }
  if (a->STRpointsize      != NULL) { free(a->STRpointsize     ); a->STRpointsize      = NULL; }
  if (a->STRpointtype      != NULL) { free(a->STRpointtype     ); a->STRpointtype      = NULL; }
  if (a->STRcolourR        != NULL) { free(a->STRcolourR       ); a->STRcolourR        = NULL; }
  if (a->STRcolourG        != NULL) { free(a->STRcolourG       ); a->STRcolourG        = NULL; }
  if (a->STRcolourB        != NULL) { free(a->STRcolourB       ); a->STRcolourB        = NULL; }
  if (a->STRfillcolourR    != NULL) { free(a->STRfillcolourR   ); a->STRfillcolourR    = NULL; }
  if (a->STRfillcolourG    != NULL) { free(a->STRfillcolourG   ); a->STRfillcolourG    = NULL; }
  if (a->STRfillcolourB    != NULL) { free(a->STRfillcolourB   ); a->STRfillcolourB    = NULL; }
  return;
 }

void with_words_copy(with_words *out, const with_words *in)
 {
  void *tmp;
  *out = *in;
  out->malloced = 1;
  if (in->STRlinetype      != NULL) { out->STRlinetype      = (char   *)XWWMALLOC(strlen(in->STRlinetype      )+1); strcpy(out->STRlinetype      , in->STRlinetype      ); }
  if (in->STRlinewidth     != NULL) { out->STRlinewidth     = (char   *)XWWMALLOC(strlen(in->STRlinewidth     )+1); strcpy(out->STRlinewidth     , in->STRlinewidth     ); }
  if (in->STRpointlinewidth!= NULL) { out->STRpointlinewidth= (char   *)XWWMALLOC(strlen(in->STRpointlinewidth)+1); strcpy(out->STRpointlinewidth, in->STRpointlinewidth); }
  if (in->STRpointsize     != NULL) { out->STRpointsize     = (char   *)XWWMALLOC(strlen(in->STRpointsize     )+1); strcpy(out->STRpointsize     , in->STRpointsize     ); }
  if (in->STRpointtype     != NULL) { out->STRpointtype     = (char   *)XWWMALLOC(strlen(in->STRpointtype     )+1); strcpy(out->STRpointtype     , in->STRpointtype     ); }
  if (in->STRcolourR       != NULL) { out->STRcolourR       = (char   *)XWWMALLOC(strlen(in->STRcolourR       )+1); strcpy(out->STRcolourR       , in->STRcolourR       ); }
  if (in->STRcolourG       != NULL) { out->STRcolourG       = (char   *)XWWMALLOC(strlen(in->STRcolourG       )+1); strcpy(out->STRcolourG       , in->STRcolourG       ); }
  if (in->STRcolourB       != NULL) { out->STRcolourB       = (char   *)XWWMALLOC(strlen(in->STRcolourB       )+1); strcpy(out->STRcolourB       , in->STRcolourB       ); }
  if (in->STRfillcolourR   != NULL) { out->STRfillcolourR   = (char   *)XWWMALLOC(strlen(in->STRfillcolourR   )+1); strcpy(out->STRfillcolourR   , in->STRfillcolourR   ); }
  if (in->STRfillcolourG   != NULL) { out->STRfillcolourG   = (char   *)XWWMALLOC(strlen(in->STRfillcolourG   )+1); strcpy(out->STRfillcolourG   , in->STRfillcolourG   ); }
  if (in->STRfillcolourB   != NULL) { out->STRfillcolourB   = (char   *)XWWMALLOC(strlen(in->STRfillcolourB   )+1); strcpy(out->STRfillcolourB   , in->STRfillcolourB   ); }
  return;
 }

// ------------------------------------------------------
// Functions for creating and destroying axis descriptors
// ------------------------------------------------------

void DestroyAxis(settings_axis *in)
 {
  int i;
  if (in->format    != NULL) { free(in->format   ); in->format    = NULL; }
  if (in->label     != NULL) { free(in->label    ); in->label     = NULL; }
  if (in->linkusing != NULL) { free(in->linkusing); in->linkusing = NULL; }
  if (in->MTickList != NULL) { free(in->MTickList); in->MTickList = NULL; }
  if (in->MTickStrs != NULL)
   {
    for (i=0; in->MTickStrs[i]!=NULL; i++) free(in->MTickStrs[i]);
    free(in->MTickStrs);
    in->MTickStrs = NULL;
   }
  if (in->TickList  != NULL) { free(in->TickList ); in->TickList  = NULL; }
  if (in->TickStrs  != NULL)
   {
    for (i=0; in->TickStrs[i]!=NULL; i++) free(in->TickStrs[i]);
    free(in->TickStrs );
    in->TickStrs  = NULL;
   }
  return;
 }

// settings_axis_default is a safe fallback axis because it contains no malloced strings
#define XMALLOC(X) (tmp = malloc(X)); if (tmp==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *out = settings_axis_default; return; }

void CopyAxis(settings_axis *out, const settings_axis *in)
 {
  void *tmp;
  *out = *in;
  if (in->format    != NULL) { out->format   = (char   *)XMALLOC(strlen(in->format    )+1); strcpy(out->format   , in->format    ); }
  if (in->label     != NULL) { out->label    = (char   *)XMALLOC(strlen(in->label     )+1); strcpy(out->label    , in->label     ); }
  if (in->linkusing != NULL) { out->linkusing= (char   *)XMALLOC(strlen(in->linkusing )+1); strcpy(out->linkusing, in->linkusing ); }
  CopyAxisTics (out,in);
  CopyAxisMTics(out,in);
  return;
 }

void CopyAxisTics(settings_axis *out, const settings_axis *in)
 {
  int   i,j;
  void *tmp;
  if (in->TickStrs != NULL)
   {
    for (i=0; in->TickStrs[i]!=NULL; i++);
    out->TickStrs= XMALLOC((i+1)*sizeof(char *));
    for (j=0; j<i; j++) { out->TickStrs[j] = XMALLOC(strlen(in->TickStrs[j])+1); strcpy(out->TickStrs[j], in->TickStrs[j]); }
    out->TickStrs[i] = NULL;
   }
  if (in->TickList != NULL)
   {
    out->TickList= (double *)XMALLOC((i+1)*sizeof(double));
    memcpy(out->TickList, in->TickList, (i+1)*sizeof(double)); // NB: For this to be safe, TickLists MUST have double to correspond to NULL in TickStrs
   }
  return;
 }

void CopyAxisMTics(settings_axis *out, const settings_axis *in)
 {
  int   i,j;
  void *tmp;
  if (in->MTickStrs != NULL)
   {
    for (i=0; in->MTickStrs[i]!=NULL; i++);
    out->MTickStrs= XMALLOC((i+1)*sizeof(char *));
    for (j=0; j<i; j++) { out->MTickStrs[j] = XMALLOC(strlen(in->MTickStrs[j])+1); strcpy(out->MTickStrs[j], in->MTickStrs[j]); }
    out->MTickStrs[i] = NULL;
   }
  if (in->MTickList != NULL)
   {
    out->MTickList= (double *)XMALLOC((i+1)*sizeof(double));
    memcpy(out->MTickList, in->MTickList, (i+1)*sizeof(double)); // NB: For this to be safe, TickLists MUST have double to correspond to NULL in TickStrs
   }
  return;
 }

unsigned char CompareAxisTics(const settings_axis *a, const settings_axis *b)
 {
  int i,j;
  if ((a->TickList==NULL)&&(b->TickList==NULL)) return 1;
  if ((a->TickList==NULL)||(b->TickList==NULL)) return 0;
  for (i=0; a->TickStrs[i]!=NULL; i++);
  for (j=0; b->TickStrs[j]!=NULL; j++);
  if (i!=j) return 0; // Tick lists have different lengths
  for (j=0; j<i; j++)
   {
    if (a->TickList[j] != b->TickList[j]) return 0;
    if (strcmp(a->TickStrs[j], b->TickStrs[j])!=0) return 0;
   }
  return 1;
 }

unsigned char CompareAxisMTics(const settings_axis *a, const settings_axis *b)
 {
  int i,j;
  if ((a->MTickList==NULL)&&(b->MTickList==NULL)) return 1;
  if ((a->MTickList==NULL)||(b->MTickList==NULL)) return 0;
  for (i=0; a->MTickStrs[i]!=NULL; i++);
  for (j=0; b->MTickStrs[j]!=NULL; j++);
  if (i!=j) return 0; // Tick lists have different lengths
  for (j=0; j<i; j++)
   {
    if (a->MTickList[j] != b->MTickList[j]) return 0;
    if (strcmp(a->MTickStrs[j], b->MTickStrs[j])!=0) return 0;
   }
  return 1;
 }

