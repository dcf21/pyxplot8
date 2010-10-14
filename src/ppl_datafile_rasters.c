// ppl_datafile_rasters.c
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

#define _PPL_DATAFILE_RASTERS_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

#include "ppl_error.h"
#include "ppl_settings.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

void DataFile_FromFunctions_CheckSpecialRaster(char **fnlist, int fnlist_len, char *DummyVar, double *min, double *max, double **OrdinateRaster, int *RasterLen)
 {
  int   i, j, pos, CommaPos=0, BracketLevel=0, ContainsOperator=0, ContainsDummy=0, NWords=0, OutContext, ContextRough=-1;
  int   DummyArgNo=0;
  char  NewWord=1, fail=0;
  char  c;
  char *buf = temp_err_string;
  char *CurrentFnName="", *DummyInFunction=NULL;
  List *BracketStack, *BracketCommaPos;
  FunctionDescriptor *FuncPtr;

  // Make temporary rough workspace
  OutContext      = lt_GetMemContext();
  ContextRough    = lt_DescendIntoNewContext();
  BracketStack    = ListInit();
  BracketCommaPos = ListInit();

  for (i=0; i<fnlist_len; i++)
   {
    j=pos=0;
    while (1)
     {
      c = fnlist[i][j++];
      if ((isalnum(c)) || (c=='_')) { NewWord=0; buf[pos++] = c; continue; }
      if ((!NewWord) && (pos>0)) NWords++;
      if (!NewWord) buf[pos++] = '\0';
      NewWord=1;
      pos=0;
      if ((c<=' ') && (c!='\0')) continue;
      if (strcmp(buf, DummyVar)==0)
       {
        ContainsDummy=1;
        if (ContainsOperator || (NWords>1)) fail=1;
        if ((DummyInFunction != NULL) && (CurrentFnName != NULL))
         {
          if ((strcmp(DummyInFunction, CurrentFnName)!=0) || (DummyArgNo != CommaPos)) fail=1;
         }
        if (CurrentFnName==NULL) fail=1;
        else
         {
          DummyInFunction = CurrentFnName;
          DummyArgNo      = CommaPos;
         }
       }
      if (c=='(')
       {
        if (ContainsDummy) fail=1;
        ListAppendInt(BracketCommaPos, CommaPos);
        ListAppendString(BracketStack, buf);
        BracketLevel++;
        CurrentFnName = (char *)ListLast(BracketStack);
        ContainsOperator = 0;
        ContainsDummy = 0;
        NWords = 0;
        buf[0]='\0';
        continue;
       }
      buf[0]='\0';
      if (c==')')
       {
        if (BracketLevel>0)
         {
          ListPop(BracketStack);
          CurrentFnName = (char *)ListLast(BracketStack);
          CommaPos = *(int *)ListPop(BracketCommaPos);
          ContainsOperator = 1;
          ContainsDummy = 0;
          NWords = 10;
          BracketLevel--;
         }
        continue;
       }
      if (c==',')
       {
        CommaPos++;
        ContainsOperator = 0;
        ContainsDummy = 0;
        NWords = 0;
        continue;
       }
      if (c=='\0') break;
      ContainsOperator=1;
      if (ContainsDummy) fail=1;
     }
   }

  if (fail || (DummyInFunction==NULL) || (DummyInFunction[0]=='\0')) goto CLEANUP;

  // Look up function which dummy variable is an argument to
  DictLookup(_ppl_UserSpace_Funcs, DummyInFunction, NULL, (void *)&FuncPtr);
  if (FuncPtr==NULL) goto CLEANUP;
  if ((FuncPtr->FunctionType!=PPL_USERSPACE_HISTOGRAM) && (FuncPtr->FunctionType!=PPL_USERSPACE_FFT)) goto CLEANUP;
  if (DummyArgNo >= FuncPtr->NumberArguments) goto CLEANUP;

  // Make custom raster
  if (FuncPtr->FunctionType == PPL_USERSPACE_HISTOGRAM)
   {
    HistogramDescriptor *desc = (HistogramDescriptor *)FuncPtr->FunctionPtr;
    double *OutputRaster;
    int     RasterCount=0;
    long    NInput = desc->Nbins-1;
    if (NInput<1) NInput=1;

    OutputRaster = (double *)lt_malloc_incontext(NInput*sizeof(double), OutContext);
    if (OutputRaster == NULL) goto CLEANUP;

    for (i=0; i<(desc->Nbins-1); i++)
     {
      double midpoint = desc->log?sqrt(desc->bins[i]*desc->bins[i+1])
                                 :   ((desc->bins[i]+desc->bins[i+1])/2);
      if ( ((min==NULL)||(*min<=midpoint)) && ((max==NULL)||(*max>=midpoint)) )
       { OutputRaster[ RasterCount++ ] = midpoint; }
     }

    *OrdinateRaster = OutputRaster;
    *RasterLen      = RasterCount;
   }
  else if (FuncPtr->FunctionType == PPL_USERSPACE_FFT)
   {
    FFTDescriptor *desc = (FFTDescriptor *)FuncPtr->FunctionPtr;
    double *OutputRaster;
    int     RasterCount=0;

    if (DummyArgNo >= desc->Ndims) goto CLEANUP;
    OutputRaster = (double *)lt_malloc_incontext(desc->XSize[DummyArgNo]*sizeof(double), OutContext);
    if (OutputRaster == NULL) goto CLEANUP;

    for (i=0; i<desc->XSize[DummyArgNo]; i++)
     {
      double x = (i-desc->XSize[DummyArgNo]/2.0) / desc->range[DummyArgNo].real;
      if ( ((min==NULL)||(*min<=x)) && ((max==NULL)||(*max>=x)) ) { OutputRaster[ RasterCount++ ] = x; }
     }

    *OrdinateRaster = OutputRaster;
    *RasterLen      = RasterCount;
   }

CLEANUP:
  // Delete rough workspace
  if (ContextRough>0) lt_AscendOutOfContext(ContextRough);
  return;
 }

