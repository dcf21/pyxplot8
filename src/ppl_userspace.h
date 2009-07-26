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

#include "ppl_units.h"

#define PPL_USERSPACE_NUMERIC 32000
#define PPL_USERSPACE_STRING  32001

#define PPL_USERSPACE_USERDEF 32050
#define PPL_USERSPACE_SYSTEM  32051
#define PPL_USERSPACE_STRFUNC 32052
#define PPL_USERSPACE_SPLINE  32053
#define PPL_USERSPACE_UNIT    32054
#define PPL_USERSPACE_INT     32055

typedef struct FunctionDescriptor {
 int   FunctionType;
 unsigned char modified;
 int   NumberArguments;
 void *FunctionPtr;
 char *ArgList;
 value *min, *max; // Range of values over which this function definition can be used; used in function splicing
 unsigned char *MinActive, *MaxActive;
 struct FunctionDescriptor *next; // A linked list of spliced alternative function definitions
 char *LaTeX;
 char *description;
 } FunctionDescriptor;

#ifndef _PPL_USERSPACE_C
extern Dict *_ppl_UserSpace_Vars;
extern Dict *_ppl_UserSpace_Funcs;
#endif

void ppl_UserSpaceInit          ();
void ppl_UserSpace_SetVarStr    (char *name, char   *inval, int modified);
void ppl_UserSpace_SetVarNumeric(char *name, value  *inval, int modified);
void ppl_UserSpace_UnsetVar     (char *name);
void ppl_UserSpace_SetFunc      (char *definition, int modified, int *status, char *errtext);
void ppl_UserSpace_FuncDestroy  (FunctionDescriptor *in);
void ppl_UserSpace_FuncDuplicate(FunctionDescriptor *in, int modified);

void ppl_GetQuotedString(char *in, char   *out, int start, int *end, unsigned char DollarAllowed, int *errpos, char *errtext, int RecursionDepth);
void ppl_EvaluateAlgebra(char *in, value  *out, int start, int *end, unsigned char DollarAllowed, int *errpos, char *errtext, int RecursionDepth);
void ppl_GetExpression  (const char *in, int *end, int DollarAllowed, unsigned char *status, unsigned char *OpList, int *errpos, char *errtext);

#endif

