// ppl_flowctrl.h
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

#ifndef _PPL_FLOWCTRL_H
#define _PPL_FLOWCTRL_H 1

#include "ListTools/lt_dict.h"

#include "ppl_units.h"

// Maximum permitted iteration level
#define MAX_ITERLEVEL_DEPTH 50

typedef struct cmd_chain_item {
 char *line;
 struct cmd_chain_item *next;
 int linenumber;
 char *description;
} cmd_chain_item;

typedef struct cmd_chain_item *cmd_chain;

typedef struct SubroutineDescriptor {
 int        NumberArguments;
 char      *ArgList;
 cmd_chain  commands;
} SubroutineDescriptor;

#ifndef _PPL_FLOWCTRL_C
extern char *PPL_FLOWCTRL_LOOPNAME[MAX_ITERLEVEL_DEPTH+1];
extern int   PPL_FLOWCTRL_BREAKABLE;
extern int   PPL_FLOWCTRL_RETURNABLE;
extern int   PPL_FLOWCTRL_BREAKLEVEL;
extern int   PPL_FLOWCTRL_BROKEN;
extern int   PPL_FLOWCTRL_CONTINUED;
extern int   PPL_FLOWCTRL_RETURNED;
extern int   PPL_FLOWCTRL_RETURNCONTEXT;
extern value PPL_FLOWCTRL_RETURNVAL, PPL_FLOWCTRL_RETURNTOALGEBRA;
extern Dict *PPL_SUBROUTINES;
#endif

int directive_do     (Dict *command, int IterLevel);
int directive_for    (Dict *command, int IterLevel);
int directive_foreach(Dict *command, int IterLevel);
int directive_if     (Dict *command, int IterLevel);
int directive_while  (Dict *command, int IterLevel);

int directive_call      (Dict *command, int IterLevel);
int CallSubroutineFromAlgebra(char *FunctionName, char *ArgStart, int *endpos, int *errpos, char *errtext, int IterLevel);
int directive_subroutine(Dict *command, int IterLevel);

void directive_foreach_LoopOverData(Dict *command, char *filename, cmd_chain *chain, cmd_chain *chainiter, int IterLevel, int *status);

#endif

