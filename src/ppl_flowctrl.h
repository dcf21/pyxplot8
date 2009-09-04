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

typedef struct cmd_chain_item {
 char *line;
 struct cmd_chain_item *next;
 int linenumber;
 char *description;
} cmd_chain_item;

typedef struct cmd_chain_item *cmd_chain;

#ifndef _PPL_FLOWCTRL_C
extern int PPL_FLOWCTRL_BREAKABLE;
extern int PPL_FLOWCTRL_BROKEN;
extern int PPL_FLOWCTRL_CONTINUED;
#endif

int directive_do     (Dict *command, int IterLevel);
int directive_for    (Dict *command, int IterLevel);
int directive_foreach(Dict *command, int IterLevel);
int directive_if     (Dict *command, int IterLevel);
int directive_while  (Dict *command, int IterLevel);

void directive_foreach_LoopOverData(Dict *command, char *filename, cmd_chain *chain, cmd_chain *chainiter, int IterLevel, int *status);

#endif

