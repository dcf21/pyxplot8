// ppl_setting_types.h
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-8 Dominic Ford <coders@pyxplot.org.uk>
//               2008   Ross Church
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

#ifndef _PPL_SETTING_TYPES
#define _PPL_SETTING_TYPES 1

// Boolean Switches

#define SW_BOOL_TRUE  10001
#define SW_BOOL_FALSE 10002

extern const char *SW_BOOL_STR[];
extern const int   SW_BOOL_ACL[];
extern const int   SW_BOOL_INT[];

// On/Off Switches

#define SW_ONOFF_ON  11001
#define SW_ONOFF_OFF 11002

extern const char *SW_ONOFF_STR[];
extern const int   SW_ONOFF_ACL[];
extern const int   SW_ONOFF_INT[];

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

extern const char *SW_STYLE_STR[];
extern const int   SW_STYLE_ACL[];
extern const int   SW_STYLE_INT[];

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

extern const char *SW_TERMTYPE_STR[];
extern const int   SW_TERMTYPE_ACL[];
extern const int   SW_TERMTYPE_INT[];

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

extern const char *SW_KEYPOS_STR[];
extern const int   SW_KEYPOS_ACL[];
extern const int   SW_KEYPOS_INT[];

// Tick Direction Switches

#define SW_TICDIR_IN   15001
#define SW_TICDIR_OUT  15002
#define SW_TICDIR_BOTH 15003

extern const char *SW_TICDIR_STR[];
extern const int   SW_TICDIR_ACL[];
extern const int   SW_TICDIR_INT[];

// Text Horizontal Alignment

#define SW_HALIGN_LEFT  16001
#define SW_HALIGN_CENT  16002
#define SW_HALIGN_RIGHT 16003

extern const char *SW_HALIGN_STR[];
extern const int   SW_HALIGN_ACL[];
extern const int   SW_HALIGN_INT[];

// Text Vertical Alignment

#define SW_VALIGN_TOP  16001
#define SW_VALIGN_CENT 16002
#define SW_VALIGN_BOT  16003

extern const char *SW_VALIGN_STR[];
extern const int   SW_VALIGN_ACL[];
extern const int   SW_VALIGN_INT[];

#endif
