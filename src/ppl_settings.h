// ppl_settings.h
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

#ifndef _PPL_SETTINGS_H
#define _PPL_SETTINGS_H 1

#include "StringTools/str_constants.h"

#include "ppl_constants.h"
#include "ppl_units.h"

typedef struct with_words {
 int    colour, fillcolour, linespoints, linetype, pointtype, style; // Core style settings which can be placed after the 'with' modifier
 double linewidth, pointlinewidth, pointsize;
 int    colourR, colourG, colourB, fillcolourR, fillcolourG, fillcolourB; // Alternatives to the colour and fillcolour settings, RGB settings
 char  *STRlinetype, *STRlinewidth, *STRpointlinewidth, *STRpointsize, *STRpointtype; // Alternatives to the above, where expressions are evaluated per use, e.g. $4
 char  *STRcolourR, *STRcolourG, *STRcolourB, *STRfillcolourR, *STRfillcolourG, *STRfillcolourB;
 unsigned char USEcolour, USEfillcolour, USElinespoints, USElinetype, USElinewidth, USEpointlinewidth, USEpointsize, USEpointtype, USEstyle, USEcolourRGB, USEfillcolourRGB; // Set to 1 to indicate settings to be used
 unsigned char malloced; // Indicates whether we need to free strings
 } with_words;

typedef struct settings_terminal {
 int    backup, CalendarIn, CalendarOut, colour, ComplexNumbers, display, ExplicitErrors, landscape, multiplot, NumDisplay, SignificantFigures, TermAntiAlias, TermType, TermEnlarge, TermInvert, TermTransparent, UnitScheme, UnitDisplayPrefix, UnitDisplayAbbrev, viewer;
 double dpi;
 unsigned char BinOriginAuto, BinWidthAuto;
 value  BinOrigin, BinWidth, PaperHeight, PaperWidth;
 char   output[FNAME_LENGTH];
 char   PaperName[FNAME_LENGTH];
 char   LatexPreamble[FNAME_LENGTH];
 } settings_terminal;

typedef struct settings_graph {
 int           AutoAspect, AxesColour, AxesColourR, AxesColourG, AxesColourB, AxisUnitStyle, clip, grid, GridMajColour, GridMajColourR, GridMajColourG, GridMajColourB, GridMinColour, GridMinColourR, GridMinColourG, GridMinColourB, key, KeyColumns, KeyPos, samples, TextColour, TextColourR, TextColourG, TextColourB, TextHAlign, TextVAlign, Tlog;
 double        aspect, bar, FontSize, LineWidth, PointSize, PointLineWidth, projection;
 unsigned char GridAxisX[MAX_AXES], GridAxisY[MAX_AXES], GridAxisZ[MAX_AXES];
 unsigned char BoxFromAuto, BoxWidthAuto;
 value         BoxFrom, BoxWidth, KeyXOff, KeyYOff, OriginX, OriginY, TitleXOff, TitleYOff, Tmin, Tmax, width, XYview, YZview;
 char          title[FNAME_LENGTH];
 with_words    DataStyle, FuncStyle;
 } settings_graph;

typedef struct settings_session {
 int   splash, colour, colour_rep, colour_wrn, colour_err;
 char  cwd[FNAME_LENGTH];
 char  tempdir[FNAME_LENGTH];
 char  homedir[FNAME_LENGTH];
 } settings_session;

// Variables defined in ppl_settings.c
#ifndef _PPL_SETTINGS_C
extern settings_terminal settings_term_default;
extern settings_terminal settings_term_current;
extern settings_graph    settings_graph_default;
extern settings_graph    settings_graph_current;
extern settings_session  settings_session_default;
extern int               settings_palette_current[], settings_paletteR_current[], settings_paletteG_current[], settings_paletteB_current[];
extern int               settings_palette_default[], settings_paletteR_default[], settings_paletteG_default[], settings_paletteB_default[];
extern with_words        settings_plot_styles[], settings_plot_styles_default[];
#endif

void  ppl_settings_makedefault();
void  ppl_settings_readconfig();
void  with_words_zero    (with_words *a, const unsigned char malloced);
int   with_words_compare (const with_words *a, const with_words *b);
int   with_words_compare_zero(const with_words *a);
void  with_words_merge   (with_words *out, const with_words *a, const with_words *b, const with_words *c, const with_words *d, const with_words *e, const unsigned char ExpandStyles);
void  with_words_print   (const with_words *defn, char *out);
void  with_words_destroy (with_words *a);
void  with_words_copy    (with_words *out, const with_words *in);

// Can now safely include these headers, which need some of the settings above
#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"

#define AXISLINEARINTERPOLATION_NPOINTS 2045

int   colour_fromdict    (Dict *in, char *prefix, int *outcol, int *outcolR, int *outcolG, int *outcolB, char **outcolRS, char **outcolGS, char **outcolBS, unsigned char *USEcol, unsigned char *USEcolRGB, int *errpos, unsigned char malloced);
void  with_words_fromdict(Dict *in, with_words *out, const unsigned char MallocNew);

typedef struct settings_axis {
 unsigned char atzero, enabled, invisible, linked, RangeReversed, topbottom, MTickMaxSet, MTickMinSet, MTickStepSet, TickMaxSet, TickMinSet, TickStepSet;
 int     ArrowType, LinkedAxisCanvasID, LinkedAxisToXYZ, LinkedAxisToNum, log, MaxSet, MinSet, MirrorType, MTickDir, TickDir, TickLabelRotation;
 double  LabelRotate, LogBase, max, min, MTickMax, MTickMin, MTickStep, TickLabelRotate, TickMax, TickMin, TickStep;
 char   *format, *label, *linkusing;
 double *MTickList, *TickList;
 char  **MTickStrs,**TickStrs;
 value   unit;

 // Temporary data fields which are used when rendering an axis to postscript
 int           AxisValueTurnings;
 double       *AxisLinearInterpolation;
 int          *AxisTurnings;
 unsigned char CrossedAtZero;
 unsigned char MinUsedSet, MaxUsedSet, DataUnitSet, RangeFinalised, FinalActive;
 double        MinUsed, MaxUsed, MinFinal, MaxFinal, *OrdinateRaster;
 double        HardMin, HardMax; // Contains ranges set via plot [foo:bar]
 unsigned char HardMinSet, HardMaxSet, HardAutoMinSet, HardAutoMaxSet, HardUnitSet, Mode0BackPropagated;
 int           OrdinateRasterLen;
 value         HardUnit, DataUnit;
 char         *FinalAxisLabel;
 unsigned char TickListFinalised;
 double       *TickListPositions,  *MTickListPositions;
 char        **TickListStrings  , **MTickListStrings;
 } settings_axis;

typedef struct arrow_object {
 int        id;
 value      x0       ,y0       ,z0       ,x1       ,y1       ,z1;
 int        system_x0,system_y0,system_z0,system_x1,system_y1,system_z1;
 int        axis_x0  ,axis_y0  ,axis_z0  ,axis_x1  ,axis_y1  ,axis_z1;
 int        arrow_style;
 with_words style;
 struct arrow_object *next;
 } arrow_object;

typedef struct label_object {
 int         id;
 value       x       ,y       ,z;
 int         system_x,system_y,system_z;
 int         axis_x  ,axis_y  ,axis_z;
 char       *text;
 with_words  style;
 double      rotation, gap;
 int         HAlign, VAlign;
 struct label_object *next;
 } label_object;

// Variables defined in ppl_settings.c
#ifndef _PPL_SETTINGS_C
extern settings_axis     settings_axis_default;
extern settings_axis     XAxes[], XAxesDefault[];
extern settings_axis     YAxes[], YAxesDefault[];
extern settings_axis     ZAxes[], ZAxesDefault[];
extern Dict             *settings_filters;
extern arrow_object     *arrow_list, *arrow_list_default;
extern label_object     *label_list, *label_list_default;
#endif

void arrow_add         (arrow_object **list, Dict *in);
void arrow_remove      (arrow_object **list, Dict *in);
void arrow_unset       (arrow_object **list, Dict *in);
void arrow_default     (arrow_object **list, Dict *in);
unsigned char arrow_compare(arrow_object *a, arrow_object *b);
void arrow_list_copy   (arrow_object **out, arrow_object **in);
void arrow_list_destroy(arrow_object **list);
void arrow_print       (arrow_object  *in, char *out);
void label_add         (label_object **list, Dict *in);
void label_remove      (label_object **list, Dict *in);
void label_unset       (label_object **list, Dict *in);
void label_default     (label_object **list, Dict *in);
unsigned char label_compare(label_object *a, label_object *b);
void label_list_copy   (label_object **out, label_object **in);
void label_list_destroy(label_object **list);
void label_print       (label_object  *in, char *out);

void DestroyAxis       (settings_axis *in);
void CopyAxis          (settings_axis *out, const settings_axis *in);
void CopyAxisTics      (settings_axis *out, const settings_axis *in);
void CopyAxisMTics     (settings_axis *out, const settings_axis *in);
unsigned char CompareAxisTics (const settings_axis *a, const settings_axis *b);
unsigned char CompareAxisMTics(const settings_axis *a, const settings_axis *b);

// Functions defined in ppl_readconfig.c
#ifndef _PPL_READCONFIG_C
extern int ppl_termtype_set_in_configfile;
void ReadConfigFile(char *ConfigFname);
#endif

#endif
