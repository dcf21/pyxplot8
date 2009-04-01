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
 float dpi;
 char  output[FNAME_LENGTH];
 } settings_terminal;

typedef struct settings_graph {
 int   AxesColour, aspect, AutoAspect, DataStyle, FontSize, FuncStyle, grid, GridAxisX, GridAxisY, GridMajColour, GridMinColour, key, KeyColumns, KeyPos, samples, TextColour, TextHAlign, TextVAlign;
 float bar, BinOrigin, BinWidth, BoxFrom, BoxWidth, KeyXOff, KeyYOff, LineWidth, OriginX, OriginY, PointSize, PointLineWidth, TitleXOff, TitleYOff, width;
 char  title[FNAME_LENGTH];
 } settings_graph;

#endif
