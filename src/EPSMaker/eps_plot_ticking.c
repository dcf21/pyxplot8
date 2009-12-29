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

#include "ListTools/lt_memory.h"

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"

#include "eps_plot_canvas.h"
#include "eps_plot_ticking.h"

void eps_plot_ticking(settings_axis *axis, int xyz, int axis_n, int canvas_id, const double *HardMin, const double *HardMax, unsigned char HardAutoMin, unsigned char HardAutoMax)
 {
  const double logmin = 1e-10;

  axis->FinalActive = axis->FinalActive || axis->enabled || (HardMin!=NULL) || (HardMax!=NULL) || (HardAutoMin) || (HardAutoMax);
  if (!axis->FinalActive) { axis->RangeFinalised = 0; return; } // Axis is not in use

  // First of all, work out what axis range to use
  if (!axis->RangeFinalised)
   {
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
    if ((axis->log == SW_BOOL_TRUE) && (axis->MinFinal <= 1e-200)) { axis->MinFinal = logmin; sprintf(temp_err_string, "Range for logarithmic axis %c%d set below zero; defaulting to 1e-10.", "xyz"[xyz], axis_n); ppl_warning(ERR_NUMERIC, temp_err_string); }
    if ((axis->log == SW_BOOL_TRUE) && (axis->MaxFinal <= 1e-200)) { axis->MaxFinal = logmin; sprintf(temp_err_string, "Range for logarithmic axis %c%d set below zero; defaulting to 1e-10.", "xyz"[xyz], axis_n); ppl_warning(ERR_NUMERIC, temp_err_string); }

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
   }

  // Secondly, decide what ticks to place on this axis
  if (!axis->TickListFinalised)
   {

    // MAJOR TICKS
    if (axis->TickList != NULL) // Ticks have been specified as an explicit list
     {
      int i,j,N;
      for (N=0; axis->TickStrs[N]!=NULL; N++); // Find length of list of ticks
      axis->TickListPositions = (double  *)lt_malloc((N+1) * sizeof(double));
      axis->TickListStrings   = (char   **)lt_malloc((N+1) * sizeof(char *));
      if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
      for (i=j=0; i<N; i++)
       {
        axis->TickListPositions[j] = eps_plot_axis_GetPosition( axis->TickList[i] , axis);
        if ( (axis->TickListPositions[j]<0.0) || (axis->TickListPositions[j]>1.0) ) continue; // Filter out ticks which are off the end of the axis
        axis->TickListStrings[j]   = axis->TickStrs[i];
        j++;
       }
      axis->TickListStrings[j] = NULL; // null terminate list
     }
    else
     {
      int i,N;
      N=4;
      axis->TickListPositions = (double  *)lt_malloc((N+1) * sizeof(double));
      axis->TickListStrings   = (char   **)lt_malloc((N+1) * sizeof(char *));
      if ((axis->TickListPositions==NULL) || (axis->TickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
      for (i=0; i<N; i++)
       {
        axis->TickListPositions[i] = eps_plot_axis_GetPosition( axis->MinFinal + (axis->MaxFinal-axis->MinFinal)*i/(N-1) , axis);
        axis->TickListStrings[i]   = (char *)lt_malloc(64);
        if (axis->TickListStrings[i]==NULL) { ppl_error(ERR_MEMORY, "Out of memory"); axis->TickListPositions = NULL; axis->TickListStrings = NULL; return; }
        sprintf(axis->TickListStrings[i],"%f",axis->MinFinal + (axis->MaxFinal-axis->MinFinal)*i/(N-1));
       }
      axis->TickListStrings[i] = NULL; // null terminate list
     }

    // MINOR TICKS
    if (axis->TickList != NULL) // Ticks have been specified as an explicit list
     {
      int i,j,N;
      for (N=0; axis->MTickStrs[N]!=NULL; N++); // Find length of list of ticks
      axis->MTickListPositions = (double  *)lt_malloc((N+1) * sizeof(double));
      axis->MTickListStrings   = (char   **)lt_malloc((N+1) * sizeof(char *));
      if ((axis->MTickListPositions==NULL) || (axis->MTickListStrings==NULL)) { ppl_error(ERR_MEMORY, "Out of memory"); axis->MTickListPositions = NULL; axis->MTickListStrings = NULL; return; }
      for (i=j=0; i<N; i++)
       {
        axis->MTickListPositions[j] = eps_plot_axis_GetPosition( axis->MTickList[i] , axis);
        if ( (axis->MTickListPositions[j]<0.0) || (axis->MTickListPositions[j]>1.0) ) continue; // Filter out ticks which are off the end of the axis
        axis->MTickListStrings[j]   = axis->MTickStrs[i];
        j++;
       }
      axis->MTickListStrings[j] = NULL; // null terminate list
     }

    // Set flag to show that we have finalised the ticking of this axis
    axis->TickListFinalised = 1;
   }

  return;
 }

