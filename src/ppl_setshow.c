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

void directive_set(Dict *command)
 {
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
  char *out, *buf, *bufp;
  int   i=0, p=0,j,k,l,m;
  DictIterator *DictIter;
  FunctionDescriptor *FDiter;
  out = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Accumulate our whole output text here
  buf = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Put the value of each setting in here
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
    sprintf(buf, "%s", (char *)NumericDisplay(sg->bar,0));
    directive_show3(out+i, ItemSet, interactive, "bar", buf, (settings_graph_default.bar == sg->bar), "Sets the size of the strokes which mark the lower and upper limits of errorbars");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binorigin",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(sg->BinOrigin,0));
    directive_show3(out+i, ItemSet, interactive, "BinOrigin", buf, (settings_graph_default.BinOrigin == sg->BinOrigin), "this sets the something");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binwidth",1)>=0))
   { 
    if (sg->BinWidth > 0.0) sprintf(buf, "%s",(char *)NumericDisplay(sg->BinWidth,0));
    else                    sprintf(buf, "auto");
    directive_show3(out+i, ItemSet, interactive, "BinWidth", buf, (settings_graph_default.BinWidth == sg->BinWidth), "Sets the width of bins used when constructing histograms");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxwidth",1)>=0))
   {
    if (sg->BoxWidth > 0.0) sprintf(buf, "%s",(char *)NumericDisplay(sg->BoxWidth,0));
    else                    sprintf(buf, "auto");
    directive_show3(out+i, ItemSet, interactive, "BoxWidth", buf, (settings_graph_default.BoxWidth == sg->BoxWidth), "Sets the width of bars on barcharts and histograms");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxfrom",1)>=0))
   {
    sprintf(buf, "%s", (char *)NumericDisplay(sg->BoxFrom,0));
    directive_show3(out+i, ItemSet, interactive, "BoxFrom", buf, (settings_graph_default.BoxFrom == sg->BoxFrom), "Sets the vertical level from which the bars of barcharts and histograms are drawn");
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
    sprintf(buf, "%s", (char *)NumericDisplay(settings_term_current.dpi,0));
    directive_show3(out+i, ItemSet, interactive, "DPI", buf, (settings_term_default.dpi == settings_term_current.dpi), "Sets the pixel resolution used when producing gif, jpg or png output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "fontsize",1)>=0) || (StrAutocomplete(word, "fountsize",1)>=0))
   { 
    sprintf(buf, "%d", sg->FontSize);
    directive_show3(out+i, ItemSet, interactive, "FountSize", buf, (settings_graph_default.FontSize == sg->FontSize), "Sets the default fount size of text output; -4 is the smallest and 5 is the largest");
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
    directive_show3(out+i, ItemSet, interactive, "key", buf, ((settings_graph_default.KeyPos == sg->KeyPos)&&(settings_graph_default.KeyXOff.number == sg->KeyXOff.number)&&(settings_graph_default.KeyYOff.number == sg->KeyYOff.number)), "Selects where legends are orientated on graphs");
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
    sprintf(buf, "%s", NumericDisplay(sg->LineWidth,0));
    directive_show3(out+i, ItemSet, interactive, "LineWidth", buf, (settings_graph_default.LineWidth == sg->LineWidth), "Sets the widths of lines drawn on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "multiplot", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.multiplot, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "multiplot", buf, (settings_term_default.multiplot == settings_term_current.multiplot), "Selects whether multiplot mode is currently active");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "origin", 1)>=0))
   { 
    sprintf(buf, "%s , %s", ppl_units_NumericDisplay(&(sg->OriginX),0,0), ppl_units_NumericDisplay(&(sg->OriginY),1,0));
    directive_show3(out+i, ItemSet, interactive, "origin", buf, ((settings_graph_default.OriginX.number == sg->OriginX.number)&&(settings_graph_default.OriginY.number == sg->OriginY.number)), "Selects where, in cm, the bottom-left corners of graphs are located on multiplot pages");
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
    for (j=k=0; settings_palette[j]>0; j++)
     {
      if (j>0) { sprintf(buf+k, ", "); k+=strlen(buf+k); }
      sprintf(buf+k, "%s", (char *)FetchSettingName(settings_palette[j], SW_COLOUR_INT, (void **)SW_COLOUR_STR)); k+=strlen(buf+k);
     }
    directive_show3(out+i, ItemSet, interactive, "palette", buf, (0==0), "The sequence of colours used to plot datasets on colour graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "papersize", 1)>=0))
   {
    sprintf(buf, "%s , %s", ppl_units_NumericDisplay(&(settings_term_current.PaperWidth),0,0), ppl_units_NumericDisplay(&(settings_term_current.PaperHeight),1,0));
    directive_show3(out+i, ItemSet, interactive, "PaperSize", buf, ((settings_term_default.PaperWidth.number==settings_term_current.PaperWidth.number)&&(settings_term_default.PaperHeight.number==settings_term_current.PaperHeight.number)), "The current papersize for postscript output, in mm");
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
    sprintf(buf, "%s", NumericDisplay(sg->PointLineWidth,0));
    directive_show3(out+i, ItemSet, interactive, "PointLineWidth", buf, (settings_graph_default.PointLineWidth==sg->PointLineWidth), "The width of the strokes used to mark points on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointsize",1)>=0) || (StrAutocomplete(word, "ps",2)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(sg->PointSize,0));
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
    else                               sprintf(buf, "%s", NumericDisplay(sg->aspect, 0));
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
    directive_show3(out+i, ItemSet, interactive, "title", buf, ((strcmp(settings_graph_default.title,sg->title)==0)&&(settings_graph_default.TitleXOff.number==sg->TitleXOff.number)&&(settings_graph_default.TitleYOff.number==sg->TitleYOff.number)), "A title to be display above graphs");
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
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.UnitDisplayTypeable, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, ItemSet, interactive, "unit display typeable", buf, (settings_term_default.UnitDisplayTypeable==settings_term_current.UnitDisplayTypeable), "Selects whether units are displayed in a way which can be copied into a terminal");
    i += strlen(out+i) ; p=1;
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.UnitScheme, SW_UNITSCH_INT, (void **)SW_UNITSCH_STR));
    directive_show3(out+i, ItemSet, interactive, "unit scheme", buf, (settings_term_default.UnitScheme==settings_term_current.UnitScheme), "Selects the scheme (e.g. SI or Imperial) of prefered units");
    i += strlen(out+i) ; p=1;
    for (j=0; j<ppl_unit_pos; j++) if (ppl_unit_database[j].UserSel != 0)
     {
      sprintf(buf, "unit of %s", ppl_unit_database[j].quantity);
      directive_show3(out+i, ItemSet, interactive, buf, ppl_unit_database[j].nameFs, 0, "Selects a user-prefered unit for a particular quantity");
      i += strlen(out+i) ; p=1;
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "width", 1)>=0) || (StrAutocomplete(word, "size", 1)>=0))
   { 
    sprintf(buf, "%s", ppl_units_NumericDisplay(&(sg->width), 0, 0));
    directive_show3(out+i, ItemSet, interactive, "width", buf, (settings_graph_default.width.number==sg->width.number), "The width, in cm, of graphs");
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
      if (DictIter->DataType == DATATYPE_STRING)
       {
        SHOW_HIGHLIGHT(0);
        StrEscapify((char *)DictIter->data, buf);
        sprintf(out+i, "%s = \"%s\"\n", DictIter->key, buf);
        i += strlen(out+i);
        SHOW_DEHIGHLIGHT;
       }
      else if (DictIter->DataType == DATATYPE_VALUE)
       {
        SHOW_HIGHLIGHT((((value *)DictIter->data)->modified==0));
        sprintf(out+i, "%s = %s\n", DictIter->key, ppl_units_NumericDisplay((value *)DictIter->data, 0, 0));
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

    SHOW_HIGHLIGHT(1);
    sprintf(out+i, "\n# User-Defined Functions:\n\n"); i += strlen(out+i); p=1;
    SHOW_DEHIGHLIGHT;

    DictIter = DictIterateInit(_ppl_UserSpace_Funcs);
    while (DictIter != NULL)
     {
      FDiter = (FunctionDescriptor *)DictIter->data;
      while (FDiter != NULL)
       {
        SHOW_HIGHLIGHT((FDiter->modified==0));
        if (FDiter->FunctionType == PPL_USERSPACE_USERDEF)
         {
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
          sprintf(out+i,"=%s\n",(char *)FDiter->FunctionPtr); i+=strlen(out+i);
         }
        else if (FDiter->FunctionType == PPL_USERSPACE_SPLINE)
         {
          sprintf(out+i, "%s\n", FDiter->description);
         }
        i += strlen(out+i);
        SHOW_DEHIGHLIGHT;
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
        if (strcmp(ppl_unit_database[m].nameAp, ppl_unit_database[m].nameAs) != 0) { sprintf(out+i, " '%s' or", ppl_unit_database[m].nameAp); i+=strlen(out+i); k=1; }
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
  free(out); free(buf);
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

