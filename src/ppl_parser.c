// ppl_parser.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asciidouble.h"
#include "ppl_list.h"
#include "ppl_dict.h"
#include "ppl_error.h"
#include "ppl_constants.h"
#include "ppl_parser.h"

// --------------------------------------------------------------------------
// PART I: READ SYNTAX SPECIFICATION
//
// The syntaxes of PyXPlot commands are specified in gp_commands.
//
// We make a tree structure -- a list of lists. Each list takes the form:
//  [ [type, output_variable_name, output_variable_value, grammar_symbol] , ---- Members of structure ]
//
// type can be:
//   seq -- match each member of structure in turn
//   opt -- entire structure is optional. typically contains a seq structure.
//   rep -- repeat structure as many times as it matches input
//   per -- members of this structure may be found in any order, but each may only match once
//   ora -- match any ONE member of this structure
//
// The special type "item" has structure:
// [ ["item", match string, autocomplete level, varname, var_value] ]
//
// Only "item" and "rep" structures are allowed to return output variables "varname".
// Upon parsing a line of user input, a dictionary is returning, containing the values of all these variables.

void ppl_commands_read()
 {
  return;
 }

// --------------------------------------------------------------------------
// PART II: PARSE A LINE OF USER INPUT

// PARSE(): Top-level interface. Parses a commandline "line" from the user.
// It expects that ; and `` have already been dealt with by pyxplot.py

Dict *parse(char *line)
 {
  return NULL;
 }

// PARSE_DESCEND(): We go through command definition structure, recursively descending into sub-structures
//   line -- input line from user
//   vars -- PyXPlot's user-defined variables; used for evaluating expressions
//   linepos -- how far through line have we got with our parsing efforts so far?
//   expecting -- used to build up a list of all possible match items which could be used for next word. Used for intelligent syntax errors.
//   algebra_linepos -- if we encounter an error evaluating an expression, we store the position in the line of the error here.
//   algebra_error -- if we encounter an error evaluating an expression, we store the error message here.
//   command -- the command definition structure into which we are descending
//   match -- we set this to true when we've got match to be sure this was the command that the user wanted, even if he made a syntax error.
//   dict -- we populate this dictionary with settings from the user's input

void parse_descend(char *line, int linepos, char *expecting, int algebra_linepos, char *algebra_error, List *command, int match, Dict *dict)
 {
  return;
 }

