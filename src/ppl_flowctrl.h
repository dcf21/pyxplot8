// ppl_flowctrl.h
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

#ifndef _PPL_FLOWCTRL_H
#define _PPL_FLOWCTRL_H 1

#include "ListTools/lt_dict.h"

#ifndef _PPL_FLOWCTRL_C
extern int PPL_FLOWCTRL_BREAKABLE;
extern int PPL_FLOWCTRL_BROKEN;
extern int PPL_FLOWCTRL_CONTINUED;
#endif

void directive_do     (Dict *command);
void directive_for    (Dict *command);
void directive_foreach(Dict *command);
void directive_if     (Dict *command);
void directive_while  (Dict *command);

#endif

