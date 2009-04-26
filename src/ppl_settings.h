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
#include "ListTools/lt_list.h"

#include "ppl_units.h"

typedef struct with_words {
 int    colour, fillcolour, linestyle, linetype, pointtype, style;
 double linewidth, pointlinewidth, pointsize;
 } with_words;

typedef struct settings_terminal {
 int   backup, colour, display, landscape, multiplot, TermAntiAlias, TermType, TermEnlarge, TermInvert, TermTransparent;
 float dpi;
 value PaperHeight, PaperWidth;
 char  output[FNAME_LENGTH];
 char  PaperName[FNAME_LENGTH];
 char  LatexPreamble[FNAME_LENGTH];
 } settings_terminal;

typedef struct settings_graph {
 int   aspect, AutoAspect, AxesColour, FontSize, grid, GridAxisX, GridAxisY, GridAxisZ, GridMajColour, GridMinColour, key, KeyColumns, KeyPos, samples, TextColour, TextHAlign, TextVAlign;
 float bar, BinOrigin, BinWidth, BoxFrom, BoxWidth, LineWidth, PointSize, PointLineWidth;
 value KeyXOff, KeyYOff, OriginX, OriginY, TitleXOff, TitleYOff, width;
 char  title[FNAME_LENGTH];
 with_words DataStyle, FuncStyle;
 } settings_graph;

typedef struct settings_axis {
 unsigned char enabled;
 int MaxSet, MinSet, log, TickDirection, MTickMaxSet, MTickMinSet, MTickStepSet, TickMaxSet, TickMinSet, TickStepSet;
 float LogBase, max, min, MTickMax, MTickMin, MTickStep, TickMax, TickMin, TickStep;
 char label[FNAME_LENGTH];
 List *MTickList, *TickList;
 } settings_axis;

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
extern settings_axis     settings_axis_default;
extern settings_session  settings_session_default;
extern int               settings_palette[];

void  ppl_settings_term_init();
void *FetchSettingName      (int id, int *id_list, void **name_list);
int   FetchSettingByName    (char *name, int *id_list, char **name_list);
#endif

// Functions defined in ppl_readconfig.c
#ifndef _PPL_READCONFIG_C
extern int ppl_termtype_set_in_configfile;
void ReadConfigFile(char *ConfigFname);
#endif

#endif
