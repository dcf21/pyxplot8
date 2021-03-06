// ppl_userspace.h
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

#ifndef _PPL_USERSPACE_H
#define _PPL_USERSPACE_H 1

#include <gsl/gsl_spline.h>

#ifdef HAVE_FFTW3
#include <fftw3.h>
#else
#include <fftw.h>
#endif

#include "ListTools/lt_dict.h"

#include "ppl_units.h"

#define PPL_USERSPACE_NUMERIC 32000
#define PPL_USERSPACE_STRING  32001

#define PPL_USERSPACE_USERDEF    32050
#define PPL_USERSPACE_SYSTEM     32051
#define PPL_USERSPACE_STRFUNC    32052
#define PPL_USERSPACE_SPLINE     32053
#define PPL_USERSPACE_INTERP2D   32054
#define PPL_USERSPACE_BMPDATA    32055
#define PPL_USERSPACE_HISTOGRAM  32056
#define PPL_USERSPACE_FFT        32057
#define PPL_USERSPACE_UNIT       32058
#define PPL_USERSPACE_INT        32059
#define PPL_USERSPACE_SUBROUTINE 32060

typedef struct SplineDescriptor {
 gsl_spline       *SplineObj;
 gsl_interp_accel *accelerator;
 value             UnitX, UnitY, UnitZ;
 long              SizeX, SizeY;
 unsigned char     LogInterp;
 char             *filename, *SplineType;
 } SplineDescriptor;

typedef struct HistogramDescriptor {
 long int      Nbins;
 double       *bins;
 double       *binvals;
 unsigned char log;
 value         unit;
 char         *filename;
 } HistogramDescriptor;

typedef struct FFTDescriptor {
 int           Ndims;
 int          *XSize;
 fftw_complex *datagrid;
 value        *range, *invrange, OutputUnit;
 double        normalisation;
 } FFTDescriptor;

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
extern Dict *_ppl_UserSpace_Funcs2;
#endif

void ppl_UserSpaceInit              ();
void ppl_UserSpace_SetVarStr        (char *name, char   *inval, int modified);
void ppl_UserSpace_SetVarNumeric    (char *name, value  *inval, int modified);
void ppl_UserSpace_UnsetVar         (char *name);
void ppl_UserSpace_GetVarPointer    (char *name, value **output, value *temp);
void ppl_UserSpace_RestoreVarPointer(value **output, value *temp);
void ppl_UserSpace_SetFunc          (char *definition, int modified, int *status, char *errtext);
void ppl_UserSpace_FuncDestroy      (FunctionDescriptor *in);
void ppl_UserSpace_FuncDuplicate    (FunctionDescriptor *in, int modified);

void ppl_GetQuotedString(char *in, char   *out, int start, int *end, unsigned char DollarAllowed, int *errpos, char *errtext, int RecursionDepth);
void ppl_EvaluateAlgebra(char *in, value  *out, int start, int *end, unsigned char DollarAllowed, int *errpos, char *errtext, int RecursionDepth);
void ppl_GetExpression  (const char *in, int *end, int DollarAllowed, unsigned char *status, unsigned char *OpList, int *errpos, char *errtext);

#endif

