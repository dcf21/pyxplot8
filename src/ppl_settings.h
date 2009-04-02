// ppl_settings.h
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

#ifndef _PPL_SETTINGS
#define _PPL_SETTINGS 1

#include "ppl_constants.h"

typedef struct settings_terminal {
 int   backup, colour, display, landscape, multiplot, TermAntiAlias, TermType, TermEnlarge, TermInvert, TermTransparent;
 float dpi, PaperHeight, PaperWidth;
 char  output[FNAME_LENGTH];
 char  PaperName[FNAME_LENGTH];
 char  LatexPreamble[FNAME_LENGTH];
 } settings_terminal;

typedef struct settings_graph {
 int   AxesColour, aspect, AutoAspect, DataStyle, FontSize, FuncStyle, grid, GridAxisX, GridAxisY, GridMajColour, GridMinColour, key, KeyColumns, KeyPos, samples, TextColour, TextHAlign, TextVAlign;
 float bar, BinOrigin, BinWidth, BoxFrom, BoxWidth, KeyXOff, KeyYOff, LineWidth, OriginX, OriginY, PointSize, PointLineWidth, TitleXOff, TitleYOff, width;
 char  title[FNAME_LENGTH];
 } settings_graph;

typedef struct settings_session {
 int   splash, colour, colour_rep, colour_wrn, colour_err;
 char  cwd[FNAME_LENGTH];
 char  tempdir[FNAME_LENGTH];
 char  homedir[FNAME_LENGTH];
 } settings_session;

// Variables defined in ppl_settings_term.c
#ifndef PPL_SETTINGS_TERM
extern settings_terminal settings_term_default;
extern settings_terminal settings_term_current;
extern settings_graph settings_graph_default;
extern settings_graph settings_graph_current;
extern settings_session settings_session_default;

void  ppl_settings_term_init();
void *FetchSettingName(int id, int *id_list, void **name_list);
#endif

#endif
