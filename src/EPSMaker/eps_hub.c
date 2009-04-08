// eps_error.c
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

#include "StringTools/str_constants.h"

#include "eps_latex.h"

char eps_ErrorString[LSTR_LENGTH] = ""; // A string where error messages get stored

void(*_eps_fatal)(char *, int, char *); // Storage for error handlers
void(*_eps_log)(char *);

int eps_HubInit( void(*eps_fatal_handler)(char *, int, char *) , void(*eps_log_handler)(char *) )
 {
  _eps_fatal = eps_fatal_handler;
  _eps_log   = eps_log_handler;
  eps_LaTeXInit();
  return 0;
 }

void eps_log(char *msg) { (*_eps_log)(msg); }

void eps_fatal(char *file, int line, char *msg) { (*_eps_fatal)(file,line,msg); }

