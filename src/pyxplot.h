// pyxplot.h
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

#ifndef _PYXPLOT_H
#define _PYXPLOT_H 1

#include <stdio.h>
#include <stdlib.h>

#include "ListTools/lt_dict.h"

/* Strings defined in ppl_commands.c */
extern char ppl_commands[];

/* Functions defined in ppl_shell.c */
#ifndef _PPL_SHELL_C
extern int  PPL_SHELL_EXITING;
extern char *DirectiveLinebuffer;
#endif
void        InteractiveSession  ();
void        ProcessPyXPlotScript(char *input, int IterLevel);
int         ProcessDirective    (char *in, int interactive, int IterLevel);
int         ProcessDirective2   (char *in, int interactive, int IterLevel);
int         ProcessDirective3   (char *in, Dict *command, int interactive, int memcontext, int IterLevel);
void        directive_cd        (Dict *command);
void        directive_print     (Dict *command);

/* Strings defined in ppl_text.c */
extern char txt_version[];
extern char txt_help[];
extern char txt_init[];
extern char txt_invalid[];
extern char txt_valid_set_options[];
extern char txt_valid_show_options[];
extern char txt_set_noword[];
extern char txt_unset_noword[];
extern char txt_set[];
extern char txt_unset[];
extern char txt_show[];
void   ppl_text_init   ();

/* Functions defined in pyxplot.c */

#ifndef _PYXPLOT_C
#include <setjmp.h>
extern int         WillBeInteractive;
extern sigjmp_buf  sigjmp_ToMain;
extern sigjmp_buf  sigjmp_ToInteractive;
extern sigjmp_buf  sigjmp_ToDirective;
extern sigjmp_buf *sigjmp_FromSigInt;
#endif
void               SigIntHandler(int signo);

#endif
