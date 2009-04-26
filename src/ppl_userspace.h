// ppl_userspace.h
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

#ifndef _PPL_USERSPACE_H
#define _PPL_USERSPACE_H 1

#include "ListTools/lt_dict.h"

void ppl_UserSpaceInit          ();
void ppl_UserSpace_SetVarStr    (char *name, char   *value);
void ppl_UserSpace_SetVarNumeric(char *name, double  value);
void ppl_UserSpace_SetFunc      (char *name, char   *value);

void ppl_GetQuotedString(char *in, char   *out, int start, int *end, Dict *Local1Vars, Dict *Local2Vars, int *errpos, char *errtext, int RecursionDepth);
void ppl_EvaluateAlgebra(char *in, value  *out, int start, int *end, Dict *Local1Vars, Dict *Local2Vars, int *errpos, char *errtext, int RecursionDepth);
void ppl_GetExpression  (char *in, int *end, int DollarAllowed, unsigned char *status, unsigned char *OpList, int *errpos, char *errtext);

#endif

