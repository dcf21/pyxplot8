// ppl_text.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "pyxplot.h"
#include "ppl_constants.h"

// Contains text messages which pyxplot displays

char txt_version            [SSTR_LENGTH];
char txt_version_underline  [SSTR_LENGTH];
char txt_help               [LSTR_LENGTH];
char txt_init               [LSTR_LENGTH];
char txt_invalid            [LSTR_LENGTH];
char txt_valid_set_options  [LSTR_LENGTH];
char txt_set_noword         [LSTR_LENGTH];
char txt_unset_noword       [LSTR_LENGTH];
char txt_set                [LSTR_LENGTH];
char txt_unset              [LSTR_LENGTH];
char txt_show               [LSTR_LENGTH];

void ppl_text_init()
{
sprintf(txt_version, "PyXPlot %s", VERSION);

sprintf(txt_help   , "%s\n\
%s\n\
\n\
Usage: pyxplot <options> <filelist>\n\
  -h, --help:       Display this help.\n\
  -v, --version:    Display version number.\n\
  -q, --quiet:      Turn off initial welcome message.\n\
  -V, --verbose:    Turn on initial welcome message.\n\
  -c, --colour:     Use coloured highlighting of output.\n\
  -m, --monochrome: Turn off coloured highlighting.\n\
\n\
A brief introduction to PyXPlot can be obtained by typing 'man pyxplot'; the\n\
full Users' Guide can be found in the file:\n\
%s%spyxplot.pdf\n\
\n\
For the latest information on PyXPlot development, see the project website:\n\
<http://www.pyxplot.org.uk>\n\
\n\
Please report bugs to <coders@pyxplot.org.uk>\n", txt_version, StrUnderline(txt_version, txt_version_underline), DOCDIR, PATHLINK);

sprintf(txt_init, "\n\
 ____       __  ______  _       _      PYXPLOT\n\
|  _ \\ _   _\\ \\/ /  _ \\| | ___ | |_    Version %s\n\
| |_) | | | |\\  /| |_) | |/ _ \\| __|   %s\n\
|  __/| |_| |/  \\|  __/| | (_) | |_ \n\
|_|    \\__, /_/\\_\\_|   |_|\\___/ \\__|   Copyright (C) 2006-2010 Dominic Ford\n\
       |___/                                         2008-2010 Ross Church\n\
\n\
\n\
Send comments, bug reports, feature requests and coffee supplies to:\n\
<coders@pyxplot.org.uk>\n\
", VERSION, DATE);

sprintf(txt_invalid, "\n\
 %%s\n\
/|\\ \n\
 |\n\
Error: Unrecognised command.\n\
");

sprintf(txt_valid_set_options, "\n\
'arrow', 'autoscale', 'axescolour', 'axis', 'axisunitstyle', 'backup', 'bar',\n\
'binorigin', 'binwidth', 'boxfrom', 'boxwidth', 'calendar', 'clip', 'colmap',\n\
'colkey', 'contours', 'c<n>range', 'data style', 'display', 'filter',\n\
'fontsize', 'function style', 'grid', 'gridmajcolour', 'gridmincolour', 'key',\n\
'keycolumns', 'label', 'linearscale', 'linewidth', 'logscale', 'multiplot',\n\
'noarrow', 'noaxis', 'nobackup', 'nodisplay', 'nogrid', 'nokey', 'nolabel',\n\
'nologscale', 'nomultiplot', 'nostyle', 'notitle', 'no<m>[xyz]<n>format',\n\
'no<m>[xyz]<n>tics', 'numerics', 'origin', 'output', 'palette', 'papersize',\n\
'pointlinewidth', 'pointsize', 'preamble', 'samples', 'seed', 'size', 'size\n\
noratio', 'size ratio', 'size square', 'style', 'terminal', 'textcolour',\n\
'texthalign', 'textvalign', 'title', 'trange', 'unit', 'view', 'viewer',\n\
'width', '[xyz]<n>format', '[xyz]<n>label', '[xyz]<n>range', '<m>[xyz]<n>tics'\n\
");

sprintf(txt_set_noword, "\n\
Set options which PyXPlot recognises are: [] = choose one, <> = optional\n\
%s\n\
", txt_valid_set_options);

sprintf(txt_unset_noword, "\n\
Unset options which PyXPlot recognises are: [] = choose one, <> = optional\n\
\n\
'arrow', 'autoscale', 'axescolour', 'axis', 'axisunitstyle', 'backup', 'bar',\n\
'binorigin', 'binwidth', 'boxfrom', 'boxwidth', 'calendar', 'clip', 'colmap',\n\
'colkey', 'contours', 'c<n>range', 'data style', 'display', 'filter',\n\
'fontsize', 'function style', 'grid', 'gridmajcolour', 'gridmincolour', 'key',\n\
'keycolumns', 'label', 'linewidth', 'logscale', 'multiplot', 'noarrow',\n\
'noaxis', 'nobackup', 'nodisplay', 'nogrid', 'nokey', 'nolabel', 'nologscale',\n\
'nomultiplot', 'notitle', 'no<m>[xyz]<n>tics', 'numerics', 'origin', 'output',\n\
'palette', 'papersize', 'pointlinewidth', 'pointsize', 'preamble', 'samples',\n\
'size', 'style', 'terminal', 'textcolour', 'texthalign', 'textvalign', 'title',\n\
'trange', 'unit', 'view', 'viewer', 'width', '[xyz]<n>format', '[xyz]<n>label',\n\
'[xyz]<n>range', '<m>[xyz]<n>tics'\n\
");

sprintf(txt_set, "\n\
Error: Invalid set option '%%s'.\n\
\n\
%s", txt_set_noword);

sprintf(txt_unset, "\n\
Error: Invalid unset option '%%s'.\n\
\n\
%s", txt_unset_noword);

sprintf(txt_show, "\n\
Valid 'show' options are:\n\
\n\
'all', 'arrows', 'axes', 'functions', 'settings', 'labels', 'linestyles',\n\
'units', 'userfunctions', 'variables'\n\
\n\
or any of the following set options:\n\
'arrow', 'autoscale', 'axescolour', 'axis', 'axisunitstyle', 'backup', 'bar',\n\
'binorigin', 'binwidth', 'boxfrom', 'boxwidth', 'calendar', 'clip', 'colmap',\n\
'colkey', 'contours', 'c<n>range', 'data style', 'display', 'filter',\n\
'fontsize', 'function style', 'grid', 'gridmajcolour', 'gridmincolour', 'key',\n\
'keycolumns', 'label', 'linearscale', 'linewidth', 'logscale', 'multiplot',\n\
'numerics', 'origin', 'output', 'palette', 'papersize', 'pointlinewidth',\n\
'pointsize', 'preamble', 'samples', 'seed', 'size', 'size noratio', 'size\n\
ratio', 'size square', 'style', 'terminal', 'textcolour', 'texthalign',\n\
'textvalign', 'title', 'trange', 'unit', 'view', 'viewer', 'width',\n\
'[xyz]<n>format', '[xyz]<n>label', '[xyz]<n>range', '<m>[xyz]<n>tics'\n\
");
}

