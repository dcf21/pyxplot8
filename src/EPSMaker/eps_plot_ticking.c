// eps_plot_ticking.c
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

#define _PPL_EPS_PLOT_TICKING_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ppl_settings.h"
#include "ppl_setting_types.h"

void eps_plot_ticking(settings_axis *axis, const double *HardMin, const double *HardMax, unsigned char HardAutoMin, unsigned char HardAutoMax)
 {
  axis->FinalActive = axis->FinalActive || axis->enabled || (HardMin!=NULL) || (HardMax!=NULL) || (HardAutoMin) || (HardAutoMax);
  if (!axis->FinalActive) { axis->RangeFinalised = 0; return; } // Axis is not in use

  // Temporary fudge to work out axis range
  if       (HardMin != NULL)                               axis->MinFinal = *HardMin;
  else if ((axis->MinSet==SW_BOOL_TRUE) && (!HardAutoMin)) axis->MinFinal = axis->min;
  else if  (axis->MinUsedSet)                              axis->MinFinal = axis->MinUsed;
  else                                                     axis->MinFinal = (axis->log == SW_ONOFF_ON) ? 1.0 : -10.0;

  if       (HardMax != NULL)                               axis->MaxFinal = *HardMax;
  else if ((axis->MaxSet==SW_BOOL_TRUE) && (!HardAutoMax)) axis->MaxFinal = axis->max;
  else if  (axis->MaxUsedSet)                              axis->MaxFinal = axis->MaxUsed;
  else                                                     axis->MaxFinal = (axis->log == SW_ONOFF_ON) ? 10.0 : 10.0;

  // Set flag to show that we have finalised the range of this axis
  axis->RangeFinalised = 1;
  return;
 }

