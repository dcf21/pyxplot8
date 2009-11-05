// ppl_settings.c
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

#define _PPL_SETTINGS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

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

int               settings_palette_default[PALETTE_LENGTH] = {COLOUR_BLACK, COLOUR_RED, COLOUR_BLUE, COLOUR_MAGENTA, COLOUR_CYAN, COLOUR_BROWN, COLOUR_SALMON, COLOUR_GRAY, COLOUR_GREEN, COLOUR_NAVYBLUE, COLOUR_PERIWINKLE, COLOUR_PINEGREEN, COLOUR_SEAGREEN, COLOUR_GREENYELLOW, COLOUR_ORANGE, COLOUR_CARNATIONPINK, COLOUR_PLUM, -1};
int               settings_palette_current[PALETTE_LENGTH];

Dict             *settings_filters;

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
  settings_term_default.PaperHeight.real    = 297.0 / 1000;
  settings_term_default.PaperHeight.dimensionless = 0; settings_term_default.PaperHeight.exponent[UNIT_LENGTH] = 1;
  strcpy(settings_term_default.PaperName, "A4");
  ppl_units_zero(&(settings_term_default.PaperWidth));
  settings_term_default.PaperWidth.real     = 210.0 / 1000;
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

  // Default Graph Settings, used when these values are not changed by any configuration files
  settings_graph_default.aspect                = 1.0;
  settings_graph_default.AutoAspect            = SW_ONOFF_ON;
  settings_graph_default.AxesColour            = COLOUR_BLACK;
  settings_graph_default.bar                   = 1.0;
  ppl_units_zero(&(settings_graph_default.BoxFrom));
  settings_graph_default.BoxFromAuto           = 1;
  ppl_units_zero(&(settings_graph_default.BoxWidth));
  settings_graph_default.BoxWidthAuto          = 1;
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
  settings_graph_default.GridMinColour = COLOUR_GREY90;
  settings_graph_default.key           = SW_ONOFF_ON;
  settings_graph_default.KeyColumns    = 1;
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
  settings_graph_default.TextHAlign    = SW_HALIGN_LEFT;
  settings_graph_default.TextVAlign = SW_VALIGN_BOT;
  strcpy(settings_graph_default.title, "");
  ppl_units_zero(&(settings_graph_default.TitleXOff));
  settings_graph_default.TitleXOff.real= 0.0;
  settings_graph_default.TitleXOff.dimensionless = 0; settings_graph_default.TitleXOff.exponent[UNIT_LENGTH] = 1;
  ppl_units_zero(&(settings_graph_default.TitleYOff));
  settings_graph_default.TitleYOff.real= 0.0;
  settings_graph_default.TitleYOff.dimensionless = 0; settings_graph_default.TitleYOff.exponent[UNIT_LENGTH] = 1;
  ppl_units_zero(&(settings_graph_default.Tmin));
  ppl_units_zero(&(settings_graph_default.Tmax));
  settings_graph_default.Tmin.real     = 0.0;
  settings_graph_default.Tmax.real     = 1.0;
  ppl_units_zero(&(settings_graph_default.width));
  settings_graph_default.width.real    = 0.08; // 8cm
  settings_graph_default.width.dimensionless = 0; settings_graph_default.width.exponent[UNIT_LENGTH] = 1;

  // Default Axis Settings, used whenever a new axis is created
  settings_axis_default.atzero      = 0;
  settings_axis_default.enabled     = 0;
  settings_axis_default.invisible   = 0;
  settings_axis_default.linked      = 0;
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
  settings_axis_default.MTickMaxSet = SW_BOOL_FALSE;
  settings_axis_default.MTickMinSet = SW_BOOL_FALSE;
  settings_axis_default.MTickStepSet= SW_BOOL_FALSE;
  settings_axis_default.TickDir     = SW_TICDIR_IN;
  settings_axis_default.TickLabelRotation  = SW_TICLABDIR_HORI;
  settings_axis_default.TickMaxSet  = SW_BOOL_FALSE;
  settings_axis_default.TickMinSet  = SW_BOOL_FALSE;
  settings_axis_default.TickStepSet = SW_BOOL_FALSE;
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

  // Set up array of plot styles
  for (i=0; i<MAX_PLOTSTYLES; i++) with_words_zero(&(settings_plot_styles        [i]),1);
  for (i=0; i<MAX_PLOTSTYLES; i++) with_words_zero(&(settings_plot_styles_default[i]),1);

  // Set up current axes
  for (i=0; i<MAX_AXES; i++) XAxes[i] = YAxes[i] = ZAxes[i] = settings_axis_default;
  XAxes[1].enabled = YAxes[1].enabled = ZAxes[1].enabled = 1;
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
     } else {
      if (DEBUG) ppl_log("$PAPERSIZE returned an unrecognised paper size."); 
     }
   }

  // Copy Default Settings to Current Settings
  settings_term_current  = settings_term_default;
  settings_graph_current = settings_graph_default;
  for (i=0; i<PALETTE_LENGTH; i++) settings_palette_current[i] = settings_palette_default[i];
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
  for (i=0; i<PALETTE_LENGTH; i++) settings_palette_current[i] = settings_palette_default[i];
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

void with_words_fromdict(Dict *in, with_words *out, const unsigned char MallocNew)
 {
  int    *tempint, i;
  double *tempdbl;
  char   *tempstr;
  with_words_zero(out, MallocNew);
  DictLookup(in,"linetype",NULL,(void **)&tempint); // TO DO: Need to be able to read colours. Need to be able to read string versions of settings.
  if (tempint != NULL) { out->linetype = *tempint; out->USElinetype = 1; }
  DictLookup(in,"linewidth",NULL,(void **)&tempdbl);
  if (tempint != NULL) { out->linewidth = *tempdbl; out->USElinewidth = 1; }
  DictLookup(in,"pointsize",NULL,(void **)&tempdbl);
  if (tempint != NULL) { out->pointsize = *tempdbl; out->USEpointsize = 1; }
  DictLookup(in,"pointtype",NULL,(void **)&tempint);
  if (tempint != NULL) { out->pointtype = *tempint; out->USEpointtype = 1; }
  DictLookup(in,"style_number",NULL,(void **)&tempint);
  if (tempint != NULL) { out->style = *tempint; out->USEstyle = 1; }
  DictLookup(in,"pointlinewidth",NULL,(void **)&tempdbl);
  if (tempint != NULL) { out->pointlinewidth = *tempdbl; out->USEpointlinewidth = 1; }
  DictLookup(in,"style",NULL,(void **)&tempstr);
  if (tempstr != NULL)
   {
    i = FetchSettingByName(tempstr, SW_STYLE_INT, SW_STYLE_STR);
    out->style = i;
    out->USEstyle = 1;
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
void with_words_merge(with_words *out, const with_words *a, const with_words *b, const with_words *c, const with_words *d, const with_words *e)
 {
  int i;
  const with_words *InputArray[5] = {a,b,c,d,e};
  const with_words *x;
  with_words_zero(out,0); // To do: expand x->style? But if it's a string, can't do that yet
  for (i=4; i<=0; i--) // No. Can't use strings for style number.
   {
    x = InputArray[i];
    if (x == NULL) continue;
    if (x->STRcolourR       !=NULL) { out->STRcolourR = x->STRcolourR; out->STRcolourG = x->STRcolourG; out->STRcolourB = x->STRcolourB; }
    if (x->USEcolourRGB           ) { out->colourR = x->colourR; out->colourG = x->colourG; out->colourB = x->colourB; out->USEcolourRGB = 1; }
    if (x->USEcolour              ) { out->colour = x->colour; out->USEcolour = 1; }
    if (x->STRfillcolourR   !=NULL) { out->STRfillcolourR = x->STRfillcolourR; out->STRfillcolourG = x->STRfillcolourG; out->STRfillcolourB = x->STRfillcolourB; }
    if (x->USEfillcolourRGB       ) { out->fillcolourR = x->fillcolourR; out->fillcolourG = x->fillcolourG; out->fillcolourB = x->fillcolourB; out->USEfillcolourRGB = 1; }
    if (x->USEfillcolour          ) { out->fillcolour = x->fillcolour; out->USEfillcolour = 1; }
    if (x->USElinespoints         ) { out->linespoints = x->linespoints; out->USElinespoints = 1; }
    if (x->STRlinetype      !=NULL) { out->STRlinetype = x->STRlinetype; }
    if (x->USElinetype            ) { out->linetype = x->linetype; out->USElinetype = 1; }
    if (x->STRlinewidth     !=NULL) { out->STRlinewidth = x->STRlinewidth; }
    if (x->USElinewidth           ) { out->linewidth = x->linewidth; out->USElinewidth = 1; }
    if (x->STRpointlinewidth!=NULL) { out->STRpointlinewidth = x->STRpointlinewidth; }
    if (x->USEpointlinewidth      ) { out->pointlinewidth = x->pointlinewidth; out->USEpointlinewidth = 1; }
    if (x->STRpointsize     !=NULL) { out->STRpointsize = x->STRpointsize; }
    if (x->USEpointsize           ) { out->pointsize = x->pointsize; out->USEpointsize = 1; }
    if (x->STRpointtype     !=NULL) { out->STRpointtype = x->STRpointtype; }
    if (x->USEpointtype           ) { out->pointtype = x->pointtype; out->USEpointtype = 1; }
    if (x->USEstyle               ) { out->style = x->style; out->USEstyle = 1; }
   }
  return;
 }

#define NUMDISP(X) NumericDisplay(X,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L))

void with_words_print(const with_words *defn, char *out)
 {
  int i=0;
  if      (defn->USElinespoints)          { sprintf(out+i, "%s "                     , (char *)FetchSettingName(defn->linespoints, SW_STYLE_INT , (void **)SW_STYLE_STR )); i += strlen(out+i); }
  if      (defn->STRcolourR!=NULL)        { sprintf(out+i, "colour rgb:%s,%s,%s "    , defn->STRcolourR, defn->STRcolourG, defn->STRcolourB);                               i += strlen(out+i); }
  else if (defn->USEcolourRGB)            { sprintf(out+i, "colour rgb:%d,%d,%d "    , defn->colourR, defn->colourG, defn->colourB);                                        i += strlen(out+i); }
  else if (defn->USEcolour)               { sprintf(out+i, "colour %s "              , (char *)FetchSettingName(defn->colour     , SW_COLOUR_INT, (void **)SW_COLOUR_STR)); i += strlen(out+i); }
  if      (defn->STRfillcolourR!=NULL)    { sprintf(out+i, "fillcolour rgb:%s,%s,%s ", defn->STRfillcolourR, defn->STRfillcolourG, defn->STRfillcolourB);                   i += strlen(out+i); }
  else if (defn->USEfillcolourRGB)        { sprintf(out+i, "fillcolour rgb:%d,%d,%d ", defn->fillcolourR, defn->fillcolourG, defn->fillcolourB);                            i += strlen(out+i); }
  else if (defn->USEfillcolour)           { sprintf(out+i, "fillcolour %s "          , (char *)FetchSettingName(defn->fillcolour , SW_COLOUR_INT, (void **)SW_COLOUR_STR)); i += strlen(out+i); }
  if      (defn->STRlinetype!=NULL)       { sprintf(out+i, "linetype %s "            , defn->STRlinetype);                                                                  i += strlen(out+i); }
  else if (defn->USElinetype)             { sprintf(out+i, "linetype %d "            , defn->linetype);                                                                     i += strlen(out+i); }
  if      (defn->STRlinewidth!=NULL)      { sprintf(out+i, "linewidth %s "           , defn->STRlinewidth);                                                                 i += strlen(out+i); }
  else if (defn->USElinewidth)            { sprintf(out+i, "linewidth %s "           , NUMDISP(defn->linewidth));                                                           i += strlen(out+i); }
  if      (defn->STRpointlinewidth!=NULL) { sprintf(out+i, "pointlinewidth %s "      , defn->STRpointlinewidth);                                                            i += strlen(out+i); }
  else if (defn->USEpointlinewidth)       { sprintf(out+i, "pointlinewidth %s "      , NUMDISP(defn->pointlinewidth));                                                      i += strlen(out+i); }
  if      (defn->STRpointsize!=NULL)      { sprintf(out+i, "pointsize %s "           , defn->STRpointsize);                                                                 i += strlen(out+i); }
  else if (defn->USEpointsize)            { sprintf(out+i, "pointsize %s "           , NUMDISP(defn->pointsize));                                                           i += strlen(out+i); }
  if      (defn->STRpointtype!=NULL)      { sprintf(out+i, "pointtype %s "           , defn->STRpointtype);                                                                 i += strlen(out+i); }
  else if (defn->USEpointtype)            { sprintf(out+i, "pointtype %d "           , defn->pointtype);                                                                    i += strlen(out+i); }
  if      (defn->USEstyle)                { sprintf(out+i, "style %d "               , defn->style);                                                                        i += strlen(out+i); }
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

#define XWWMALLOC(X) (tmp = malloc(X)); if (tmp==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); with_words_zero(out,1); return; }

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

void DestroyAxis(settings_axis *in, const settings_axis *def)
 {
  int i;
  if (((def==NULL) || (in->format    != def->format   )) && (in->format    != NULL)) { free(in->format   ); in->format    = NULL; }
  if (((def==NULL) || (in->label     != def->label    )) && (in->label     != NULL)) { free(in->label    ); in->label     = NULL; }
  if (((def==NULL) || (in->linkusing != def->linkusing)) && (in->linkusing != NULL)) { free(in->linkusing); in->linkusing = NULL; }
  if (((def==NULL) || (in->MTickList != def->MTickList)) && (in->MTickList != NULL)) { free(in->MTickList); in->MTickList = NULL; }
  if (((def==NULL) || (in->MTickStrs != def->MTickStrs)) && (in->MTickStrs != NULL))
   {
    for (i=0; in->MTickStrs[i]!=NULL; i++) free(in->MTickStrs);
    free(in->MTickStrs);
    in->MTickStrs = NULL;
   }
  if (((def==NULL) || (in->TickList  != def->TickList )) && (in->TickList  != NULL)) { free(in->TickList ); in->TickList  = NULL; }
  if (((def==NULL) || (in->TickStrs  != def->TickStrs )) && (in->TickStrs  != NULL))
   {
    for (i=0; in->TickStrs[i]!=NULL; i++) free(in->TickStrs);
    free(in->TickStrs );
    in->TickStrs  = NULL;
   }
  return;
 }

#define XMALLOC(X) (tmp = malloc(X)); if (tmp==NULL) { ppl_error(ERR_MEMORY,"Out of memory"); *out = settings_axis_default; return; }

void CopyAxis(settings_axis *out, const settings_axis *in)
 {
  int   i,j;
  void *tmp;
  *out = *in;
  if (in->format    != NULL) { out->format   = (char   *)XMALLOC(strlen(in->format    )+1); strcpy(out->format   , in->format    ); }
  if (in->label     != NULL) { out->label    = (char   *)XMALLOC(strlen(in->label     )+1); strcpy(out->label    , in->label     ); }
  if (in->linkusing != NULL) { out->linkusing= (char   *)XMALLOC(strlen(in->linkusing )+1); strcpy(out->linkusing, in->linkusing ); }
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

