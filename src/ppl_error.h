// ppl_error.h
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

// Functions for returning messages to the user

#ifndef _PPL_ERROR_H
#define _PPL_ERROR_H 1

extern char ppl_error_source[];
extern char temp_err_string[];

void ppl_error_setstreaminfo(int linenumber,char *filename);
void ppl_error(char *msg);
void ppl_fatal(char *file, int line, char *msg);
void ppl_warning(char *msg);
void ppl_report(char *msg);
void ppl_log(char *msg);

#endif
