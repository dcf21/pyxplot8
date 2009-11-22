// eps_core.h
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

#ifndef _PPL_EPS_CORE_H
#define _PPL_EPS_CORE_H 1

#define IF_NOT_INVISIBLE if (x->LastEPSColour[0]!='\0')

void eps_core_clear                (EPSComm *x);
void eps_core_SetColour            (EPSComm *x, with_words *ww);
void eps_core_SetFillColour        (EPSComm *x, with_words *ww);
void eps_core_SwitchTo_FillColour  (EPSComm *x);
void eps_core_SwitchFrom_FillColour(EPSComm *x);
void eps_core_SetLinewidth         (EPSComm *x, double lw, int lt);
void eps_core_BoundingBox          (EPSComm *x, double xpos, double ypos, double lw);

#endif

