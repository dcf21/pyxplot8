# colourlist_generate.py
#
# The code in this file is part of PyXPlot
# <http://www.pyxplot.org.uk>
#
# Copyright (C) 2006-8 Dominic Ford <coders@pyxplot.org.uk>
#               2008   Ross Church
#
# $Id$
#
# PyXPlot is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# You should have received a copy of the GNU General Public License along with
# PyXPlot; if not, write to the Free Software Foundation, Inc., 51 Franklin
# Street, Fifth Floor, Boston, MA  02110-1301, USA

# ----------------------------------------------------------------------------

colour_list={
"GREENYELLOW": [0.15, 0, 0.69, 0],
"YELLOW": [0, 0, 1, 0],
"GOLDENROD": [0, 0.10, 0.84, 0],
"DANDELION": [0, 0.29, 0.84, 0],
"APRICOT": [0, 0.32, 0.52, 0],
"PEACH": [0, 0.50, 0.70, 0],
"MELON": [0, 0.46, 0.50, 0],
"YELLOWORANGE": [0, 0.42, 1, 0],
"ORANGE": [0, 0.61, 0.87, 0],
"BURNTORANGE": [0, 0.51, 1, 0],
"BITTERSWEET": [0, 0.75, 1, 0.24],
"REDORANGE": [0, 0.77, 0.87, 0],
"MAHOGANY": [0, 0.85, 0.87, 0.35],
"MAROON": [0, 0.87, 0.68, 0.32],
"BRICKRED": [0, 0.89, 0.94, 0.28],
"RED": [0, 1, 1, 0],
"ORANGERED": [0, 1, 0.50, 0],
"RUBINERED": [0, 1, 0.13, 0],
"WILDSTRAWBERRY": [0, 0.96, 0.39, 0],
"SALMON": [0, 0.53, 0.38, 0],
"CARNATIONPINK": [0, 0.63, 0, 0],
"MAGENTA": [0, 1, 0, 0],
"VIOLETRED": [0, 0.81, 0, 0],
"RHODAMINE": [0, 0.82, 0, 0],
"MULBERRY": [0.34, 0.90, 0, 0.02],
"REDVIOLET": [0.07, 0.90, 0, 0.34],
"FUCHSIA": [0.47, 0.91, 0, 0.08],
"LAVENDER": [0, 0.48, 0, 0],
"THISTLE": [0.12, 0.59, 0, 0],
"ORCHID": [0.32, 0.64, 0, 0],
"DARKORCHID": [0.40, 0.80, 0.20, 0],
"PURPLE": [0.45, 0.86, 0, 0],
"PLUM": [0.50, 1, 0, 0],
"VIOLET": [0.79, 0.88, 0, 0],
"ROYALPURPLE": [0.75, 0.90, 0, 0],
"BLUEVIOLET": [0.86, 0.91, 0, 0.04],
"PERIWINKLE": [0.57, 0.55, 0, 0],
"CADETBLUE": [0.62, 0.57, 0.23, 0],
"CORNFLOWERBLUE": [0.65, 0.13, 0, 0],
"MIDNIGHTBLUE": [0.98, 0.13, 0, 0.43],
"NAVYBLUE": [0.94, 0.54, 0, 0],
"ROYALBLUE": [1, 0.50, 0, 0],
"BLUE": [1, 1, 0, 0],
"CERULEAN": [0.94, 0.11, 0, 0],
"CYAN": [1, 0, 0, 0],
"PROCESSBLUE": [0.96, 0, 0, 0],
"SKYBLUE": [0.62, 0, 0.12, 0],
"TURQUOISE": [0.85, 0, 0.20, 0],
"TEALBLUE": [0.86, 0, 0.34, 0.02],
"AQUAMARINE": [0.82, 0, 0.30, 0],
"BLUEGREEN": [0.85, 0, 0.33, 0],
"EMERALD": [1, 0, 0.50, 0],
"JUNGLEGREEN": [0.99, 0, 0.52, 0],
"SEAGREEN": [0.69, 0, 0.50, 0],
"GREEN": [1, 0, 1, 0],
"FORESTGREEN": [0.91, 0, 0.88, 0.12],
"PINEGREEN": [0.92, 0, 0.59, 0.25],
"LIMEGREEN": [0.50, 0, 1, 0],
"YELLOWGREEN": [0.44, 0, 0.74, 0],
"SPRINGGREEN": [0.26, 0, 0.76, 0],
"OLIVEGREEN": [0.64, 0, 0.95, 0.40],
"RAWSIENNA": [0, 0.72, 1, 0.45],
"SEPIA": [0, 0.83, 1, 0.70],
"BROWN": [0, 0.81, 1, 0.60],
"TAN": [0.14, 0.42, 0.56, 0],
"GRAY": [0, 0, 0, 0.50],
"GREY": [0, 0, 0, 0.50],
"BLACK": [0, 0, 0, 1],
"WHITE": [0, 0, 0, 0],
}

for greylevel in range(0,101):
 colour_list["GREY%02d"%greylevel] = [0, 0, 0, float(greylevel)/100]
 colour_list["GRAY%02d"%greylevel] = [0, 0, 0, float(greylevel)/100]

colour_names = colour_list.keys()
colour_names.sort()

# Write ppl_colours.h
output = open("src/ppl_colours.h","w")
output.write("""// This file auto-generated by colours_generate.py

#ifndef _PPL_COLOURS
#define _PPL_COLOURS 1
""")

for i in range(len(colour_names)): output.write("#define COLOUR_%s %d\n"%(colour_names[i] , i+20000))
output.write("\n")
output.write("extern char *SW_COLOUR_STR[];\n")
output.write("extern int   SW_COLOUR_DCL[];\n")
output.write("extern int   SW_COLOUR_INT[];\n\n")
output.write("extern float SW_COLOUR_CMYK_C[];\n")
output.write("extern float SW_COLOUR_CMYK_M[];\n")
output.write("extern float SW_COLOUR_CMYK_Y[];\n")
output.write("extern float SW_COLOUR_CMYK_K[];\n\n")
output.write("#endif\n")
output.close()

# Write ppl_colours.c
output = open("src/ppl_colours.c","w")
output.write("""// This file auto-generated by colours_generate.py

#include "ppl_colours.h"

char *SW_COLOUR_STR[] = {""")
for i in range(len(colour_names)): output.write(""" "%s" , """%colour_names[i])
output.write("};\nint   SW_COLOUR_DCL[] = {")
for i in range(len(colour_names)): output.write(""" %d , """%len(colour_names[i]))
output.write("};\nint   SW_COLOUR_INT[] = {")
for i in range(len(colour_names)): output.write(""" COLOUR_%s , """%(colour_names[i]))
output.write("-1};\nfloat SW_COLOUR_CMYK_C[] = {")
for i in range(len(colour_names)): output.write(""" %f , """%(colour_list[colour_names[i]][0]))
output.write("};\nfloat SW_COLOUR_CMYK_M[] = {")
for i in range(len(colour_names)): output.write(""" %f , """%(colour_list[colour_names[i]][1]))
output.write("};\nfloat SW_COLOUR_CMYK_Y[] = {")
for i in range(len(colour_names)): output.write(""" %f , """%(colour_list[colour_names[i]][2]))
output.write("};\nfloat SW_COLOUR_CMYK_K[] = {")
for i in range(len(colour_names)): output.write(""" %f , """%(colour_list[colour_names[i]][3]))
output.write("};\n")

output.close()

