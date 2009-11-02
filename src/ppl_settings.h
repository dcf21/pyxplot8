// ppl_settings.h
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

#ifndef _PPL_SETTINGS_H
#define _PPL_SETTINGS_H 1

#include "StringTools/str_constants.h"

#include "ppl_constants.h"
#include "ppl_units.h"

typedef struct with_words {
 int    colour, fillcolour, linestyle, linetype, pointtype, style;
 double linewidth, pointlinewidth, pointsize;
 } with_words;

typedef struct settings_terminal {
 int    backup, CalendarIn, CalendarOut, colour, ComplexNumbers, display, ExplicitErrors, landscape, multiplot, NumDisplay, SignificantFigures, TermAntiAlias, TermType, TermEnlarge, TermInvert, TermTransparent, UnitScheme, UnitDisplayPrefix, UnitDisplayAbbrev;
 double dpi;
 unsigned char BinOriginAuto, BinWidthAuto;
 value  BinOrigin, BinWidth, PaperHeight, PaperWidth;
 char   output[FNAME_LENGTH];
 char   PaperName[FNAME_LENGTH];
 char   LatexPreamble[FNAME_LENGTH];
 } settings_terminal;

typedef struct settings_graph {
 int           aspect, AutoAspect, AxesColour, grid, GridMajColour, GridMinColour, key, KeyColumns, KeyPos, samples, TextColour, TextHAlign, TextVAlign;
 double        bar, FontSize, LineWidth, PointSize, PointLineWidth, projection;
 unsigned int  GridAxisX[MAX_AXES], GridAxisY[MAX_AXES], GridAxisZ[MAX_AXES];
 unsigned char BoxFromAuto, BoxWidthAuto;
 value         BoxFrom, BoxWidth, KeyXOff, KeyYOff, OriginX, OriginY, TitleXOff, TitleYOff, Tmin, Tmax, width;
 char          title[FNAME_LENGTH];
 with_words    DataStyle, FuncStyle;
 } settings_graph;

typedef struct settings_session {
 int   splash, colour, colour_rep, colour_wrn, colour_err;
 char  cwd[FNAME_LENGTH];
 char  tempdir[FNAME_LENGTH];
 char  homedir[FNAME_LENGTH];
 } settings_session;

// Variables defined in ppl_settings.c
#ifndef _PPL_SETTINGS_C
extern settings_terminal settings_term_default;
extern settings_terminal settings_term_current;
extern settings_graph    settings_graph_default;
extern settings_graph    settings_graph_current;
extern settings_session  settings_session_default;
extern int               settings_palette_current[];
extern int               settings_palette_default[];

void  ppl_settings_makedefault();
void  ppl_settings_readconfig();
#endif

// Can now safely include these headers, which need some of the settings above
#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

typedef struct settings_axis {
 unsigned char atzero, enabled, invisible, linked, topbottom;
 int     ArrowType, LinkedAxisCanvasID, LinkedAxisToXYZ, LinkedAxisToNum, log, MaxSet, MinSet, MirrorType, MTickDir, MTickMaxSet, MTickMinSet, MTickStepSet, TickDir, TickLabelRotation, TickMaxSet, TickMinSet, TickStepSet;
 double  LabelRotate, LogBase, max, min, MTickMax, MTickMin, MTickStep, TickLabelRotate, TickMax, TickMin, TickStep;
 char   *format, *label, *linkusing;
 double *MTickList, *TickList;
 char  **MTickStrs,**TickStrs;
 value   unit;
 } settings_axis;

// Variables defined in ppl_settings.c
#ifndef _PPL_SETTINGS_C
extern settings_axis     settings_axis_default;
extern settings_axis     XAxes[], XAxesDefault[];
extern settings_axis     YAxes[], YAxesDefault[];
extern settings_axis     ZAxes[], ZAxesDefault[];
extern Dict             *settings_filters;
#endif

// Functions defined in ppl_readconfig.c
#ifndef _PPL_READCONFIG_C
extern int ppl_termtype_set_in_configfile;
void ReadConfigFile(char *ConfigFname);
#endif

#endif
