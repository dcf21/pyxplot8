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

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

void eps_plot_ticking(settings_axis *axis, int xyz, int axis_n, int canvas_id, const double *HardMin, const double *HardMax, unsigned char HardAutoMin, unsigned char HardAutoMax)
 {
  const double logmin = 1e-10;

  axis->FinalActive = axis->FinalActive || axis->enabled || (HardMin!=NULL) || (HardMax!=NULL) || (HardAutoMin) || (HardAutoMax);
  if (!axis->FinalActive) { axis->RangeFinalised = 0; return; } // Axis is not in use

  // Temporary fudge to work out axis range
  if       (HardMin != NULL)                               axis->MinFinal = *HardMin;
  else if ((axis->MinSet==SW_BOOL_TRUE) && (!HardAutoMin)) axis->MinFinal = axis->min;
  else if  (axis->MinUsedSet)                              axis->MinFinal = axis->MinUsed;
  else                                                     axis->MinFinal = (axis->log == SW_BOOL_TRUE) ? 1.0 : -10.0;

  if       (HardMax != NULL)                               axis->MaxFinal = *HardMax;
  else if ((axis->MaxSet==SW_BOOL_TRUE) && (!HardAutoMax)) axis->MaxFinal = axis->max;
  else if  (axis->MaxUsedSet)                              axis->MaxFinal = axis->MaxUsed;
  else                                                     axis->MaxFinal = (axis->log == SW_BOOL_TRUE) ? 10.0 : 10.0;

  // Check that log axes do not venture too close to zero
  if ((axis->log == SW_BOOL_TRUE) && (axis->MinFinal <= 1e-200)) { axis->MinFinal = logmin; ppl_warning(ERR_NUMERIC,"Range for logarithmic axis set below zero; defaulting to 1e-10."); }
  if ((axis->log == SW_BOOL_TRUE) && (axis->MaxFinal <= 1e-200)) { axis->MaxFinal = logmin; ppl_warning(ERR_NUMERIC,"Range for logarithmic axis set below zero; defaulting to 1e-10."); }

  // Print out debugging report
  if (DEBUG)
   {
    int i;
    sprintf(temp_err_string,"Determined range for axis %c%d of plot %d. Usage was [", "xyz"[xyz], axis_n, canvas_id);
    i = strlen(temp_err_string);
    if (axis->MinUsedSet) { sprintf(temp_err_string+i, "%f", axis->MinUsed); i+=strlen(temp_err_string+i); }
    else                  temp_err_string[i++] = '*';
    temp_err_string[i++] = ':';
    if (axis->MaxUsedSet) { sprintf(temp_err_string+i, "%f", axis->MaxUsed); i+=strlen(temp_err_string+i); }
    else                  temp_err_string[i++] = '*';
    sprintf(temp_err_string+i,"]. Final range was [%f:%f].",axis->MinFinal,axis->MaxFinal);
    ppl_log(temp_err_string);
   }

  // Set flag to show that we have finalised the range of this axis
  axis->RangeFinalised = 1;
  return;
 }

