// eps_plot_ticking_auto2.c
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

// This file contains routines for working out the ranges of axes, and where to
// put axis ticks along them

#define _PPL_EPS_PLOT_TICKING_AUTO2_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ListTools/lt_memory.h"

#include "MathsTools/dcfmath.h"

#include "StringTools/asciidouble.h"

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "eps_plot_canvas.h"
#include "eps_plot_ticking.h"
#include "eps_plot_ticking_auto.h"
#include "eps_plot_ticking_auto2.h"

#define MAX_FACTORS 32 /* Maximum number of factors of LogBase which we consider */

// Return up to a maximum of MaxFactors factors of in. Factors are returned to array out.
//static void factorise(int in, int *out, int MaxFactors, int FactorMax, int *NFactors)
// {
//  int i,j=0,k,N=sqrt(in);
//  for (i=2; i<=N; i++) if (in%i==0) { out[j]=i; out[MaxFactors-1-j]=in/i; j++; if (j>=MaxFactors/2) break; }
//  for (i=MaxFactors-j; i<=MaxFactors-1; i++) { out[j] = out[i]; if (out[j]!=out[j-1]) j++; }
//  for (i=0; i<j; i++) if (in/out[i] <= FactorMax) break;
//  for (k=0; i<j; i++, k++) out[k] = out[i];
//  *NFactors=k;
//  return;
// }

// Main entry point for automatic ticking of axes
void eps_plot_ticking_auto2(settings_axis *axis, int xyz, double UnitMultiplier, unsigned char *AutoTicks, double length, double tick_sep_major, double tick_sep_minor)
 {
  //int    OutContext, ContextRough=-1;

  // Make temporary rough workspace
  //OutContext   = lt_GetMemContext();
  //ContextRough = lt_DescendIntoNewContext();

  eps_plot_ticking_auto(axis, xyz, UnitMultiplier, AutoTicks, length, tick_sep_major, tick_sep_minor);

  // Delete rough workspace
  //if (ContextRough>0) lt_AscendOutOfContext(ContextRough);
  return;
 }

