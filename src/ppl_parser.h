// ppl_parser.h
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

#ifndef _PPL_PARSER_H
#define _PPL_PARSER_H 1

#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

void   ppl_commands_read();
void   ppl_parser_bnreset();
Dict  *parse             (char *line, int IterLevel);
char **ppl_rl_completion (const char *text, int start, int end);

#endif

