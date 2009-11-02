// ppl_setting_types.h
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

#ifndef _PPL_SETTING_TYPES_H
#define _PPL_SETTING_TYPES_H 1

// Boolean Switches

#define SW_BOOL_TRUE  10001
#define SW_BOOL_FALSE 10002

extern char *SW_BOOL_STR[];
extern int   SW_BOOL_ACL[];
extern int   SW_BOOL_INT[];

// On/Off Switches

#define SW_ONOFF_ON  11001
#define SW_ONOFF_OFF 11002

extern char *SW_ONOFF_STR[];
extern int   SW_ONOFF_ACL[];
extern int   SW_ONOFF_INT[];

// Plot Style Switches

#define SW_STYLE_POINTS         12001
#define SW_STYLE_LINES          12002
#define SW_STYLE_LINESPOINTS    12003
#define SW_STYLE_XERRORBARS     12004
#define SW_STYLE_YERRORBARS     12005
#define SW_STYLE_XYERRORBARS    12006
#define SW_STYLE_XERRORRANGE    12007
#define SW_STYLE_YERRORRANGE    12008
#define SW_STYLE_XYERRORRANGE   12009
#define SW_STYLE_DOTS           12010
#define SW_STYLE_IMPULSES       12011
#define SW_STYLE_BOXES          12012
#define SW_STYLE_WBOXES         12013
#define SW_STYLE_STEPS          12014
#define SW_STYLE_FSTEPS         12015
#define SW_STYLE_HISTEPS        12016
#define SW_STYLE_ARROWS_HEAD    12017
#define SW_STYLE_ARROWS_NOHEAD  12018
#define SW_STYLE_ARROWS_TWOHEAD 12019
#define SW_STYLE_CSPLINES       12020
#define SW_STYLE_ACSPLINES      12021

extern char *SW_STYLE_STR[];
extern int   SW_STYLE_ACL[];
extern int   SW_STYLE_INT[];

// Terminal Type Switches

#define SW_TERMTYPE_X11S 13001
#define SW_TERMTYPE_X11M 13002
#define SW_TERMTYPE_X11P 13003
#define SW_TERMTYPE_PS   13004
#define SW_TERMTYPE_EPS  13005
#define SW_TERMTYPE_PDF  13006
#define SW_TERMTYPE_PNG  13007
#define SW_TERMTYPE_JPG  13008
#define SW_TERMTYPE_GIF  13009
#define SW_TERMTYPE_BMP  13010
#define SW_TERMTYPE_TIF  13011

extern char *SW_TERMTYPE_STR[];
extern int   SW_TERMTYPE_ACL[];
extern int   SW_TERMTYPE_INT[];

// Key Position Switches

#define SW_KEYPOS_TR 14001
#define SW_KEYPOS_TM 14002
#define SW_KEYPOS_TL 14003
#define SW_KEYPOS_MR 14004
#define SW_KEYPOS_MM 14005
#define SW_KEYPOS_ML 14006
#define SW_KEYPOS_BR 14007
#define SW_KEYPOS_BM 14008
#define SW_KEYPOS_BL 14009
#define SW_KEYPOS_BELOW 14010
#define SW_KEYPOS_OUTSIDE 14011

extern char *SW_KEYPOS_STR[];
extern int   SW_KEYPOS_ACL[];
extern int   SW_KEYPOS_INT[];

// Tick Direction Switches

#define SW_TICDIR_IN   15001
#define SW_TICDIR_OUT  15002
#define SW_TICDIR_BOTH 15003

extern char *SW_TICDIR_STR[];
extern int   SW_TICDIR_ACL[];
extern int   SW_TICDIR_INT[];

// Tick Label Text Direction Switches

#define SW_TICLABDIR_HORI 15011
#define SW_TICLABDIR_VERT 15012
#define SW_TICLABDIR_ROT  15013

extern char *SW_TICLABDIR_STR[];
extern int   SW_TICLABDIR_ACL[];
extern int   SW_TICLABDIR_INT[];

// Axis Display Schemes

#define SW_AXISDISP_NOARR 15021
#define SW_AXISDISP_ARROW 15022
#define SW_AXISDISP_TWOAR 15024
#define SW_AXISDISP_BACKA 15026

extern char *SW_AXISDISP_STR[];
extern int   SW_AXISDISP_ACL[];
extern int   SW_AXISDISP_INT[];

// Axis Display Mirroring Schemes

#define SW_AXISMIRROR_AUTO       15031
#define SW_AXISMIRROR_MIRROR     15032
#define SW_AXISMIRROR_NOMIRROR   15033
#define SW_AXISMIRROR_FULLMIRROR 15034

extern char *SW_AXISMIRROR_STR[];
extern int   SW_AXISMIRROR_ACL[];
extern int   SW_AXISMIRROR_INT[];

// Graph Projection Schemes

#define SW_PROJ_FLAT 15041
#define SW_PROJ_GNOM 15042

extern char *SW_PROJ_STR[];
extern int   SW_PROJ_ACL[];
extern int   SW_PROJ_INT[];

// Text Horizontal Alignment

#define SW_HALIGN_LEFT  16001
#define SW_HALIGN_CENT  16002
#define SW_HALIGN_RIGHT 16003

extern char *SW_HALIGN_STR[];
extern int   SW_HALIGN_ACL[];
extern int   SW_HALIGN_INT[];

// Text Vertical Alignment

#define SW_VALIGN_TOP  16011
#define SW_VALIGN_CENT 16012
#define SW_VALIGN_BOT  16013

extern char *SW_VALIGN_STR[];
extern int   SW_VALIGN_ACL[];
extern int   SW_VALIGN_INT[];

// Colours for displaying on terminals

#define SW_TERMCOL_NOR 17001
#define SW_TERMCOL_RED 17002
#define SW_TERMCOL_GRN 17003
#define SW_TERMCOL_BRN 17004
#define SW_TERMCOL_BLU 17005
#define SW_TERMCOL_MAG 17006
#define SW_TERMCOL_CYN 17007
#define SW_TERMCOL_WHT 17008

extern char *SW_TERMCOL_STR[];
extern int   SW_TERMCOL_ACL[];
extern int   SW_TERMCOL_INT[];
extern char *SW_TERMCOL_TXT[];

// Schemes in which units can be displayed

#define SW_UNITSCH_SI  18001
#define SW_UNITSCH_CGS 18002
#define SW_UNITSCH_ANC 18003
#define SW_UNITSCH_IMP 18004
#define SW_UNITSCH_US  18005
#define SW_UNITSCH_PLK 18006

extern char *SW_UNITSCH_STR[];
extern int   SW_UNITSCH_ACL[];
extern int   SW_UNITSCH_INT[];

// Schemes in which we can display numeric results

#define SW_DISPLAY_N 18050
#define SW_DISPLAY_T 18051
#define SW_DISPLAY_L 18052

extern char *SW_DISPLAY_STR[];
extern int   SW_DISPLAY_ACL[];
extern int   SW_DISPLAY_INT[];

// Calendars which we can use

#define SW_CALENDAR_GREGORIAN 19001
#define SW_CALENDAR_JULIAN    19002
#define SW_CALENDAR_BRITISH   19003
#define SW_CALENDAR_FRENCH    19004
#define SW_CALENDAR_CATHOLIC  19005
#define SW_CALENDAR_RUSSIAN   19006
#define SW_CALENDAR_GREEK     19007
#define SW_CALENDAR_HEBREW    19020
#define SW_CALENDAR_ISLAMIC   19021

extern char *SW_CALENDAR_STR[];
extern int   SW_CALENDAR_ACL[];
extern int   SW_CALENDAR_INT[];

void *FetchSettingName      (int id, int *id_list, void **name_list);
int   FetchSettingByName    (char *name, int *id_list, char **name_list);

#endif
