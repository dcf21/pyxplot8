// ppl_userspace.c
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
#include <math.h>

#include "ppl_dict.h"

#define PPL_USERSPACE_NUMERIC 32000
#define PPL_USERSPACE_STRING  32001

#define PPL_USERSPACE_USERDEF 32100
#define PPL_USERSPACE_SYSTEM  32101
#define PPL_USERSPACE_SPLINE  32102

Dict *_ppl_UserSpace_Vars;
Dict *_ppl_UserSpace_Funcs;

void ppl_UserSpaceInit()
 {
  _ppl_UserSpace_Vars  = DictInit();
  _ppl_UserSpace_Funcs = DictInit();

  DictAppendFloat(_ppl_UserSpace_Vars, "pi", PPL_USERSPACE_NUMERIC, M_PI);
  DictAppendFloat(_ppl_UserSpace_Vars, "e" , PPL_USERSPACE_NUMERIC, M_E );
  return;
 }

void ppl_UserSpace_SetVarStr(char *name, char *value)
 {
  return;
 }

void ppl_UserSpace_SetVarNumeric(char *name, double value)
 {
  return;
 }

void ppl_UserSpace_SetFunc(char *name, char *value)
 {
  return;
 }

