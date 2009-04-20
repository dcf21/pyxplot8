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

void directive_show3(char *out, int interactive, char *setting_name, char *setting_value, int modified, char *description)
 {
  int i=0,j,k;

  if (interactive!=0) // On interactive sessions, highlight those settings which have been manually set by the user
   {
    if (modified == 0) strcpy(out+i, (char *)FetchSettingName( settings_session_default.colour_wrn , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT) );
    else               strcpy(out+i, (char *)FetchSettingName( settings_session_default.colour_rep , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT) );
    i += strlen(out+i);
   }

  strcpy(out+i, "set "); i += strlen(out+i); // Start off with a set command

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

  if (interactive!=0) // On interactive sessions, highlight those settings which have been manually set by the user
   {
    strcpy(out+i, (char *)FetchSettingName( SW_TERMCOL_NOR                      , SW_TERMCOL_INT , (void **)SW_TERMCOL_TXT) );
    i += strlen(out+i);
   }
  return;
 }

void directive_show2(char *word, int interactive)
 {
  char *out, *buf;
  int   i=0, p=0,j,k;
  out = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Accumulate our whole output text here
  buf = (char *)malloc(LSTR_LENGTH*sizeof(char)); // Put the value of each setting in here
  out[0] = buf[0] = '\0';
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "axescolour",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.AxesColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, interactive, "AxesColour", buf, (settings_graph_default.AxesColour == settings_graph_current.AxesColour), "The colour used to draw graph axes");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "backup", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.backup, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "backup", buf, (settings_term_default.backup == settings_term_current.backup), "Selects whether existing files are overwritten (Off) or moved (On)");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "bar",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(settings_graph_default.bar,0));
    directive_show3(out+i, interactive, "bar", buf, (settings_graph_default.bar == settings_graph_current.bar), "Sets the size of the strokes which mark the lower and upper limits of errorbars");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binorigin",1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(settings_graph_current.BinOrigin,0));
    directive_show3(out+i, interactive, "BinOrigin", buf, (settings_graph_default.BinOrigin == settings_graph_current.BinOrigin), "this sets the something");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "binwidth",1)>=0))
   { 
    if (settings_graph_current.BinWidth > 0.0) sprintf(buf, "%s",(char *)NumericDisplay(settings_graph_current.BinWidth,0));
    else                                       sprintf(buf, "auto");
    directive_show3(out+i, interactive, "BinWidth", buf, (settings_graph_default.BinWidth == settings_graph_current.BinWidth), "Sets the width of bins used when constructing histograms");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxwidth",1)>=0))
   {
    if (settings_graph_current.BoxWidth > 0.0) sprintf(buf, "%s",(char *)NumericDisplay(settings_graph_current.BoxWidth,0));
    else                                       sprintf(buf, "auto");
    directive_show3(out+i, interactive, "BoxWidth", buf, (settings_graph_default.BoxWidth == settings_graph_current.BoxWidth), "Sets the width of bars on barcharts and histograms");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "boxfrom",1)>=0))
   {
    sprintf(buf, "%s", (char *)NumericDisplay(settings_graph_current.BoxFrom,0));
    directive_show3(out+i, interactive, "BoxFrom", buf, (settings_graph_default.BoxFrom == settings_graph_current.BoxFrom), "Sets the vertical level from which the bars of barcharts and histograms are drawn");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "display", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.display, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "display", buf, (settings_term_default.display == settings_term_current.display), "Sets whether any output is produced; turn on to improve performance whilst setting up large multiplots");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "dpi", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)NumericDisplay(settings_term_current.dpi,0));
    directive_show3(out+i, interactive, "DPI", buf, (settings_term_default.dpi == settings_term_current.dpi), "Sets the pixel resolution used when producing gif, jpg or png output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "fontsize",1)>=0) || (StrAutocomplete(word, "fountsize",1)>=0))
   { 
    sprintf(buf, "%d", settings_graph_current.FontSize);
    directive_show3(out+i, interactive, "FountSize", buf, (settings_graph_default.FontSize == settings_graph_current.FontSize), "Sets the default fount size of text output; -4 is the smallest and 5 is the largest");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "grid",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.grid, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "grid", buf, (settings_graph_default.grid == settings_graph_current.grid), "Selects whether a grid is drawn on plots");
    i += strlen(out+i) ; p=1;
    if (settings_graph_current.grid == SW_ONOFF_ON)
     {
      sprintf(buf, "x%d", settings_graph_default.GridAxisX);
      directive_show3(out+i, interactive, "grid", buf, (settings_graph_default.GridAxisX == settings_graph_current.GridAxisX), "Sets the X-axis with whose ticks gridlines are associated");
      i += strlen(out+i);
      sprintf(buf, "y%d", settings_graph_default.GridAxisY);
      directive_show3(out+i, interactive, "grid", buf, (settings_graph_default.GridAxisY == settings_graph_current.GridAxisY), "Sets the Y-axis with whose ticks gridlines are associated");
      i += strlen(out+i);
      sprintf(buf, "z%d", settings_graph_default.GridAxisZ);
      directive_show3(out+i, interactive, "grid", buf, (settings_graph_default.GridAxisZ == settings_graph_current.GridAxisZ), "Sets the Z-axis with whose ticks gridlines are associated");
      i += strlen(out+i);
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "gridmajcolour",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.GridMajColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, interactive, "GridMajColour", buf, (settings_graph_default.GridMajColour == settings_graph_current.GridMajColour), "The colour of the major gridlines on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "gridmincolour",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_default.GridMinColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, interactive, "GridMinColour", buf, (settings_graph_default.GridMinColour == settings_graph_current.GridMinColour), "The colour of the minor gridlines on graphs");
    i += strlen(out+i) ; p=1;
   }

  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "key",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.key, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "key", buf, (settings_graph_default.key == settings_graph_current.key), "Selects whether a legend is included on plots");
    i += strlen(out+i) ; p=1;
   }
  if ( ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "key",1)>=0)) && (settings_graph_current.key == SW_ONOFF_ON)  )
   {
    sprintf(buf, "%s %s , %s", (char *)FetchSettingName(settings_graph_current.KeyPos, SW_KEYPOS_INT, (void **)SW_KEYPOS_STR),NumericDisplay(settings_graph_current.KeyXOff,0),NumericDisplay(settings_graph_current.KeyYOff,1));
    directive_show3(out+i, interactive, "key", buf, ((settings_graph_default.KeyPos == settings_graph_current.KeyPos)&&(settings_graph_default.KeyXOff == settings_graph_current.KeyXOff)&&(settings_graph_default.KeyYOff == settings_graph_current.KeyYOff)), "Selects where legends are orientated on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "keycolumns",1)>=0))
   { 
    sprintf(buf, "%d", settings_graph_default.KeyColumns);
    directive_show3(out+i, interactive, "KeyColumns", buf, (settings_graph_default.KeyColumns == settings_graph_current.KeyColumns), "Sets the number of columns into which legends on graphs are sorted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "linewidth", 1)>=0) || (StrAutocomplete(word, "lw", 2)>=0))
   { 
    sprintf(buf, "%s", NumericDisplay(settings_graph_current.LineWidth,0));
    directive_show3(out+i, interactive, "LineWidth", buf, (settings_graph_default.LineWidth == settings_graph_current.LineWidth), "Sets the widths of lines drawn on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "multiplot", 1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.multiplot, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "multiplot", buf, (settings_term_default.multiplot == settings_term_current.multiplot), "Selects whether multiplot mode is currently active");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "origin", 1)>=0))
   { 
    sprintf(buf, "%s , %s", NumericDisplay(settings_graph_current.OriginX,0), NumericDisplay(settings_graph_current.OriginY,1));
    directive_show3(out+i, interactive, "origin", buf, ((settings_graph_default.OriginX == settings_graph_current.OriginX)&&(settings_graph_default.OriginY == settings_graph_current.OriginY)), "Selects where, in cm, the bottom-left corners of graphs are located on multiplot pages");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "output", 1)>=0))
   { 
    StrEscapify(settings_term_current.output, buf);
    directive_show3(out+i, interactive, "output", buf, (strcmp(settings_term_default.output,settings_term_current.output)==0), "Filename to which graphic output is sent");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "palette",1)>=0))
   {
    for (j=k=0; settings_palette[j]>0; j++)
     {
      if (j>0) { sprintf(buf+k, ", "); k+=strlen(buf+k); }
      sprintf(buf+k, "%s", (char *)FetchSettingName(settings_palette[j], SW_COLOUR_INT, (void **)SW_COLOUR_STR)); k+=strlen(buf+k);
     }
    directive_show3(out+i, interactive, "palette", buf, (0==0), "The sequence of colours used to plot datasets on colour graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "papersize", 1)>=0))
   {
    sprintf(buf, "%s , %s", NumericDisplay(settings_term_current.PaperWidth,0), NumericDisplay(settings_term_current.PaperHeight,1));
    directive_show3(out+i, interactive, "PaperSize", buf, ((settings_term_default.PaperWidth==settings_term_current.PaperWidth)&&(settings_term_default.PaperHeight==settings_term_current.PaperHeight)), "The current papersize for postscript output, in mm");
    i += strlen(out+i) ; p=1;
    if (StrAutocomplete("user", settings_term_current.PaperName, 1)<0)
     {
      StrEscapify(settings_term_current.PaperName,buf);
      directive_show3(out+i, interactive, "PaperSize", buf, (strcmp(settings_term_default.PaperName, settings_term_current.PaperName)==0), NULL);
      i += strlen(out+i) ; p=1;
     }
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointlinewidth",1)>=0) || (StrAutocomplete(word, "plw",3)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(settings_graph_current.PointLineWidth,0));
    directive_show3(out+i, interactive, "PointLineWidth", buf, (settings_graph_default.PointLineWidth==settings_graph_current.PointLineWidth), "The width of the strokes used to mark points on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "pointsize",1)>=0) || (StrAutocomplete(word, "ps",2)>=0))
   {
    sprintf(buf, "%s", NumericDisplay(settings_graph_current.PointSize,0));
    directive_show3(out+i, interactive, "PointSize", buf, (settings_graph_default.PointSize==settings_graph_current.PointSize), "The size of points marked on graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "preamble", 1)>=0))
   {
    StrEscapify(settings_term_current.LatexPreamble,buf);
    directive_show3(out+i, interactive, "preamble", buf, (strcmp(settings_term_default.LatexPreamble,settings_term_current.LatexPreamble)==0), "Configuration options sent to the LaTeX typesetting system");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "samples",1)>=0))
   {
    sprintf(buf, "%d", settings_graph_current.samples);
    directive_show3(out+i, interactive, "samples", buf, (settings_graph_default.samples==settings_graph_current.samples), "The number of samples taken when functions are plotted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "size",1)>=0))
   {
    if (settings_graph_current.AutoAspect == SW_ONOFF_ON) sprintf(buf, "auto");
    else                                                  sprintf(buf, "%s", NumericDisplay(settings_graph_current.aspect, 0));
    directive_show3(out+i, interactive, "size ratio", buf, ((settings_graph_default.aspect==settings_graph_current.aspect)&&(settings_graph_default.AutoAspect==settings_graph_current.AutoAspect)), "The aspect-ratio of graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "data", 1)>=0) || (StrAutocomplete(word, "style", 1)>=0))
   {
    with_words_print(&settings_graph_current.DataStyle, buf);
    directive_show3(out+i, interactive, "data style", buf, with_words_compare(&settings_graph_default.DataStyle,&settings_graph_current.DataStyle), "Default plot options for plotting datafiles");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "function", 1)>=0) || (StrAutocomplete(word, "style", 1)>=0))
   {
    with_words_print(&settings_graph_current.FuncStyle, buf);
    directive_show3(out+i, interactive, "function style", buf, with_words_compare(&settings_graph_default.FuncStyle,&settings_graph_current.FuncStyle), "Default plot options for plotting functions");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermType, SW_TERMTYPE_INT, (void **)SW_TERMTYPE_STR));
    directive_show3(out+i, interactive, "terminal", buf, (settings_term_default.TermType==settings_term_current.TermType), "The type of graphic output to be produced");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "antialias",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermAntiAlias, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "terminal AntiAlias", buf, (settings_term_default.TermAntiAlias==settings_term_current.TermAntiAlias), "Selects whether anti-aliasing is applied to bitmap output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "colour", 1)>=0) || (StrAutocomplete(word, "color",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.colour, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "terminal colour", buf, (settings_term_default.colour==settings_term_current.colour), "Selects whether output is colour or monochrome");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "enlargement",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermEnlarge, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "terminal enlarge", buf, (settings_term_default.TermEnlarge==settings_term_current.TermEnlarge), "Selects whether output photo-enlarged to fill the page");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "invert",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermInvert, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "terminal invert", buf, (settings_term_default.TermInvert==settings_term_current.TermInvert), "Selects whether the colours of bitmap output are inverted");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "landscape", 1)>=0) || (StrAutocomplete(word, "portrait", 1)>=0))
   {
    if (settings_term_current.landscape == SW_ONOFF_ON) sprintf(buf, "Landscape");
    else                                                sprintf(buf, "Portrait");
    directive_show3(out+i, interactive, "terminal", buf, (settings_term_default.landscape==settings_term_current.landscape), "Selects the orientation of output");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "terminal", 1)>=0) || (StrAutocomplete(word, "transparent", 1)>=0) || (StrAutocomplete(word, "solid", 1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_term_current.TermTransparent, SW_ONOFF_INT, (void **)SW_ONOFF_STR));
    directive_show3(out+i, interactive, "terminal transparent", buf, (settings_term_default.TermTransparent==settings_term_current.TermTransparent), "Selects whether gif and png output is transparent");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "textcolour",1)>=0))
   {
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.TextColour, SW_COLOUR_INT, (void **)SW_COLOUR_STR));
    directive_show3(out+i, interactive, "TextColour", buf, (settings_graph_default.TextColour==settings_graph_current.TextColour), "Selects the colour of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "texthalign",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.TextHAlign, SW_HALIGN_INT, (void **)SW_HALIGN_STR));
    directive_show3(out+i, interactive, "TextHAlign", buf, (settings_graph_default.TextHAlign==settings_graph_current.TextHAlign), "Selects the horizontal alignment of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "textvalign",1)>=0))
   { 
    sprintf(buf, "%s", (char *)FetchSettingName(settings_graph_current.TextVAlign, SW_VALIGN_INT, (void **)SW_VALIGN_STR));
    directive_show3(out+i, interactive, "TextVAlign", buf, (settings_graph_default.TextVAlign==settings_graph_current.TextVAlign), "Selects the vertical alignment of text labels");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "title", 1)>=0))
   {
    StrEscapify(settings_graph_current.title, buf); k = strlen(buf);
    sprintf(buf+k, " %s , %s", NumericDisplay(settings_graph_current.TitleXOff, 0), NumericDisplay(settings_graph_current.TitleYOff, 1));
    directive_show3(out+i, interactive, "title", buf, ((strcmp(settings_graph_default.title,settings_graph_current.title)==0)&&(settings_graph_default.TitleXOff==settings_graph_current.TitleXOff)&&(settings_graph_default.TitleYOff==settings_graph_current.TitleYOff)), "A title to be display above graphs");
    i += strlen(out+i) ; p=1;
   }
  if ((StrAutocomplete(word, "settings", 1)>=0) || (StrAutocomplete(word, "width", 1)>=0) || (StrAutocomplete(word, "size", 1)>=0))
   { 
    sprintf(buf, "%s", NumericDisplay(settings_graph_current.width, 0));
    directive_show3(out+i, interactive, "width", buf, (settings_graph_default.width==settings_graph_current.width), "The width, in cm, of graphs");
    i += strlen(out+i) ; p=1;
   }

  ppl_report(out);
  free(out); free(buf);
  return;
 }

void directive_show(Dict *command, int interactive)
 {
  List         *ShowList;
  ListIterator *ShowIterate;
  Dict         *ShowWordDict;
  char         *ShowWord;
  char          TextBuffer[SSTR_LENGTH];
  int           i=0;

  interactive = ( interactive && (settings_session_default.colour == SW_ONOFF_ON) );

  DictLookup(command, "setting_list", NULL, NULL, (void **)&ShowList);
  if (ListLen(ShowList) == 0)
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
      DictLookup(ShowWordDict,"setting",NULL,NULL,(void **)&ShowWord);
      if (StrAutocomplete(ShowWord,"all",1)>=0)
       {
        directive_show2("settings"  ,interactive);
        directive_show2("axes_"     ,interactive);
        directive_show2("linestyles",interactive);
        directive_show2("variables" ,interactive);
        directive_show2("functions" ,interactive);
       }
      else
       {
        directive_show2(ShowWord    ,interactive);
       }
     }
   }
  return;
 }

