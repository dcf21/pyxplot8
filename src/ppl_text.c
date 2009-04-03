// ppl_text.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pyxplot.h"
#include "asciidouble.h"
#include "ppl_constants.h"

// Contains text messages which pyxplot displays

char txt_version[SSTR_LENGTH];
char txt_help[LSTR_LENGTH];
char txt_init[LSTR_LENGTH];
char txt_invalid[LSTR_LENGTH];
char txt_valid_set_options[LSTR_LENGTH];
char txt_valid_show_options[LSTR_LENGTH];
char txt_set_noword[LSTR_LENGTH];
char txt_unset_noword[LSTR_LENGTH]; 
char txt_set[LSTR_LENGTH];
char txt_unset[LSTR_LENGTH];
char txt_show[LSTR_LENGTH];

void ppl_text_init()
{
sprintf(txt_version, "\nPyXPlot %s\n", VERSION);

sprintf(txt_help   , "%s\
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
<http://www.pyxplot.org.uk>\n", txt_version, StrUnderline(txt_version), DOCDIR, PATHLINK);

sprintf(txt_init, "\n\
 ____       __  ______  _       _      PYXPLOT\n\
|  _ \\ _   _\\ \\/ /  _ \\| | ___ | |_    Version %s\n\
| |_) | | | |\\  /| |_) | |/ _ \\| __|   %s\n\
|  __/| |_| |/  \\|  __/| | (_) | |_ \n\
|_|    \\__, /_/\\_\\_|   |_|\\___/ \\__|   Copyright (C) 2006-8 Dominic Ford\n\
       |___/                                         2008   Ross Church\n\
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
'arrow', 'autoscale', 'axescolour', 'axis', 'backup', 'bar', 'boxfrom',\n\
'boxwidth', 'data style', 'display', 'dpi', 'fontsize', 'function style',\n\
'grid', 'gridmajcolour', 'gridmincolour', 'key', 'keycolumns', 'label',\n\
'linestyle', 'linewidth', 'logscale', 'multiplot', 'noarrow', 'noaxis',\n\
'nobackup', 'nodisplay', 'nogrid', 'nokey', 'nolabel', 'nolinestyle',\n\
'nologscale', 'nomultiplot', 'no<m>[xyz]<n>tics', 'notitle', 'origin',\n\
'output', 'palette', 'papersize', 'pointlinewidth', 'pointsize', 'preamble',\n\
'samples', 'size', 'size noratio', 'size ratio', 'size square', 'terminal',\n\
'textcolour', 'texthalign', 'textvalign', 'title', 'width', '[xyz]<n>label',\n\
'[xyz]<n>range', '[xyz]<n>ticdir', '<m>[xyz]<n>tics'\n\
");

sprintf(txt_valid_show_options, "\n\
'autoscale', 'axescolour', 'backup', 'bar', 'boxfrom', 'boxwidth', 'colour,\n\
'data style', 'display', 'dpi', 'fontsize', 'function style', 'grid',\n\
'gridmajcolour', 'gridmincolour', 'key', 'keycolumns', 'label', 'linestyle',\n\
'linewidth', 'logscale', 'multiplot', 'origin', 'output', 'palette',\n\
'papersize', 'pointlinewidth', 'pointsize', 'preamble', 'samples', 'size',\n\
'terminal', 'textcolour', 'texthalign', 'textvalign', 'title', 'width',\n\
'[xyz]<n>label', '[xyz]<n>range', '[xyz]<n>ticdir', '[xyz]<n>tics'\n\
");

sprintf(txt_set_noword, "\n\
Set options which PyXPlot recognises are: [] = choose one, <> = optional\n\
%s\n\
Set options from gnuplot which PyXPlot DOES NOT recognise:\n\
\n\
'angles', 'border', 'clabel', 'clip', 'cntrparam', 'colorbox', 'contour',\n\
'decimalsign', 'dgrid3d', 'dummy', 'encoding', 'format', 'hidden3d',\n\
'historysize', 'isosamples', 'locale', '[blrt]margin', 'mapping', 'mouse',\n\
'offsets', 'parametric', 'pm3d', 'polar', 'print', '[rtuv]range', 'style',\n\
'surface', 'ticscale', 'ticslevel', 'timestamp', 'timefmt', 'view',\n\
'[xyz]{2}data', '{[xyz]{2}}zeroaxis', 'zero'\n\
", txt_valid_set_options);

sprintf(txt_unset_noword, "\n\
Unset options which PyXPlot recognises are: [] = choose one, <> = optional\n\
\n\
'arrow', 'autoscale', 'axescolour', 'axis', 'backup', 'bar', 'boxfrom',\n\
'boxwidth', 'display', 'dpi', 'fontsize', 'grid', 'gridmajcolour',\n\
'gridmincolour', 'key', 'keycolumns', 'label', 'linestyle', 'linewidth',\n\
'logscale', 'multiplot', 'noarrow', 'noaxis', 'nobackup', 'nodisplay',\n\
'nogrid', 'nokey', 'nolabel', 'nolinestyle', 'nolinewidth', 'nologscale',\n\
'nomultiplot', 'no<m>[xyz]<n>tics', 'origin', 'output', 'palette', 'papersize',\n\
'pointlinewidth', 'pointsize', 'preamble', 'samples', 'size', 'terminal',\n\
'textcolour', 'texthalign', 'textvalign', 'title', 'width', '[xyz]<n>label',\n\
'[xyz]<n>range', '[xyz]<n>ticdir', '<m>[xyz]<n>tics'\n\
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
'all', 'arrows', 'axes', 'settings', 'labels', 'linestyles', 'variables',\n\
'functions'\n\
\n\
or any of the following set options:\n\
%s", txt_valid_show_options);
}

