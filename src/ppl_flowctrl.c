// ppl_flowctrl.c
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

#define _PPL_FLOWCTRL_C 1

#include <stdlib.h>
#include <stdio.h>

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "pyxplot.h"
#include "ppl_units.h"

int PPL_FLOWCTRL_BREAKABLE = 0;
int PPL_FLOWCTRL_BROKEN    = 0;
int PPL_FLOWCTRL_CONTINUED = 0;

typedef struct cmd_chain_item {
 char *line;
 struct cmd_chain_item *next;
 int linenumber;
 char *description;
} cmd_chain_item;

typedef struct cmd_chain_item *cmd_chain;

void loopaddline(cmd_chain **cmd_put, char *line, int *bracegot, int *status)
 {
  int i,j;
  if (!*bracegot)
   {
    i=0; while ((line[i]!='\0')&&(line[i]<=' '));
    if (line[i]=='\0') return;
    if (line[i]!='{' ) { *status = 1; return; }
    else { *bracegot=1; line=line+i+1; }
   }
  i=0; while ((line[i]!='\0')&&(line[i]<=' '));
  if (line[i]=='\0') return;
  ppl_error_getstreaminfo(&j, NULL);
  **cmd_put = (cmd_chain_item *)lt_malloc(sizeof(cmd_chain_item));
  (**cmd_put)->line = (char *)lt_malloc(strlen(line+i)+1);
  strcpy( (**cmd_put)->line , line+i );
  (**cmd_put)->linenumber = j;
  (**cmd_put)->next       = NULL;
  *cmd_put               = (**cmd_put)->next;
  return;
 }

void directive_do(Dict *command, int IterLevel)
 {
 }

void directive_for(Dict *command, int IterLevel)
 {
 }

void directive_foreach(Dict *command, int IterLevel)
 {
 }

void directive_ifelse(Dict *command, int state, int IterLevel) // state = 0 (don't do this, haven't worked), = 1 (do do this), = 2 (have already worked)
 {
  int        bracegot=0, status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  double    *criterion;
  char      *cptr, linebuffer[LSTR_LENGTH];
  Dict      *cmd2;
  cmd_chain  chain   = NULL;
  cmd_chain *cmd_put = NULL;
  cmd_put = &chain;
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &status);
  while (status==0)
   {
    cptr = FetchInputLine(linebuffer, -1, NULL, NULL, NULL, "if ... > ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &status);
    else            { ppl_error("Error: Unterminated if clause."); return; }
   }
  if      (status== 1)
   { ppl_error("Error: if statement should be followed by { ... }."); return; }
  else if (status==-1)
   {
    cmd2 = parse(cptr);
    if (cmd2 == NULL) return;
    DictLookup(cmd2,"directive",NULL,(void **)(&cptr));
    if (strcmp(cptr,"else")!=0) { ppl_error("Error: only the statement 'else' can be placed after a } here."); return; }
    DictLookup(cmd2,"if",NULL,(void **)(&cptr));
    if (cptr == NULL)
     {
      directive_ifelse(cmd2, state==0, IterLevel+1);
     } else {
      DictLookup(cmd2,"criterion",NULL,(void **)(&criterion));
      if ppl_units_DblEqual(criterion, 0.0) directive_ifelse(cmd2, (state>0)*2, IterLevel);
      else                                  directive_ifelse(cmd2, (state>0)+1, IterLevel);
     }
   }
  if (state == 1)
   {
    cmd_put = &chain;
    while (*cmd_put != NULL)
     {
      status = ProcessDirective((*cmd_put)->line, -1, IterLevel+1);
      if (status) break;
      cmd_put = &((*cmd_put)->next);
     }
   }
  return;
 }

void directive_if(Dict *command, int IterLevel)
 {
  double *criterion;
  DictLookup(command,"criterion",NULL,(void **)(&criterion));
  if ppl_units_DblEqual(criterion, 0.0) directive_ifelse(command, 0, IterLevel);
  else                                  directive_ifelse(command, 1, IterLevel);
  return
 }

void directive_while(Dict *command, int IterLevel)
 {
 }

