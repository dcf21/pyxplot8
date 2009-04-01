// ppl_setting_types.c
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

#include "ppl_setting_types.h"

char *SW_BOOL_STR[] = {"TRUE"       , "FALSE"      };
int   SW_BOOL_ACL[] = {1            , 1            };
int   SW_BOOL_INT[] = {SW_BOOL_TRUE , SW_BOOL_FALSE , -1};

char *SW_ONOFF_STR[] = {"ON"        , "OFF"       };
int   SW_ONOFF_ACL[] = {2           , 2           };
int   SW_ONOFF_INT[] = {SW_ONOFF_ON , SW_ONOFF_OFF , -1};

char *SW_STYLE_STR[] = {"POINTS"        , "LINES"        , "LINESPOINTS"        , "XERRORBARS"        , "YERRORBARS"        , "XYERRORBARS"        , "XERRORRANGE"        , "YERRORRANGE"        , "XYERRORRANGE"        , "DOTS"        , "IMPULSES"        , "BOXES"        , "WBOXES"        , "STEPS"        , "FSTEPS"        , "HISTEPS"        , "ARROWS_HEAD"        , "ARROWS_NOHEAD"        , "ARROWS_TWOHEAD"        , "CSPLINES"        , "ACSPLINES"        };
int   SW_STYLE_ACL[] = {1               , 1              , 6                    , 1                   , 1                   , 2                    , 7                    , 7                    , 8                     , 1             , 1                 , 1              , 1               , 1              , 1               , 1                , 1                    , 8                      , 8                       , 1                 , 1                  };
int   SW_STYLE_INT[] = {SW_STYLE_POINTS , SW_STYLE_LINES , SW_STYLE_LINESPOINTS , SW_STYLE_XERRORBARS , SW_STYLE_YERRORBARS , SW_STYLE_XYERRORBARS , SW_STYLE_XERRORRANGE , SW_STYLE_YERRORRANGE , SW_STYLE_XYERRORRANGE , SW_STYLE_DOTS , SW_STYLE_IMPULSES , SW_STYLE_BOXES , SW_STYLE_WBOXES , SW_STYLE_STEPS , SW_STYLE_FSTEPS , SW_STYLE_HISTEPS , SW_STYLE_ARROWS_HEAD , SW_STYLE_ARROWS_NOHEAD , SW_STYLE_ARROWS_TWOHEAD , SW_STYLE_CSPLINES , SW_STYLE_ACSPLINES , -1};

char *SW_TERMTYPE_STR[] = {"X11_SINGLEWINDOW", "X11_MULTIWINDOW" , "X11_PERSIST"    , "PS"           , "EPS"           , "PDF"           , "PNG"           , "JPG"           , "GIF"           };
int   SW_TERMTYPE_ACL[] = {1                 , 5                 , 5                , 1              , 1               , 2               , 2               , 1               , 1               };
int   SW_TERMTYPE_INT[] = {SW_TERMTYPE_X11S  , SW_TERMTYPE_X11M  , SW_TERMTYPE_X11P , SW_TERMTYPE_PS , SW_TERMTYPE_EPS , SW_TERMTYPE_PDF , SW_TERMTYPE_PNG , SW_TERMTYPE_JPG , SW_TERMTYPE_GIF , -1}; 

char *SW_KEYPOS_STR[] = {"TOP RIGHT"  , "TOP MIDDLE" , "TOP LEFT"   , "MIDDLE RIGHT" , "MIDDLE MIDDLE" , "MIDDLE LEFT" , "BOTTOM RIGHT" , "BOTTOM MIDDLE" , "BOTTOM LEFT" , "BELOW"         , "OUTSIDE"         };
int   SW_KEYPOS_ACL[] = {1            , 5            , 5            , 1              , 8               , 8             , 1              , 8               , 8             , 2               , 1                 };
int   SW_KEYPOS_INT[] = {SW_KEYPOS_TR , SW_KEYPOS_TM , SW_KEYPOS_TL , SW_KEYPOS_MR   , SW_KEYPOS_MM    , SW_KEYPOS_ML  , SW_KEYPOS_BR   , SW_KEYPOS_BM    , SW_KEYPOS_BL  , SW_KEYPOS_BELOW , SW_KEYPOS_OUTSIDE , -1};

char *SW_TICDIR_STR[] = {"INWARDS"    , "OUTWARDS"    , "BOTH"         };
int   SW_TICDIR_ACL[] = {1            , 1             , 1              };
int   SW_TICDIR_INT[] = {SW_TICDIR_IN , SW_TICDIR_OUT , SW_TICDIR_BOTH , -1};

char *SW_HALIGN_STR[] = {"LEFT"         , "CENTRE"       , "RIGHT"         };
int   SW_HALIGN_ACL[] = {1              , 1              , 1               };
int   SW_HALIGN_INT[] = {SW_HALIGN_LEFT , SW_HALIGN_CENT , SW_HALIGN_RIGHT , -1};

char *SW_VALIGN_STR[] = {"TOP"         , "CENTRE"       , "BOTTOM"      };
int   SW_VALIGN_ACL[] = {1             , 1              , 1             };
int   SW_VALIGN_INT[] = {SW_VALIGN_TOP , SW_VALIGN_CENT , SW_VALIGN_BOT , -1};

char *SW_TERMCOL_STR[] = {"NORMAL"       , "RED"          , "GREEN"        , "BROWN"        , "BLUE"         , "MAGENTA"      , "CYAN"         , "WHITE"        };
int   SW_TERMCOL_ACL[] = {1              , 1              , 1              , 2              , 1              , 1              , 1              , 1              };
int   SW_TERMCOL_INT[] = {SW_TERMCOL_NOR , SW_TERMCOL_RED , SW_TERMCOL_GRN , SW_TERMCOL_BRN , SW_TERMCOL_BLU , SW_TERMCOL_MAG , SW_TERMCOL_CYN , SW_TERMCOL_WHT , -1}; 
char *SW_TERMCOL_TXT[] = {"\x1b[0m"      , "\x1b[01;31m"  , "\x1b[01;32m"  , "\x1b[01;33m"  , "\x1b[01;34m"  , "\x1b[01;35m"  , "\x1b[01;36m"  , "\x1b[01;37m"  };

