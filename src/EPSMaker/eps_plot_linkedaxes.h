// eps_plot_linkedaxes.h
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

#ifndef _PPL_EPS_PLOT_LINKEDAXES_H
#define _PPL_EPS_PLOT_LINKEDAXES_H 1

#include "ppl_settings.h"

#include "eps_comm.h"

void eps_plot_LinkedAxisBackPropagate(EPSComm *x, settings_axis *source, int xyz, int axis_n);
int  eps_plot_LinkedAxisLinkUsing(settings_axis *out, settings_axis *in, int xyz);
void eps_plot_DecideAxisRange(EPSComm *x, settings_axis *axis, int xyz, int axis_n);
void eps_plot_LinkedAxisForwardPropagate(EPSComm *x, settings_axis *axis, int xyz, int axis_n, int mode);

void eps_plot_LinkUsingBackPropagate(EPSComm *x, double val, settings_axis *target, int target_xyz, int target_n, int target_canvasID, settings_axis *source, int source_xyz, int source_n, int source_canvasID);

#endif

