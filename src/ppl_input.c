// ppl_input.c
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

#define _PPL_INPUT_C 1

#include <stdlib.h>
#include <stdio.h>

#define INPUT_PIPE       101
#define INPUT_READLINE   102
#define INPUT_FILE       103
#define INPUT_LOOPCHAIN  104
#define INPUT_STRING     105

static FILE      *infile = NULL;
static cmd_chain *inchain = NULL;
static int        mode = -1;
static int       *linenumber = NULL;
static int       *inpos = NULL;
static char      *filename_description = NULL;

void SetInputSourcePipe(int *linenumber_, char *filename_description_)
 {
  infile               = stdin;
  inchain              = NULL;
  mode                 = INPUT_PIPE;
  linenumber           = linenumber_;
  inpos                = NULL;
  filename_description = filename_description_;
  return;
 }

void SetInputSourceReadline(int *linenumber_, char *filename_description_)
 {
  infile               = NULL;
  inchain              = NULL;
  mode                 = INPUT_READLINE;
  linenumber           = linenumber_;
  inpos                = NULL;
  filename_description = filename_description_;
  return;
 }

void SetInputSourceFile(FILE *infile_, int *linenumber_, char *filename_description_)
 {
  infile               = infile_;
  inchain              = NULL;
  mode                 = INPUT_FILE;
  linenumber           = linenumber_;
  inpos                = NULL;
  filename_description = filename_description_;
  return;
 }

void SetInputSourceLoop(cmd_chain *inchain_)
 {
  infile               = NULL;
  inchain              = inchain_;
  mode                 = INPUT_CHAIN;
  linenumber           = NULL;
  inpos                = NULL;
  filename_description = NULL;
  return;
 }

void SetInputSourceString(char *instr, int *inpos_)
 {
  infile               = NULL;
  inchain              = NULL;
  mode                 = INPUT_STRING;
  instr                = instr_;
  inpos                = inpos_;
  // Preserve filename_description and linenumber as whatever they were before...
  return;
 }

void GetInputSource(int **lineno, char **descr)
 {
  *lineno = linenumber;
  *descr  = filename_description
  return;
 }

char *FetchInputLine(char *output, char *prompt)
 {
  char *line_ptr;
  int   i;

  if (mode == INPUT_READLINE)
   {
    ppl_error_setstreaminfo(-1, "");
    line_ptr = readline(prompt);
    if (line_ptr==NULL) return NULL;
    add_history(line_ptr);
    strcpy(output, line_ptr);
    free(line_ptr);
    return output;
   }
  else if ((mode == INPUT_PIPE) || (mode == INPUT_FILE))
   {
    ppl_error_setstreaminfo(*linenumber, filename_description);
    if ((feof(infile)) || (ferror(infile))) return NULL;
    file_readline(infile, output);
    (*linenumber)++;
    return output;
   }
  else if (mode == INPUT_CHAIN)
   {
    if (inchain == NULL) return NULL;
    ppl_error_setstreaminfo(inchain->linenumber , inchain->description);
    strcpy(output, inchain->line);
    inchain = inchain->next;
    return output;
   }
  else if (mode == INPUT_STRING)
   {
    if (instr[*inpos]=='\0') return NULL;
    for (i=0; instr[*inpos]>=' '; i++,inpos++) output[i]=instr[*inpos];
    output[i++]='\0';
    while ((instr[*inpos]!='\0')&&(instr[*inpos]<' ')) (*inpos)++;
    return output;
   }
  else
   ppl_fatal(__FILE__,__LINE__,"Illegal setting for input mode.");
  return NULL;
 }

char *FetchInputStatement(char *output, 

int FetchAndExecuteLoop(

