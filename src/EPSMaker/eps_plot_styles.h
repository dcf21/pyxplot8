// eps_plot_styles.h
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

#ifndef _PPL_EPS_PLOT_STYLES_H
#define _PPL_EPS_PLOT_STYLES_H 1

#include "ppl_settings.h"

void eps_withwords_default(with_words *output, unsigned char functions, int Fcounter, int Dcounter, unsigned char colour);
int  eps_plot_styles_NDataColumns(int style, unsigned char ThreeDim);
int  eps_plot_styles_UpdateUsage(DataTable *data, int style, unsigned char ThreeDim, settings_axis *xa, settings_axis *ya, settings_axis *za, int xn, int yn, int zn, int id);

#endif

