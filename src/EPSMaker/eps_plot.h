// eps_plot.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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

#ifndef _PPL_EPS_PLOT_H
#define _PPL_EPS_PLOT_H 1

#include "ListTools/lt_list.h"

#include "ppl_settings.h"

#include "eps_comm.h"

int eps_plot_AddUsingItemsForWithWords(with_words *ww, int *NExpect, unsigned char *AutoUsingList, List *UsingList);

void eps_plot_ReadAccessibleData(EPSComm *x);
void eps_plot_SampleFunctions(EPSComm *x);
void eps_plot_DecideAxisRanges(EPSComm *x);
void eps_plot_YieldUpText(EPSComm *x);
void eps_plot_RenderEPS(EPSComm *x);

void eps_plot_WithWordsFromUsingItems(with_words *ww, double *DataRow, int Ncolumns);
int  eps_plot_WithWordsCheckUsingItemsDimLess(with_words *ww, value *FirstValues, int Ncolumns, int *NDataCols);

#endif

