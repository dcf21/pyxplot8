// dvi_interpreter.h
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

#ifndef _PPL_DVI_INTERPRETER_H
#define _PPL_DVI_INTERPRETER_H 1

#include "dvi_read.h"

// Postscript functions
int dviPostscriptLineto       (dviInterpreterState *interp);
int dviPostscriptClosepathFill(dviInterpreterState *interp);
int dviChngFnt                (dviInterpreterState *interp, int fn);
int dviSpecialColourCommand   (dviInterpreterState *interp, char *command);
int dviSpecialColourStackPush (dviInterpreterState *interp, char *psText);
int dviSpecialColourStackPop  (dviInterpreterState *interp);

// Interpreter functions for various types of dvi operators
int dviInOpChar       (dviInterpreterState *interp, DVIOperator *op);
int dviInOpSet1234    (dviInterpreterState *interp, DVIOperator *op);
int dviInOpSetRule    (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPut1234    (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPutRule    (dviInterpreterState *interp, DVIOperator *op);
int dviInOpNop        (dviInterpreterState *interp, DVIOperator *op);
int dviInOpBop        (dviInterpreterState *interp, DVIOperator *op);
int dviInOpEop        (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPush       (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPop        (dviInterpreterState *interp, DVIOperator *op);
int dviInOpRight1234  (dviInterpreterState *interp, DVIOperator *op);
int dviInOpW0         (dviInterpreterState *interp, DVIOperator *op);
int dviInOpW1234      (dviInterpreterState *interp, DVIOperator *op);
int dviInOpX0         (dviInterpreterState *interp, DVIOperator *op);
int dviInOpX1234      (dviInterpreterState *interp, DVIOperator *op);
int dviInOpDown1234   (dviInterpreterState *interp, DVIOperator *op);
int dviInOpY0         (dviInterpreterState *interp, DVIOperator *op);
int dviInOpY1234      (dviInterpreterState *interp, DVIOperator *op);
int dviInOpZ0         (dviInterpreterState *interp, DVIOperator *op);
int dviInOpZ1234      (dviInterpreterState *interp, DVIOperator *op);
int dviInOpFnt        (dviInterpreterState *interp, DVIOperator *op);
int dviInOpFnt1234    (dviInterpreterState *interp, DVIOperator *op);
int dviInOpSpecial1234(dviInterpreterState *interp, DVIOperator *op);
int dviInOpFntdef1234 (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPre        (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPost       (dviInterpreterState *interp, DVIOperator *op);
int dviInOpPostPost   (dviInterpreterState *interp, DVIOperator *op);

// Functions called by operator interpreter functions
void dviSpecialChar      (dviInterpreterState *interp, DVIOperator *op);
int  dviSpecialImplement (dviInterpreterState *interp);
int  dviNonAsciiChar     (dviInterpreterState *interp, int c, char move);
void dviUpdateBoundingBox(dviInterpreterState *interp, double width, double height, double depth);

#endif

