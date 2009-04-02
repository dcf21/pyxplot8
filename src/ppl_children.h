// ppl_children.h
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

#ifndef PPL_CHILDREN
#define PPL_CHILDREN 1

#include "ppl_list.h"

extern char GhostView_Fname[];

// Functions to be called from main PyXPlot process

void  InitialiseCSP            ();
void  CheckForGvOutput         ();
void  SendCommandToCSP         (char *cmd);

// Functions to be called from the Child Support Process

void  CSPmain                  ();
void  CSPCheckForChildExits    ();
void  CSPCheckForNewCommands   ();
void  CSPProcessCommand        (char *in);
int   CSPForkNewGv             (char *fname, List *gv_list);
void  CSPKillAllGvs            ();
void  CSPKillLatestSinglewindow();

#endif

