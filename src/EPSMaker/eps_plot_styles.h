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

void eps_withwords_default(with_words *output, settings_graph *sg, unsigned char functions, int Fcounter, int Dcounter, unsigned char colour);
int  eps_plot_styles_NDataColumns(int style, unsigned char ThreeDim);
int  eps_plot_styles_UpdateUsage(DataTable *data, int style, unsigned char ThreeDim, settings_axis *a1, settings_axis *a2, settings_axis *a3, settings_graph *sg, int xyz1, int xyz2, int xyz3, int n1, int n2, int n3, int id);
int  eps_plot_dataset(EPSComm *x, DataTable *data, int style, unsigned char ThreeDim, settings_axis *a1, settings_axis *a2, settings_axis *a3, int xn, int yn, int zn, settings_graph *sg, canvas_plotdesc *pd, double origin_x, double origin_y, double width, double height);
void eps_plot_LegendIcon(EPSComm *x, int i, canvas_plotdesc *pd, double xpos, double ypos, double scale, settings_axis *a1, settings_axis *a2, settings_axis *a3, int xn, int yn, int zn);

#endif

