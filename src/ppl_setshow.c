// ppl_setshow.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

#include "StringTools/asciidouble.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"

#include "MathsTools/dcfmath.h" // Needed to set random seed

#include "EPSMaker/eps_colours.h"

#include "ppl_canvasitems.h"
#include "ppl_children.h"
#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_flowctrl.h"
#include "ppl_papersize.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"
#include "pyxplot.h"

void directive_seterror(Dict *command, int interactive)
 {
  char *tempstr;
  DictLookup(command,"set_option",NULL, (void **)&tempstr);
  if (tempstr != NULL)
   {
    if (!interactive) { sprintf(temp_err_string, "Unrecognised set option '%s'.", tempstr); ppl_error(ERR_SYNTAX, -1, -1, temp_err_string); }
    else              { sprintf(temp_err_string, txt_set                        , tempstr); ppl_error(ERR_PREFORMED, -1, -1, temp_err_string); }
   }
  else
   {
    if (!interactive) { ppl_error(ERR_SYNTAX, -1, -1, "set command detected with no set option following it."); }
    else              { ppl_error(ERR_PREFORMED, -1, -1, txt_set_noword); }
   }
 }

void directive_unseterror(Dict *command, int interactive)
 {
  char *tempstr;
  DictLookup(command,"set_option",NULL, (void **)&tempstr);
  if (tempstr != NULL)
   {
    if (!interactive) { sprintf(temp_err_string, "Unrecognised set option '%s'.", tempstr); ppl_error(ERR_SYNTAX, -1, -1, temp_err_string); }
    else              { sprintf(temp_err_string, txt_unset                      , tempstr); ppl_error(ERR_PREFORMED, -1, -1, temp_err_string); }
   }
  else
   {
    if (!interactive) { ppl_error(ERR_SYNTAX, -1, -1, "Unset command detected with no set option following it."); }
    else              { ppl_error(ERR_PREFORMED, -1, -1, txt_unset_noword); }
   }
 }

void directive_set(Dict *command)
 {
  int     i, j, k, l, m, p, pp, *EditNo, errpos, multiplier;
  long    li;
  char   *directive, *setoption;
  value   valobj, valobj2;
  value  *tempval, *tempval2, *tempval3;
  int    *tempint, ten=10;
  double *tempdbl, dbl1, dbl2;
  char   *tempstr, *tempstr2, *tempstr3, *tempstr4;
  List   *templist;
  Dict   *tempdict;
  ListIterator *listiter;
  with_words *tempstyle, ww_temp1, ww_temp2;
  canvas_item *ptr;
  settings_graph *sg;
  arrow_object  **al;
  label_object  **ll;
  settings_axis  *xa, *ya, *za, *tempaxis, *tempaxis2;

  DictLookup(command,"editno",NULL,(void **)(&EditNo));
  if (EditNo == NULL)
   {
    sg = &settings_graph_current;
    al = &arrow_list;
    ll = &label_list;
    xa = XAxes; ya = YAxes; za = ZAxes;
   }
  else
   {
    if ((*EditNo < 1) || (*EditNo>MULTIPLOT_MAXINDEX) || (canvas_items == NULL)) {sprintf(temp_err_string, "No multiplot item with index %d.", *EditNo); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return;}
    ptr = canvas_items->first;
    for (i=1; i<*EditNo; i++)
     {
      if (ptr==NULL) break;
      ptr=ptr->next;
     }
    if (ptr == NULL) { sprintf(temp_err_string, "No multiplot item with index %d.", *EditNo); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; }

    sg = &(ptr->settings);
    al = &(ptr->arrow_list);
    ll = &(ptr->label_list);
    xa = ptr->XAxes; ya = ptr->YAxes; za = ptr->ZAxes;
    if ((xa==NULL)||(ya==NULL)||(za==NULL)) { al=NULL; ll=NULL; } // Objects which do not store axes also do not store any text labels or arrows
   }

  DictLookup(command,"directive",NULL,(void **)(&directive));
  DictLookup(command,"set_option",NULL,(void **)(&setoption));

  if      ((strcmp(directive,"set")==0) && (strcmp(setoption,"arrow")==0)) /* set arrow */
   {
    if (al != NULL) arrow_add(al, command);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"arrow")==0)) /* unset arrow */
   {
    if (al != NULL) arrow_unset(al, command);
   }
  else if ((strcmp(setoption,"autoscale")==0)) /* set autoscale | unset autoscale */
   {

#define SET_AUTOSCALE_AXIS \
 { \
  if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; } \
  else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; } \
  else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; } \
  if (set && ((!SetAll) || (tempaxis->enabled))) \
   { \
    tempaxis->enabled = 1; \
    tempaxis->MinSet  = SW_BOOL_FALSE; \
    tempaxis->MaxSet  = SW_BOOL_FALSE; \
   } else { \
    tempaxis->MinSet  = tempaxis2->MinSet; \
    tempaxis->MaxSet  = tempaxis2->MaxSet; \
   } \
  tempaxis->min     = tempaxis2->min; \
  tempaxis->max     = tempaxis2->max; \
 }

    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      unsigned char set = (directive[0]=='s'), SetAll;
      DictLookup(command,"axes",NULL,(void **)&templist);
      if ((templist != NULL) && (ListLen(templist)>0))
       {
        SetAll=0;
        listiter = ListIterateInit(templist);
        while (listiter != NULL)
         {
          tempdict = (Dict *)listiter->data;
          DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
          i = (int)GetFloat(tempstr+1,NULL);
          SET_AUTOSCALE_AXIS;
          listiter = ListIterate(listiter, NULL);
         }
       }
      else
       {
        int i,j;
        SetAll=1;
        for (j=0; j<2; j++)
         {
          if      (j==1) tempstr="y";
          else if (j==2) tempstr="z";
          else           tempstr="x";
          for (i=0; i<MAX_AXES; i++) SET_AUTOSCALE_AXIS;
         }
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"axis")==0)) /* set axis */
   {
    DictLookup(command,"axes",NULL,(void **)&templist);
    listiter = ListIterateInit(templist);
    while (listiter != NULL)
     {
      tempdict = (Dict *)listiter->data;
      DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
      i = (int)GetFloat(tempstr+1,NULL);
      if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
       {
        if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
        else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
        else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
        tempaxis->enabled=1;
        DictLookup(command,"invisible",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->invisible=1;
        DictLookup(command,"visible",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->invisible=0;
        DictLookup(command,"atzero",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->atzero=1;
        DictLookup(command,"notatzero",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->atzero=0;
        DictLookup(command,"notlinked",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->linked=0;
        DictLookup(command,"linked",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->linked=1;
        DictLookup(command,"xorient",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL)
         {
          if (tempstr[0]!='x') ppl_warning(ERR_SYNTAX, "Can only specify the positions 'top' or 'bottom' for x axes.");
          else                 tempaxis->topbottom=(strcmp(tempstr2,"on")==0);
         }
        DictLookup(command,"yorient",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL)
         {
          if (tempstr[0]!='y') ppl_warning(ERR_SYNTAX, "Can only specify the positions 'left' and 'right' for y axes.");
          else                 tempaxis->topbottom=(strcmp(tempstr2,"on")==0);
         }
        DictLookup(command,"zorient",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL)
         {
          if (tempstr[0]!='z') ppl_warning(ERR_SYNTAX, "Can only specify the positions 'front' and 'back' for z axes.");
          else                 tempaxis->topbottom=(strcmp(tempstr2,"on")==0);
         }
        DictLookup(command,"mirror",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->MirrorType = FetchSettingByName(tempstr2, SW_AXISMIRROR_INT, SW_AXISMIRROR_STR);
        DictLookup(command,"axisdisp",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL) tempaxis->ArrowType  = FetchSettingByName(tempstr2, SW_AXISDISP_INT, SW_AXISDISP_STR);
        DictLookup(command,"linkaxis",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL)
         {
          j = (int)GetFloat(tempstr2+1,NULL);
          tempaxis->LinkedAxisCanvasID = -1;
          if (tempaxis->linkusing != NULL) { if (tempaxis->linkusing!=tempaxis2->linkusing) free(tempaxis->linkusing); tempaxis->linkusing = NULL; }
          tempaxis->LinkedAxisToNum    = j;
          if      (tempstr2[0]=='y') { tempaxis->LinkedAxisToXYZ = 1; }
          else if (tempstr2[0]=='z') { tempaxis->LinkedAxisToXYZ = 2; }
          else                       { tempaxis->LinkedAxisToXYZ = 0; }
         }
        DictLookup(command,"linktoid",NULL,(void **)&tempint);
        if (tempint != NULL) tempaxis->LinkedAxisCanvasID = *tempint;
        DictLookup(command,"usingexp",NULL,(void **)&tempstr2);
        if (tempstr2 != NULL)
         {
          tempaxis->linkusing = (char *)malloc(strlen(tempstr2)+1);
          if (tempaxis->linkusing == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory."); return; }
          strcpy(tempaxis->linkusing, tempstr2);
         }
        listiter = ListIterate(listiter, NULL);
       }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"axis")==0)) /* unset axis */
   {
    DictLookup(command,"axes",NULL,(void **)&templist);
    listiter = ListIterateInit(templist);
    while (listiter != NULL)
     {
      tempdict = (Dict *)listiter->data;
      DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
      i = (int)GetFloat(tempstr+1,NULL);
      if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
       {
        if      (tempstr[0]=='y') { DestroyAxis( &(ya[i]) ); CopyAxis(&(ya[i]), &(YAxesDefault[i])); }
        else if (tempstr[0]=='z') { DestroyAxis( &(za[i]) ); CopyAxis(&(za[i]), &(ZAxesDefault[i])); }
        else                      { DestroyAxis( &(xa[i]) ); CopyAxis(&(xa[i]), &(XAxesDefault[i])); }
       }
      listiter = ListIterate(listiter, NULL);
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"axisunitstyle")==0)) /* set axisunitstyle */
   {
    DictLookup(command,"unitstyle",NULL,(void **)&tempstr);
    sg->AxisUnitStyle = FetchSettingByName(tempstr, SW_AXISUNITSTY_INT, SW_AXISUNITSTY_STR);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"axisunitstyle")==0)) /* unset axisunitstyle */
   {
    sg->AxisUnitStyle = settings_graph_default.AxisUnitStyle;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"backup")==0)) /* set backup */
   {
    settings_term_current.backup = SW_ONOFF_ON;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"backup")==0)) /* unset backup */
   {
    settings_term_current.backup = settings_term_default.backup;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"bar")==0)) /* set bar */
   {
    DictLookup(command,"bar_size"      ,NULL,(void **)&tempdbl );
    DictLookup(command,"bar_size_large",NULL,(void **)&tempstr );
    DictLookup(command,"bar_size_small",NULL,(void **)&tempstr2);

    if     ((tempdbl  != NULL) && (!gsl_finite(*tempdbl))) { ppl_error(ERR_NUMERIC, -1, -1, "The bar size specified in the 'set bar' command was not finite."); return; }

    if      (tempdbl  != NULL)  sg->bar = *tempdbl;
    else if (tempstr  != NULL)  sg->bar = 1.0;
    else if (tempstr2 != NULL)  sg->bar = 0.0;
    else                        sg->bar = 1.0;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"bar")==0)) /* unset bar */
   {
    sg->bar = settings_graph_default.bar;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"binorigin")==0)) /* set binorigin */
   {
    DictLookup(command,"auto",NULL,(void **)&tempval);
    if (tempval!=NULL)
     {
      settings_term_current.BinOriginAuto = 1;
     } else {
      DictLookup(command,"bin_origin",NULL,(void **)&tempval);
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The specified bin origin was not finite."); return; }
      settings_term_current.BinOrigin = *tempval;
      settings_term_current.BinOriginAuto = 0;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"binorigin")==0)) /* unset binorigin */
   {
    settings_term_current.BinOrigin     = settings_term_default.BinOrigin;
    settings_term_current.BinOriginAuto = settings_term_default.BinOriginAuto;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"binwidth")==0)) /* set binwidth */
   {
    DictLookup(command,"auto",NULL,(void **)&tempval);
    if (tempval!=NULL) 
     { 
      settings_term_current.BinWidthAuto = 1;
     } else {
      DictLookup(command,"bin_width",NULL,(void **)&tempval);
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The specified bin width was not finite."); return; }
      if (tempval->real <= 0.0) { ppl_error(ERR_GENERAL, -1, -1, "Width of histogram bins must be greater than zero."); return; }
      settings_term_current.BinWidth = *tempval;
      settings_term_current.BinWidthAuto = 0;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"binwidth")==0)) /* unset binwidth */
   {
    settings_term_current.BinWidth     = settings_term_default.BinWidth;
    settings_term_current.BinWidthAuto = settings_term_default.BinWidthAuto;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"boxfrom")==0)) /* set boxfrom */
   {
    DictLookup(command,"auto",NULL,(void **)&tempval);
    if (tempval!=NULL) 
     { 
      sg->BoxFromAuto = 1;
     } else {
      DictLookup(command,"box_from",NULL,(void **)&tempval);
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The value supplied to the 'set boxfrom' command was not finite."); return; }
      sg->BoxFrom = *tempval;
      sg->BoxFromAuto = 0;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"boxfrom")==0)) /* unset boxfrom */
   {
    sg->BoxFrom     = settings_graph_default.BoxFrom;
    sg->BoxFromAuto = settings_graph_default.BoxFromAuto;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"boxwidth")==0)) /* set boxwidth */
   {
    DictLookup(command,"auto",NULL,(void **)&tempval);
    if (tempval!=NULL)
     {
      sg->BoxWidthAuto = 1;
     } else {
      DictLookup(command,"box_width",NULL,(void **)&tempval);
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The width supplied to the 'set boxwidth' command was not finite."); return; }
      sg->BoxWidth = *tempval;
      sg->BoxWidthAuto = 0;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"boxwidth")==0)) /* unset boxwidth */
   {
    sg->BoxWidth     = settings_graph_default.BoxWidth;
    sg->BoxWidthAuto = settings_graph_default.BoxWidthAuto;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"calendar")==0)) /* set calendar */
   {
    DictLookup(command,"calendar"   ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.CalendarIn  =
                         settings_term_current.CalendarOut = FetchSettingByName(tempstr, SW_CALENDAR_INT, SW_CALENDAR_STR);
    DictLookup(command,"calendarin" ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.CalendarIn  = FetchSettingByName(tempstr, SW_CALENDAR_INT, SW_CALENDAR_STR);
    DictLookup(command,"calendarout",NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.CalendarOut = FetchSettingByName(tempstr, SW_CALENDAR_INT, SW_CALENDAR_STR);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"calendar")==0)) /* unset calendar */
   {
    settings_term_current.CalendarIn  = settings_term_default.CalendarIn;
    settings_term_current.CalendarOut = settings_term_default.CalendarOut;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"clip")==0)) /* set clip */
   {
    sg->clip = SW_ONOFF_ON;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"clip")==0)) /* unset clip */
   {
    sg->clip = settings_graph_default.clip;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"display")==0)) /* set display */
   {
    settings_term_current.display = SW_ONOFF_ON;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"display")==0)) /* unset display */
   {
    settings_term_current.display = settings_term_default.display;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"filter")==0)) /* set filter */
   {
    DictLookup(command,"filename",NULL,(void **)&tempstr);
    DictLookup(command,"filter",NULL,(void **)&tempstr2);
    ppl_units_zero(&valobj);
    valobj.string = tempstr2;
    valobj.modified = 1;
    DictAppendValue(settings_filters,tempstr,valobj);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"filter")==0)) /* unset filter */
   {
    DictLookup(command,"filename",NULL,(void **)&tempstr);
    DictLookup(settings_filters,tempstr,NULL,(void **)&tempstr2);
    if (tempstr2 == NULL) { ppl_warning(ERR_GENERAL, "Attempt to unset a filter which did not exist."); return; }
    DictRemoveKey(settings_filters,tempstr);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"fontsize")==0)) /* set fontsize */
   {
    DictLookup(command,"fontsize",NULL,(void **)&tempdbl);
    if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The value supplied to the 'set fontsize' command was not finite."); return; }
    if (*tempdbl <= 0.0) { ppl_error(ERR_GENERAL, -1, -1, "Font sizes are not allowed to be less than or equal to zero."); return; }
    sg->FontSize = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"fontsize")==0)) /* unset fontsize */
   {
    sg->FontSize = settings_graph_default.FontSize;
   }
  else if ((strcmp(setoption,"axescolour")==0) || (strcmp(setoption,"gridmajcolour")==0) || (strcmp(setoption,"gridmincolour")==0) || (strcmp(setoption,"textcolour")==0)) /* set axescolour | set gridmajcolour | set gridmincolour */
   {
    if (strcmp(directive,"unset")==0)
     {
      if (strcmp(setoption,"axescolour"   )==0) { sg->AxesColour=settings_graph_default.AxesColour; sg->AxesCol1234Space=settings_graph_default.AxesCol1234Space; sg->AxesColour1=settings_graph_default.AxesColour1; sg->AxesColour2=settings_graph_default.AxesColour2; sg->AxesColour3=settings_graph_default.AxesColour3; sg->AxesColour4=settings_graph_default.AxesColour4; }
      if (strcmp(setoption,"gridmajcolour")==0) { sg->GridMajColour=settings_graph_default.GridMajColour; sg->GridMajCol1234Space=settings_graph_default.GridMajCol1234Space; sg->GridMajColour1=settings_graph_default.GridMajColour1; sg->GridMajColour2=settings_graph_default.GridMajColour2; sg->GridMajColour3=settings_graph_default.GridMajColour3; sg->GridMajColour4=settings_graph_default.GridMajColour4; }
      if (strcmp(setoption,"gridmincolour")==0) { sg->GridMinColour=settings_graph_default.GridMinColour; sg->GridMinCol1234Space=settings_graph_default.GridMinCol1234Space; sg->GridMinColour1=settings_graph_default.GridMinColour1; sg->GridMinColour2=settings_graph_default.GridMinColour2; sg->GridMinColour3=settings_graph_default.GridMinColour3; sg->GridMinColour4=settings_graph_default.GridMinColour4; }
      if (strcmp(setoption,"textcolour"   )==0) { sg->TextColour=settings_graph_default.TextColour; sg->TextCol1234Space=settings_graph_default.TextCol1234Space; sg->TextColour1=settings_graph_default.TextColour1; sg->TextColour2=settings_graph_default.TextColour2; sg->TextColour3=settings_graph_default.TextColour3; sg->TextColour4=settings_graph_default.TextColour4; }
     } else {
      if (strcmp(setoption,"axescolour"   )==0) colour_fromdict(command,"",&sg->AxesColour   ,&sg->AxesCol1234Space   ,&sg->AxesColour1   ,&sg->AxesColour2   ,&sg->AxesColour3   ,&sg->AxesColour4   ,NULL,NULL,NULL,NULL,NULL,NULL,&errpos,1);
      if (strcmp(setoption,"gridmajcolour")==0) colour_fromdict(command,"",&sg->GridMajColour,&sg->GridMajCol1234Space,&sg->GridMajColour1,&sg->GridMajColour2,&sg->GridMajColour3,&sg->GridMajColour4,NULL,NULL,NULL,NULL,NULL,NULL,&errpos,1);
      if (strcmp(setoption,"gridmincolour")==0) colour_fromdict(command,"",&sg->GridMinColour,&sg->GridMinCol1234Space,&sg->GridMinColour1,&sg->GridMinColour2,&sg->GridMinColour3,&sg->GridMinColour4,NULL,NULL,NULL,NULL,NULL,NULL,&errpos,1);
      if (strcmp(setoption,"textcolour"   )==0) colour_fromdict(command,"",&sg->TextColour   ,&sg->TextCol1234Space   ,&sg->TextColour1   ,&sg->TextColour2   ,&sg->TextColour3   ,&sg->TextColour4   ,NULL,NULL,NULL,NULL,NULL,NULL,&errpos,1);
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"grid")==0)) /* set grid */
   {
    DictLookup(command,"axes",NULL,(void **)&templist);
    if (ListLen(templist)==0)
     {
      if (sg->grid != SW_ONOFF_ON)
       {
        for (i=0; i<MAX_AXES; i++) sg->GridAxisX[i] = settings_graph_default.GridAxisX[i];
        for (i=0; i<MAX_AXES; i++) sg->GridAxisY[i] = settings_graph_default.GridAxisY[i];
        for (i=0; i<MAX_AXES; i++) sg->GridAxisZ[i] = settings_graph_default.GridAxisZ[i];
       }
      sg->grid = SW_ONOFF_ON;
     } else {
      if (sg->grid != SW_ONOFF_ON)
       {
        for (i=0; i<MAX_AXES; i++) sg->GridAxisX[i] = 0;
        for (i=0; i<MAX_AXES; i++) sg->GridAxisY[i] = 0;
        for (i=0; i<MAX_AXES; i++) sg->GridAxisZ[i] = 0;
       }
      sg->grid = SW_ONOFF_ON;
      listiter = ListIterateInit(templist);
      while (listiter != NULL)
       {   
        tempdict = (Dict *)listiter->data;
        DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
        i = (int)GetFloat(tempstr+1,NULL); 
        if      (tempstr[0]=='y') { sg->GridAxisY[i] = 1; }
        else if (tempstr[0]=='z') { sg->GridAxisZ[i] = 1; }
        else                      { sg->GridAxisX[i] = 1; }
        listiter = ListIterate(listiter, NULL);
       }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"grid")==0)) /* unset grid */
   {
    sg->grid = settings_graph_default.grid;
    for (i=0; i<MAX_AXES; i++) sg->GridAxisX[i] = settings_graph_default.GridAxisX[i];
    for (i=0; i<MAX_AXES; i++) sg->GridAxisY[i] = settings_graph_default.GridAxisY[i];
    for (i=0; i<MAX_AXES; i++) sg->GridAxisZ[i] = settings_graph_default.GridAxisZ[i];
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"key")==0)) /* set key */
   {
    sg->key = SW_ONOFF_ON; // Turn key on
    DictLookup(command,"x_offset",NULL,(void **)&tempval); // Horizontal offset
    if (tempval != NULL)
     {
      if (!(tempval->dimensionless))
       { 
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval->exponent[i] != (i==UNIT_LENGTH))
          { 
           sprintf(temp_err_string, "The horizontal offset supplied to the 'set key' command must have dimensions of length. Supplied input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       }
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The horizontal offset supplied to the 'set key' command was not finite."); return; }
      sg->KeyXOff.real = tempval->real;
     }
    DictLookup(command,"y_offset",NULL,(void **)&tempval); // Vertical offset
    if (tempval != NULL)
     {
      if (!(tempval->dimensionless))
       {
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "The vertical offset supplied to the 'set key' command must have dimensions of length. Supplied input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       }
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The vertical offset supplied to the 'set key' command was not finite."); return; }
      sg->KeyYOff.real = tempval->real;
     }

    // Now work out position of key
    DictLookup(command,"pos",NULL,(void **)&tempstr);
    if (tempstr != NULL)
     { sg->KeyPos = FetchSettingByName(tempstr, SW_KEYPOS_INT, SW_KEYPOS_STR); }
    else
     {
      DictLookup(command,"xpos",NULL,(void **)&tempstr);
      if (tempstr != NULL)
       {
        if (strcmp(tempstr,"left")==0)
         {
          if      ((sg->KeyPos==SW_KEYPOS_TR)||(sg->KeyPos==SW_KEYPOS_TM)||(sg->KeyPos==SW_KEYPOS_TL)) sg->KeyPos=SW_KEYPOS_TL;
          else if ((sg->KeyPos==SW_KEYPOS_BR)||(sg->KeyPos==SW_KEYPOS_BM)||(sg->KeyPos==SW_KEYPOS_BL)) sg->KeyPos=SW_KEYPOS_BL;
          else                                                                                         sg->KeyPos=SW_KEYPOS_ML;
         }
        if (strcmp(tempstr,"xcentre")==0)
         {
          if      ((sg->KeyPos==SW_KEYPOS_TR)||(sg->KeyPos==SW_KEYPOS_TM)||(sg->KeyPos==SW_KEYPOS_TL)) sg->KeyPos=SW_KEYPOS_TM;
          else if ((sg->KeyPos==SW_KEYPOS_BR)||(sg->KeyPos==SW_KEYPOS_BM)||(sg->KeyPos==SW_KEYPOS_BL)) sg->KeyPos=SW_KEYPOS_BM;
          else                                                                                         sg->KeyPos=SW_KEYPOS_MM;
         }
        if (strcmp(tempstr,"right")==0)
         {
          if      ((sg->KeyPos==SW_KEYPOS_TR)||(sg->KeyPos==SW_KEYPOS_TM)||(sg->KeyPos==SW_KEYPOS_TL)) sg->KeyPos=SW_KEYPOS_TR;
          else if ((sg->KeyPos==SW_KEYPOS_BR)||(sg->KeyPos==SW_KEYPOS_BM)||(sg->KeyPos==SW_KEYPOS_BL)) sg->KeyPos=SW_KEYPOS_BR;
          else                                                                                         sg->KeyPos=SW_KEYPOS_MR;
         }
       }
      DictLookup(command,"ypos",NULL,(void **)&tempstr);
      if (tempstr != NULL)
       {
        if (strcmp(tempstr,"top")==0)
         {
          if      ((sg->KeyPos==SW_KEYPOS_TL)||(sg->KeyPos==SW_KEYPOS_ML)||(sg->KeyPos==SW_KEYPOS_BL)) sg->KeyPos=SW_KEYPOS_TL;
          else if ((sg->KeyPos==SW_KEYPOS_TR)||(sg->KeyPos==SW_KEYPOS_MR)||(sg->KeyPos==SW_KEYPOS_BR)) sg->KeyPos=SW_KEYPOS_TR;
          else                                                                                         sg->KeyPos=SW_KEYPOS_TM;
         }
        if (strcmp(tempstr,"ycentre")==0)
         {
          if      ((sg->KeyPos==SW_KEYPOS_TL)||(sg->KeyPos==SW_KEYPOS_ML)||(sg->KeyPos==SW_KEYPOS_BL)) sg->KeyPos=SW_KEYPOS_ML;
          else if ((sg->KeyPos==SW_KEYPOS_TR)||(sg->KeyPos==SW_KEYPOS_MR)||(sg->KeyPos==SW_KEYPOS_BR)) sg->KeyPos=SW_KEYPOS_MR;
          else                                                                                         sg->KeyPos=SW_KEYPOS_MM;
         }
        if (strcmp(tempstr,"bottom")==0)
         {
          if      ((sg->KeyPos==SW_KEYPOS_TL)||(sg->KeyPos==SW_KEYPOS_ML)||(sg->KeyPos==SW_KEYPOS_BL)) sg->KeyPos=SW_KEYPOS_BL;
          else if ((sg->KeyPos==SW_KEYPOS_TR)||(sg->KeyPos==SW_KEYPOS_MR)||(sg->KeyPos==SW_KEYPOS_BR)) sg->KeyPos=SW_KEYPOS_BR;
          else                                                                                         sg->KeyPos=SW_KEYPOS_BM;
         }
       }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"key")==0)) /* unset key */
   {
    sg->key     = settings_graph_default.key;
    sg->KeyPos  = settings_graph_default.KeyPos;
    sg->KeyXOff = settings_graph_default.KeyXOff;
    sg->KeyYOff = settings_graph_default.KeyYOff;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"keycolumns")==0)) /* set keycolumns */
   {
    DictLookup(command,"key_columns",NULL,(void **)&tempint);
    if (tempint == NULL) sg->KeyColumns = 0; // automatic columns
    else
     {
      if (*tempint <= 0.0) { ppl_error(ERR_GENERAL, -1, -1, "Keys cannot have fewer than one columns."); return; }
      sg->KeyColumns = *tempint;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"keycolumns")==0)) /* unset keycolumns */
   {
    sg->KeyColumns = settings_graph_default.KeyColumns;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"label")==0)) /* set label */
   {
    if (ll != NULL) label_add(ll, command);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"label")==0)) /* unset label */
   {
    if (ll != NULL) label_unset(ll, command);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"linewidth")==0)) /* set linewidth */
   {
    DictLookup(command,"linewidth",NULL,(void **)&tempdbl);
    if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The value supplied to the 'set linewidth' command was not finite."); return; }
    if (*tempdbl <= 0.0) { ppl_error(ERR_GENERAL, -1, -1, "Line widths are not allowed to be less than or equal to zero."); return; }
    sg->LineWidth = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"linewidth")==0)) /* unset linewidth */
   {
    sg->LineWidth = settings_graph_default.LineWidth;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"logscale")==0))         /* set logscale */
   {
    DictLookup(command,"base",NULL,(void **)&tempint);
    if (tempint!=NULL)
     {
      if ((*tempint < 2) || (*tempint > 1024))
       {
        sprintf(temp_err_string, "Attempt to use log axis with base %d. PyXPlot only supports bases in the range 2 - 1024. Defaulting to base 10.", *tempint);
        ppl_warning(ERR_GENERAL, temp_err_string);
        tempint = &ten;
       }
     }
    DictLookup(command,"axes",NULL,(void **)&templist);
    if (templist != NULL)
     {
      listiter = ListIterateInit(templist);
      while (listiter != NULL)
       {
        tempdict = (Dict *)listiter->data;
        DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
        if (tempstr != NULL)
         {
          i = (int)GetFloat(tempstr+1,NULL);
          if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
           {
            if      (tempstr[0]=='y') { tempaxis = &ya[i]; }
            else if (tempstr[0]=='z') { tempaxis = &za[i]; }
            else                      { tempaxis = &xa[i]; }
            tempaxis->enabled = 1;
            tempaxis->log     = SW_BOOL_TRUE;
            if (tempint!=NULL) tempaxis->LogBase = (double)(*tempint);
           }
         } else {
          sg->Tlog = SW_BOOL_TRUE; // No concept of log base on T axis as it's never drawn as an axis with ticks
         }
        listiter = ListIterate(listiter, NULL);
       } 
     }
    else
     {
      sg->Tlog = SW_BOOL_TRUE;
      for (i=0;i<MAX_AXES;i++) { tempaxis=&xa[i]; if (tempaxis->enabled) { tempaxis->log=SW_BOOL_TRUE; if (tempint!=NULL) tempaxis->LogBase=(double)(*tempint); } }
      for (i=0;i<MAX_AXES;i++) { tempaxis=&ya[i]; if (tempaxis->enabled) { tempaxis->log=SW_BOOL_TRUE; if (tempint!=NULL) tempaxis->LogBase=(double)(*tempint); } }
      for (i=0;i<MAX_AXES;i++) { tempaxis=&za[i]; if (tempaxis->enabled) { tempaxis->log=SW_BOOL_TRUE; if (tempint!=NULL) tempaxis->LogBase=(double)(*tempint); } }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"logscale")==0)) /* unset logscale */
   {
    DictLookup(command,"axes",NULL,(void **)&templist);
    if (templist != NULL)
     {
      listiter = ListIterateInit(templist);
      while (listiter != NULL)
       {
        tempdict = (Dict *)listiter->data;
        DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
        if (tempstr != NULL)
         {
          i = (int)GetFloat(tempstr+1,NULL);
          if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
           {
            if      (tempstr[0]=='y') { ya[i].log = YAxesDefault[i].log; ya[i].LogBase = YAxesDefault[i].LogBase; }
            else if (tempstr[0]=='z') { za[i].log = ZAxesDefault[i].log; za[i].LogBase = ZAxesDefault[i].LogBase; }
            else                      { xa[i].log = XAxesDefault[i].log; xa[i].LogBase = XAxesDefault[i].LogBase; }
           }
         } else {
          sg->Tlog = settings_graph_default.Tlog;
         }
        listiter = ListIterate(listiter, NULL);
       }
     } 
    else 
     {
      sg->Tlog = settings_graph_default.Tlog;
      for (i=0;i<MAX_AXES;i++) { tempaxis=&xa[i]; tempaxis->log=XAxesDefault[i].log; tempaxis->LogBase=XAxesDefault[i].LogBase; }
      for (i=0;i<MAX_AXES;i++) { tempaxis=&ya[i]; tempaxis->log=YAxesDefault[i].log; tempaxis->LogBase=YAxesDefault[i].LogBase; }
      for (i=0;i<MAX_AXES;i++) { tempaxis=&za[i]; tempaxis->log=ZAxesDefault[i].log; tempaxis->LogBase=ZAxesDefault[i].LogBase; }
     } 
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"multiplot")==0)) /* set multiplot */
   {
    settings_term_current.multiplot = SW_ONOFF_ON;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"multiplot")==0)) /* unset multiplot */
   {
    if ((settings_term_default.multiplot == SW_ONOFF_OFF) && (settings_term_current.multiplot == SW_ONOFF_ON)) directive_clear();
    settings_term_current.multiplot = settings_term_default.multiplot;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"noarrow")==0)) /* set noarrow */
   {
    if (al != NULL) arrow_remove(al, command);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nobackup")==0)) /* set nobackup */
   {
    settings_term_current.backup = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"noclip")==0)) /* set noclip */
   {
    sg->clip = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nodisplay")==0)) /* set nodisplay */
   {
    settings_term_current.display = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nogrid")==0)) /* set nogrid */
   {
    DictLookup(command,"axes",NULL,(void **)&templist);
    if (ListLen(templist)==0)
     {
      sg->grid = SW_ONOFF_OFF;
     } else {
      listiter = ListIterateInit(templist);
      while (listiter != NULL)
       {   
        tempdict = (Dict *)listiter->data;
        DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
        i = (int)GetFloat(tempstr+1,NULL);
        if      (tempstr[0]=='y') { sg->GridAxisY[i] = 0; }
        else if (tempstr[0]=='z') { sg->GridAxisZ[i] = 0; }
        else                      { sg->GridAxisX[i] = 0; }
        listiter = ListIterate(listiter, NULL);
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nokey")==0)) /* set nokey */
   {
    sg->key = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nolabel")==0)) /* set nolabel */
   {
    if (ll != NULL) label_remove(ll, command);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nologscale")==0)) /* set nologscale */
   {
    DictLookup(command,"axes",NULL,(void **)&templist);
    if (templist != NULL)
     {
      listiter = ListIterateInit(templist);
      while (listiter != NULL)
       {
        tempdict = (Dict *)listiter->data;
        DictLookup(tempdict,"axis",NULL,(void **)&tempstr);
        if (tempstr != NULL)
         {
          i = (int)GetFloat(tempstr+1,NULL);
          if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
           {
            if      (tempstr[0]=='y') { ya[i].enabled=1; ya[i].log = SW_BOOL_FALSE; }
            else if (tempstr[0]=='z') { za[i].enabled=1; za[i].log = SW_BOOL_FALSE; }
            else                      { xa[i].enabled=1; xa[i].log = SW_BOOL_FALSE; }
           }
         } else {
          sg->Tlog = SW_BOOL_FALSE;
         }
        listiter = ListIterate(listiter, NULL);
       }
     }
    else
     {
      sg->Tlog = SW_BOOL_FALSE;
      for (i=0;i<MAX_AXES;i++) { tempaxis=&xa[i]; if (tempaxis->enabled) { tempaxis->log=SW_BOOL_FALSE; } }
      for (i=0;i<MAX_AXES;i++) { tempaxis=&ya[i]; if (tempaxis->enabled) { tempaxis->log=SW_BOOL_FALSE; } }
      for (i=0;i<MAX_AXES;i++) { tempaxis=&za[i]; if (tempaxis->enabled) { tempaxis->log=SW_BOOL_FALSE; } }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nomultiplot")==0)) /* set nomultiplot */
   {
    if (settings_term_current.multiplot != SW_ONOFF_OFF) directive_clear();
    settings_term_current.multiplot = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"notics")==0)) /* set notics */
   {

#define SET_NOTICS \
 { \
      DictLookup(command,"minor",NULL,(void **)&tempstr2); \
      if (tempstr2==NULL) \
       { \
        tempaxis->TickMin      = tempaxis2->TickMin; \
        tempaxis->TickMax      = tempaxis2->TickMax; \
        tempaxis->TickStep     = tempaxis2->TickStep; \
        tempaxis->TickMinSet   = 0; \
        tempaxis->TickMaxSet   = 0; \
        tempaxis->TickStepSet  = 0; \
        if (tempaxis-> TickList != NULL) { free(tempaxis-> TickList); tempaxis-> TickList = NULL; } /* Delete old explicit tick lists */ \
        if (tempaxis-> TickStrs != NULL) \
         { \
          for (i=0; tempaxis->TickStrs[i]!=NULL; i++) free(tempaxis->TickStrs[i]); \
          free(tempaxis->TickStrs ); \
          tempaxis->TickStrs  = NULL; \
         } \
        tempaxis->TickList = malloc(sizeof(double)); \
        tempaxis->TickStrs = malloc(sizeof(char *)); \
        if (tempaxis->TickStrs!=NULL) *(tempaxis->TickStrs) = NULL; \
       } else { \
        tempaxis->MTickMin     = tempaxis2->MTickMin; \
        tempaxis->MTickMax     = tempaxis2->MTickMax; \
        tempaxis->MTickStep    = tempaxis2->MTickStep; \
        tempaxis->MTickMinSet  = 0; \
        tempaxis->MTickMaxSet  = 0; \
        tempaxis->MTickStepSet = 0; \
        if (tempaxis->MTickList != NULL) { free(tempaxis->MTickList); tempaxis->MTickList = NULL; } \
        if (tempaxis->MTickStrs != NULL) \
         { \
          for (i=0; tempaxis->MTickStrs[i]!=NULL; i++) free(tempaxis->MTickStrs[i]); \
          free(tempaxis->MTickStrs ); \
          tempaxis->MTickStrs  = NULL; \
         } \
        tempaxis->MTickList = malloc(sizeof(double)); \
        tempaxis->MTickStrs = malloc(sizeof(char *)); \
        if (tempaxis->MTickStrs!=NULL) *(tempaxis->MTickStrs) = NULL; \
       } \
 }

    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      DictLookup(command,"axis",NULL,(void **)&tempstr);
      if (tempstr != NULL)
       {
        i = (int)GetFloat(tempstr+1,NULL);
        if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
        else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
        else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
        SET_NOTICS;
       }
      else
       {
        for (i=0; i<MAX_AXES; i++) { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; if (tempaxis->enabled) { SET_NOTICS; } }
        for (i=0; i<MAX_AXES; i++) { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; if (tempaxis->enabled) { SET_NOTICS; } }
        for (i=0; i<MAX_AXES; i++) { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; if (tempaxis->enabled) { SET_NOTICS; } }
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"numerics")==0)) /* set numerics */
   {
    DictLookup(command,"complex", NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.ComplexNumbers     = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"errortype", NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.ExplicitErrors     = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"number_significant_figures", NULL,(void **)&tempint);
    if (tempint != NULL)
     {
      if (*tempint <  1) { ppl_error(ERR_GENERAL, -1, -1, "Numbers cannot be displayed to fewer than one significant figure."); return; }
      if (*tempint > 30) { ppl_error(ERR_GENERAL, -1, -1, "It is not sensible to try to display numbers to more than 30 significant figures. Calculations in PyXPlot are only accurate to double precision."); return; }
      settings_term_current.SignificantFigures = *tempint;
     }
    DictLookup(command,"display", NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.NumDisplay = FetchSettingByName(tempstr, SW_DISPLAY_INT, SW_DISPLAY_STR);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"numerics")==0)) /* unset numerics */
   {
    settings_term_current.ComplexNumbers     = settings_term_default.ComplexNumbers;
    settings_term_current.ExplicitErrors     = settings_term_default.ExplicitErrors;
    settings_term_current.NumDisplay         = settings_term_default.NumDisplay;
    settings_term_current.SignificantFigures = settings_term_default.SignificantFigures;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"numerics_sigfig")==0)) /* unset numerics sigfig */
   {
    settings_term_current.SignificantFigures = settings_term_default.SignificantFigures;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"numerics_errors")==0)) /* unset numerics errors */
   {
    settings_term_current.ExplicitErrors     = settings_term_default.ExplicitErrors;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"numerics_sigfig")==0)) /* unset numerics complex */
   {
    settings_term_current.ComplexNumbers     = settings_term_default.ComplexNumbers;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"numerics_display")==0)) /* unset numerics display */
   {
    settings_term_current.NumDisplay         = settings_term_default.NumDisplay;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"origin")==0)) /* set origin */
   {
    DictLookup(command,"x_origin",NULL,(void **)&tempval);
    DictLookup(command,"y_origin",NULL,(void **)&tempval2);
    if (!(tempval->dimensionless))
     {
      for (i=0; i<UNITS_MAX_BASEUNITS; i++)
       if (tempval->exponent[i] != (i==UNIT_LENGTH))
        {
         sprintf(temp_err_string, "The position supplied to the 'set origin' command must have dimensions of length. Supplied x input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
         ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
         return;
        }
     }
    else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
    if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The coordinates supplied to the 'set origin' command was not finite."); return; }
    if (!(tempval2->dimensionless))
     {
      for (i=0; i<UNITS_MAX_BASEUNITS; i++)
       if (tempval2->exponent[i] != (i==UNIT_LENGTH))
        {
         sprintf(temp_err_string, "The position supplied to the 'set origin' command must have dimensions of length. Supplied y input has units of <%s>.", ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 1, 0));
         ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
         return;
        }
     }
    else { tempval2->real /= 100; } // By default, dimensionless positions are in centimetres
    if (!gsl_finite(tempval2->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The coordinates supplied to the 'set origin' command was not finite."); return; }
    sg->OriginX.real = tempval ->real;
    sg->OriginY.real = tempval2->real;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"origin")==0)) /* unset origin */
   {
    sg->OriginX = settings_graph_default.OriginX;
    sg->OriginY = settings_graph_default.OriginY;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"output")==0)) /* set output */
   {
    DictLookup(command,"filename",NULL,(void **)&tempstr);
    strncpy(settings_term_current.output, tempstr, FNAME_LENGTH-4);
    sg->title[FNAME_LENGTH-4]='\0';
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"output")==0)) /* unset output */
   {
    strncpy(settings_term_current.output, settings_term_default.output, FNAME_LENGTH-4);
    sg->title[FNAME_LENGTH-4]='\0';
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"palette")==0)) /* set palette */
   {
    DictLookup(command,"palette,",NULL,(void **)&templist);
    listiter = ListIterateInit(templist);
    i=0;
    while (listiter != NULL)
     {
      if (i == PALETTE_LENGTH-1) { ppl_warning(ERR_GENERAL, "The set palette command has been passed a palette which is too long; truncating it."); break; }
      tempdict = (Dict *)listiter->data;
      DictLookup(tempdict,"colour",NULL,(void **)&tempstr);
      if (tempstr != NULL)
       {
        j = FetchSettingByName(tempstr, SW_COLOUR_INT, SW_COLOUR_STR);
        if (j<0) { sprintf(temp_err_string, "The set palette command has been passed an unrecognised colour '%s'; ignoring this.", tempstr); ppl_warning(ERR_GENERAL, temp_err_string); }
        else     { settings_palette_current[i] = j; settings_paletteS_current[i]=0; settings_palette1_current[i]=settings_palette2_current[i]=settings_palette3_current[i]=settings_palette4_current[i]=0.0; i++; }
       } else {
        value *colR, *colG, *colB, *colH, *colS, *colC, *colM, *colY, *colK;
        unsigned char fail=0;

        DictLookup(tempdict,"colourR",NULL,(void **)&colR);
        DictLookup(tempdict,"colourG",NULL,(void **)&colG);
        DictLookup(tempdict,"colourB",NULL,(void **)&colB);
        DictLookup(tempdict,"colourH",NULL,(void **)&colH);
        DictLookup(tempdict,"colourS",NULL,(void **)&colS);
        DictLookup(tempdict,"colourC",NULL,(void **)&colC);
        DictLookup(tempdict,"colourM",NULL,(void **)&colM);
        DictLookup(tempdict,"colourY",NULL,(void **)&colY);
        DictLookup(tempdict,"colourK",NULL,(void **)&colK);

#define CHECK_COLCOMPS(COLX) \
        { \
         if ((!fail) && (COLX!=NULL) && (!COLX->dimensionless)) { sprintf(temp_err_string, "Colour components should be dimensionless quantities; the specified quantity has units of <%s>.", ppl_units_GetUnitStr(COLX, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); fail=1; } \
         if ((!fail) && (COLX!=NULL) && (COLX->imag>1e-6)) { sprintf(temp_err_string, "Colour components should be real numbers; the specified quantity is complex."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); fail=1; } \
         if ((!fail) && (COLX!=NULL) && (!gsl_finite(COLX->real))) { sprintf(temp_err_string, "Supplied colour components is not a finite number."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); fail=1; } \
        }

        CHECK_COLCOMPS(colR); CHECK_COLCOMPS(colG); CHECK_COLCOMPS(colB);
        CHECK_COLCOMPS(colH); CHECK_COLCOMPS(colS);
        CHECK_COLCOMPS(colC); CHECK_COLCOMPS(colM); CHECK_COLCOMPS(colY); CHECK_COLCOMPS(colK);

#define MAP01(X) (X->real <= 0.0) ? 0.0 : ((X->real >= 1.0) ? 1.0 : X->real); /* Make sure that colour component is in the range 0-1 */

        if (!fail)
         {
           settings_palette4_current[i] = 0.0;
          if (colR!=NULL)
           {
            settings_paletteS_current[i] = SW_COLSPACE_RGB;
            settings_palette1_current[i] = MAP01(colR);
            settings_palette2_current[i] = MAP01(colG);
            settings_palette3_current[i] = MAP01(colB);
           }
          else if (colH!=NULL)
           {
            settings_paletteS_current[i] = SW_COLSPACE_HSB;
            settings_palette1_current[i] = MAP01(colH);
            settings_palette2_current[i] = MAP01(colS);
            settings_palette3_current[i] = MAP01(colB);
           }
          else
           {
            settings_paletteS_current[i] = SW_COLSPACE_CMYK;
            settings_palette1_current[i] = MAP01(colC);
            settings_palette2_current[i] = MAP01(colM);
            settings_palette3_current[i] = MAP01(colY);
            settings_palette4_current[i] = MAP01(colK);
           }
          settings_palette_current[i++] = 0;
         }
       }
      listiter = ListIterate(listiter, NULL);
     }
    if (i==0) { ppl_error(ERR_GENERAL, -1, -1, "The set palette command has been passed a palette which does not contain any colours."); return; }
    settings_palette_current[i] = -1;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"palette")==0)) /* unset palette */
   {
    for (i=0; i<PALETTE_LENGTH; i++)
     {
      settings_palette_current [i] = settings_palette_default [i];
      settings_paletteS_current[i] = settings_paletteS_default[i];
      settings_palette1_current[i] = settings_palette1_default[i];
      settings_palette2_current[i] = settings_palette2_default[i];
      settings_palette3_current[i] = settings_palette3_default[i];
      settings_palette4_current[i] = settings_palette4_default[i];
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"papersize")==0)) /* set papersize */
   {
    DictLookup(command,"paper_name",NULL,(void **)&tempstr);
    if (tempstr != NULL)
     {
      ppl_PaperSizeByName(tempstr, &dbl1, &dbl2);
      if (dbl1 > 0)
       {
        settings_term_current.PaperHeight.real   = dbl1/1000;
        settings_term_current.PaperWidth.real    = dbl2/1000;
        ppl_GetPaperName(settings_term_current.PaperName, &dbl1, &dbl2);
       } else {
        sprintf(temp_err_string, "Unrecognised paper size '%s'.", tempstr); ppl_error(ERR_GENERAL, -1, -1, temp_err_string);
       }
     } else {
      DictLookup(command,"x_size",NULL,(void **)&tempval);
      DictLookup(command,"y_size",NULL,(void **)&tempval2);
      if (!(tempval->dimensionless))
       { 
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "The size supplied to the 'set papersize' command must have dimensions of length. Supplied x input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));  
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       } 
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The size coordinates supplied to the 'set papersize' command was not finite."); return; }
      if (!(tempval2->dimensionless))
       {
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval2->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "The size supplied to the 'set papersize' command must have dimensions of length. Supplied y input has units of <%s>.", ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 1, 0));
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       }
      else { tempval2->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval2->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The size coordinates supplied to the 'set papersize' command was not finite."); return; }
      settings_term_current.PaperWidth .real = tempval ->real;
      settings_term_current.PaperHeight.real = tempval2->real;
      tempval ->real *= 1000; // Function below takes size input in mm
      tempval2->real *= 1000;
      ppl_GetPaperName(settings_term_current.PaperName, &(tempval2->real), &(tempval ->real));
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"papersize")==0)) /* unset papersize */
   {
    settings_term_current.PaperHeight.real = settings_term_default.PaperHeight.real;
    settings_term_current.PaperWidth .real = settings_term_default.PaperWidth .real;
    strcpy(settings_term_current.PaperName, settings_term_default.PaperName);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"pointlinewidth")==0)) /* set pointlinewidth */
   {
    DictLookup(command,"pointlinewidth",NULL,(void **)&tempdbl);
    if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The value supplied to the 'set pointlinewidth' command was not finite."); return; }
    if (*tempdbl <= 0.0) { ppl_error(ERR_GENERAL, -1, -1, "Point line widths are not allowed to be less than or equal to zero."); return; }
    sg->PointLineWidth = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"pointlinewidth")==0)) /* unset pointlinewidth */
   {
    sg->PointLineWidth = settings_graph_default.PointLineWidth;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"pointsize")==0)) /* set pointsize */
   {
    DictLookup(command,"pointsize",NULL,(void **)&tempdbl);
    if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The value supplied to the 'set pointsize' command was not finite."); return; }
    if (*tempdbl <= 0.0) { ppl_error(ERR_GENERAL, -1, -1, "Point sizes are not allowed to be less than or equal to zero."); return; }
    sg->PointSize = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"pointsize")==0)) /* unset pointsize */
   {
    sg->PointSize = settings_graph_default.PointSize;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"preamble")==0)) /* set preamble */
   {
    DictLookup(command,"preamble",NULL,(void **)&tempstr);
    strncpy(settings_term_current.LatexPreamble, tempstr, FNAME_LENGTH-4);
    settings_term_current.LatexPreamble[FNAME_LENGTH-4]='\0';
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"preamble")==0)) /* unset preamble */
   {
    strncpy(settings_term_current.LatexPreamble, settings_term_default.LatexPreamble, FNAME_LENGTH-4);
    settings_term_current.LatexPreamble[FNAME_LENGTH-4]='\0';
   }
//  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"projection")==0)) /* set projection */
//   {
//    DictLookup(command,"projection",NULL,(void **)&tempstr);
//    sg->projection = FetchSettingByName(tempstr, SW_PROJ_INT, SW_PROJ_STR);
//   }
//  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"projection")==0)) /* unset projection */
//   {
//    sg->projection = settings_graph_default.projection;
//   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"samples")==0)) /* set samples */
   {
    DictLookup(command,"samples",NULL,(void **)&tempint);
    if (*tempint < 2.0) { ppl_error(ERR_GENERAL, -1, -1, "Graphs cannot be constucted based on fewer than two samples."); return; }
    sg->samples = *tempint;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"samples")==0)) /* unset samples */
   {
    sg->samples = settings_graph_default.samples;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"seed")==0)) /* set seed */
   {
    DictLookup(command,"seed",NULL,(void **)&tempdbl);
    if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The value supplied to the 'set seed' command was not finite."); return; }
    if      (*tempdbl < LONG_MIN) li = LONG_MIN;
    else if (*tempdbl > LONG_MAX) li = LONG_MAX;
    else                          li = (long)(*tempdbl);
    settings_term_current.RandomSeed = li;
    dcfmath_SetRandomSeed(li);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"seed")==0)) /* unset seed */
   {
    settings_term_current.RandomSeed = settings_term_default.RandomSeed;
    dcfmath_SetRandomSeed(settings_term_current.RandomSeed);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"size")==0)) /* set size | set width */
   {
    if (DictContains(command,"width"))
     {
      DictLookup(command,"width",NULL,(void **)&tempval);
      if (!(tempval->dimensionless))
       {
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "The widths specified for graphs must have dimensions of length. Supplied value has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       }
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The width supplied to the 'set size' command was not finite."); return; }
      sg->width.real = tempval->real;
     }
    if (DictContains(command,"ratio"))
     {
      DictLookup(command,"ratio",NULL,(void **)&tempdbl);
      if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The aspect ratio supplied to the 'set size' command was not finite."); return; }
      if ((fabs(*tempdbl) < 1e-6) || (fabs(*tempdbl) > 1e4)) { ppl_error(ERR_GENERAL, -1, -1, "The requested aspect ratios for graphs must be in the range 1e-6 to 10000."); return; }
      sg->aspect = *tempdbl;
      sg->AutoAspect = SW_ONOFF_OFF;
     }
    if (DictContains(command,"square"))
     {
      sg->aspect = 1.0;
      sg->AutoAspect = SW_ONOFF_OFF;
     }
    if (DictContains(command,"noratio"))
     {
      sg->AutoAspect = SW_ONOFF_ON;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"size")==0)) /* unset size */
   {
    sg->width.real   = settings_graph_default.width.real;
    sg->aspect       = settings_graph_default.aspect;
    sg->AutoAspect   = settings_graph_default.AutoAspect;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"style")==0)) /* set style data / function */
   {
    DictLookup(command,"dataset_type",NULL,(void **)&tempstr);
    if (tempstr[0]=='d') tempstyle = &sg->DataStyle;
    else                 tempstyle = &sg->FuncStyle;
    with_words_fromdict(command, &ww_temp1, 1);
    with_words_merge(&ww_temp2, &ww_temp1, tempstyle, NULL, NULL, NULL, 0);
    *tempstyle = ww_temp2;
    tempstyle->malloced = 1;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"style_numbered")==0)) /* set style */
   {
    DictLookup(command,"style_set_number"   ,NULL,(void **)&tempint);
    if ((*tempint<0)||(*tempint>=MAX_PLOTSTYLES)) { sprintf(temp_err_string, "plot style numbers must be in the range 0-%d", MAX_PLOTSTYLES-1); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; }
    with_words_fromdict(command, &ww_temp1, 0);
    with_words_destroy (&(settings_plot_styles[*tempint]));
    with_words_copy    (&(settings_plot_styles[*tempint]), &ww_temp1);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"style")==0)) /* unset style */
   {
    DictLookup(command,"dataset_type",NULL,(void **)&tempstr);
    if (tempstr != NULL)
     {
      if (tempstr[0]=='d') sg->DataStyle = settings_graph_default.DataStyle;
      else                 sg->FuncStyle = settings_graph_default.FuncStyle;
     } else {
      DictLookup(command,"style_ids,",NULL,(void **)&templist);
      listiter = ListIterateInit(templist);
      while (listiter != NULL)
       {   
        tempdict = (Dict *)listiter->data;
        DictLookup(tempdict,"id",NULL,(void **)&tempint);
        if ((*tempint<0)||(*tempint>=MAX_PLOTSTYLES)) { sprintf(temp_err_string, "plot style numbers must be in the range 0-%d", MAX_PLOTSTYLES-1); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
        else
         {
          with_words_destroy(&(settings_plot_styles[*tempint]));
          with_words_copy   (&(settings_plot_styles[*tempint]) , &(settings_plot_styles_default[*tempint]));
         }
        listiter = ListIterate(listiter, NULL);
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"terminal")==0)) /* set terminal */
   {
    DictLookup(command,"term"   ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermType        = FetchSettingByName(tempstr, SW_TERMTYPE_INT, SW_TERMTYPE_STR);
    DictLookup(command,"antiali",NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermAntiAlias   = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"col"    ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.colour          = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"dpi",NULL,(void **)&tempdbl);
    if (tempdbl != NULL)
     {
      if (!gsl_finite(*tempdbl)) { ppl_error(ERR_NUMERIC, -1, -1, "The DPI resolution supplied to the 'set terminal' command was not finite."); }
      else if (*tempdbl <= 2.0)  { ppl_error(ERR_GENERAL, -1, -1, "Output image resolutions below two dots per inch are not supported."); }
      else                       { settings_term_current.dpi = *tempdbl; }
     }
    DictLookup(command,"enlarge",NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermEnlarge     = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"land"   ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.landscape       = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"trans"  ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermTransparent = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"invert" ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermInvert      = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"terminal")==0)) /* unset terminal */
   {
    settings_term_current.colour         = settings_term_default.colour;
    settings_term_current.dpi            = settings_term_default.dpi;
    settings_term_current.landscape      = settings_term_default.landscape;
    settings_term_current.TermAntiAlias  = settings_term_default.TermAntiAlias;
    settings_term_current.TermType       = settings_term_default.TermType;
    settings_term_current.TermEnlarge    = settings_term_default.TermEnlarge;
    settings_term_current.TermInvert     = settings_term_default.TermInvert;
    settings_term_current.TermTransparent= settings_term_default.TermTransparent;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"texthalign")==0)) /* set texthalign */
   {
    if (DictContains(command,"left"  )) sg->TextHAlign = SW_HALIGN_LEFT;
    if (DictContains(command,"centre")) sg->TextHAlign = SW_HALIGN_CENT;
    if (DictContains(command,"right" )) sg->TextHAlign = SW_HALIGN_RIGHT;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"texthalign")==0)) /* unset texthalign */
   {
    settings_graph_default.TextHAlign = sg->TextHAlign;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"textvalign")==0)) /* set textvalign */
   {
    if (DictContains(command,"top"   )) sg->TextVAlign = SW_VALIGN_TOP;
    if (DictContains(command,"centre")) sg->TextVAlign = SW_VALIGN_CENT;
    if (DictContains(command,"bottom")) sg->TextVAlign = SW_VALIGN_BOT;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"textvalign")==0)) /* unset textvalign */
   {
    settings_graph_default.TextVAlign = sg->TextVAlign;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"tics")==0)) /* set tics */
   {

#define SET_TICS \
      DictLookup(command,"minor"     ,NULL,(void **)&tempstr2); \
      DictLookup(command,"dir"       ,NULL,(void **)&tempstr3); \
      DictLookup(command,"autofreq"  ,NULL,(void **)&tempstr4); \
      DictLookup(command,"start"     ,NULL,(void **)&tempval); \
      DictLookup(command,"increment" ,NULL,(void **)&tempval2); \
      DictLookup(command,"end"       ,NULL,(void **)&tempval3); \
      DictLookup(command,"tick_list,",NULL,(void **)&templist); \
      if (tempstr2==NULL) /* major ticks */ \
       { \
        if (tempstr3 != NULL) tempaxis->TickDir = FetchSettingByName(tempstr3, SW_TICDIR_INT, SW_TICDIR_STR); \
        if ((tempstr4 != NULL) || (tempval != NULL) || (tempval != NULL)) /* delete old tick settings */ \
         { \
          tempaxis->TickMin      = tempaxis2->TickMin; \
          tempaxis->TickMax      = tempaxis2->TickMax; \
          tempaxis->TickStep     = tempaxis2->TickStep; \
          tempaxis->TickMinSet   = 0; \
          tempaxis->TickMaxSet   = 0; \
          tempaxis->TickStepSet  = 0; \
          if (tempaxis-> TickList != NULL) { free(tempaxis-> TickList); tempaxis-> TickList = NULL; } /* Delete old explicit tick lists */ \
          if (tempaxis-> TickStrs != NULL) \
           { \
            for (i=0; tempaxis->TickStrs[i]!=NULL; i++) free(tempaxis->TickStrs[i]); \
            free(tempaxis->TickStrs ); \
            tempaxis->TickStrs  = NULL; \
           } \
         } \
        if (tempval != NULL) /* start , increment , end */ \
         { \
          if (tempval2 == NULL) { tempval2 = tempval; tempval = NULL; } /* If only one number specified; it is a stepsize */ \
          if      ((tempval  != NULL) && (!ppl_units_DimEqual(tempval , &tempaxis->unit))) { sprintf(temp_err_string, "Invalid starting value for axis ticks. Value supplied (%s) is not dimensionally compatible with the range set for this axis which has units of <%s>.", ppl_units_NumericDisplay(tempval ,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval2 != NULL) && (!ppl_units_DimEqual(tempval2, &tempaxis->unit))) { sprintf(temp_err_string, "Invalid step size for axis ticks. Value supplied (%s) is not dimensionally compatible with the range set for this axis which has units of <%s>."     , ppl_units_NumericDisplay(tempval2,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval3 != NULL) && (!ppl_units_DimEqual(tempval3, &tempaxis->unit))) { sprintf(temp_err_string, "Invalid end value for axis ticks. Value supplied (%s) is not dimensionally compatible with the range set for this axis which has units of <%s>."     , ppl_units_NumericDisplay(tempval3,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval  != NULL) && (!gsl_finite(tempval->real))) { sprintf(temp_err_string, "Invalid starting value for axis ticks. Value supplied is not finite."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval2 != NULL) && (!gsl_finite(tempval2->real))) { sprintf(temp_err_string, "Invalid step size for axis ticks. Value supplied is not finite."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval3 != NULL) && (!gsl_finite(tempval3->real))) { sprintf(temp_err_string, "Invalid end value for axis ticks. Value supplied is not finite."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
 \
          if (tempval  != NULL) { tempaxis->TickMin  = tempval->real;       tempaxis->TickMinSet  = 1; } \
          else                  { tempaxis->TickMin  = tempaxis2->TickMin;  tempaxis->TickMinSet  = 0; } \
 \
          if (tempval2 != NULL) { tempaxis->TickStep = tempval2->real;      tempaxis->TickStepSet = 1; } \
          else                  { tempaxis->TickStep = tempaxis2->TickStep; tempaxis->TickStepSet = 0; } \
 \
          if (tempval3 != NULL) { tempaxis->TickMax  = tempval3->real;      tempaxis->TickMaxSet  = 1; } \
          else                  { tempaxis->TickMax  = tempaxis2->TickMax;  tempaxis->TickMaxSet  = 0; } \
 \
         } else if (templist != NULL) { /* list of tick marks */ \
          j = ListLen(templist); \
          tempaxis->TickList = (double *)malloc((j+1)*sizeof(double)); \
          if (tempaxis->TickList == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return; } \
          tempaxis->TickStrs = (char  **)malloc((j+1)*sizeof(char *)); \
          if (tempaxis->TickStrs == NULL) { free(tempaxis->TickList); tempaxis->TickList=NULL; ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return; } \
          listiter = ListIterateInit(templist); \
          k=0; \
          while (listiter != NULL) \
           { \
            tempdict = (Dict *)listiter->data; \
            listiter = ListIterate(listiter, NULL); \
            DictLookup(tempdict,"x"    ,NULL,(void **)&tempval); \
            DictLookup(tempdict,"label",NULL,(void **)&tempstr); \
            if (!ppl_units_DimEqual(tempval, &tempaxis->unit)) { sprintf(temp_err_string, "Ignoring axis label for ordinate value %s; this is not dimensionally compatible with the range set for this axis which has units of <%s>.", ppl_units_NumericDisplay(tempval,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_warning(ERR_GENERAL, temp_err_string); continue; } \
            if (!gsl_finite(tempval->real)) { sprintf(temp_err_string, "Ignoring axis label for ordinate value %s; this ordinate value is not finite.", ppl_units_NumericDisplay(tempval,0,0,0)); ppl_warning(ERR_GENERAL, temp_err_string); continue; } \
            tempaxis->TickList[k] = tempval->real; \
            if (tempstr==NULL) tempstr="\xFF"; \
            tempaxis->TickStrs[k] = (char *)malloc(strlen(tempstr)+1); \
            if (tempaxis->TickStrs[k] == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return; } \
            strcpy(tempaxis->TickStrs[k], tempstr); \
            k++; \
           } \
          tempaxis->TickStrs[k] = NULL; tempaxis->TickList[k] = 0.0; \
         } \
       } else { /* minor ticks */ \
        if (tempstr3 != NULL) tempaxis->MTickDir = FetchSettingByName(tempstr3, SW_TICDIR_INT, SW_TICDIR_STR); \
        if ((tempstr4 != NULL) || (tempval != NULL) || (tempval != NULL)) /* delete old tick settings */ \
         { \
          tempaxis->MTickMin      = tempaxis2->MTickMin; \
          tempaxis->MTickMax      = tempaxis2->MTickMax; \
          tempaxis->MTickStep     = tempaxis2->MTickStep; \
          tempaxis->MTickMinSet   = 0; \
          tempaxis->MTickMaxSet   = 0; \
          tempaxis->MTickStepSet  = 0; \
          if (tempaxis-> MTickList != NULL) { free(tempaxis-> MTickList); tempaxis-> MTickList = NULL; } /* Delete old explicit tick lists */ \
          if (tempaxis-> MTickStrs != NULL) \
           { \
            for (i=0; tempaxis->MTickStrs[i]!=NULL; i++) free(tempaxis->MTickStrs[i]); \
            free(tempaxis->MTickStrs ); \
            tempaxis->MTickStrs  = NULL; \
           } \
         } \
        if (tempval != NULL) /* start , increment , end */ \
         { \
          if (tempval2 == NULL) { tempval2 = tempval; tempval = NULL; } /* If only one number specified; it is a stepsize */ \
          if      ((tempval  != NULL) && (!ppl_units_DimEqual(tempval , &tempaxis->unit))) { sprintf(temp_err_string, "Invalid starting value for axis ticks. Value supplied (%s) is not dimensionally compatible with the range set for this axis which has units of <%s>.", ppl_units_NumericDisplay(tempval ,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval2 != NULL) && (!ppl_units_DimEqual(tempval2, &tempaxis->unit))) { sprintf(temp_err_string, "Invalid step size for axis ticks. Value supplied (%s) is not dimensionally compatible with the range set for this axis which has units of <%s>."     , ppl_units_NumericDisplay(tempval2,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval3 != NULL) && (!ppl_units_DimEqual(tempval3, &tempaxis->unit))) { sprintf(temp_err_string, "Invalid end value for axis ticks. Value supplied (%s) is not dimensionally compatible with the range set for this axis which has units of <%s>."     , ppl_units_NumericDisplay(tempval3,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval  != NULL) && (!gsl_finite(tempval->real))) { sprintf(temp_err_string, "Invalid starting value for axis ticks. Value supplied is not finite."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval2 != NULL) && (!gsl_finite(tempval2->real))) { sprintf(temp_err_string, "Invalid step size for axis ticks. Value supplied is not finite."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
          else if ((tempval3 != NULL) && (!gsl_finite(tempval3->real))) { sprintf(temp_err_string, "Invalid end value for axis ticks. Value supplied is not finite."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; } \
 \
          if (tempval  != NULL) { tempaxis->MTickMin  = tempval->real;        tempaxis->MTickMinSet  = 1; } \
          else                  { tempaxis->MTickMin  = tempaxis2->MTickMin;  tempaxis->MTickMinSet  = 0; } \
 \
          if (tempval2 != NULL) { tempaxis->MTickStep = tempval2->real;       tempaxis->MTickStepSet = 1; } \
          else                  { tempaxis->MTickStep = tempaxis2->MTickStep; tempaxis->MTickStepSet = 0; } \
 \
          if (tempval3 != NULL) { tempaxis->MTickMax  = tempval3->real;       tempaxis->MTickMaxSet  = 1; } \
          else                  { tempaxis->MTickMax  = tempaxis2->MTickMax;  tempaxis->MTickMaxSet  = 0; } \
 \
         } else if (templist != NULL) { /* list of tick marks */ \
          j = ListLen(templist); \
          tempaxis->MTickList = (double *)malloc((j+1)*sizeof(double)); \
          if (tempaxis->MTickList == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return; } \
          tempaxis->MTickStrs = (char  **)malloc((j+1)*sizeof(char *)); \
          if (tempaxis->MTickStrs == NULL) { free(tempaxis->MTickList); tempaxis->MTickList=NULL; ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return; } \
          listiter = ListIterateInit(templist); \
          k=0; \
          while (listiter != NULL) \
           { \
            tempdict = (Dict *)listiter->data; \
            listiter = ListIterate(listiter, NULL); \
            DictLookup(tempdict,"x"    ,NULL,(void **)&tempval); \
            DictLookup(tempdict,"label",NULL,(void **)&tempstr); \
            if (!ppl_units_DimEqual(tempval, &tempaxis->unit)) { sprintf(temp_err_string, "Ignoring axis label for ordinate value %s; this is not dimensionally compatible with the range set for this axis which has units of <%s>.", ppl_units_NumericDisplay(tempval,0,0,0), ppl_units_GetUnitStr(&tempaxis->unit, NULL, NULL, 1, 1, 0)); ppl_warning(ERR_GENERAL, temp_err_string); continue; } \
            if (!gsl_finite(tempval->real)) { sprintf(temp_err_string, "Ignoring axis label for ordinate value %s; this ordinate value is not finite.", ppl_units_NumericDisplay(tempval,0,0,0)); ppl_warning(ERR_GENERAL, temp_err_string); continue; } \
            tempaxis->MTickList[k] = tempval->real; \
            if (tempstr==NULL) tempstr="\xFF"; \
            tempaxis->MTickStrs[k] = (char *)malloc(strlen(tempstr)+1); \
            if (tempaxis->MTickStrs[k] == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory"); return; } \
            strcpy(tempaxis->MTickStrs[k], tempstr); \
            k++; \
           } \
          tempaxis->MTickStrs[k] = NULL; tempaxis->MTickList[k] = 0.0; \
         } \
       } \

    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      DictLookup(command,"axis",NULL,(void **)&tempstr);
      if (tempstr != NULL)
       {
        i = (int)GetFloat(tempstr+1,NULL);
        if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
        else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
        else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
        tempaxis->enabled=1; // Activate axis
        SET_TICS;
       }
      else
       {
        for (i=0; i<MAX_AXES; i++) { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; if (tempaxis->enabled) SET_TICS; }
        for (i=0; i<MAX_AXES; i++) { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; if (tempaxis->enabled) SET_TICS; }
        for (i=0; i<MAX_AXES; i++) { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; if (tempaxis->enabled) SET_TICS; }
       }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"tics")==0)) /* unset tics */
   {

#define UNSET_TICS \
      DictLookup(command,"minor",NULL,(void **)&tempstr2); \
      if (tempstr2==NULL) \
       { \
        tempaxis->TickDir      = tempaxis2->TickDir; \
        tempaxis->TickMin      = tempaxis2->TickMin; \
        tempaxis->TickMax      = tempaxis2->TickMax; \
        tempaxis->TickStep     = tempaxis2->TickStep; \
        tempaxis->TickMinSet   = tempaxis2->TickMinSet; \
        tempaxis->TickMaxSet   = tempaxis2->TickMaxSet; \
        tempaxis->TickStepSet  = tempaxis2->TickStepSet; \
        if (tempaxis-> TickList != NULL) { free(tempaxis-> TickList); tempaxis-> TickList = NULL; } /* Delete old explicit tick lists */ \
        if (tempaxis-> TickStrs != NULL) \
         { \
          for (i=0; tempaxis->TickStrs[i]!=NULL; i++) free(tempaxis->TickStrs[i]); \
          free(tempaxis->TickStrs ); \
          tempaxis->TickStrs  = NULL; \
         } \
        CopyAxisTics(tempaxis, tempaxis2); \
       } else { \
        tempaxis->MTickDir     = tempaxis2->MTickDir; \
        tempaxis->MTickMin     = tempaxis2->MTickMin; \
        tempaxis->MTickMax     = tempaxis2->MTickMax; \
        tempaxis->MTickStep    = tempaxis2->MTickStep; \
        tempaxis->MTickMinSet  = tempaxis2->MTickMinSet; \
        tempaxis->MTickMaxSet  = tempaxis2->MTickMaxSet; \
        tempaxis->MTickStepSet = tempaxis2->MTickStepSet; \
        if (tempaxis->MTickList != NULL) { free(tempaxis->MTickList); tempaxis->MTickList = NULL; } \
        if (tempaxis->MTickStrs != NULL) \
         { \
          for (i=0; tempaxis->MTickStrs[i]!=NULL; i++) free(tempaxis->MTickStrs[i]); \
          free(tempaxis->MTickStrs ); \
          tempaxis->MTickStrs  = NULL; \
         } \
        CopyAxisMTics(tempaxis, tempaxis2); \
       } \

    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      DictLookup(command,"axis",NULL,(void **)&tempstr);
      if (tempstr != NULL)
       {
        i = (int)GetFloat(tempstr+1,NULL);
        if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
        else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
        else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
        UNSET_TICS;
       }
      else
       {
        for (i=0; i<MAX_AXES; i++) { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; UNSET_TICS; }
        for (i=0; i<MAX_AXES; i++) { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; UNSET_TICS; }
        for (i=0; i<MAX_AXES; i++) { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; UNSET_TICS; }
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"title")==0)) /* set title */
   {
    DictLookup(command,"title",NULL,(void **)&tempstr);
    strncpy(sg->title, tempstr, FNAME_LENGTH-4);
    sg->title[FNAME_LENGTH-4]='\0';
    DictLookup(command,"x_offset",NULL,(void **)&tempval);
    DictLookup(command,"y_offset",NULL,(void **)&tempval2);
    if (tempval != NULL)
     {
      if (!(tempval->dimensionless))
       {
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "The offset position supplied to the 'set title' command must have dimensions of length. Supplied x input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       }
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The offset coordinates supplied to the 'set title' command were not finite."); return; }
     }
    if (tempval2!= NULL) 
     { 
      if (!(tempval2->dimensionless))
       {
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval2->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "The offset position supplied to the 'set title' command must have dimensions of length. Supplied y input has units of <%s>.", ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 1, 0));
           ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
           return;
          }
       }
      else { tempval2->real /= 100; } // By default, dimensionless positions are in centimetres
      if (!gsl_finite(tempval2->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The offset coordinates supplied to the 'set title' command were not finite."); return; }
     }
    if (tempval != NULL) sg->TitleXOff.real = tempval ->real;
    if (tempval2!= NULL) sg->TitleYOff.real = tempval2->real;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"notitle")==0)) /* set notitle */
   {
    strcpy(sg->title, "");
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"noxformat")==0)) /* set noxformat */
   {
    DictLookup(command,"axis",NULL,(void **)&tempstr);
    i = (int)GetFloat(tempstr+1,NULL);
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
      else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
      else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
      if (tempaxis->format != NULL) { free(tempaxis->format); tempaxis->format = NULL; }
      tempaxis->TickLabelRotation = tempaxis2->TickLabelRotation;
      tempaxis->TickLabelRotate   = tempaxis2->TickLabelRotate;
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"title")==0)) /* unset title */
   {
    strncpy(sg->title, settings_graph_default.title, FNAME_LENGTH-4);
    sg->title[FNAME_LENGTH-4]='\0';
    sg->TitleXOff = settings_graph_default.TitleXOff;
    sg->TitleYOff = settings_graph_default.TitleYOff;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"trange")==0)) /* set trange */
   {
    DictLookup(command,"min",NULL,(void **)&tempval);
    DictLookup(command,"max",NULL,(void **)&tempval2);
    if (tempval == NULL) tempval = &sg->Tmin;
    if (tempval2== NULL) tempval2= &sg->Tmax;
    if (!gsl_finite(tempval ->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The range supplied to the 'set trange' command had non-finite limits."); return; }
    if (!gsl_finite(tempval2->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The range supplied to the 'set trange' command had non-finite limits."); return; }
    if (!ppl_units_DimEqual(tempval,tempval2)) { ppl_error(ERR_NUMERIC, -1, -1, "Attempt to set trange with dimensionally incompatible minimum and maximum."); return; }
    sg->Tmin = *tempval;
    sg->Tmax = *tempval2;
    DictLookup(command,"reverse",NULL,(void **)&tempstr);
    if (tempstr != NULL) { valobj = sg->Tmin; sg->Tmin = sg->Tmax; sg->Tmax = valobj; }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"trange")==0)) /* unset trange */
   {
    sg->Tmin = settings_graph_default.Tmin;
    sg->Tmax = settings_graph_default.Tmax;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"unit")==0)) /* set unit */
   {
    DictLookup(command,"angle"    , NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.UnitAngleDimless    = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"prefix"  , NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.UnitDisplayPrefix   = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"scheme"  , NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.UnitScheme          = FetchSettingByName(tempstr, SW_UNITSCH_INT, SW_UNITSCH_STR);

    DictLookup(command,"preferred_unit"  , NULL,(void **)&tempstr2);
    DictLookup(command,"unpreferred_unit", NULL,(void **)&tempstr3);
    if ((tempstr2!=NULL) || (tempstr3!=NULL))
     {
      char *buf = (char *)lt_malloc(LSTR_LENGTH);
      PreferredUnit *pu, *pui;
      if (buf==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory."); }
      else
       for (i=0; i<2; i++)
        {
         ListIterator *listiter;
         if (i==0) tempstr=tempstr2;
         else      tempstr=tempstr3;
         if (tempstr == NULL) continue;
         MakePreferredUnit(&pu, tempstr, 0, &errpos, buf);

         // Remove any preferred unit which is dimensionally equal to new preferred unit
         listiter = ListIterateInit(ppl_unit_PreferredUnits);
         while (listiter != NULL)
          {
           pui = (PreferredUnit *)listiter->data;
           listiter = ListIterate(listiter, NULL);
           if (ppl_units_DimEqual(&pui->value , &pu->value) && (pui->value.TempType == pu->value.TempType))
            {
             ListRemovePtr(ppl_unit_PreferredUnits, (void *)pui );
            }
          }

         // Add new preferred unit
         if (i==0) ListAppendPtrCpy((void *)ppl_unit_PreferredUnits, pu, sizeof(PreferredUnit), DATATYPE_VOID);
        }
     }

    DictLookup(command,"prefered_units,",NULL,(void **)&templist);
    listiter = ListIterateInit(templist);
    while (listiter != NULL)
     {
      tempdict = (Dict *)listiter->data;
      DictLookup(tempdict,"quantity",NULL,(void **)&tempstr);
      DictLookup(tempdict,"unit"    ,NULL,(void **)&tempstr2);
      i=0; // Quantity recognised
      pp=p=0; // Unit recognised
      for (j=0; j<ppl_unit_pos; j++)
       {
        if (i>1) i=1;
        if ((ppl_unit_database[j].quantity != NULL) && (StrCmpNoCase(ppl_unit_database[j].quantity , tempstr) == 0))
         {
          i=2;
          ppl_unit_database[j].UserSel = 0;
         }

        if (pp!=0) continue;
        multiplier = 8;
        if      ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].nameAp,1))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].nameAs,1))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].nameFp,0))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].nameFs,0))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].nameFp,0))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].alt1  ,0))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].alt2  ,0))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].alt3  ,0))!=0) p=1;
        else if ((k = UnitNameCmp(tempstr2, ppl_unit_database[j].alt4  ,0))!=0) p=1;
        else
         {
          for (l=ppl_unit_database[j].MinPrefix/3+8; l<=ppl_unit_database[j].MaxPrefix/3+8; l++)
           {
            if (l==8) continue;
            for (k=0; ((SIprefixes_full[l][k]!='\0') && (toupper(SIprefixes_full[l][k])==toupper(tempstr2[k]))); k++);
            if (SIprefixes_full[l][k]=='\0')
             {
              if      ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].nameFp,0))!=0) { p=1; k+=m; multiplier=l; break; }
              else if ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].nameFs,0))!=0) { p=1; k+=m; multiplier=l; break; }
              else if ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].alt1  ,0))!=0) { p=1; k+=m; multiplier=l; break; }
              else if ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].alt2  ,0))!=0) { p=1; k+=m; multiplier=l; break; }
              else if ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].alt3  ,0))!=0) { p=1; k+=m; multiplier=l; break; }
              else if ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].alt4  ,0))!=0) { p=1; k+=m; multiplier=l; break; }
             }
            for (k=0; ((SIprefixes_abbrev[l][k]!='\0') && (SIprefixes_abbrev[l][k]==tempstr2[k])); k++);
            if (SIprefixes_abbrev[l][k]=='\0')
             {
              if      ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].nameAp,1))!=0) { p=1; k+=m; multiplier=l; break; }
              else if ((m = UnitNameCmp(tempstr2+k, ppl_unit_database[j].nameAs,1))!=0) { p=1; k+=m; multiplier=l; break; }
             }
           }
         }
        if (p==0) continue;
        if (i!=2)
         {
          if ((ppl_unit_database[j].quantity!=NULL) && (ppl_unit_database[j].quantity[0]!='\0'))
           { sprintf(temp_err_string, "'%s' is not a unit of '%s', but of '%s'.", tempstr2, tempstr, ppl_unit_database[j].quantity); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
          else
           { sprintf(temp_err_string, "'%s' is not a unit of '%s'.", tempstr2, tempstr); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
         }
        ppl_unit_database[j].UserSel = 1;
        ppl_unit_database[j].UserSelPrefix = multiplier;
        pp=1;
       }
      if (i==0) { sprintf(temp_err_string, "No such quantity as a '%s'.", tempstr); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
      if (p==0) { sprintf(temp_err_string, "No such unit as a '%s'.", tempstr2); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); }
      listiter = ListIterate(listiter, NULL);
     }

   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit")==0)) /* unset unit */
   {
    double temp;
    settings_term_current.UnitAngleDimless    = settings_term_default.UnitAngleDimless;
    settings_term_current.UnitDisplayAbbrev   = settings_term_default.UnitDisplayAbbrev;
    settings_term_current.UnitDisplayPrefix   = settings_term_default.UnitDisplayPrefix;
    settings_term_current.UnitScheme          = settings_term_default.UnitScheme;
    for (i=0; i<ppl_unit_pos; i++) ppl_unit_database[i].UserSel = 0;
    temp = lt_GetMemContext();
    _lt_SetMemContext(0);
    ppl_unit_PreferredUnits = ListCopy(ppl_unit_PreferredUnits_default, 1);
    _lt_SetMemContext(temp);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit_angle")==0)) /* unset unit angle */
   {
    settings_term_current.UnitAngleDimless    = settings_term_default.UnitAngleDimless;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit_display")==0)) /* unset unit display */
   {
    settings_term_current.UnitDisplayAbbrev   = settings_term_default.UnitDisplayAbbrev;
    settings_term_current.UnitDisplayPrefix   = settings_term_default.UnitDisplayPrefix;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit_scheme")==0)) /* unset unit scheme */
   {
    settings_term_current.UnitScheme          = settings_term_default.UnitScheme;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit_preferred")==0)) /* unset unit preferred */
   {
    double temp;
    temp = lt_GetMemContext();
    _lt_SetMemContext(0);
    ppl_unit_PreferredUnits = ListCopy(ppl_unit_PreferredUnits_default, 1);
    _lt_SetMemContext(temp);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit_of")==0)) /* unset unit of */
   {
    DictLookup(command,"quantity",NULL,(void **)&tempstr);
    i=0; // Quantity recognised
    for (j=0; j<ppl_unit_pos; j++)
     {
      if ((ppl_unit_database[j].quantity != NULL) && (StrCmpNoCase(ppl_unit_database[j].quantity , tempstr) == 0))
       {
        i=1;
        ppl_unit_database[j].UserSel = 0;
       }
     }
    if (i==0) { sprintf(temp_err_string, "No such quantity as a '%s'.", tempstr); ppl_warning(ERR_GENERAL, temp_err_string); }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"width")==0)) /* unset width */
   {
    sg->width.real = settings_graph_default.width.real;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"view")==0)) /* set view */
   {
    DictLookup(command,"xy_angle",NULL,(void **)&tempval);
    DictLookup(command,"yz_angle",NULL,(void **)&tempval2);
    if (!(tempval->dimensionless))
     {
      for (i=0; i<UNITS_MAX_BASEUNITS; i++)
       if (tempval->exponent[i] != (i==UNIT_ANGLE))
        {
         sprintf(temp_err_string, "The rotation angle supplied to the 'set view' command must have dimensions of angle. Supplied input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
         ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
         return;
        }
     }
    else { tempval->real *= M_PI / 180.0; } // By default, dimensionless angles are in degrees
    if (!gsl_finite(tempval->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The viewing angles supplied to the 'set view' command were not finite."); return; }
    if (!(tempval2->dimensionless))
     {
      for (i=0; i<UNITS_MAX_BASEUNITS; i++)
       if (tempval2->exponent[i] != (i==UNIT_ANGLE))
        {
         sprintf(temp_err_string, "The rotation angle supplied to the 'set view' command must have dimensions of angle. Supplied input has units of <%s>.", ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 1, 0));
         ppl_error(ERR_NUMERIC, -1, -1, temp_err_string);
         return;
        }
     }
    else { tempval2->real *= M_PI / 180.0; } // By default, dimensionless angles are in degrees
    if (!gsl_finite(tempval2->real)) { ppl_error(ERR_NUMERIC, -1, -1, "The viewing angles supplied to the 'set view' command were not finite."); return; }
    sg->XYview.real = fmod(tempval ->real , 2*M_PI);
    sg->YZview.real = fmod(tempval2->real , 2*M_PI);
    while (sg->XYview.real < 0.0) sg->XYview.real += 2*M_PI;
    while (sg->YZview.real < 0.0) sg->YZview.real += 2*M_PI;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"view")==0)) /* unset view */
   {
    sg->XYview.real = settings_graph_default.XYview.real;
    sg->YZview.real = settings_graph_default.YZview.real;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"viewer")==0)) /* set viewer */
   {
    int ViewerOld = settings_term_current.viewer;
    DictLookup(command,"viewer",NULL,(void **)&tempstr);
    if      ((strcmp(tempstr,"gv"    )==0) && (strcmp(GHOSTVIEW_COMMAND,"/bin/false")!=0)) settings_term_current.viewer = SW_VIEWER_GV;
    else if ((strcmp(tempstr,"ggv"   )==0) && (strcmp(GGV_COMMAND      ,"/bin/false")!=0)) settings_term_current.viewer = SW_VIEWER_GGV;
    else if ((strcmp(tempstr,"evince")==0) && (strcmp(EVINCE_COMMAND   ,"/bin/false")!=0)) settings_term_current.viewer = SW_VIEWER_EVINCE;
    else if ((strcmp(tempstr,"okular")==0) && (strcmp(OKULAR_COMMAND   ,"/bin/false")!=0)) settings_term_current.viewer = SW_VIEWER_OKULAR;
    else
     {
      sprintf(temp_err_string,"Could not set current viewer to %s since this program was not installed when PyXPlot was installed.",tempstr);
      ppl_error(ERR_GENERAL, -1, -1,temp_err_string);
     }
    if (settings_term_current.viewer != ViewerOld) SendCommandToCSP("A"); // Clear away SingleWindow viewer
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"viewer")==0)) /* unset viewer */
   {
    int ViewerOld = settings_term_current.viewer;
    settings_term_current.viewer = settings_term_default.viewer;
    if (settings_term_current.viewer != ViewerOld) SendCommandToCSP("A"); // Clear away SingleWindow viewer
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"xformat")==0)) /* set xformat */
   {
    DictLookup(command,"axis",NULL,(void **)&tempstr);
    i = (int)GetFloat(tempstr+1,NULL);
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
      else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
      else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
      if (tempaxis->format != NULL) { free(tempaxis->format); tempaxis->format = NULL; }
      DictLookup(command,"format_string",NULL,(void **)&tempstr2);
      if (tempstr2 != NULL)
       {
        tempaxis->format = (char *)malloc(strlen(tempstr2)+1);
        if (tempaxis->format == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); return; }
        strcpy(tempaxis->format , tempstr2);
       }
      DictLookup(command,"orient",NULL,(void **)&tempstr2);
      if (tempstr2 != NULL)
       {
        tempaxis->TickLabelRotation = FetchSettingByName(tempstr2, SW_TICLABDIR_INT, SW_TICLABDIR_STR);
        tempaxis->TickLabelRotate   = tempaxis2->TickLabelRotate;
       }
      DictLookup(command,"rotation",NULL,(void **)&tempval);
      if (tempval != NULL)
       {
        if (!gsl_finite(tempval->real)) { sprintf(temp_err_string, "The rotation angle supplied to the 'set %sformat' command was not finite.", tempstr); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return; }
        if (!(tempval->dimensionless))
         {
          for (i=0; i<UNITS_MAX_BASEUNITS; i++)
           if (tempval->exponent[i] != (i==UNIT_ANGLE))
            {
             sprintf(temp_err_string, "The rotation argument to the 'set %sformat' command must have dimensions of angle. Supplied input has dimensions of <%s>.", tempstr, ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
             ppl_error(ERR_SYNTAX, -1, -1, temp_err_string);
             return;
            }
          tempaxis->TickLabelRotate = tempval->real;
         } else { tempaxis->TickLabelRotate = tempval->real * M_PI / 180; }
       }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"xformat")==0)) /* unset xformat */
   {
    DictLookup(command,"axis",NULL,(void **)&tempstr);
    i = (int)GetFloat(tempstr+1,NULL);
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
      else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
      else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
      if (tempaxis->format != NULL) { free(tempaxis->format); tempaxis->format = NULL; }
      if (tempaxis2->format != NULL)
       {
        tempaxis->format = (char *)malloc(strlen(tempaxis2->format)+1);
        if (tempaxis->format == NULL) { ppl_error(ERR_MEMORY, -1, -1,"Out of memory"); return; }
        strcpy(tempaxis->format , tempaxis2->format);
       }
      tempaxis->TickLabelRotation = tempaxis2->TickLabelRotation;
      tempaxis->TickLabelRotate   = tempaxis2->TickLabelRotate;
     }
   }
  else if ((strcmp(setoption,"xlabel")==0)) /* set xlabel / unset xlabel */
   {
    DictLookup(command,"axis",NULL,(void **)&tempstr);
    i = (int)GetFloat(tempstr+1,NULL);
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
      else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
      else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
      if (tempaxis->label != NULL) { free(tempaxis->label); tempaxis->label = NULL; }
      if (strcmp(directive,"unset")==0)
       {
        tempaxis->label       = tempaxis2->label;
        tempaxis->LabelRotate = tempaxis2->LabelRotate;
       }
      else
       {
        tempaxis->enabled=1;
        DictLookup(command,"label_text",NULL,(void **)&tempstr2);
        if ((tempstr2!=NULL)&&(strlen(tempstr2)>0))
         {
          tempaxis->label = (char *)malloc(strlen(tempstr2)+1);
          if (tempaxis->label == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory."); return; }
          strcpy(tempaxis->label, tempstr2);
         }
        DictLookup(command,"rotation",NULL,(void **)&tempval);
        if (tempval!=NULL)
         {
          if (!gsl_finite(tempval->real)) { sprintf(temp_err_string, "The rotation angle supplied to the 'set %slabel' command was not finite.", tempstr); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return; }
          if (!(tempval->dimensionless))
           {
            for (i=0; i<UNITS_MAX_BASEUNITS; i++)
             if (tempval->exponent[i] != (i==UNIT_ANGLE))
              {
               sprintf(temp_err_string, "The rotation argument to the 'set %slabel' command must have dimensions of angle. Supplied input has dimensions of <%s>.", tempstr, ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 1, 0));
               ppl_error(ERR_SYNTAX, -1, -1, temp_err_string);
               return;
              }
            tempaxis->LabelRotate = tempval->real;
           } else { tempaxis->LabelRotate = tempval->real * M_PI / 180; }
         }
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"range")==0)) /* set xrange */
   {
    DictLookup(command,"axis",NULL,(void **)&tempstr);
    i = (int)GetFloat(tempstr+1,NULL);
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
      else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
      else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
      tempaxis->enabled=1; // Activate axis
      DictLookup(command,"minauto",NULL,(void **)&tempstr2);
      if (tempstr2!=NULL) { tempaxis->MinSet = SW_BOOL_FALSE; tempaxis->min = tempaxis2->min; } // Remove range information if user has specified a *
      DictLookup(command,"maxauto",NULL,(void **)&tempstr2);
      if (tempstr2!=NULL) { tempaxis->MaxSet = SW_BOOL_FALSE; tempaxis->max = tempaxis2->max; }
      DictLookup(command,"min",NULL,(void **)&tempval); // Read specified min/max. If these are not specified, read existing values
      DictLookup(command,"max",NULL,(void **)&tempval2);
      if ((tempval == NULL) && (tempaxis->MinSet==SW_BOOL_TRUE)) { tempval = &valobj ; valobj = tempaxis->unit; valobj .real = tempaxis->min; }
      if ((tempval2== NULL) && (tempaxis->MaxSet==SW_BOOL_TRUE)) { tempval2= &valobj2; valobj2= tempaxis->unit; valobj2.real = tempaxis->max; }
      if ((tempval!=NULL)&&(!gsl_finite(tempval->real))) { sprintf(temp_err_string, "The range specified to the 'set %srange' command had a non-finite lower limit.", tempstr); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return; }
      if ((tempval2!=NULL)&&(!gsl_finite(tempval2->real))) { sprintf(temp_err_string, "The range specified to the 'set %srange' command had a non-finite upper limit.", tempstr); ppl_error(ERR_NUMERIC, -1, -1, temp_err_string); return; }
      if ((tempval!=NULL)&&(tempval2!=NULL)&&(!ppl_units_DimEqual(tempval,tempval2))) { ppl_error(ERR_NUMERIC, -1, -1, "Attempt to set axis range with dimensionally incompatible minimum and maximum."); return; }
      // Write new values, having ensured that they're dimensionally compatible
      if (tempval != NULL) { tempaxis->min = tempval ->real; tempaxis->MinSet=SW_BOOL_TRUE; }
      if (tempval2!= NULL) { tempaxis->max = tempval2->real; tempaxis->MaxSet=SW_BOOL_TRUE; }
      if (tempval2!= NULL) tempval = tempval2; // Check whether we're changing the units of this axis. If so, ticking information needs clearing out
      if ((tempval != NULL) && (!ppl_units_DimEqual(tempval,&tempaxis->unit)))
       {
        tempaxis->unit         = *tempval;
        tempaxis->TickMin      = tempaxis2->TickMin;
        tempaxis->TickMax      = tempaxis2->TickMax;
        tempaxis->TickStep     = tempaxis2->TickStep;
        tempaxis->TickMinSet   = tempaxis2->TickMinSet;
        tempaxis->TickMaxSet   = tempaxis2->TickMaxSet;
        tempaxis->TickStepSet  = tempaxis2->TickStepSet;
        tempaxis->MTickMin     = tempaxis2->MTickMin;
        tempaxis->MTickMax     = tempaxis2->MTickMax;
        tempaxis->MTickStep    = tempaxis2->MTickStep;
        tempaxis->MTickMinSet  = tempaxis2->MTickMinSet;
        tempaxis->MTickMaxSet  = tempaxis2->MTickMaxSet;
        tempaxis->MTickStepSet = tempaxis2->MTickStepSet;
        if (tempaxis-> TickList != NULL) { free(tempaxis-> TickList); tempaxis-> TickList = NULL; } // Delete old explicit tick lists
        if (tempaxis-> TickStrs != NULL)
         {
          for (i=0; tempaxis->TickStrs[i]!=NULL; i++) free(tempaxis->TickStrs[i]);
          free(tempaxis->TickStrs );
          tempaxis->TickStrs  = NULL;
         }
        if (tempaxis->MTickList != NULL) { free(tempaxis->MTickList); tempaxis->MTickList = NULL; } // Delete old explicit tick lists
        if (tempaxis->MTickStrs != NULL)
         {
          for (i=0; tempaxis->MTickStrs[i]!=NULL; i++) free(tempaxis->MTickStrs[i]);
          free(tempaxis->MTickStrs);
          tempaxis->MTickStrs  = NULL;
         }
       }
      DictLookup(command,"reverse"  ,NULL,(void **)&tempstr);
      if (tempstr != NULL) { tempaxis->RangeReversed = 1; }
      DictLookup(command,"noreverse",NULL,(void **)&tempstr);
      if (tempstr != NULL) { tempaxis->RangeReversed = 0; }
     }
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"range")==0)) /* unset xrange */
   {
    DictLookup(command,"axis",NULL,(void **)&tempstr);
    i = (int)GetFloat(tempstr+1,NULL);
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      if      (tempstr[0]=='y') { tempaxis = &ya[i]; tempaxis2 = &YAxesDefault[i]; }
      else if (tempstr[0]=='z') { tempaxis = &za[i]; tempaxis2 = &ZAxesDefault[i]; }
      else                      { tempaxis = &xa[i]; tempaxis2 = &XAxesDefault[i]; }
      tempaxis->max           = tempaxis2->max;
      tempaxis->MaxSet        = tempaxis2->MaxSet;
      tempaxis->min           = tempaxis2->min;
      tempaxis->MinSet        = tempaxis2->MinSet;
      tempaxis->RangeReversed = tempaxis2->RangeReversed;
     }
   }
  else
   {
    ppl_error(ERR_INTERNAL, -1, -1, "PyXPlot's set command could not find handler for this set command.");
   }
  return;
 }

#define SHOW_HIGHLIGHT(modified) \
if (interactive!=0) /* On interactive sessions, highlight those settings which have been manually set by the user */ \
 { \
  if (modified == 0) strcpy(out+i, *(char **)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *)) ); \
  else               strcpy(out+i, *(char **)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *)) ); \
  i += strlen(out+i); \
 }

#define SHOW_DEHIGHLIGHT \
if (interactive!=0) /* On interactive sessions, highlight those settings which have been manually set by the user */ \
 { \
  strcpy(out+i, *(char **)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *)) ); \
  i += strlen(out+i); \
 } \


void directive_show3(char *out, char *ItemSet, unsigned char ItemSetShow, int interactive, char *setting_name, char *setting_value, int modified, char *description)
 {
  int i=0,j,k;

  SHOW_HIGHLIGHT(modified);

  sprintf(out+i, "set %*s", (int)strlen(ItemSet), ItemSetShow ? ItemSet : ""); i += strlen(out+i); // Start off with a set command

  if (strcmp(setting_value, "On")==0)
   {
    sprintf(out+i, "%-41s", setting_name);
    i += strlen(out+i);
   }
  else if (strcmp(setting_value, "Off")==0)
   {
    for (j=0,k=-1; setting_name[j]!='\0'; j++) if (setting_name[j]==' ') k=j; // Find last space in setting name
    for (j=0; j<=k; j++) out[i+j] = setting_name[j];
    out[i+k+1] = 'n'; out[i+k+2] = 'o'; // Insert 'no' after this space
    for (j=k+1; setting_name[j]!='\0'; j++) out[i+j+2] = setting_name[j];
    for (; j<39; j++) out[i+j+2] = ' '; // Pad with spaces up to 45 characters
    out[i+j+2] = '\0';
    i += strlen(out+i);
   }
  else
   {
    sprintf(out+i, "%-16s %-24s", setting_name, setting_value);
    i += strlen(out+i);
   }

  if (description!=NULL) { sprintf(out+i, " # %s.", description); i += strlen(out+i); } // Finally put a decriptive comment after the setting
  strcpy(out+i, "\n"); i += strlen(out+i); // and a linefeed

  SHOW_DEHIGHLIGHT;
  return;
 }

int directive_show2(char *word, char *ItemSet, int interactive, settings_graph *sg, arrow_object **al, label_object **ll, settings_axis *xa, settings_axis *ya, settings_axis *za)
 {
  char *out, *buf, *buf2, *bufp, *bufp2, temp1[32], temp2[32];
  int   i=0, p=0,j,k,l,m,n;
  unsigned char unchanged;
  DictIterator *DictIter;
  FunctionDescriptor *FDiter;
  SubroutineDescriptor *SDiter;
  value *tempval, valobj;
  settings_axis *AxisPtr, *AxisPtrDef;
  arrow_object *ai, *ai_default, *ai_default_prev;
  label_object *li, *li_default, *li_default_prev;

  out = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Accumulate our whole output text here
  buf = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Put the value of each setting in here
  buf2= (char *)malloc(FNAME_LENGTH*sizeof(char));

  if ((out==NULL)||(buf==NULL)||(buf2==NULL))
   {
    ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst trying to allocate buffers in show command.");
    if (out!=NULL) free(out); if (buf!=NULL) free(buf); if (buf2!=NULL) free(buf2);
    return 1;
   }

  out[0] = buf[0] = '\0';
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "axescolour",1)>=0) || (StrAutocomplete(word, "axescolor",1)>=0))
   {
#define S_RGB(X,Y) (char *)NumericDisplay(X,Y,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L))
#define SHOW_COLOUR(COLOUR,COL1234SPACE,COL1,COL2,COL3,COL4  ,  DEFAULT,DEF1234SPACE,DEF1,DEF2,DEF3,DEF4  ,  NAME1, NAME2) \
    { \
     if      (COLOUR>0)                       sprintf(buf, "%s", *(char **)FetchSettingName(COLOUR, SW_COLOUR_INT, (void *)SW_COLOUR_STR, sizeof(char *))); \
     else if (COL1234SPACE==SW_COLSPACE_RGB ) sprintf(buf, "rgb%s:%s:%s", S_RGB(COL1,0), S_RGB(COL2,1), S_RGB(COL3,2)); \
     else if (COL1234SPACE==SW_COLSPACE_HSB ) sprintf(buf, "hsb%s:%s:%s", S_RGB(COL1,0), S_RGB(COL2,1), S_RGB(COL3,2)); \
     else if (COL1234SPACE==SW_COLSPACE_CMYK) sprintf(buf, "cmyk%s:%s:%s:%s", S_RGB(COL1,0), S_RGB(COL2,1), S_RGB(COL3,2), S_RGB(COL4,3)); \
     directive_show3(out+i, ItemSet, 1, interactive, NAME1, buf, ((COLOUR==DEFAULT)&&(COL1234SPACE==DEF1234SPACE)&&(COL1==DEF1)&&(COL2==DEF2)&&(COL3==DEF3)&&(COL4==DEF4)), NAME2); \
    }

    SHOW_COLOUR(sg->AxesColour,sg->AxesCol1234Space,sg->AxesColour1,sg->AxesColour2,sg->AxesColour3,sg->AxesColour4  ,
                settings_graph_default.AxesColour,settings_graph_default.AxesCol1234Space,settings_graph_default.AxesColour1,settings_graph_default.AxesColour2,settings_graph_default.AxesColour3,settings_graph_default.AxesColour4  ,
                "AxesColour", "The colour used to draw graph axes");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "axisunitstyle", 1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(sg->AxisUnitStyle, SW_AXISUNITSTY_INT, (void *)SW_AXISUNITSTY_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "AxisUnitStyle", buf, (settings_graph_default.AxisUnitStyle == sg->AxisUnitStyle), "Select how the physical units associated with axes are appended to axis labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "backup", 1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.backup, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "backup", buf, (settings_term_default.backup == settings_term_current.backup), "Selects whether existing files are overwritten (nobackup) or moved (backup)");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "bar",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(sg->bar,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 1, interactive, "bar", buf, (settings_graph_default.bar == sg->bar), "Sets the size of the strokes which mark the lower and upper limits of errorbars");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binorigin",1)>=0))
   {
    bufp = "Used to control the exact position of the edges of the bins used by the histogram command";
    if (settings_term_current.BinOriginAuto)
     {
      directive_show3(out+i, ItemSet, 0, interactive, "BinOrigin", "auto", settings_term_current.BinOriginAuto==settings_term_default.BinOriginAuto, bufp);
     } else {
      directive_show3(out+i, ItemSet, 0, interactive, "BinOrigin", ppl_units_NumericDisplay(&(settings_term_current.BinOrigin),0,0,0),
                      (settings_term_current.BinOriginAuto==settings_term_default.BinOriginAuto) &&
                      ppl_units_DblEqual( settings_term_default.BinOrigin.real , settings_term_current.BinOrigin.real) &&
                      ppl_units_DblEqual( settings_term_default.BinOrigin.imag , settings_term_current.BinOrigin.imag) &&
                      ppl_units_DimEqual(&settings_term_default.BinOrigin      ,&settings_term_current.BinOrigin     )    ,
                      bufp
                     );
     }
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binwidth",1)>=0))
   {
    bufp = "Sets the width of bins used by the histogram command";
    if (settings_term_current.BinWidthAuto) 
     { 
      directive_show3(out+i, ItemSet, 0, interactive, "BinWidth", "auto", settings_term_current.BinWidthAuto==settings_term_default.BinWidthAuto, bufp);
     } else {
      directive_show3(out+i, ItemSet, 0, interactive, "BinWidth", ppl_units_NumericDisplay(&(settings_term_current.BinWidth),0,0,0),
                      (settings_term_current.BinWidthAuto==settings_term_default.BinWidthAuto) &&
                      ppl_units_DblEqual( settings_term_default.BinWidth.real , settings_term_current.BinWidth.real) &&
                      ppl_units_DblEqual( settings_term_default.BinWidth.imag , settings_term_current.BinWidth.imag) &&
                      ppl_units_DimEqual(&settings_term_default.BinWidth      ,&settings_term_current.BinWidth     )    ,
                      bufp
                     );
     }
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxfrom",1)>=0))
   {
    bufp = "Sets the vertical level from which the bars of barcharts and histograms are drawn";
    if (sg->BoxFromAuto)
     { 
      directive_show3(out+i, ItemSet, 0, interactive, "BoxFrom", "auto", sg->BoxFromAuto==settings_graph_default.BoxFromAuto, bufp);
     } else {
      directive_show3(out+i, ItemSet, 0, interactive, "BoxFrom", ppl_units_NumericDisplay(&(sg->BoxFrom),0,0,0),
                      (sg->BoxFromAuto==settings_graph_default.BoxFromAuto) &&
                      ppl_units_DblEqual( settings_graph_default.BoxFrom.real , sg->BoxFrom.real) &&
                      ppl_units_DblEqual( settings_graph_default.BoxFrom.imag , sg->BoxFrom.imag) &&
                      ppl_units_DimEqual(&settings_graph_default.BoxFrom      ,&sg->BoxFrom     )    ,
                      bufp
                     );
     }
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxwidth",1)>=0))
   {
    bufp = "Sets the width of bars on barcharts and histograms";
    if (sg->BoxWidthAuto) 
     { 
      directive_show3(out+i, ItemSet, 0, interactive, "BoxWidth", "auto", sg->BoxWidthAuto==settings_graph_default.BoxWidthAuto, bufp);
     } else {
      directive_show3(out+i, ItemSet, 0, interactive, "BoxWidth", ppl_units_NumericDisplay(&(sg->BoxWidth),0,0,0),
                      (sg->BoxWidthAuto==settings_graph_default.BoxWidthAuto) &&
                      ppl_units_DblEqual( settings_graph_default.BoxWidth.real , sg->BoxWidth.real) &&
                      ppl_units_DblEqual( settings_graph_default.BoxWidth.imag , sg->BoxWidth.imag) &&
                      ppl_units_DimEqual(&settings_graph_default.BoxWidth      ,&sg->BoxWidth     )    ,
                      bufp
                     );
     }
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "calendarin",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.CalendarIn, SW_CALENDAR_INT, (void *)SW_CALENDAR_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "calendarin", buf, (settings_term_current.CalendarIn == settings_term_default.CalendarIn), "Selects the historical year in which the transition is made between Julian and Gregorian calendars when dates are being input");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "calendarout",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.CalendarOut, SW_CALENDAR_INT, (void *)SW_CALENDAR_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "calendarout", buf, (settings_term_current.CalendarOut == settings_term_default.CalendarOut), "Selects the historical year in which the transition is made between Julian and Gregorian calendars when displaying dates");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "clip",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(sg->clip, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "clip", buf, (sg->clip == settings_graph_default.clip), "Selects whether point symbols which extend over the axes of graphs are allowed to do so, or are clipped at the edges");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "display", 1)>=0))
   { 
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.display, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "display", buf, (settings_term_default.display == settings_term_current.display), "Sets whether any output is produced; turn on to improve performance whilst setting up large multiplots");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "filters",1)>=0))
   {
    DictIter = DictIterateInit(settings_filters);
    while (DictIter != NULL)
     {
      tempval = (value *)DictIter->data;
      StrEscapify(DictIter->key, buf+16);
      StrEscapify(tempval->string, buf2);
      sprintf(buf,"%s %s",buf+16,buf2);
      directive_show3(out+i, ItemSet, 0, interactive, "filter", buf, (!tempval->modified), "Sets an input filter to be used when reading datafiles");
      i += strlen(out+i) ; p=1;
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "fontsize",1)>=0) || (StrAutocomplete(word, "fountsize",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(sg->FontSize,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 1, interactive, "FontSize", buf, (settings_graph_default.FontSize == sg->FontSize), "Sets the font size of text output: 1.0 is the default, and other values multiply this default size");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "axes_", 1)>=0) || (StrAutocomplete(word, "axis", 1)>=0) || (StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "grid",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(sg->grid, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "grid", buf, (settings_graph_default.grid == sg->grid), "Selects whether a grid is drawn on plots");
    i += strlen(out+i) ; p=1;
    if (sg->grid == SW_ONOFF_ON)
     {
      bufp = buf; k=1;
      for (j=0; j<MAX_AXES; j++)
       {
        if (sg->GridAxisX[j] != 0                                   ) { sprintf(bufp, "x%d", j); bufp += strlen(bufp); }
        if (sg->GridAxisX[j] != settings_graph_default.GridAxisX[j] ) k=0;
       }
      if (bufp != buf) directive_show3(out+i, ItemSet, 1, interactive, "grid", buf, k, "Sets the x axis with whose ticks gridlines are associated");
      i += strlen(out+i);

      bufp = buf; k=1;
      for (j=0; j<MAX_AXES; j++)
       {
        if (sg->GridAxisY[j] != 0                                   ) { sprintf(bufp, "y%d", j); bufp += strlen(bufp); }
        if (sg->GridAxisY[j] != settings_graph_default.GridAxisY[j] ) k=0;
       }
      if (bufp != buf) directive_show3(out+i, ItemSet, 1, interactive, "grid", buf, k, "Sets the y axis with whose ticks gridlines are associated");
      i += strlen(out+i);

      bufp = buf; k=1;
      for (j=0; j<MAX_AXES; j++)
       {
        if (sg->GridAxisZ[j] != 0                                   ) { sprintf(bufp, "z%d", j); bufp += strlen(bufp); }
        if (sg->GridAxisZ[j] != settings_graph_default.GridAxisZ[j] ) k=0;
       }
      if (bufp != buf) directive_show3(out+i, ItemSet, 1, interactive, "grid", buf, k, "Sets the z axis with whose ticks gridlines are associated");
      i += strlen(out+i);
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "gridmajcolour",1)>=0) || (StrAutocomplete(word, "gridmajcolor",1)>=0))
   {
    SHOW_COLOUR(sg->GridMajColour,sg->GridMajCol1234Space,sg->GridMajColour1,sg->GridMajColour2,sg->GridMajColour3,sg->GridMajColour4  ,
                settings_graph_default.GridMajColour,settings_graph_default.GridMajCol1234Space,settings_graph_default.GridMajColour1,settings_graph_default.GridMajColour2,settings_graph_default.GridMajColour3,settings_graph_default.GridMajColour4  ,
                "GridMajColour", "The colour of the major gridlines on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "gridmincolour",1)>=0) || (StrAutocomplete(word, "gridmincolor",1)>=0))
   {
    SHOW_COLOUR(sg->GridMinColour,sg->GridMinCol1234Space,sg->GridMinColour1,sg->GridMinColour2,sg->GridMinColour3,sg->GridMinColour4  ,
                settings_graph_default.GridMinColour,settings_graph_default.GridMinCol1234Space,settings_graph_default.GridMinColour1,settings_graph_default.GridMinColour2,settings_graph_default.GridMinColour3,settings_graph_default.GridMinColour4  ,
                "GridMinColour", "The colour of the minor gridlines on graphs");
    i += strlen(out+i) ; p=1;
   }

  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "key",1)>=0))
   { 
    sprintf(buf, "%s", *(char **)FetchSettingName(sg->key, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 1, interactive, "key", buf, (settings_graph_default.key == sg->key), "Selects whether a legend is included on plots");
    i += strlen(out+i) ; p=1;
   }
  if ( ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "key",1)>=0)) && (sg->key == SW_ONOFF_ON)  )
   {
    sprintf(buf, "%s %s , %s", *(char **)FetchSettingName(sg->KeyPos, SW_KEYPOS_INT, (void *)SW_KEYPOS_STR, sizeof(char *)),ppl_units_NumericDisplay(&(sg->KeyXOff),0,0,0),ppl_units_NumericDisplay(&(sg->KeyYOff),1,0,0));
    directive_show3(out+i, ItemSet, 1, interactive, "key", buf, ((settings_graph_default.KeyPos == sg->KeyPos)&&(settings_graph_default.KeyXOff.real == sg->KeyXOff.real)&&(settings_graph_default.KeyYOff.real == sg->KeyYOff.real)), "Selects where legends are orientated on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "keycolumns",1)>=0))
   { 
    if (sg->KeyColumns>0) sprintf(buf, "%d", sg->KeyColumns);
    else                  sprintf(buf, "auto");
    directive_show3(out+i, ItemSet, 1, interactive, "KeyColumns", buf, (settings_graph_default.KeyColumns == sg->KeyColumns), "Sets the number of columns into which legends on graphs are sorted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "linewidth", 1)>=0) || (StrAutocomplete(word, "lw", 2)>=0))
   { 
    sprintf(buf, "%s", NumericDisplay(sg->LineWidth,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 1, interactive, "LineWidth", buf, (settings_graph_default.LineWidth == sg->LineWidth), "Sets the widths of lines drawn on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "multiplot", 1)>=0))
   { 
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.multiplot, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "multiplot", buf, (settings_term_default.multiplot == settings_term_current.multiplot), "Selects whether multiplot mode is currently active");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "numerics", 1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.ComplexNumbers, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "numerics complex", buf, (settings_term_default.ComplexNumbers==settings_term_current.ComplexNumbers), "Selects whether numbers are allowed to have imagnary components; affects the behaviour of functions such as sqrt()");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.ExplicitErrors,  SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "numerics errors explicit", buf, (settings_term_default.ExplicitErrors==settings_term_current.ExplicitErrors), "Selects whether numerical errors quietly produce not-a-number results, or throw explicit errors");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)NumericDisplay(settings_term_current.SignificantFigures,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 0, interactive, "numerics sigfig", buf, (settings_term_default.SignificantFigures == settings_term_current.SignificantFigures), "Sets the (minimum) number of significant figures to which decimal numbers are displayed by default");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.NumDisplay, SW_DISPLAY_INT, (void *)SW_DISPLAY_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "numerics display", buf, (settings_term_default.NumDisplay==settings_term_current.NumDisplay), "Selects how numerical results are displayed: in a natural textual way, in a way which can be copied into a terminal, or as LaTeX");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "origin", 1)>=0))
   { 
    sprintf(buf, "%s , %s", ppl_units_NumericDisplay(&(sg->OriginX),0,0,0), ppl_units_NumericDisplay(&(sg->OriginY),1,0,0));
    directive_show3(out+i, ItemSet, 1, interactive, "origin", buf, ((settings_graph_default.OriginX.real == sg->OriginX.real)&&(settings_graph_default.OriginY.real == sg->OriginY.real)), "Selects where the bottom-left corners of graphs are located on multiplot pages");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "output", 1)>=0))
   { 
    StrEscapify(settings_term_current.output, buf);
    directive_show3(out+i, ItemSet, 0, interactive, "output", buf, (strcmp(settings_term_default.output,settings_term_current.output)==0), "Filename to which graphic output is sent");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "palette",1)>=0))
   {
    l=0;
    for (j=0; j<PALETTE_LENGTH; j++) // Check whether the palette has been changed from its default setting
     {
      if ((settings_palette_current[j] == -1) && (settings_palette_default[j] == -1)) break;
      if ((settings_palette_current[j] == settings_palette_default[j])&&(settings_paletteS_current[j] == settings_paletteS_default[j])&&(settings_palette1_current[j] == settings_palette1_default[j])&&(settings_palette2_current[j] == settings_palette2_default[j])&&(settings_palette3_current[j] == settings_palette3_default[j])&&(settings_palette4_current[j] == settings_palette4_default[j])) continue;
      l=1; break;
     }
    for (j=k=0; settings_palette_current[j]>=0; j++)
     {
      if (j>0) { sprintf(buf+k, ", "); k+=strlen(buf+k); }
      if (settings_palette_current[j]>0) sprintf(buf+k, "%s", *(char **)FetchSettingName(settings_palette_current[j], SW_COLOUR_INT, (void *)SW_COLOUR_STR, sizeof(char *)));
      else if (settings_paletteS_current[j]==SW_COLSPACE_RGB ) sprintf(buf, "rgb%s:%s:%s"    , S_RGB(settings_palette1_current[j],0), S_RGB(settings_palette2_current[j],1), S_RGB(settings_palette3_current[j],2));
      else if (settings_paletteS_current[j]==SW_COLSPACE_HSB ) sprintf(buf, "hsb%s:%s:%s"    , S_RGB(settings_palette1_current[j],0), S_RGB(settings_palette2_current[j],1), S_RGB(settings_palette3_current[j],2));
      else if (settings_paletteS_current[j]==SW_COLSPACE_CMYK) sprintf(buf, "cmyk%s:%s:%s:%s", S_RGB(settings_palette1_current[j],0), S_RGB(settings_palette2_current[j],1), S_RGB(settings_palette3_current[j],2), S_RGB(settings_palette4_current[j],3));
      k+=strlen(buf+k);
     }
    directive_show3(out+i, ItemSet, 0, interactive, "palette", buf, !l, "The sequence of colours used to plot datasets on colour graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "papersize", 1)>=0))
   {
    sprintf(buf, "%s , %s", ppl_units_NumericDisplay(&(settings_term_current.PaperWidth),0,0,0), ppl_units_NumericDisplay(&(settings_term_current.PaperHeight),1,0,0));
    directive_show3(out+i, ItemSet, 0, interactive, "PaperSize", buf, ((settings_term_default.PaperWidth.real==settings_term_current.PaperWidth.real)&&(settings_term_default.PaperHeight.real==settings_term_current.PaperHeight.real)), "The current papersize for postscript output, in mm");
    i += strlen(out+i) ; p=1;
    if (StrAutocomplete("user", settings_term_current.PaperName, 1)<0)
     {
      StrEscapify(settings_term_current.PaperName,buf);
      directive_show3(out+i, ItemSet, 0, interactive, "PaperSize", buf, (strcmp(settings_term_default.PaperName, settings_term_current.PaperName)==0), NULL);
      i += strlen(out+i) ; p=1;
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointlinewidth",1)>=0) || (StrAutocomplete(word, "plw",3)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(sg->PointLineWidth,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 1, interactive, "PointLineWidth", buf, (settings_graph_default.PointLineWidth==sg->PointLineWidth), "The width of the strokes used to mark points on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointsize",1)>=0) || (StrAutocomplete(word, "ps",2)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(sg->PointSize,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 1, interactive, "PointSize", buf, (settings_graph_default.PointSize==sg->PointSize), "The size of points marked on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "preamble", 1)>=0))
   {
    StrEscapify(settings_term_current.LatexPreamble,buf);
    directive_show3(out+i, ItemSet, 0, interactive, "preamble", buf, (strcmp(settings_term_default.LatexPreamble,settings_term_current.LatexPreamble)==0), "Configuration options sent to the LaTeX typesetting system");
    i += strlen(out+i) ; p=1;
   }
//  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "projection", 1)>=0))
//   {
//    sprintf(buf, "%s", *(char **)FetchSettingName(settings_graph_current.projection, SW_PROJ_INT, (void *)SW_PROJ_STR, sizeof(char *)));
//    directive_show3(out+i, ItemSet, 0, interactive, "projection", buf, (settings_graph_default.projection==settings_graph_current.projection), "The projection used when representing (x,y) data on a graph");
//    i += strlen(out+i) ; p=1;
//   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "samples",1)>=0))
   {
    sprintf(buf, "%d", sg->samples);
    directive_show3(out+i, ItemSet, 1, interactive, "samples", buf, (settings_graph_default.samples==sg->samples), "The number of samples taken when functions are plotted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "seed",1)>=0))
   {
    sprintf(buf, "%ld", settings_term_current.RandomSeed);
    directive_show3(out+i, ItemSet, 1, interactive, "seed", buf, (settings_term_default.RandomSeed==settings_term_current.RandomSeed), "The last seed set for the random number generator");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "size",1)>=0))
   {
    if (sg->AutoAspect == SW_ONOFF_ON) sprintf(buf, "auto");
    else                               sprintf(buf, "%s", NumericDisplay(sg->aspect, 0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 1, interactive, "size ratio", buf, ((settings_graph_default.aspect==sg->aspect)&&(settings_graph_default.AutoAspect==sg->AutoAspect)), "The aspect-ratio of graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "data", 1)>=0) || (StrAutocomplete(word, "style", 1)>=0))
   {
    with_words_print(&sg->DataStyle, buf);
    directive_show3(out+i, ItemSet, 1, interactive, "data style", buf, with_words_compare(&settings_graph_default.DataStyle,&sg->DataStyle), "Default plot options for plotting datafiles");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "function", 1)>=0) || (StrAutocomplete(word, "style", 1)>=0))
   {
    with_words_print(&sg->FuncStyle, buf);
    directive_show3(out+i, ItemSet, 1, interactive, "function style", buf, with_words_compare(&settings_graph_default.FuncStyle,&sg->FuncStyle), "Default plot options for plotting functions");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.TermType, SW_TERMTYPE_INT, (void *)SW_TERMTYPE_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal", buf, (settings_term_default.TermType==settings_term_current.TermType), "The type of graphic output to be produced");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "antialias",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.TermAntiAlias, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal AntiAlias", buf, (settings_term_default.TermAntiAlias==settings_term_current.TermAntiAlias), "Selects whether anti-aliasing is applied to bitmap output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "colour", 1)>=0) || (StrAutocomplete(word, "color",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.colour, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal colour", buf, (settings_term_default.colour==settings_term_current.colour), "Selects whether output is colour or monochrome");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "dpi", 1)>=0))
   {                  
    sprintf(buf, "%s", (char *)NumericDisplay(settings_term_current.dpi,0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal dpi", buf, (settings_term_default.dpi == settings_term_current.dpi), "Sets the pixel resolution used when producing bitmap graphic output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "enlargement",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.TermEnlarge, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal enlarge", buf, (settings_term_default.TermEnlarge==settings_term_current.TermEnlarge), "Selects whether output photo-enlarged to fill the page");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "invert",1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.TermInvert, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal invert", buf, (settings_term_default.TermInvert==settings_term_current.TermInvert), "Selects whether the colours of bitmap output are inverted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "landscape", 1)>=0) || (StrAutocomplete(word, "portrait", 1)>=0))
   {
    if (settings_term_current.landscape == SW_ONOFF_ON) sprintf(buf, "Landscape");
    else                                                sprintf(buf, "Portrait");
    directive_show3(out+i, ItemSet, 0, interactive, "terminal", buf, (settings_term_default.landscape==settings_term_current.landscape), "Selects the orientation of output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "transparent", 1)>=0) || (StrAutocomplete(word, "solid", 1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.TermTransparent, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "terminal transparent", buf, (settings_term_default.TermTransparent==settings_term_current.TermTransparent), "Selects whether gif and png output is transparent");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "textcolour",1)>=0) || (StrAutocomplete(word, "textcolor",1)>=0))
   {
    SHOW_COLOUR(sg->TextColour,sg->TextCol1234Space,sg->TextColour1,sg->TextColour2,sg->TextColour3,sg->TextColour4  ,
                settings_graph_default.TextColour,settings_graph_default.TextCol1234Space,settings_graph_default.TextColour1,settings_graph_default.TextColour2,settings_graph_default.TextColour3,settings_graph_default.TextColour4  ,
                "TextColour", "Selects the colour of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "texthalign",1)>=0))
   { 
    sprintf(buf, "%s", *(char **)FetchSettingName(sg->TextHAlign, SW_HALIGN_INT, (void *)SW_HALIGN_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 1, interactive, "TextHAlign", buf, (settings_graph_default.TextHAlign==sg->TextHAlign), "Selects the horizontal alignment of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "textvalign",1)>=0))
   { 
    sprintf(buf, "%s", *(char **)FetchSettingName(sg->TextVAlign, SW_VALIGN_INT, (void *)SW_VALIGN_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 1, interactive, "TextVAlign", buf, (settings_graph_default.TextVAlign==sg->TextVAlign), "Selects the vertical alignment of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "title", 1)>=0))
   {
    StrEscapify(sg->title, buf); k = strlen(buf);
    sprintf(buf+k, " %s , %s", ppl_units_NumericDisplay(&(sg->TitleXOff), 0, 0, 0), ppl_units_NumericDisplay(&(sg->TitleYOff), 1, 0, 0));
    directive_show3(out+i, ItemSet, 1, interactive, "title", buf, ((strcmp(settings_graph_default.title,sg->title)==0)&&(settings_graph_default.TitleXOff.real==sg->TitleXOff.real)&&(settings_graph_default.TitleYOff.real==sg->TitleYOff.real)), "A title to be displayed above graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "trange", 1)>=0))
   {
    sprintf(buf, "[%s:%s]", ppl_units_NumericDisplay(&(sg->Tmin), 0, 0, 0), ppl_units_NumericDisplay(&(sg->Tmax), 1, 0, 0));
    directive_show3(out+i, ItemSet, 1, interactive, "trange", buf, (settings_graph_default.Tmin.real==sg->Tmin.real)&&ppl_units_DimEqual(&(settings_graph_default.Tmin),&(sg->Tmin))&&(settings_graph_default.Tmax.real==sg->Tmax.real)&&ppl_units_DimEqual(&(settings_graph_default.Tmax),&(sg->Tmax)), "The range of input values used in constructing parametric function plots");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "logscale", 1)>=0) || (StrAutocomplete(word, "linearscale", 1)>=0))
   {
    if (sg->Tlog==SW_BOOL_TRUE) bufp = "logscale";
    else                        bufp = "nologscale";
    sprintf(buf, "t");
    sprintf(buf2, "Sets whether the t axis scales linearly or logarithmically");
    directive_show3(out+i, ItemSet, 1, interactive, bufp, buf, (sg->Tlog==settings_graph_default.Tlog), buf2);
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "units", 1)>=0))
   {
    ListIterator *listiter;

    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.UnitAngleDimless, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "unit angle dimensionless", buf, (settings_term_default.UnitAngleDimless==settings_term_current.UnitAngleDimless), "Selects whether angles are treated as dimensionless quantities");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.UnitDisplayAbbrev, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "unit display abbreviated", buf, (settings_term_default.UnitDisplayAbbrev==settings_term_current.UnitDisplayAbbrev), "Selects whether units are displayed in abbreviated form ('m' vs. 'metres')");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.UnitDisplayPrefix, SW_ONOFF_INT, (void *)SW_ONOFF_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "unit display prefix", buf, (settings_term_default.UnitDisplayPrefix==settings_term_current.UnitDisplayPrefix), "Selects whether SI units are displayed with prefixes");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.UnitScheme, SW_UNITSCH_INT, (void *)SW_UNITSCH_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "unit scheme", buf, (settings_term_default.UnitScheme==settings_term_current.UnitScheme), "Selects the scheme (e.g. SI or Imperial) of prefered units");
    i += strlen(out+i) ; p=1;
    for (j=0; j<ppl_unit_pos; j++) if (ppl_unit_database[j].UserSel != 0)
     {
      sprintf(buf, "unit of %s", ppl_unit_database[j].quantity);
      if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON) sprintf(buf2, "%s%s", SIprefixes_abbrev[ppl_unit_database[j].UserSelPrefix], ppl_unit_database[j].nameAs);
      else                                                        sprintf(buf2, "%s%s", SIprefixes_full  [ppl_unit_database[j].UserSelPrefix], ppl_unit_database[j].nameFs);
      directive_show3(out+i, ItemSet, 0, interactive, buf, buf2, 0, "Selects a user-prefered unit for a particular quantity");
      i += strlen(out+i) ; p=1;
     }

    // show preferred units
    listiter = ListIterateInit(ppl_unit_PreferredUnits);
    while (listiter != NULL)
     {
      int pbuf=0, ppu;
      PreferredUnit *pu = (PreferredUnit *)listiter->data;
      buf[0]='\0';
      for (ppu=0; ppu<pu->NUnits; ppu++)
       {
        if (ppu>0) sprintf(buf+pbuf, "*");
        pbuf+=strlen(buf+pbuf);
        if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON) sprintf(buf+pbuf, "%s%s", (pu->prefix[ppu]>=1)?SIprefixes_abbrev[ pu->prefix[ppu] ]:"", ppl_unit_database[ pu->UnitID[ppu] ].nameAs);
        else                                                        sprintf(buf+pbuf, "%s%s", (pu->prefix[ppu]>=1)?SIprefixes_full  [ pu->prefix[ppu] ]:"", ppl_unit_database[ pu->UnitID[ppu] ].nameFs);
        pbuf+=strlen(buf+pbuf);
        if (pu->exponent[ppu]!=1) sprintf(buf+pbuf, "**%s", (char *)NumericDisplay(pu->exponent[ppu],0,settings_term_current.SignificantFigures,(settings_term_current.NumDisplay==SW_DISPLAY_L)));
        pbuf+=strlen(buf+pbuf);
       }
      directive_show3(out+i, ItemSet, 0, interactive, "unit preferred", buf, !pu->modified, "Specifies a user-preferred physical unit");
      i += strlen(out+i) ; p=1;
      listiter = ListIterate(listiter, NULL);
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "width", 1)>=0) || (StrAutocomplete(word, "size", 1)>=0))
   { 
    sprintf(buf, "%s", ppl_units_NumericDisplay(&(sg->width), 0, 0, 0));
    directive_show3(out+i, ItemSet, 1, interactive, "width", buf, (settings_graph_default.width.real==sg->width.real), "The width of graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "view", 1)>=0))
   {       
    sprintf(buf, "%s,%s", ppl_units_NumericDisplay(&(sg->XYview), 0, 0, 0), ppl_units_NumericDisplay(&(sg->YZview), 1, 0, 0));
    directive_show3(out+i, ItemSet, 1, interactive, "view", buf, (settings_graph_default.XYview.real==sg->XYview.real)&&(settings_graph_default.YZview.real==sg->YZview.real), "The rotation angle of 3d graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "viewer", 1)>=0))
   {
    sprintf(buf, "%s", *(char **)FetchSettingName(settings_term_current.viewer, SW_VIEWER_INT, (void *)SW_VIEWER_STR, sizeof(char *)));
    directive_show3(out+i, ItemSet, 0, interactive, "viewer", buf, (settings_term_default.UnitScheme==settings_term_current.UnitScheme), "Selects the postscript viewer used by the X11 terminals");
    i += strlen(out+i) ; p=1;
   }

  // Show axes
  l=0;
  if ((StrAutocomplete(word, "axes_", 1)>=0) || (StrAutocomplete(word, "axis", 1)>=0)) l=1;
  if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
   {
    for (k=0; k<3; k++)
     for (j=0; j<MAX_AXES; j++)
      {
       switch (k)
        {
         case 1 : { AxisPtr = &(ya[j]); AxisPtrDef = &(YAxesDefault[j]); break; }
         case 2 : { AxisPtr = &(za[j]); AxisPtrDef = &(ZAxesDefault[j]); break; }
         default: { AxisPtr = &(xa[j]); AxisPtrDef = &(XAxesDefault[j]); break; }
        }
       if (!AxisPtr->enabled) // Do not show any information for inactive axes, except that they're disabled
        {
         if (AxisPtrDef->enabled)
          {
           sprintf(temp1, "%c%d", "xyz"[k], j);
           sprintf(buf2, "Axis %s has been disabled", temp1);
           directive_show3(out+i, ItemSet, 1, interactive, "noaxis", temp1, 0, buf2);
           i += strlen(out+i) ; p=1;
          }
         continue;
        }

       sprintf(temp1, "%c%d", "xyz"[k], j);
       sprintf(temp2, "%c"  , "xyz"[k]   );
       if (l || (StrAutocomplete(word, temp1, 1)>=0) || ((j==1)&&(StrAutocomplete(word, temp2, 1)>=0)))
        {
         sprintf(buf  , "%c%d ", "xyz"[k], j); m = strlen(buf);
         sprintf(buf+m, "%s ", (AxisPtr->invisible ? "invisible" : "visible"  )); m += strlen(buf+m);
         if      (k==1) sprintf(buf+m, "%s ", (AxisPtr->topbottom ? "right" : "left"  ));
         else if (k==2) sprintf(buf+m, "%s ", (AxisPtr->topbottom ? "back"  : "front" ));
         else           sprintf(buf+m, "%s ", (AxisPtr->topbottom ? "top"   : "bottom"));
         m += strlen(buf+m);
         sprintf(buf+m, "%s ", *(char **)FetchSettingName(AxisPtr->ArrowType, SW_AXISDISP_INT, (void *)SW_AXISDISP_STR, sizeof(char *))); m += strlen(buf+m);
         sprintf(buf+m, "%s ", (AxisPtr->atzero    ? "atzero"    : "notatzero")); m += strlen(buf+m);
         sprintf(buf+m, "%s ", *(char **)FetchSettingName(AxisPtr->MirrorType, SW_AXISMIRROR_INT, (void *)SW_AXISMIRROR_STR, sizeof(char *))); m += strlen(buf+m);
         if (!AxisPtr->linked)
          {
           sprintf(buf+m, "notlinked"); m += strlen(buf+m);
          }
         else
          {
           strcpy(buf+m, "linked"); m += strlen(buf+m);
           if (AxisPtr->LinkedAxisCanvasID > 0) { sprintf(buf+m, " item %d", AxisPtr->LinkedAxisCanvasID); m += strlen(buf+m); }
           sprintf(buf+m, " %c%d", "xyz"[AxisPtr->LinkedAxisToXYZ], AxisPtr->LinkedAxisToNum); m += strlen(buf+m);
           if (AxisPtr->linkusing != NULL) { sprintf(buf+m, " using %s", AxisPtr->linkusing); m += strlen(buf+m); }
          }
         sprintf(buf2, "Settings for the %c%d axis", "xyz"[k], j);
         directive_show3(out+i, ItemSet, 1, interactive, "axis", buf,
                         (AxisPtr->atzero             == AxisPtrDef->atzero            ) &&
                         (AxisPtr->enabled            == AxisPtrDef->enabled           ) &&
                         (AxisPtr->invisible          == AxisPtrDef->invisible         ) &&
                         (AxisPtr->linked             == AxisPtrDef->linked            ) &&
                         (AxisPtr->topbottom          == AxisPtrDef->topbottom         ) &&
                         (AxisPtr->ArrowType          == AxisPtrDef->ArrowType         ) &&
                         (AxisPtr->LinkedAxisCanvasID == AxisPtrDef->LinkedAxisCanvasID) &&
                         (AxisPtr->LinkedAxisToXYZ    == AxisPtrDef->LinkedAxisToXYZ   ) &&
                         (AxisPtr->LinkedAxisToNum    == AxisPtrDef->LinkedAxisToNum   ) &&
                         (AxisPtr->MirrorType         == AxisPtrDef->MirrorType        )    ,
                         buf2
                        );
         i += strlen(out+i) ; p=1;
        }
                       
       sprintf(temp1, "%c%dformat", "xyz"[k], j);
       sprintf(temp2, "%cformat"  , "xyz"[k]   );
       if (l || (StrAutocomplete(word, temp1, 1)>=0) || ((j==1)&&(StrAutocomplete(word, temp2, 1)>=0)))
        {
         if (AxisPtr->format != NULL) sprintf(buf, "%s ", AxisPtr->format);
         else                         buf[0]='\0';
         m = strlen(buf);                
         sprintf(buf+m, "%s", *(char **)FetchSettingName(AxisPtr->TickLabelRotation, SW_TICLABDIR_INT, (void *)SW_TICLABDIR_STR, sizeof(char *))); m += strlen(buf+m);
         if (AxisPtr->TickLabelRotation == SW_TICLABDIR_ROT)
          {
           ppl_units_zero(&valobj); valobj.exponent[UNIT_ANGLE] = 1; valobj.dimensionless = 0; valobj.real = AxisPtr->TickLabelRotate;
           sprintf(buf+m, " %s", ppl_units_NumericDisplay(&valobj,0,0,0));
          }
         sprintf(buf2, "Format string for the tick labels on the %c%d axis", "xyz"[k], j);
         directive_show3(out+i, ItemSet, 1, interactive, temp1, buf,
                         (  ( AxisPtr->TickLabelRotate  ==AxisPtrDef->TickLabelRotate  ) &&
                            ( AxisPtr->TickLabelRotation==AxisPtrDef->TickLabelRotation) &&
                           (((AxisPtr->format==NULL)&&(AxisPtrDef->format==NULL)) ||
                            ((AxisPtr->format!=NULL)&&(AxisPtrDef->format!=NULL)&&(strcmp(AxisPtr->format,AxisPtrDef->format)==0)))
                         ) ,
                         buf2);
         i += strlen(out+i) ; p=1;
        }

       sprintf(temp1, "%c%dlabel", "xyz"[k], j);
       sprintf(temp2, "%clabel"  , "xyz"[k]   );
       if (l || (StrAutocomplete(word, temp1, 1)>=0) || ((j==1)&&(StrAutocomplete(word, temp2, 1)>=0)))
        {
         StrEscapify(AxisPtr->label==NULL ? "" : AxisPtr->label , buf); m = strlen(buf);
         ppl_units_zero(&valobj); valobj.exponent[UNIT_ANGLE] = 1; valobj.dimensionless = 0; valobj.real = AxisPtr->LabelRotate;
         sprintf(buf+m, " rotate %s", ppl_units_NumericDisplay(&valobj,0,0,0));
         sprintf(buf2, "Textual label for the %c%d axis", "xyz"[k], j);
         directive_show3(out+i, ItemSet, 1, interactive, temp1, buf,
                         (  ( AxisPtr->LabelRotate==AxisPtrDef->LabelRotate) &&
                           (((AxisPtr->label==NULL)&&(AxisPtrDef->label==NULL)) ||
                            ((AxisPtr->label!=NULL)&&(AxisPtrDef->label!=NULL)&&(strcmp(AxisPtr->label,AxisPtrDef->label)==0)))
                         ) ,
                         buf2);
         i += strlen(out+i) ; p=1;
        }

       if (l || (StrAutocomplete(word, "logscale", 1)>=0) || ((j==1)&&(StrAutocomplete(word, "linearscale", 1)>=0)))
        {
         if (AxisPtr->log==SW_BOOL_TRUE) bufp = "logscale";
         else                            bufp = "nologscale";
         sprintf(buf, "%c%d", "xyz"[k], j); m = strlen(buf);
         if (AxisPtr->log==SW_BOOL_TRUE) sprintf(buf+m, " base %d", (int)AxisPtr->LogBase);
         sprintf(buf2, "Sets whether the %c%d axis scales linearly or logarithmically", "xyz"[k], j);
         directive_show3(out+i, ItemSet, 1, interactive, bufp, buf, (AxisPtr->log==AxisPtrDef->log), buf2);
         i += strlen(out+i) ; p=1;
        }

       sprintf(temp1, "%c%drange", "xyz"[k], j);
       sprintf(temp2, "%crange"  , "xyz"[k]   );
       if (l || (StrAutocomplete(word, "autoscale", 1)>=0) || (StrAutocomplete(word, temp1, 1)>=0) || ((j==1)&&(StrAutocomplete(word, temp2, 1)>=0)))
        {
         AxisPtr->unit.real = AxisPtr->min;
         if (AxisPtr->MinSet==SW_BOOL_TRUE) bufp  = ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0);
         else                               bufp  = "*";
         AxisPtr->unit.real = AxisPtr->max;
         if (AxisPtr->MaxSet==SW_BOOL_TRUE) bufp2 = ppl_units_NumericDisplay(&(AxisPtr->unit),1,0,0);
         else                               bufp2 = "*";
         sprintf(buf , "[%s:%s]%s", bufp, bufp2, AxisPtr->RangeReversed ? " reversed" : "");
         sprintf(buf2, "Sets the range of the %c%d axis", "xyz"[k], j);
         directive_show3(out+i, ItemSet, 1, interactive, temp1, buf, (AxisPtr->min    == AxisPtrDef->min   ) &&
                                                                     (AxisPtr->MinSet == AxisPtrDef->MinSet) &&
                                                                     (AxisPtr->max    == AxisPtrDef->max   ) &&
                                                                     (AxisPtr->MaxSet == AxisPtrDef->MaxSet)    , buf2);
         i += strlen(out+i) ; p=1;
        }

       sprintf(temp1, "%c%dtics", "xyz"[k], j);
       sprintf(temp2, "%ctics"  , "xyz"[k]   );
       m=0;
       if (l || (StrAutocomplete(word, temp1, 1)>=0) || ((j==1)&&(StrAutocomplete(word, temp2, 1)>=0)))
        {
         sprintf(buf2, "Sets where the major ticks are placed along the %c%d axis, and how they appear", "xyz"[k], j);
         sprintf(buf, "%s ", *(char **)FetchSettingName(AxisPtr->TickDir, SW_TICDIR_INT, (void *)SW_TICDIR_STR, sizeof(char *))); m = strlen(buf);
         if      ((!AxisPtr->TickStepSet) && (AxisPtr->TickList == NULL))
          {
           sprintf(buf+m, "autofreq");
          }
         else if (AxisPtr->TickList == NULL)
          {
           if (AxisPtr->TickMinSet)
            {
             AxisPtr->unit.real = AxisPtr->TickMin;
             sprintf(buf+m, "%s", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0)); m += strlen(buf+m);
            }
           if (AxisPtr->TickStepSet)
            {
             AxisPtr->unit.real = AxisPtr->TickStep;
             sprintf(buf+m, "%s%s", (AxisPtr->TickMinSet)?", ":"", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0)); m += strlen(buf+m);
            }
           if (AxisPtr->TickMaxSet)
            {
             AxisPtr->unit.real = AxisPtr->TickMax;
             sprintf(buf+m, ", %s", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0)); m += strlen(buf+m);
            }
          }
         else
          {
           buf[m++]='(';
           for (n=0; AxisPtr->TickStrs[n]!=NULL; n++)
            {
             strcpy(buf+m, (n==0)?"":", "); m += strlen(buf+m);
             if (AxisPtr->TickStrs[n][0]!='\xFF') { StrEscapify(AxisPtr->TickStrs[n], buf+m); m += strlen(buf+m); }
             AxisPtr->unit.real = AxisPtr->TickList[n];
             sprintf(buf+m, " %s", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0));
             m += strlen(buf+m);
            }
           sprintf(buf+m, ")");
          }
         directive_show3(out+i, ItemSet, 1, interactive, temp1, buf, (AxisPtr->TickDir     == AxisPtrDef->TickDir    ) &&
                                                                     (AxisPtr->TickMax     == AxisPtrDef->TickMax    ) &&
                                                                     (AxisPtr->TickMaxSet  == AxisPtrDef->TickMaxSet ) &&
                                                                     (AxisPtr->TickStep    == AxisPtrDef->TickStep   ) &&
                                                                     (AxisPtr->TickStepSet == AxisPtrDef->TickStepSet) &&
                                                                     (AxisPtr->TickMin     == AxisPtrDef->TickMin    ) &&
                                                                     (AxisPtr->TickMinSet  == AxisPtrDef->TickMinSet ) &&
                                                                     (CompareAxisTics(AxisPtr, AxisPtrDef)          )    ,
                         buf2);
         i += strlen(out+i) ; p=1;
         m=1; // If we've shown major tics, also show minor ticks too.
        }

       sprintf(temp1, "m%c%dtics", "xyz"[k], j);
       sprintf(temp2, "m%ctics"  , "xyz"[k]   );
       if (l || m || (StrAutocomplete(word, temp1, 1)>=0) || ((j==1)&&(StrAutocomplete(word, temp2, 1)>=0)))
        {
         sprintf(buf2, "Sets where the minor ticks are placed along the %c%d axis, and how they appear", "xyz"[k], j);
         sprintf(buf, "%s ", *(char **)FetchSettingName(AxisPtr->MTickDir, SW_TICDIR_INT, (void *)SW_TICDIR_STR, sizeof(char *))); m = strlen(buf);
         if      ((!AxisPtr->MTickStepSet) && (AxisPtr->MTickList == NULL))
          {
           sprintf(buf+m, "autofreq");
          }
         else if (AxisPtr->MTickList == NULL)
          {
           if (AxisPtr->MTickMinSet)
            {
             AxisPtr->unit.real = AxisPtr->MTickMin;
             sprintf(buf+m, "%s", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0)); m += strlen(buf+m);
            }
           if (AxisPtr->MTickStepSet)
            { 
             AxisPtr->unit.real = AxisPtr->MTickStep;
             sprintf(buf+m, "%s%s", (AxisPtr->MTickMinSet)?", ":"", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0)); m += strlen(buf+m);
            }
           if (AxisPtr->MTickMaxSet)
            {
             AxisPtr->unit.real = AxisPtr->MTickMax;
             sprintf(buf+m, ", %s", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0)); m += strlen(buf+m);
            }
          }
         else
          {  
           buf[m++]='(';
           for (n=0; AxisPtr->MTickStrs[n]!=NULL; n++)
            {
             strcpy(buf+m, (n==0)?"":", "); m += strlen(buf+m);
             if (AxisPtr->MTickStrs[n][0]!='\xFF') { StrEscapify(AxisPtr->MTickStrs[n], buf+m); m += strlen(buf+m); }
             AxisPtr->unit.real = AxisPtr->MTickList[n];
             sprintf(buf+m, " %s", ppl_units_NumericDisplay(&(AxisPtr->unit),0,0,0));
             m += strlen(buf+m);
            }
           sprintf(buf+m, ")");
          }
         directive_show3(out+i, ItemSet, 1, interactive, temp1, buf, (AxisPtr->MTickDir      == AxisPtrDef->MTickDir    ) &&
                                                                     (AxisPtr->MTickMax      == AxisPtrDef->MTickMax    ) &&
                                                                     (AxisPtr->MTickMaxSet   == AxisPtrDef->MTickMaxSet ) &&
                                                                     (AxisPtr->MTickStep     == AxisPtrDef->MTickStep   ) &&
                                                                     (AxisPtr->MTickStepSet  == AxisPtrDef->MTickStepSet) &&
                                                                     (AxisPtr->MTickMin      == AxisPtrDef->MTickMin    ) &&
                                                                     (AxisPtr->MTickMinSet   == AxisPtrDef->MTickMinSet ) &&
                                                                     (CompareAxisMTics(AxisPtr, AxisPtrDef)             )    ,
                         buf2);
         i += strlen(out+i) ; p=1;
        }

      } // loop over axes
   } // if axis data structures are not null

  // Showed numbered arrows
  if (StrAutocomplete(word, "arrows", 1)>=0)
   {
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      SHOW_HIGHLIGHT(1);
      sprintf(out+i, "\n# Numbered arrows:\n\n"); i += strlen(out+i); p=1;
      SHOW_DEHIGHLIGHT;
      ai_default = ai_default_prev = arrow_list_default;
      for (ai=*al; ai!=NULL; ai=ai->next)
       {
        while ((ai_default != NULL) && (ai_default->id <= ai->id)) 
         {
          if (ai_default->id < ai->id)
           { 
            sprintf(buf2, "noarrow %6d", ai_default->id); 
            sprintf(buf,"remove arrow %6d", ai_default->id);
            directive_show3(out+i, ItemSet, 1, interactive, buf2, "", 1, buf);
            i += strlen(out+i);
           }
          ai_default_prev = ai_default;
          ai_default      = ai_default->next;
         }
        arrow_print(ai,buf);
        sprintf(buf2, "arrow %6d", ai->id);
        if ((unchanged = ((ai_default_prev != NULL) && (ai_default_prev->id == ai->id)))!=0) unchanged = arrow_compare(ai , ai_default_prev);
        directive_show3(out+i, ItemSet, 1, interactive, buf2, buf, unchanged, buf2);
        i += strlen(out+i);
       }
      while (ai_default != NULL)
       {
        sprintf(buf2, "noarrow %6d", ai_default->id); 
        sprintf(buf,"remove arrow %6d", ai_default->id);
        directive_show3(out+i, ItemSet, 1, interactive, buf2, "", 1, buf);
        i += strlen(out+i);
        ai_default      = ai_default->next;
       }
     }
   }

  // Show numbered text labels
  if (StrAutocomplete(word, "labels", 1)>=0)
   {
    if ( !((xa==NULL)||(ya==NULL)||(za==NULL)) )
     {
      SHOW_HIGHLIGHT(1);
      sprintf(out+i, "\n# Numbered text labels:\n\n"); i += strlen(out+i); p=1;
      SHOW_DEHIGHLIGHT;
      li_default = li_default_prev = label_list_default;
      for (li=*ll; li!=NULL; li=li->next)
       {
        while ((li_default != NULL) && (li_default->id <= li->id))
         {
          if (li_default->id < li->id)
           {
            sprintf(buf2, "nolabel %6d", li_default->id);
            sprintf(buf,"remove label %6d", li_default->id);
            directive_show3(out+i, ItemSet, 1, interactive, buf2, "", 1, buf);
            i += strlen(out+i);
           }
          li_default_prev = li_default;
          li_default      = li_default->next;
         }
        label_print(li,buf);
        sprintf(buf2, "label %6d", li->id);
        if ((unchanged = ((li_default_prev != NULL) && (li_default_prev->id == li->id)))!=0) unchanged = label_compare(li , li_default_prev);
        directive_show3(out+i, ItemSet, 1, interactive, buf2, buf, unchanged, buf2);
        i += strlen(out+i);
       }
      while (li_default != NULL)
       {
        sprintf(buf2, "nolabel %6d", li_default->id);
        sprintf(buf,"remove label %6d", li_default->id);
        directive_show3(out+i, ItemSet, 1, interactive, buf2, "", 1, buf);
        i += strlen(out+i);
        li_default      = li_default->next;
       }
     }
   }

  // Show numbered styles
  if ((StrAutocomplete(word, "styles", 1)>=0) || (StrAutocomplete(word, "linestyles", 1)>=0))
   {
    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# Numbered styles:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;
    for (j=0; j<MAX_PLOTSTYLES; j++)
     {
      if (with_words_compare_zero(&(settings_plot_styles[j]))) continue;
      with_words_print(&(settings_plot_styles[j]),buf);
      sprintf(buf2, "style %4d", j);
      directive_show3(out+i, ItemSet, 0, interactive, buf2, buf, !with_words_compare(&(settings_plot_styles[j]),&(settings_plot_styles_default[j])), buf2);
      i += strlen(out+i);
     }
   }


  // Show variables
  if ((StrAutocomplete(word, "variables", 1)>=0) || (StrAutocomplete(word, "vars", 1)>=0))
   {
    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# Variables:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;
    DictIter = DictIterateInit(_ppl_UserSpace_Vars);
    while (DictIter != NULL)
     {
      if (((value *)DictIter->data)->modified!=2)
       {
        SHOW_HIGHLIGHT((((value *)DictIter->data)->modified==0));
        if (((value *)DictIter->data)->string!=NULL)
         {
          StrEscapify(((value *)DictIter->data)->string, buf);
          sprintf(out+i, "%s = %s\n", DictIter->key, buf);
         }
        else
         {
          sprintf(out+i, "%s = %s\n", DictIter->key, ppl_units_NumericDisplay((value *)DictIter->data, 0, 0, 0));
         }
        i += strlen(out+i);
        SHOW_DEHIGHLIGHT;
       }
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }

  // Show system functions
  if ((StrAutocomplete(word, "functions", 1)>=0) || (StrAutocomplete(word, "funcs", 1)>=0))
   {
    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# System-Defined Functions:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;

    DictIter = DictIterateInit(_ppl_UserSpace_Funcs);
    while (DictIter != NULL)
     {
      FDiter = (FunctionDescriptor *)DictIter->data;
      SHOW_HIGHLIGHT((FDiter->modified==0));
      if ( (FDiter->FunctionType != PPL_USERSPACE_USERDEF) && (FDiter->FunctionType != PPL_USERSPACE_SPLINE) && (FDiter->FunctionType != PPL_USERSPACE_HISTOGRAM) && (FDiter->FunctionType != PPL_USERSPACE_FFT) && (FDiter->FunctionType != PPL_USERSPACE_SUBROUTINE))
       {
        sprintf(out+i, "# %-17s: %s.\n", DictIter->key, FDiter->description);
       }
      i += strlen(out+i);
      SHOW_DEHIGHLIGHT;
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }

  // Show user functions
  if ((StrAutocomplete(word, "functions", 1)>=0) || (StrAutocomplete(word, "funcs", 1)>=0) || (StrAutocomplete(word, "userfunctions", 1)>=0) || (StrAutocomplete(word, "userfuncs", 1)>=0))
   {
    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# User-Defined Functions:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;

    DictIter = DictIterateInit(_ppl_UserSpace_Funcs);
    while (DictIter != NULL)
     {
      FDiter = (FunctionDescriptor *)DictIter->data;
      while (FDiter != NULL)
       {
        if (FDiter->FunctionType == PPL_USERSPACE_USERDEF)
         {
          SHOW_HIGHLIGHT((FDiter->modified==0));
          // Let j be the number of ranges _used_ by this function definition
          j=-1;
          for (k=0; k<FDiter->NumberArguments; k++) if (FDiter->MinActive[k] || FDiter->MaxActive[k]) j=k;

          // Now compose a textual description of this function definition
          sprintf(out+i,"%s(",DictIter->key); i+=strlen(out+i);
          for (l=0, m=0; l<FDiter->NumberArguments; l++, m++)
           {
            for ( ; FDiter->ArgList[m]!='\0'; m++) *(out+(i++)) = FDiter->ArgList[m];
            *(out+(i++)) = ',';
           }
          if (FDiter->NumberArguments>0) i--; // Remove final comma from list of arguments
          *(out+(i++)) = ')';
          for (k=0; k<=j; k++)
           {
            *(out+(i++)) = '[';
            if (FDiter->MinActive[k]) { sprintf(out+i,"%s", ppl_units_NumericDisplay(FDiter->min+k, 0, 0, 0)); i+=strlen(out+i); }
            *(out+(i++)) = ':';
            if (FDiter->MaxActive[k]) { sprintf(out+i,"%s", ppl_units_NumericDisplay(FDiter->max+k, 0, 0, 0)); i+=strlen(out+i); }
            *(out+(i++)) = ']';
           }
          sprintf(out+i,"=%s\n",(char *)FDiter->description); i+=strlen(out+i);
          SHOW_DEHIGHLIGHT;
         }
        else if (FDiter->FunctionType == PPL_USERSPACE_SPLINE)
         {
          SHOW_HIGHLIGHT((FDiter->modified==0));
          sprintf(out+i,"%s(x)= [%s interpolation of data from the file '%s']\n",DictIter->key,
                                                                                ((SplineDescriptor *)FDiter->FunctionPtr)->SplineType,
                                                                                ((SplineDescriptor *)FDiter->FunctionPtr)->filename );
          i+=strlen(out+i);
          SHOW_DEHIGHLIGHT;
         }
        else if (FDiter->FunctionType == PPL_USERSPACE_HISTOGRAM)
         {
          SHOW_HIGHLIGHT((FDiter->modified==0));
          sprintf(out+i,"%s(x)= [histogram of data from the file '%s']\n",DictIter->key,
                                                                                ((HistogramDescriptor *)FDiter->FunctionPtr)->filename );
          i+=strlen(out+i);
          SHOW_DEHIGHLIGHT;
         }
        else if (FDiter->FunctionType == PPL_USERSPACE_FFT)
         {
          SHOW_HIGHLIGHT((FDiter->modified==0));
          sprintf(out+i,"%s(x)= [%d-dimensional fft]\n",DictIter->key,
                                                                                ((FFTDescriptor *)FDiter->FunctionPtr)->Ndims );
          i+=strlen(out+i);
          SHOW_DEHIGHLIGHT;
         }
        else if (FDiter->FunctionType == PPL_USERSPACE_SUBROUTINE)
         {
          int l,m;
          SHOW_HIGHLIGHT((FDiter->modified==0));
          SDiter = (SubroutineDescriptor *)FDiter->FunctionPtr;
          sprintf(out+i,"%s(", DictIter->key); i+=strlen(out+i);
          for (l=0, m=0; l<SDiter->NumberArguments; l++, m++)
           {
            for ( ; SDiter->ArgList[m]!='\0'; m++) *(out+(i++)) = SDiter->ArgList[m];
            *(out+(i++)) = ',';
           }
          if (SDiter->NumberArguments>0) i--; // Remove final comma from list of arguments
          sprintf(out+i,") = [subroutine]\n");
          i+=strlen(out+i);
          SHOW_DEHIGHLIGHT;
         }
        FDiter = FDiter->next;
       }
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }

  // Show list of recognised units
  if (StrAutocomplete(word, "units", 5)>=0)
   {
    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# Recognised Physical Units:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;
    l=-1;
    do
     {
      m=-1;
      for (j=0; j<ppl_unit_pos; j++)
       {
        if      ( (l==-1) && (m==-1)                                                                                                                                                               ) m=j;
        else if ( (l==-1) && (StrCmpNoCase(ppl_unit_database[j].nameFs , ppl_unit_database[m].nameFs)<0)                                                                                           ) m=j;
        else if ( (l>= 0) && (m==-1) &&                                                                                (StrCmpNoCase(ppl_unit_database[j].nameFs , ppl_unit_database[l].nameFs)>0) ) m=j;
        else if ( (l>= 0) && (m>= 0) && (StrCmpNoCase(ppl_unit_database[j].nameFs , ppl_unit_database[m].nameFs)<0) && (StrCmpNoCase(ppl_unit_database[j].nameFs , ppl_unit_database[l].nameFs)>0) ) m=j;
       }
      l=m;
      if (m!=-1)
       {
        k=0;
        SHOW_HIGHLIGHT((ppl_unit_database[m].modified==0));

        #define SHOW_ALL_UNIT_NAMES 0

        sprintf(out+i, "# The '%s', also known as", ppl_unit_database[m].nameFs); i+=strlen(out+i);
        if ((SHOW_ALL_UNIT_NAMES) || (strcmp(ppl_unit_database[m].nameFp, ppl_unit_database[m].nameFs) != 0)) { sprintf(out+i, " '%s' or", ppl_unit_database[m].nameFp); i+=strlen(out+i); k=1; }
        if ((SHOW_ALL_UNIT_NAMES) || (strcmp(ppl_unit_database[m].nameAs, ppl_unit_database[m].nameFs) != 0)) { sprintf(out+i, " '%s' or", ppl_unit_database[m].nameAs); i+=strlen(out+i); k=1; }
        if ((SHOW_ALL_UNIT_NAMES) ||((strcmp(ppl_unit_database[m].nameAp, ppl_unit_database[m].nameAs) != 0) &&
           (strcmp(ppl_unit_database[m].nameAp, ppl_unit_database[m].nameFp) != 0))){sprintf(out+i, " '%s' or", ppl_unit_database[m].nameAp); i+=strlen(out+i); k=1; }

        if (       ppl_unit_database[m].alt1 != NULL                             ) { sprintf(out+i, " '%s' or", ppl_unit_database[m].alt1  ); i+=strlen(out+i); k=1; }
        if (       ppl_unit_database[m].alt2 != NULL                             ) { sprintf(out+i, " '%s' or", ppl_unit_database[m].alt2  ); i+=strlen(out+i); k=1; }
        if (       ppl_unit_database[m].alt3 != NULL                             ) { sprintf(out+i, " '%s' or", ppl_unit_database[m].alt3  ); i+=strlen(out+i); k=1; }
        if (       ppl_unit_database[m].alt4 != NULL                             ) { sprintf(out+i, " '%s' or", ppl_unit_database[m].alt4  ); i+=strlen(out+i); k=1; }
        if (k==0) { i-=15; } else { i-=3; out[i++]=','; }
        sprintf(out+i, " is a unit of %s", ppl_unit_database[m].quantity); i += strlen(out+i);
        if (ppl_unit_database[m].comment != NULL) { sprintf(out+i, " (%s)", ppl_unit_database[m].comment); i += strlen(out+i); }
        sprintf(out+i, ".\n"); i += strlen(out+i);
        SHOW_DEHIGHLIGHT;
       }
     }
    while (m!=-1);
   }

  if (p!=0) ppl_report(out);
  free(out); free(buf); free(buf2);
  return p;
 }

void directive_show(Dict *command, int interactive)
 {
  List         *ShowList;
  ListIterator *ShowIterate;
  Dict         *ShowWordDict;
  char         *ShowWord;
  char          TextBuffer[SSTR_LENGTH], ItemSet[32];
  int           i=0, p=0, *EditNo;
  canvas_item    *ptr;
  settings_graph *sg;
  arrow_object  **al;
  label_object  **ll;
  settings_axis  *xa, *ya, *za;

  interactive = ( interactive && (settings_session_default.colour == SW_ONOFF_ON) );

  DictLookup(command,"editno",NULL,(void **)&EditNo);
  if (EditNo == NULL)
   {
    sg = &settings_graph_current;
    al = &arrow_list;
    ll = &label_list;
    xa = XAxes; ya = YAxes; za = ZAxes;
    ItemSet[0]='\0';
   }
  else
   {
    if ((*EditNo<1) || (*EditNo>MULTIPLOT_MAXINDEX) || (canvas_items == NULL)) { sprintf(temp_err_string, "No multiplot item with index %d.", *EditNo); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; }
    ptr = canvas_items->first;
    for (i=1; i<*EditNo; i++)
     {   
      if (ptr==NULL) break;
      ptr=ptr->next;
     }
    if (ptr == NULL) { sprintf(temp_err_string, "No multiplot item with index %d.", *EditNo); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); return; }
  
    sg = &(ptr->settings);
    al = &(ptr->arrow_list);
    ll = &(ptr->label_list);
    xa = ptr->XAxes; ya = ptr->YAxes; za = ptr->ZAxes;
    sprintf(ItemSet, "item %d ", *EditNo);
   }

  DictLookup(command, "setting_list", NULL, (void **)&ShowList);
  if ((ShowList==NULL) || (ListLen(ShowList) == 0))
   { ppl_error(ERR_PREFORMED, -1, -1, txt_show); }
  else
   {
    if (interactive!=0) // On interactive sessions, highlight those settings which have been manually set by the user
     {
      sprintf(TextBuffer+i,"%sSettings which have not been changed by the user are shown in %s.%s\n",
              *(char **)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *)),
              *(char **)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void *)SW_TERMCOL_STR, sizeof(char *)),
              *(char **)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *))
             );
      i += strlen(TextBuffer+i);
      sprintf(TextBuffer+i,"%sSettings which have been changed by the user are shown in %s.%s\n",
              *(char **)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *)),
              *(char **)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void *)SW_TERMCOL_STR, sizeof(char *)),
              *(char **)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void *)SW_TERMCOL_TXT, sizeof(char *))
             );
      i += strlen(TextBuffer+i);
      ppl_report(TextBuffer);
     }
    ShowIterate = ListIterateInit(ShowList);
    while (ShowIterate != NULL)
     {
      ShowIterate = ListIterate(ShowIterate, (void **)&ShowWordDict);
      DictLookup(ShowWordDict,"setting",NULL,(void **)&ShowWord);
      if (StrAutocomplete(ShowWord,"all",1)>=0)
       {
        directive_show2("settings"   ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("axes_"      ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("arrows"     ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("labels"     ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("linestyles" ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("variables"  ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("functions"  ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("subroutines",ItemSet, interactive, sg, al, ll, xa, ya, za);
        directive_show2("units"      ,ItemSet, interactive, sg, al, ll, xa, ya, za);
        p=1;
       }
      else
       {
        p = (directive_show2(ShowWord, ItemSet, interactive, sg, al, ll, xa, ya, za) || p);
       }
     }
    if (p==0) { ppl_error(ERR_SYNTAX, -1, -1, "Invalid show option."); ppl_error(ERR_PREFORMED, -1, -1, txt_show); }
   }
  return;
 }

