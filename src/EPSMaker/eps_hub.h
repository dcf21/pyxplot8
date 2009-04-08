// eps_hub.h
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

#ifndef _EPS_HUB_H
#define _EPS_HUB_H 1

#ifndef _EPS_HUB_C
extern char eps_ErrorString[];
#endif

int  eps_HubInit  ( void(*mem_fatal_handler)(char *, int, char *) , void(*mem_log_handler)(char *) );
void eps_log      (char *msg);
void eps_fatal    (char *file, int line, char *msg);

#endif

