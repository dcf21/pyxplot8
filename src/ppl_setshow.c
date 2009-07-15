// ppl_setshow.c
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

#include "StringTools/asciidouble.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"

#include "EPSMaker/eps_colours.h"

#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_papersize.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_userspace.h"
#include "pyxplot.h"

int with_words_compare(with_words *a, with_words *b)
 {
  int i;
  for (i=0; i<sizeof(with_words); i++) if ( *(((char *)a)+i) != *(((char *)b)+i)  ) return (0 == 1);
  return (0 == 0);
 }

void with_words_print(with_words *defn, char *out)
 {
  int i=0;

  if (defn->style         >0) { sprintf(out+i, "%s "               , (char *)FetchSettingName(defn->style     , SW_STYLE_INT , (void **)SW_STYLE_STR )); i += strlen(out+i); }
  if (defn->colour        >0) { sprintf(out+i, "colour %s "        , (char *)FetchSettingName(defn->colour    , SW_COLOUR_INT, (void **)SW_COLOUR_STR)); i += strlen(out+i); }
  if (defn->fillcolour    >0) { sprintf(out+i, "fillcolour %s "    , (char *)FetchSettingName(defn->fillcolour, SW_COLOUR_INT, (void **)SW_COLOUR_STR)); i += strlen(out+i); }
  if (defn->linestyle     >0) { sprintf(out+i, "linestyle %d "     ,                          defn->linestyle                                         ); i += strlen(out+i); }
  if (defn->linetype      >0) { sprintf(out+i, "linetype %d "      ,                          defn->linetype                                          ); i += strlen(out+i); }
  if (defn->linewidth     >0) { sprintf(out+i, "linewidth %e "     ,                          defn->linewidth                                         ); i += strlen(out+i); }
  if (defn->pointlinewidth>0) { sprintf(out+i, "pointlinewidth %f ",                          defn->pointlinewidth                                    ); i += strlen(out+i); }
  if (defn->pointsize     >0) { sprintf(out+i, "pointsize %f "     ,                          defn->pointsize                                         ); i += strlen(out+i); }
  if (defn->pointtype     >0) { sprintf(out+i, "pointtype %d "     ,                          defn->pointtype                                         ); i += strlen(out+i); }

  return;
 }

void directive_seterror(Dict *command, int interactive)
 {
  char *tempstr;
  DictLookup(command,"set_option",NULL, (void **)&tempstr);
  if (tempstr != NULL)
   {
    if (!interactive) { sprintf(temp_err_string, "Error: unrecognised set option '%s'.", tempstr); ppl_error(temp_err_string); }
    else              { sprintf(temp_err_string, txt_set                               , tempstr); ppl_error(temp_err_string); }
   }
  else
   {
    if (!interactive) { ppl_error("Error: set command detected with no set option following it."); }
    else              { ppl_error(txt_set_noword); }
   }
 }

void directive_unseterror(Dict *command, int interactive)
 {
  char *tempstr;
  DictLookup(command,"set_option",NULL, (void **)&tempstr);
  if (tempstr != NULL)
   {
    if (!interactive) { sprintf(temp_err_string, "Error: unrecognised set option '%s'.", tempstr); ppl_error(temp_err_string); }
    else              { sprintf(temp_err_string, txt_unset                             , tempstr); ppl_error(temp_err_string); }
   }
  else
   {
    if (!interactive) { ppl_error("Error: unset command detected with no set option following it."); }
    else              { ppl_error(txt_unset_noword); }
   }
 }

void directive_set(Dict *command)
 {
  int     i, j, k, l, m, p, pp, errpos, multiplier;
  char   *directive, *setoption;
  value   valobj;
  value  *tempval, *tempval2;
  int    *tempint, *tempint2;
  double *tempdbl;
  char   *tempstr, *tempstr2;
  List   *templist;
  Dict   *tempdict;
  ListIterator *listiter;
  with_words *tempstyle;
  settings_graph *sg;

  sg = &settings_graph_current;

  DictLookup(command,"directive",NULL,(void **)(&directive));
  DictLookup(command,"set_option",NULL,(void **)(&setoption));

  if      ((strcmp(directive,"set")==0) && (strcmp(setoption,"arrow")==0)) /* set arrow */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"arrow")==0)) /* unset arrow */
   {
   }
  else if ((strcmp(setoption,"autoscale")==0)) /* set autoscale | unset autoscale */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"axis")==0)) /* set axis */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"axis")==0)) /* unset axis */
   {
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
    DictLookup(command,"bar_size",NULL,(void **)&tempdbl);
    sg->bar = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"bar")==0)) /* unset bar */
   {
    sg->bar = settings_graph_default.bar;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"binorigin")==0)) /* set binorigin */
   {
    DictLookup(command,"bin_origin",NULL,(void **)&tempdbl);
    sg->BinOrigin = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"binorigin")==0)) /* unset binorigin */
   {
    sg->BinOrigin = settings_graph_default.BinOrigin;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"binwidth")==0)) /* set binwidth */
   {
    DictLookup(command,"bin_width",NULL,(void **)&tempdbl);
    if (*tempdbl <= 0.0) { ppl_error("Error: width of histogram bins must be greater than zero."); return; }
    sg->BinWidth = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"binwidth")==0)) /* unset binwidth */
   {
    sg->BinWidth = settings_graph_default.BinWidth;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"boxfrom")==0)) /* set boxfrom */
   {
    DictLookup(command,"box_from",NULL,(void **)&tempdbl);
    sg->BoxFrom = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"boxfrom")==0)) /* unset boxfrom */
   {
    sg->BoxFrom = settings_graph_default.BoxFrom;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"boxwidth")==0)) /* set boxwidth */
   {
    DictLookup(command,"box_width",NULL,(void **)&tempdbl);
    sg->BoxWidth = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"boxwidth")==0)) /* unset boxwidth */
   {
    sg->BoxWidth = settings_graph_default.BoxWidth;
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
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"display")==0)) /* set display */
   {
    settings_term_current.display = SW_ONOFF_ON;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"display")==0)) /* unset display */
   {
    settings_term_current.display = settings_term_default.display;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"dpi")==0)) /* set dpi */
   {
    DictLookup(command,"dpi",NULL,(void **)&tempdbl);
    if (*tempdbl <= 2.0) { ppl_error("Error: output image resolutions below two dots per inch are not supported."); return; }
    settings_term_current.dpi = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"dpi")==0)) /* unset dpi */
   {
    settings_term_current.dpi = settings_term_default.dpi;
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
    if (tempstr2 == NULL) { ppl_warning("Warning: attempt to unset a filter which did not exist."); return; }
    DictRemoveKey(settings_filters,tempstr);
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"fontsize")==0)) /* set fontsize */
   {
    DictLookup(command,"fontsize",NULL,(void **)&tempdbl);
    if (*tempdbl <= 0.0) { ppl_error("Error: font sizes are not allowed to be less than or equal to zero."); return; }
    sg->FontSize = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"fontsize")==0)) /* unset fontsize */
   {
    sg->FontSize = settings_graph_default.FontSize;
   }
  else if ((strcmp(setoption,"axescolour")==0) || (strcmp(setoption,"gridmajcolour")==0) || (strcmp(setoption,"gridmincolour")==0) || (strcmp(setoption,"textcolour")==0)) /* set axescolour | set gridmajcolour | set gridmincolour */
   {
    if (strcmp(setoption,"axescolour"   )==0) { tempint = &sg->AxesColour    ; tempint2 = &settings_graph_default.AxesColour;    }
    if (strcmp(setoption,"gridmajcolour")==0) { tempint = &sg->GridMajColour ; tempint2 = &settings_graph_default.GridMajColour; }
    if (strcmp(setoption,"gridmincolour")==0) { tempint = &sg->GridMinColour ; tempint2 = &settings_graph_default.GridMinColour; }
    if (strcmp(setoption,"textcolour"   )==0) { tempint = &sg->TextColour    ; tempint2 = &settings_graph_default.TextColour;    }

    if (strcmp(directive,"unset")==0)
     {
      *tempint = *tempint2;
     }
    else
     {
      DictLookup(command,"colour",NULL,(void **)&tempstr);
      i = FetchSettingByName(tempstr, SW_COLOUR_INT, SW_COLOUR_STR);
      if (i >= 0)
       {
        *tempint = i;
       }
      else
       {
        j = strlen(tempstr);
        errpos = -1;
        ppl_EvaluateAlgebra(tempstr, &valobj, 0, &j, &errpos, temp_err_string, 0);
        if (errpos>=0) { ppl_error(temp_err_string); return; }
        if (!valobj.dimensionless) { sprintf(temp_err_string, "Error: colour indices should be dimensionless quantities; the specified quantity has units of <%s>.", ppl_units_GetUnitStr(&valobj, NULL, NULL, 1, 0)); ppl_error(temp_err_string); return; }
        if ((valobj.real <= INT_MIN) || (valobj.real >= INT_MAX)) { sprintf(temp_err_string, "Error: colour indices should be in the range %d to %d.", INT_MIN, INT_MAX); ppl_error(temp_err_string); return; }
        for (j=1; j<PALETTE_LENGTH; j++) if (settings_palette_current[j]==-1) break;
        *tempint = settings_palette_current[((int)valobj.real)%j];
       }
     }
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"grid")==0)) /* set grid */
   {
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
    if (*tempint <= 0.0) { ppl_error("Error: keys cannot have fewer than one columns."); return; }
    sg->KeyColumns = *tempint;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"keycolumns")==0)) /* unset keycolumns */
   {
    sg->KeyColumns = settings_graph_default.KeyColumns;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"label")==0)) /* set label */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"linestyle")==0)) /* set linestyle */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"linestyle")==0)) /* unset linestyle */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"linewidth")==0)) /* set linewidth */
   {
    DictLookup(command,"linewidth",NULL,(void **)&tempdbl);
    if (*tempdbl <= 0.0) { ppl_error("Error: line widths are not allowed to be less than or equal to zero."); return; }
    sg->LineWidth = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"linewidth")==0)) /* unset linewidth */
   {
    sg->LineWidth = settings_graph_default.LineWidth;
   }
  else if ((strcmp(setoption,"logscale")==0))         /* set logscale */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"multiplot")==0)) /* set multiplot */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"multiplot")==0)) /* unset multiplot */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nobackup")==0)) /* set nobackup */
   {
    settings_term_current.backup = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nodisplay")==0)) /* set nodisplay */
   {
    settings_term_current.display = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nogrid")==0)) /* set nogrid */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nokey")==0)) /* set nokey */
   {
    sg->key = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"label")==0)) /* set nolabel | unset label */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nologscale")==0)) /* set nologscale */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"nomultiplot")==0)) /* set nomultiplot */
   {
    settings_term_current.multiplot = SW_ONOFF_OFF;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"notics")==0)) /* set notics */
   {
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
      if (*tempint <  1) { ppl_error("Error: Numbers cannot be displayed to fewer than one significant figure."); return; }
      if (*tempint > 30) { ppl_error("Error: It is not sensible to try to display numbers to more than 30 significant figures. Calculations in PyXPlot are only accurate to double precision."); return; }
      settings_term_current.SignificantFigures = *tempint;
     }
    DictLookup(command,"typeable", NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.NumDisplayTypeable = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"numerics")==0)) /* set numerics */
   {
    settings_term_current.ComplexNumbers     = settings_term_default.ComplexNumbers;
    settings_term_current.ExplicitErrors     = settings_term_default.ExplicitErrors;
    settings_term_current.NumDisplayTypeable = settings_term_default.NumDisplayTypeable;
    settings_term_current.SignificantFigures = settings_term_default.SignificantFigures;
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
         sprintf(temp_err_string, "Error: The position supplied to the 'set origin' command must have dimensions of length. Supplied x input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 0));
         ppl_error(temp_err_string);
         return;
        }
     }
    else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
    if (!(tempval2->dimensionless))
     {
      for (i=0; i<UNITS_MAX_BASEUNITS; i++)
       if (tempval2->exponent[i] != (i==UNIT_LENGTH))
        {
         sprintf(temp_err_string, "Error: The position supplied to the 'set origin' command must have dimensions of length. Supplied y input has units of <%s>.", ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 0));
         ppl_error(temp_err_string);
         return;
        }
     }
    else { tempval2->real /= 100; } // By default, dimensionless positions are in centimetres
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
      if (i == PALETTE_LENGTH-1) { ppl_warning("Warning: The set palette command has been passed a palette which is too long; truncating it."); break; }
      tempdict = (Dict *)listiter->data;
      DictLookup(tempdict,"colour",NULL,(void **)&tempstr);
      j = FetchSettingByName(tempstr, SW_COLOUR_INT, SW_COLOUR_STR);
      if (j<0) { sprintf(temp_err_string, "Warning: The set palette command has been passed an unrecognised colour '%s'; ignoring this.", tempstr); ppl_warning(temp_err_string); }
      else     { settings_palette_current[i++] = j; }
      listiter = ListIterate(listiter, NULL);
     }
    if (i==0) { ppl_error("Error: The set palette command has been passed a palette which does not contain any colours."); return; }
    settings_palette_current[i] = -1;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"palette")==0)) /* unset palette */
   {
    for (i=0; i<PALETTE_LENGTH; i++) settings_palette_current[i] = settings_palette_default[i];
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"papersize")==0)) /* set papersize */
   {
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
    if (*tempdbl <= 0.0) { ppl_error("Error: point line widths are not allowed to be less than or equal to zero."); return; }
    sg->PointLineWidth = *tempdbl;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"pointlinewidth")==0)) /* unset pointlinewidth */
   {
    sg->PointLineWidth = settings_graph_default.PointLineWidth;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"pointsize")==0)) /* set pointsize */
   {
    DictLookup(command,"pointsize",NULL,(void **)&tempdbl);
    if (*tempdbl <= 0.0) { ppl_error("Error: point sizes are not allowed to be less than or equal to zero."); return; }
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
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"samples")==0)) /* set samples */
   {
    DictLookup(command,"samples",NULL,(void **)&tempint);
    if (*tempint <= 2.0) { ppl_error("Error: graphs cannot be constucted based on fewer than two samples."); return; }
    sg->samples = *tempint;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"samples")==0)) /* unset samples */
   {
    sg->samples = settings_graph_default.samples;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"trange")==0)) /* set trange */
   {
    DictLookup(command,"min",NULL,(void **)&tempval);
    DictLookup(command,"max",NULL,(void **)&tempval2);
    if (tempval == NULL) tempval = &sg->Tmin;
    if (tempval2== NULL) tempval2= &sg->Tmax;
    if (!ppl_units_DimEqual(tempval,tempval2)) { ppl_error("Error: Attempt to set trange with dimensionally incompatible minimum and maximum."); return; }
    sg->Tmin = *tempval;
    sg->Tmax = *tempval2;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"trange")==0)) /* unset trange */
   {
    sg->Tmin = settings_graph_default.Tmin;
    sg->Tmax = settings_graph_default.Tmax;
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
           sprintf(temp_err_string, "Error: The widths specified for graphs must have dimensions of length. Supplied value has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 0));
           ppl_error(temp_err_string);
           return;
          }
       }
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
      sg->width.real = tempval->real;
     }
    if (DictContains(command,"ratio"))
     {
      DictLookup(command,"ratio",NULL,(void **)&tempdbl);
      if ((fabs(*tempdbl) < 1e-4) || (fabs(*tempdbl) > 1e4)) { ppl_error("Error: The requested aspect ratios for graphs must be in the range 0.001 to 10000."); return; }
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
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"style")==0)) /* set style */
   {
    DictLookup(command,"dataset_type",NULL,(void **)&tempstr);
    if (tempstr[0]=='d') tempstyle = &sg->DataStyle;
    else                 tempstyle = &sg->FuncStyle;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"terminal")==0)) /* set terminal */
   {
    DictLookup(command,"term"   ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermType        = FetchSettingByName(tempstr, SW_TERMTYPE_INT, SW_TERMTYPE_STR);
    DictLookup(command,"antiali",NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.TermAntiAlias   = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"col"    ,NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.colour          = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
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
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"tics")==0)) /* unset tics */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"ticdir")==0)) /* set ticdir */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"ticdir")==0)) /* unset ticdir */
   {
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
           sprintf(temp_err_string, "Error: The offset position supplied to the 'set title' command must have dimensions of length. Supplied x input has units of <%s>.", ppl_units_GetUnitStr(tempval, NULL, NULL, 1, 0));
           ppl_error(temp_err_string);
           return;
          }
       }
      else { tempval->real /= 100; } // By default, dimensionless positions are in centimetres
     }
    if (tempval2!= NULL) 
     { 
      if (!(tempval2->dimensionless))
       {
        for (i=0; i<UNITS_MAX_BASEUNITS; i++)
         if (tempval2->exponent[i] != (i==UNIT_LENGTH))
          {
           sprintf(temp_err_string, "Error: The offset position supplied to the 'set title' command must have dimensions of length. Supplied y input has units of <%s>.", ppl_units_GetUnitStr(tempval2, NULL, NULL, 1, 0));
           ppl_error(temp_err_string);
           return;
          }
       }
      else { tempval2->real /= 100; } // By default, dimensionless positions are in centimetres
     }
    if (tempval != NULL) sg->TitleXOff.real = tempval ->real;
    if (tempval2!= NULL) sg->TitleYOff.real = tempval2->real;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"notitle")==0)) /* set notitle */
   {
    strcpy(sg->title, "");
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"title")==0)) /* unset title */
   {
    strncpy(sg->title, settings_graph_default.title, FNAME_LENGTH-4);
    sg->title[FNAME_LENGTH-4]='\0';
    sg->TitleXOff = settings_graph_default.TitleXOff;
    sg->TitleYOff = settings_graph_default.TitleYOff;
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"unit")==0)) /* set unit */
   {
    DictLookup(command,"abbrev"   , NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.UnitDisplayAbbrev   = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"prefix"  , NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.UnitDisplayPrefix   = FetchSettingByName(tempstr, SW_ONOFF_INT, SW_ONOFF_STR);
    DictLookup(command,"scheme"  , NULL,(void **)&tempstr);
    if (tempstr != NULL) settings_term_current.UnitScheme          = FetchSettingByName(tempstr, SW_UNITSCH_INT, SW_UNITSCH_STR);

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
        if (p==0) { for (k=0; ((ppl_unit_database[j].nameAp[k]!='\0') && (ppl_unit_database[j].nameAp[k]==tempstr2[k])); k++);
                    if ((ppl_unit_database[j].nameAp[k]=='\0') && (!(isalnum(tempstr2[k]) || (tempstr2[k]=='_')))) p=1; }
        if (p==0) { for (k=0; ((ppl_unit_database[j].nameAs[k]!='\0') && (ppl_unit_database[j].nameAs[k]==tempstr2[k])); k++);
                    if ((ppl_unit_database[j].nameAs[k]=='\0') && (!(isalnum(tempstr2[k]) || (tempstr2[k]=='_')))) p=1; }
        if (p==0) { for (k=0; ((ppl_unit_database[j].nameFp[k]!='\0') && (toupper(ppl_unit_database[j].nameFp[k])==toupper(tempstr2[k]))); k++);
                    if ((ppl_unit_database[j].nameFp[k]=='\0') && (!(isalnum(tempstr2[k]) || (tempstr2[k]=='_')))) p=1; }
        if (p==0) { for (k=0; ((ppl_unit_database[j].nameFs[k]!='\0') && (toupper(ppl_unit_database[j].nameFs[k])==toupper(tempstr2[k]))); k++);
                    if ((ppl_unit_database[j].nameFs[k]=='\0') && (!(isalnum(tempstr2[k]) || (tempstr2[k]=='_')))) p=1; }
        if (p==0)
         {
          for (l=ppl_unit_database[j].MinPrefix/3+8; l<=ppl_unit_database[j].MaxPrefix/3+8; l++)
           {
            if (l==8) continue;
            for (k=0; ((SIprefixes_full[l][k]!='\0') && (toupper(SIprefixes_full[l][k])==toupper(tempstr2[k]))); k++);
            if (SIprefixes_full[l][k]=='\0')
             {
              for (m=0; ((ppl_unit_database[j].nameFp[m]!='\0') && (toupper(ppl_unit_database[j].nameFp[m])==toupper(tempstr2[k+m]))); m++);
              if ((ppl_unit_database[j].nameFp[m]=='\0') && (!(isalnum(tempstr2[k+m]) || (tempstr2[k+m]=='_')))) { p=1; k+=m; multiplier=l; break; }
              for (m=0; ((ppl_unit_database[j].nameFs[m]!='\0') && (toupper(ppl_unit_database[j].nameFs[m])==toupper(tempstr2[k+m]))); m++);
              if ((ppl_unit_database[j].nameFs[m]=='\0') && (!(isalnum(tempstr2[k+m]) || (tempstr2[k+m]=='_')))) { p=1; k+=m; multiplier=l; break; }
             }
            for (k=0; ((SIprefixes_abbrev[l][k]!='\0') && (SIprefixes_abbrev[l][k]==tempstr2[k])); k++);
            if (SIprefixes_abbrev[l][k]=='\0')
             {
              for (m=0; ((ppl_unit_database[j].nameAp[m]!='\0') && (ppl_unit_database[j].nameAp[m]==tempstr2[k+m])); m++);
              if ((ppl_unit_database[j].nameAp[m]=='\0') && (!(isalnum(tempstr2[k+m]) || (tempstr2[k+m]=='_')))) { p=1; k+=m; multiplier=l; break; }
              for (m=0; ((ppl_unit_database[j].nameAs[m]!='\0') && (ppl_unit_database[j].nameAs[m]==tempstr2[k+m])); m++);
              if ((ppl_unit_database[j].nameAs[m]=='\0') && (!(isalnum(tempstr2[k+m]) || (tempstr2[k+m]=='_')))) { p=1; k+=m; multiplier=l; break; }
             }
           }
         }
        if (p==0) continue;
        if (i!=2)
         {
          if ((ppl_unit_database[j].quantity!=NULL) && (ppl_unit_database[j].quantity[0]!='\0'))
           { sprintf(temp_err_string, "Warning: '%s' is not a unit of '%s', but of '%s'.", tempstr2, tempstr, ppl_unit_database[j].quantity); ppl_warning(temp_err_string); }
          else
           { sprintf(temp_err_string, "Warning: '%s' is not a unit of '%s'.", tempstr2, tempstr); ppl_warning(temp_err_string); }
         }
        ppl_unit_database[j].UserSel = 1;
        ppl_unit_database[j].UserSelPrefix = multiplier;
        pp=1;
       }
      if (i==0) { sprintf(temp_err_string, "Warning: No such quantity as a '%s'.", tempstr); ppl_warning(temp_err_string); }
      if (p==0) { sprintf(temp_err_string, "Warning: No such unit as a '%s'.", tempstr2); ppl_warning(temp_err_string); }
      listiter = ListIterate(listiter, NULL);
     }

   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"unit")==0)) /* unset unit */
   {
    settings_term_current.UnitDisplayAbbrev   = settings_term_default.UnitDisplayAbbrev;
    settings_term_current.UnitDisplayPrefix   = settings_term_default.UnitDisplayPrefix;
    settings_term_current.UnitScheme          = settings_term_default.UnitScheme;
    for (i=0; i<ppl_unit_pos; i++) ppl_unit_database[i].UserSel = 0;
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"width")==0)) /* unset width */
   {
    sg->width.real = settings_graph_default.width.real;
   }
  else if ((strcmp(setoption,"xlabel")==0)) /* set xlabel / unset xlabel */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"range")==0)) /* set xrange */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"range")==0)) /* unset xrange */
   {
   }
  else if ((strcmp(directive,"set")==0) && (strcmp(setoption,"xformat")==0)) /* set xformat */
   {
   }
  else if ((strcmp(directive,"unset")==0) && (strcmp(setoption,"xformat")==0)) /* unset xformat */
   {
   }
  else
   {
    ppl_error("Internal Error in PyXPlot's set command: could not find handler for this set command.");
   }
  return;
 }

#define SHOW_HIGHLIGHT(modified) \
if (interactive!=0) /* On interactive sessions, highlight those settings which have been manually set by the user */ \
 { \
  if (modified == 0) strcpy(out+i, (char *)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT) ); \
  else               strcpy(out+i, (char *)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT) ); \
  i += strlen(out+i); \
 }

#define SHOW_DEHIGHLIGHT \
if (interactive!=0) /* On interactive sessions, highlight those settings which have been manually set by the user */ \
 { \
  strcpy(out+i, (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT) ); \
  i += strlen(out+i); \
 } \


void directive_show3(char *out, char *ItemSet, int interactive, char *setting_name, char *setting_value, int modified, char *description)
 {
  int i=0,j,k;

  SHOW_HIGHLIGHT(modified);

  sprintf(out+i, "set %s", ItemSet); i += strlen(out+i); // Start off with a set command

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

int directive_show2(char *word, char *ItemSet, int interactive, settings_graph *sg, settings_axis *xa, settings_axis *ya, settings_axis *za)
 {
  char *out, *buf, *buf2, *bufp;
  int   i=0, p=0,j,k,l,m;
  DictIterator *DictIter;
  FunctionDescriptor *FDiter;
  value *tempval;
  out = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Accumulate our whole output text here
  buf = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Put the value of each setting in here
  buf2= (char *)malloc(FNAME_LENGTH*sizeof(char));

  if ((out==NULL)||(buf==NULL)||(buf2==NULL))
   {
    ppl_error("Out of memory error whilst trying to allocate buffers in show command.");
    if (out!=NULL) free(out); if (buf!=NULL) free(buf); if (buf2!=NULL) free(buf2);
    return 1;
   }

  out[0] = buf[0] = '\0';
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "axescolour",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(sg->AxesColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, ItemSet, interactive, "AxesColour", buf, (settings_graph_default.AxesColour == sg->AxesColour), "The colour used to draw graph axes");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "backup", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.backup, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "backup", buf, (settings_term_default.backup == settings_term_current.backup), "Selects whether existing files are overwritten (Off) or moved (On)");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "bar",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(sg->bar,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "bar", buf, (settings_graph_default.bar == sg->bar), "Sets the size of the strokes which mark the lower and upper limits of errorbars");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binorigin",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(sg->BinOrigin,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "BinOrigin", buf, (settings_graph_default.BinOrigin == sg->BinOrigin), "this sets the something");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binwidth",1)>=0))
   { 
    if (sg->BinWidth > 0.0) sprintf(buf, "%s",(char *)NumericDisplay(sg->BinWidth,0,settings_term_current.SignificantFigures));
    else                    sprintf(buf, "auto");
    directive_show3(out+i, ItemSet, interactive, "BinWidth", buf, (settings_graph_default.BinWidth == sg->BinWidth), "Sets the width of bins used when constructing histograms");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxwidth",1)>=0))
   {
    if (sg->BoxWidth > 0.0) sprintf(buf, "%s",(char *)NumericDisplay(sg->BoxWidth,0,settings_term_current.SignificantFigures));
    else                    sprintf(buf, "auto");
    directive_show3(out+i, ItemSet, interactive, "BoxWidth", buf, (settings_graph_default.BoxWidth == sg->BoxWidth), "Sets the width of bars on barcharts and histograms");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxfrom",1)>=0))
   {
    sprintf(buf, "%s", (char *)NumericDisplay(sg->BoxFrom,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "BoxFrom", buf, (settings_graph_default.BoxFrom == sg->BoxFrom), "Sets the vertical level from which the bars of barcharts and histograms are drawn");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "calendarin",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.CalendarIn, SW_CALENDAR_INT, (void **)SW_CALENDAR_STR));
    directive_show3(out+i, ItemSet, interactive, "calendarin", buf, (settings_term_current.CalendarIn == settings_term_default.CalendarIn), "Selects the historical year in which the transition is made between Julian and Gregorian calendars when dates are being input");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "calendarout",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.CalendarOut, SW_CALENDAR_INT, (void **)SW_CALENDAR_STR));
    directive_show3(out+i, ItemSet, interactive, "calendarout", buf, (settings_term_current.CalendarOut == settings_term_default.CalendarOut), "Selects the historical year in which the transition is made between Julian and Gregorian calendars when displaying dates");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "display", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.display, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "display", buf, (settings_term_default.display == settings_term_current.display), "Sets whether any output is produced; turn on to improve performance whilst setting up large multiplots");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "dpi", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(settings_term_current.dpi,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "DPI", buf, (settings_term_default.dpi == settings_term_current.dpi), "Sets the pixel resolution used when producing gif, jpg or png output");
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
      directive_show3(out+i, ItemSet, interactive, "filter", buf, (!tempval->modified), "Sets an input filter to be used when reading datafiles");
      i += strlen(out+i) ; p=1;
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "fontsize",1)>=0) || (StrAutocomplete(word, "fountsize",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(sg->FontSize,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "FountSize", buf, (settings_graph_default.FontSize == sg->FontSize), "Sets the fount size of text output: 1.0 is the default, and other values multiply this default size");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "grid",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(sg->grid, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "grid", buf, (settings_graph_default.grid == sg->grid), "Selects whether a grid is drawn on plots");
    i += strlen(out+i) ; p=1;
    if (sg->grid == SW_ONOFF_ON)
     {
      bufp = buf; k=1;
      for (j=0; j<MAX_AXES; j++)
       {
        if (sg->GridAxisX[j] != 0                                   ) { sprintf(bufp, "x%d", j); bufp += strlen(bufp); }
        if (sg->GridAxisX[j] != settings_graph_default.GridAxisX[j] ) k=0;
       }
      if (bufp != buf) directive_show3(out+i, ItemSet, interactive, "grid", buf, k, "Sets the x axis with whose ticks gridlines are associated");
      i += strlen(out+i);

      bufp = buf; k=1;
      for (j=0; j<MAX_AXES; j++)
       {
        if (sg->GridAxisY[j] != 0                                   ) { sprintf(bufp, "y%d", j); bufp += strlen(bufp); }
        if (sg->GridAxisY[j] != settings_graph_default.GridAxisY[j] ) k=0;
       }
      if (bufp != buf) directive_show3(out+i, ItemSet, interactive, "grid", buf, k, "Sets the y axis with whose ticks gridlines are associated");
      i += strlen(out+i);

      bufp = buf; k=1;
      for (j=0; j<MAX_AXES; j++)
       {
        if (sg->GridAxisZ[j] != 0                                   ) { sprintf(bufp, "z%d", j); bufp += strlen(bufp); }
        if (sg->GridAxisZ[j] != settings_graph_default.GridAxisZ[j] ) k=0;
       }
      if (bufp != buf) directive_show3(out+i, ItemSet, interactive, "grid", buf, k, "Sets the z axis with whose ticks gridlines are associated");
      i += strlen(out+i);
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "gridmajcolour",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(sg->GridMajColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, ItemSet, interactive, "GridMajColour", buf, (settings_graph_default.GridMajColour == sg->GridMajColour), "The colour of the major gridlines on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "gridmincolour",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(sg->GridMinColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, ItemSet, interactive, "GridMinColour", buf, (settings_graph_default.GridMinColour == sg->GridMinColour), "The colour of the minor gridlines on graphs");
    i += strlen(out+i) ; p=1;
   }

  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "key",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(sg->key, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "key", buf, (settings_graph_default.key == sg->key), "Selects whether a legend is included on plots");
    i += strlen(out+i) ; p=1;
   }
  if ( ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "key",1)>=0)) && (sg->key == SW_ONOFF_ON)  )
   {
    sprintf(buf, "%s %s , %s", (char *)FetchSettingName(sg->KeyPos, SW_KEYPOS_INT, (void **)SW_KEYPOS_STR),ppl_units_NumericDisplay(&(sg->KeyXOff),0,0),ppl_units_NumericDisplay(&(sg->KeyYOff),1,0));
    directive_show3(out+i, ItemSet, interactive, "key", buf, ((settings_graph_default.KeyPos == sg->KeyPos)&&(settings_graph_default.KeyXOff.real == sg->KeyXOff.real)&&(settings_graph_default.KeyYOff.real == sg->KeyYOff.real)), "Selects where legends are orientated on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "keycolumns",1)>=0))
   { 
    sprintf(buf, "%d", sg->KeyColumns);
    directive_show3(out+i, ItemSet, interactive, "KeyColumns", buf, (settings_graph_default.KeyColumns == sg->KeyColumns), "Sets the number of columns into which legends on graphs are sorted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "linewidth", 1)>=0) || (StrAutocomplete(word, "lw", 2)>=0))
   { 
    sprintf(buf, "%s", NumericDisplay(sg->LineWidth,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "LineWidth", buf, (settings_graph_default.LineWidth == sg->LineWidth), "Sets the widths of lines drawn on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "multiplot", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.multiplot, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "multiplot", buf, (settings_term_default.multiplot == settings_term_current.multiplot), "Selects whether multiplot mode is currently active");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "numerics", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.ComplexNumbers, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "numerics complex", buf, (settings_term_default.ComplexNumbers==settings_term_current.ComplexNumbers), "Selects whether numbers are allowed to have imagnary components; affects the behaviour of functions such as sqrt()");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.ExplicitErrors,  SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "numerics errors explicit", buf, (settings_term_default.ExplicitErrors==settings_term_current.ExplicitErrors), "Selects whether numerical errors quietly produce not-a-number results, or throw explicit errors");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)NumericDisplay(settings_term_current.SignificantFigures,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "numerics sigfig", buf, (settings_term_default.SignificantFigures == settings_term_current.SignificantFigures), "Sets the (minimum) number of significant figures to which decimal numbers are displayed by default");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.NumDisplayTypeable, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "numerics typeable", buf, (settings_term_default.NumDisplayTypeable==settings_term_current.NumDisplayTypeable), "Selects whether numerical results are displayed in a way which can be copied into a terminal");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "origin", 1)>=0))
   { 
    sprintf(buf, "%s , %s", ppl_units_NumericDisplay(&(sg->OriginX),0,0), ppl_units_NumericDisplay(&(sg->OriginY),1,0));
    directive_show3(out+i, ItemSet, interactive, "origin", buf, ((settings_graph_default.OriginX.real == sg->OriginX.real)&&(settings_graph_default.OriginY.real == sg->OriginY.real)), "Selects where the bottom-left corners of graphs are located on multiplot pages");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "output", 1)>=0))
   { 
    StrEscapify(settings_term_current.output, buf);
    directive_show3(out+i, ItemSet, interactive, "output", buf, (strcmp(settings_term_default.output,settings_term_current.output)==0), "Filename to which graphic output is sent");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "palette",1)>=0))
   {
    l=0;
    for (j=0; j<PALETTE_LENGTH; j++) // Check whether the palette has been changed from its default setting
     {
      if ((settings_palette_current[j] == -1) && (settings_palette_default[j] == -1)) break;
      if  (settings_palette_current[j] == settings_palette_default[j]) continue;
      l=1; break;
     }
    for (j=k=0; settings_palette_current[j]>0; j++)
     {
      if (j>0) { sprintf(buf+k, ", "); k+=strlen(buf+k); }
      sprintf(buf+k, "%s", (char *)FetchSettingName(settings_palette_current[j], SW_COLOUR_INT, (void **)SW_COLOUR_STR)); k+=strlen(buf+k);
     }
    directive_show3(out+i, ItemSet, interactive, "palette", buf, !l, "The sequence of colours used to plot datasets on colour graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "papersize", 1)>=0))
   {
    sprintf(buf, "%s , %s", ppl_units_NumericDisplay(&(settings_term_current.PaperWidth),0,0), ppl_units_NumericDisplay(&(settings_term_current.PaperHeight),1,0));
    directive_show3(out+i, ItemSet, interactive, "PaperSize", buf, ((settings_term_default.PaperWidth.real==settings_term_current.PaperWidth.real)&&(settings_term_default.PaperHeight.real==settings_term_current.PaperHeight.real)), "The current papersize for postscript output, in mm");
    i += strlen(out+i) ; p=1;
    if (StrAutocomplete("user", settings_term_current.PaperName, 1)<0)
     {
      StrEscapify(settings_term_current.PaperName,buf);
      directive_show3(out+i, ItemSet, interactive, "PaperSize", buf, (strcmp(settings_term_default.PaperName, settings_term_current.PaperName)==0), NULL);
      i += strlen(out+i) ; p=1;
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointlinewidth",1)>=0) || (StrAutocomplete(word, "plw",3)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(sg->PointLineWidth,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "PointLineWidth", buf, (settings_graph_default.PointLineWidth==sg->PointLineWidth), "The width of the strokes used to mark points on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointsize",1)>=0) || (StrAutocomplete(word, "ps",2)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(sg->PointSize,0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "PointSize", buf, (settings_graph_default.PointSize==sg->PointSize), "The size of points marked on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "preamble", 1)>=0))
   {
    StrEscapify(settings_term_current.LatexPreamble,buf);
    directive_show3(out+i, ItemSet, interactive, "preamble", buf, (strcmp(settings_term_default.LatexPreamble,settings_term_current.LatexPreamble)==0), "Configuration options sent to the LaTeX typesetting system");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "samples",1)>=0))
   {
    sprintf(buf, "%d", sg->samples);
    directive_show3(out+i, ItemSet, interactive, "samples", buf, (settings_graph_default.samples==sg->samples), "The number of samples taken when functions are plotted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "size",1)>=0))
   {
    if (sg->AutoAspect == SW_ONOFF_ON) sprintf(buf, "auto");
    else                               sprintf(buf, "%s", NumericDisplay(sg->aspect, 0,settings_term_current.SignificantFigures));
    directive_show3(out+i, ItemSet, interactive, "size ratio", buf, ((settings_graph_default.aspect==sg->aspect)&&(settings_graph_default.AutoAspect==sg->AutoAspect)), "The aspect-ratio of graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "data", 1)>=0) || (StrAutocomplete(word, "style", 1)>=0))
   {
    with_words_print(&sg->DataStyle, buf);
    directive_show3(out+i, ItemSet, interactive, "data style", buf, with_words_compare(&settings_graph_default.DataStyle,&sg->DataStyle), "Default plot options for plotting datafiles");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "function", 1)>=0) || (StrAutocomplete(word, "style", 1)>=0))
   {
    with_words_print(&sg->FuncStyle, buf);
    directive_show3(out+i, ItemSet, interactive, "function style", buf, with_words_compare(&settings_graph_default.FuncStyle,&sg->FuncStyle), "Default plot options for plotting functions");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermType, SW_TERMTYPE_INT, (void **)SW_TERMTYPE_STR));
    directive_show3(out+i, ItemSet, interactive, "terminal", buf, (settings_term_default.TermType==settings_term_current.TermType), "The type of graphic output to be produced");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "antialias",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermAntiAlias, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "terminal AntiAlias", buf, (settings_term_default.TermAntiAlias==settings_term_current.TermAntiAlias), "Selects whether anti-aliasing is applied to bitmap output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "colour", 1)>=0) || (StrAutocomplete(word, "color",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.colour, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "terminal colour", buf, (settings_term_default.colour==settings_term_current.colour), "Selects whether output is colour or monochrome");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "enlargement",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermEnlarge, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "terminal enlarge", buf, (settings_term_default.TermEnlarge==settings_term_current.TermEnlarge), "Selects whether output photo-enlarged to fill the page");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "invert",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermInvert, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "terminal invert", buf, (settings_term_default.TermInvert==settings_term_current.TermInvert), "Selects whether the colours of bitmap output are inverted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "landscape", 1)>=0) || (StrAutocomplete(word, "portrait", 1)>=0))
   {
    if (settings_term_current.landscape == SW_ONOFF_ON) sprintf(buf, "Landscape");
    else                                                sprintf(buf, "Portrait");
    directive_show3(out+i, ItemSet, interactive, "terminal", buf, (settings_term_default.landscape==settings_term_current.landscape), "Selects the orientation of output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "transparent", 1)>=0) || (StrAutocomplete(word, "solid", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermTransparent, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "terminal transparent", buf, (settings_term_default.TermTransparent==settings_term_current.TermTransparent), "Selects whether gif and png output is transparent");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "textcolour",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(sg->TextColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, ItemSet, interactive, "TextColour", buf, (settings_graph_default.TextColour==sg->TextColour), "Selects the colour of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "texthalign",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(sg->TextHAlign, SW_HALIGN_INT, (void **)SW_HALIGN_STR));
    directive_show3(out+i, ItemSet, interactive, "TextHAlign", buf, (settings_graph_default.TextHAlign==sg->TextHAlign), "Selects the horizontal alignment of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "textvalign",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(sg->TextVAlign, SW_VALIGN_INT, (void **)SW_VALIGN_STR));
    directive_show3(out+i, ItemSet, interactive, "TextVAlign", buf, (settings_graph_default.TextVAlign==sg->TextVAlign), "Selects the vertical alignment of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "title", 1)>=0))
   {
    StrEscapify(sg->title, buf); k = strlen(buf);
    sprintf(buf+k, " %s , %s", ppl_units_NumericDisplay(&(sg->TitleXOff), 0, 0), ppl_units_NumericDisplay(&(sg->TitleYOff), 1, 0));
    directive_show3(out+i, ItemSet, interactive, "title", buf, ((strcmp(settings_graph_default.title,sg->title)==0)&&(settings_graph_default.TitleXOff.real==sg->TitleXOff.real)&&(settings_graph_default.TitleYOff.real==sg->TitleYOff.real)), "A title to be displayed above graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "trange", 1)>=0))
   {
    sprintf(buf, "[%s:%s]", ppl_units_NumericDisplay(&(sg->Tmin), 0, 0), ppl_units_NumericDisplay(&(sg->Tmax), 1, 0));
    directive_show3(out+i, ItemSet, interactive, "trange", buf, (settings_graph_default.Tmin.real==sg->Tmin.real)&&ppl_units_DimEqual(&(settings_graph_default.Tmin),&(sg->Tmin))&&(settings_graph_default.Tmax.real==sg->Tmax.real)&&ppl_units_DimEqual(&(settings_graph_default.Tmax),&(sg->Tmax)), "The range of input values used in constructing parametric function plots");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "units", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.UnitDisplayAbbrev, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "unit display abbreviated", buf, (settings_term_default.UnitDisplayAbbrev==settings_term_current.UnitDisplayAbbrev), "Selects whether units are displayed in abbreviated form ('m' vs. 'metres')");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.UnitDisplayPrefix, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "unit display prefix", buf, (settings_term_default.UnitDisplayPrefix==settings_term_current.UnitDisplayPrefix), "Selects whether SI units are displayed with prefixes");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.UnitScheme, SW_UNITSCH_INT, (void **)SW_UNITSCH_STR));
    directive_show3(out+i, ItemSet, interactive, "unit scheme", buf, (settings_term_default.UnitScheme==settings_term_current.UnitScheme), "Selects the scheme (e.g. SI or Imperial) of prefered units");
    i += strlen(out+i) ; p=1;
    for (j=0; j<ppl_unit_pos; j++) if (ppl_unit_database[j].UserSel != 0)
     {
      sprintf(buf, "unit of %s", ppl_unit_database[j].quantity);
      if (settings_term_current.UnitDisplayAbbrev == SW_ONOFF_ON) sprintf(buf2, "%s%s", SIprefixes_abbrev[ppl_unit_database[j].UserSelPrefix], ppl_unit_database[j].nameAs);
      else                                                        sprintf(buf2, "%s%s", SIprefixes_full  [ppl_unit_database[j].UserSelPrefix], ppl_unit_database[j].nameFs);
      directive_show3(out+i, ItemSet, interactive, buf, buf2, 0, "Selects a user-prefered unit for a particular quantity");
      i += strlen(out+i) ; p=1;
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "width", 1)>=0) || (StrAutocomplete(word, "size", 1)>=0))
   { 
    sprintf(buf, "%s", ppl_units_NumericDisplay(&(sg->width), 0, 0));
    directive_show3(out+i, ItemSet, interactive, "width", buf, (settings_graph_default.width.real==sg->width.real), "The width of graphs");
    i += strlen(out+i) ; p=1;
   }
  if (StrAutocomplete(word, "variables", 1)>=0)
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
          sprintf(out+i, "%s = %s\n", DictIter->key, ppl_units_NumericDisplay((value *)DictIter->data, 0, 0));
         }
        i += strlen(out+i);
        SHOW_DEHIGHLIGHT;
       }
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }
  if (StrAutocomplete(word, "functions", 1)>=0)
   {
    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# System-Defined Functions:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;

    DictIter = DictIterateInit(_ppl_UserSpace_Funcs);
    while (DictIter != NULL)
     {
      FDiter = (FunctionDescriptor *)DictIter->data;
      SHOW_HIGHLIGHT((FDiter->modified==0));
      if ( (FDiter->FunctionType != PPL_USERSPACE_USERDEF) && (FDiter->FunctionType != PPL_USERSPACE_SPLINE) )
       {
        sprintf(out+i, "# %-15s: %s.\n", DictIter->key, FDiter->description);
       }
      i += strlen(out+i);
      SHOW_DEHIGHLIGHT;
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }
  if ((StrAutocomplete(word, "functions", 1)>=0) || (StrAutocomplete(word, "userfunctions", 1)>=0))
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
        if ( (FDiter->FunctionType == PPL_USERSPACE_USERDEF) || (FDiter->FunctionType == PPL_USERSPACE_SPLINE) )
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
            if (FDiter->MinActive[k]) { sprintf(out+i,"%s", ppl_units_NumericDisplay(FDiter->min+k, 0, 0)); i+=strlen(out+i); }
            *(out+(i++)) = ':';
            if (FDiter->MaxActive[k]) { sprintf(out+i,"%s", ppl_units_NumericDisplay(FDiter->max+k, 0, 0)); i+=strlen(out+i); }
            *(out+(i++)) = ']';
           }
          sprintf(out+i,"=%s\n",(char *)FDiter->description); i+=strlen(out+i);
          SHOW_DEHIGHLIGHT;
         }
        FDiter = FDiter->next;
       }
      DictIter = DictIterate(DictIter, NULL, NULL);
     }
   }
  if (StrAutocomplete(word, "units", 1)>=0)
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
        sprintf(out+i, "# The '%s', also known as", ppl_unit_database[m].nameFs); i+=strlen(out+i);
        if (strcmp(ppl_unit_database[m].nameFp, ppl_unit_database[m].nameFs) != 0) { sprintf(out+i, " '%s' or", ppl_unit_database[m].nameFp); i+=strlen(out+i); k=1; }
        if (strcmp(ppl_unit_database[m].nameAs, ppl_unit_database[m].nameFs) != 0) { sprintf(out+i, " '%s' or", ppl_unit_database[m].nameAs); i+=strlen(out+i); k=1; }
        if((strcmp(ppl_unit_database[m].nameAp, ppl_unit_database[m].nameAs) != 0) &&
           (strcmp(ppl_unit_database[m].nameAp, ppl_unit_database[m].nameFp) != 0)){ sprintf(out+i, " '%s' or", ppl_unit_database[m].nameAp); i+=strlen(out+i); k=1; }
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
  settings_graph *sg;
  settings_axis  *xa, *ya, *za;

  List         *ShowList;
  ListIterator *ShowIterate;
  Dict         *ShowWordDict;
  char         *ShowWord;
  char          TextBuffer[SSTR_LENGTH], ItemSet[32];
  int           i=0, p=0, *EditNo;

  interactive = ( interactive && (settings_session_default.colour == SW_ONOFF_ON) );

  DictLookup(command, "editno"      , NULL, (void **)&EditNo);
  if (EditNo == NULL)
   {
    sg = &settings_graph_current;
    xa = XAxes; ya = YAxes; za = ZAxes;
    ItemSet[0]='\0';
   } else {
    if ((*EditNo<0) || (*EditNo>MULTIPLOT_MAXINDEX))
     { sprintf(temp_err_string, "There is no multiplot item with number %d.", *EditNo); ppl_error(temp_err_string); return; }
    { sprintf(temp_err_string, "There is no multiplot item with number %d.", *EditNo); ppl_error(temp_err_string); return; }
    sprintf(ItemSet, "item %d ", *EditNo);
   }

  DictLookup(command, "setting_list", NULL, (void **)&ShowList);
  if ((ShowList==NULL) || (ListLen(ShowList) == 0))
   { ppl_error(txt_show); }
  else
   {
    if (interactive!=0) // On interactive sessions, highlight those settings which have been manually set by the user
     {
      sprintf(TextBuffer+i,"%sSettings which have not been changed by the user are shown in %s.%s\n",
              (char *)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT),
              (char *)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void **)SW_TERMCOL_STR),
              (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT)
             );
      i += strlen(TextBuffer+i);
      sprintf(TextBuffer+i,"%sSettings which have been changed by the user are shown in %s.%s\n",
              (char *)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT),
              (char *)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void **)SW_TERMCOL_STR),
              (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT)
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
        directive_show2("settings"  ,ItemSet, interactive, sg, xa, ya, za);
        directive_show2("axes_"     ,ItemSet, interactive, sg, xa, ya, za);
        directive_show2("linestyles",ItemSet, interactive, sg, xa, ya, za);
        directive_show2("variables" ,ItemSet, interactive, sg, xa, ya, za);
        directive_show2("functions" ,ItemSet, interactive, sg, xa, ya, za);
        directive_show2("units"     ,ItemSet, interactive, sg, xa, ya, za);
        p=1;
       }
      else
       {
        p = (directive_show2(ShowWord, ItemSet, interactive, sg, xa, ya, za) || p);
       }
     }
    if (p==0) { ppl_error("Invalid show option."); ppl_error(txt_show); }
   }
  return;
 }

