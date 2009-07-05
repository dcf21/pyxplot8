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
#include <string.h>
#include <glob.h>

#include "StringTools/asciidouble.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"

#include "pyxplot.h"
#include "ppl_error.h"
#include "ppl_input.h"
#include "ppl_parser.h"
#include "ppl_units.h"
#include "ppl_userspace.h"

int PPL_FLOWCTRL_BREAKABLE = 0;
int PPL_FLOWCTRL_BROKEN    = 0;
int PPL_FLOWCTRL_CONTINUED = 0;

void loopaddline(cmd_chain **cmd_put, char *line, int *bracegot, int *bracelevel, int *status)
 {
  int i,j,*k;
  char *desc, QuoteType='\0';

  // Ignore any whitespace at the beginning of this line
  i=0; while ((line[i]!='\0')&&(line[i]<=' ')) i++;
  if (line[i]=='\0') return;

  // If we haven't already had a {, we need to make sure we cut one off the beginning of this string
  if (!*bracegot)
   {
    if (line[i]!='{') { *status   = 1; return; } // We haven't got a {
    else              { *bracegot = 1; i++; }
   }

  // If this line starts with a }, and bracelevel is zero, we've reached the end of the loop
  while ((line[i]!='\0')&&(line[i]<=' ')) i++;
  if ((line[i]=='}') && (*bracelevel <= 0))
   {
    i++; while ((line[i]!='\0')&&(line[i]<=' ')) i++;
    if (line[i]=='\0') { *status = -2; return; }
    else               { *status = -1; return; }
   }

  // Check for any unquoted {s on this line, which mean we need to increase bracelevel
  for (j=i ; line[j] != '\0'; j++)
   {
    if (QuoteType != '\0') // Do not pay attention to brackets inside quoted strings
     {
      if ((line[j]==QuoteType) && (j!=0) && (line[j-1]!='\\')) QuoteType='\0';
      continue;
     }
    else if ((line[j]=='\'') || (line[j]=='\"')) // We are entering a quoted string
     { QuoteType = line[j]; }
    else if  (line[j]=='{')
     { (*bracelevel)++; }
    else if  (line[j]=='}')
     { (*bracelevel)--; }
   }

  // Add this line into the loop chain
  GetInputSource(&k, &desc);
  **cmd_put = (cmd_chain_item *)lt_malloc(sizeof(cmd_chain_item)); // Make a new chain element
  (**cmd_put)->line = (char *)lt_malloc(strlen(line+i)+1); // Write command line
  strcpy( (**cmd_put)->line , line+i );
  if (k==NULL) (**cmd_put)->linenumber  = -1; // Write source line number
  else         (**cmd_put)->linenumber  = *k;
  (**cmd_put)->next        = NULL; // Put null next tag in linked list
  if (desc == NULL)
   { (**cmd_put)->description = NULL; } // Write source filename description
  else
   {
    (**cmd_put)->description = (char *)lt_malloc(strlen(desc)+1);
    strcpy((**cmd_put)->description, desc);
   }
  *cmd_put                 = &((**cmd_put)->next); // Update where we're going to write the next line of looped commandline
  return;
 }

// Execute a loop chain once through
int loop_execute(cmd_chain *chain, int IterLevel)
 {
  int   status=0;
  char *line_ptr;

  ClearInputSource();
  while ((!status) && (PPL_SHELL_EXITING == 0) && (PPL_FLOWCTRL_BROKEN == 0) && (PPL_FLOWCTRL_CONTINUED == 0))
   {
    SetInputSourceLoop(chain);
    line_ptr = FetchInputStatement("","");
    if (line_ptr == NULL) break;
    if (StrStrip(line_ptr,line_ptr)[0] != '\0') status = ProcessDirective(line_ptr, 0, IterLevel);
   }
  ClearInputSource();
  return status;
 }

// Main entry point for the do statement
int directive_do(Dict *command, int IterLevel)
 {
  int        bracegot=0; // becomes one after we parse opening {
  int        bracelevel=0;
  int        status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  char      *criterion; // The value of the while (...) criterion
  value      criterion_val;
  int        i, j;
  char      *cptr;
  Dict      *cmd2;
  cmd_chain  chain     = NULL;
  cmd_chain  chainiter = NULL;
  cmd_chain *cmd_put   = NULL;
  cmd_put = &chain;

  // Check whether if statement had { and/or first command on same line
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("do ... > ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error("Error: Unterminated do clause."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error("Error: do statement should be followed by { ... }."); return 1; }
  else if (status == -2) { ppl_error("Error: do clause should be terminated with a while statement."); return 1; }

  // Check that final line has a while clause on it
  cmd2 = parse(cptr);
  if (cmd2 == NULL) return 1; // Parser has already thrown an error, we assume
  DictLookup(cmd2,"directive",NULL,(void **)(&cptr));
  if (strcmp(cptr,"while")!=0) { ppl_error("Error: only the statement 'while' can be placed after a } here."); return 1; }
  DictLookup(cmd2,"criterion",NULL,(void **)(&criterion));

  // Execute this do loop repeatedly
  do
   {
    chainiter = chain;
    status = loop_execute(&chainiter, IterLevel);
    if (status) break;
    i=-1; j=-1;
    ppl_EvaluateAlgebra(criterion, &criterion_val, 0, &i, &j, temp_err_string, 0);
    if (j>=0) { ppl_error("Error whilst evaluating while (...) criterion:"); ppl_error(temp_err_string); return 1; }
    if (!criterion_val.dimensionless) { sprintf(temp_err_string,"Error whilst evaluating while (...) criterion:\nThis should have been a dimensionless quantity, but instead had units of <%s>.",ppl_units_GetUnitStr(&criterion_val, NULL, NULL, 1, 0)); ppl_error(temp_err_string); return 1; }
   }
  while ((!status) && ((!ppl_units_DblEqual(criterion_val.real,0.0)) || (!ppl_units_DblEqual(criterion_val.imag,0.0))));
  return status;
 }

// Main entry point for the while command
int directive_while(Dict *command, int IterLevel)
 {
  int        bracegot=0; // becomes one after we parse opening {
  int        bracelevel=0;
  int        status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  char      *criterion; // The value of the while (...) criterion
  value      criterion_val;
  int        i, j;
  char      *cptr;
  cmd_chain  chain     = NULL;
  cmd_chain  chainiter = NULL;
  cmd_chain *cmd_put   = NULL;
  cmd_put = &chain;

  // Check whether if statement had { and/or first command on same line
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
  DictLookup(command,"criterion",NULL,(void **)(&criterion));

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("while .> ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error("Error: Unterminated while loop."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error("Error: while statement should be followed by { ... }."); return 1; }
  else if (status == -1) { ppl_error("Error: while clause should be terminated with a }."); return 1; }

  // Execute this while loop repeatedly
  do
   {
    i=-1; j=-1; status=0;
    ppl_EvaluateAlgebra(criterion, &criterion_val, 0, &i, &j, temp_err_string, 0);
    if (j>=0) { ppl_error("Error whilst evaluating while (...) criterion:"); ppl_error(temp_err_string); return 1; }
    if (!criterion_val.dimensionless) { sprintf(temp_err_string,"Error whilst evaluating while (...) criterion:\nThis should have been a dimensionless quantity, but instead had units of <%s>.",ppl_units_GetUnitStr(&criterion_val, NULL, NULL, 1, 0)); ppl_error(temp_err_string); return 1; }
    if (ppl_units_DblEqual(criterion_val.real,0.0) && ppl_units_DblEqual(criterion_val.imag,0.0)) break;
    chainiter = chain;
    status = loop_execute(&chainiter, IterLevel);
   }
  while (!status);
  return status;
 }

// Main entry point for the for statement
int directive_for(Dict *command, int IterLevel)
 {
  int        bracegot=0; // becomes one after we parse opening {
  int        bracelevel=0;
  int        status=0;   // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  char      *loopvar;    // The loop variable
  value     *start;
  value     *end;
  value     *step;
  value     *iterval;
  value      step_dummy;
  unsigned char backwards;
  char      *cptr;
  cmd_chain  chain     = NULL;
  cmd_chain  chainiter = NULL;
  cmd_chain *cmd_put   = NULL;
  cmd_put = &chain;

  // Check whether if statement had { and/or first command on same line
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
  DictLookup(command,"var_name",NULL,(void **)(&loopvar));
  DictLookup(command,"start_value",NULL,(void **)(&start));
  DictLookup(command,"final_value",NULL,(void **)(&end));
  DictLookup(command,"step_size",NULL,(void **)(&step));

  if (step == NULL)
   {
    step = &step_dummy;
    memcpy(&step_dummy, start, sizeof(value));
    step_dummy.real = 1.0; // Step must have same units as start/end values. These are gauranteed by ppl_parser to be REAL.
   }

  if (!ppl_units_DimEqual(start, end)) { sprintf(temp_err_string, "Error: The start and end values in this for loop are not dimensionally compatible. The start value has units of <%s>, while the end value has units of <%s>.", ppl_units_GetUnitStr(start, NULL, NULL, 0, 0), ppl_units_GetUnitStr(end, NULL, NULL, 1, 0)); ppl_error(temp_err_string); return 1; }
  if (!ppl_units_DimEqual(start, step)) { sprintf(temp_err_string, "Error: The start value and step size  in this for loop are not dimensionally compatible. The start value has units of <%s>, while the step size has units of <%s>.", ppl_units_GetUnitStr(start, NULL, NULL, 0, 0), ppl_units_GetUnitStr(step, NULL, NULL, 1, 0)); ppl_error(temp_err_string); return 1; }

  if (start->real < end->real) backwards=0;
  else                         backwards=1;

  if (((!backwards) && (step->real<=0)) || ((backwards) && (step->real>=0))) { sprintf(temp_err_string, "Error: The projected number of steps in this for loop is infinite."); ppl_error(temp_err_string); return 1; }

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("for... > ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error("Error: Unterminated for loop."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error("Error: for statement should be followed by { ... }."); return 1; }
  else if (status == -1) { ppl_error("Error: for loop should be terminated with a }."); return 1; }

  DictAppendValue(_ppl_UserSpace_Vars , loopvar , *start);
  DictLookup     (_ppl_UserSpace_Vars , loopvar , NULL, (void *)&iterval);

  // Execute this while loop repeatedly
  status = 0;
  while (((!backwards)&&(iterval->real < end->real)) || ((backwards)&&(iterval->real > end->real)))
   {
    chainiter = chain;
    status = loop_execute(&chainiter, IterLevel);
    if (status) break;
    iterval->real += step->real;
   }
  return status;
 }

// Main entry point for the foreach statement
int directive_foreach(Dict *command, int IterLevel)
 {
  int        bracegot=0; // becomes one after we parse opening {
  int        bracelevel=0;
  int        i,status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  char      *loopvar;    // The loop variable
  value     *iterval, *valptr;
  value      dummy;
  char      *cptr, *strptr;
  List      *listptr;
  Dict      *dictptr;
  ListIterator *listiter;
  cmd_chain  chain     = NULL;
  cmd_chain  chainiter = NULL;
  cmd_chain *cmd_put   = NULL;
  glob_t     GlobData;

  cmd_put = &chain;

  // Check whether if statement had { and/or first command on same line
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
  DictLookup(command,"var_name",NULL,(void **)(&loopvar));

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("for... > ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error("Error: Unterminated for loop."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error("Error: for statement should be followed by { ... }."); return 1; }
  else if (status == -1) { ppl_error("Error: for loop should be terminated with a }."); return 1; }

  ppl_units_zero(&dummy);
  DictAppendValue(_ppl_UserSpace_Vars , loopvar , dummy);
  DictLookup     (_ppl_UserSpace_Vars , loopvar , NULL, (void *)&iterval);

  // See if we're iterating over a globbed filename
  DictLookup     (command,"filename",NULL,(void **)(&cptr));
  if (cptr != NULL)
   {
    status=0;
    if (glob(cptr, 0, NULL, &GlobData) != 0) { ppl_error("Could not glob supplied filename."); return 1; }
    for (i=0; i<GlobData.gl_pathc; i++)
     {
      iterval->string = GlobData.gl_pathv[i];
      chainiter = chain;
      status = loop_execute(&chainiter, IterLevel);
      if (status) break;
     }
    globfree(&GlobData);
    return status;
   }

  // ... otherwise we're looping over a provided list
  DictLookup     (command,"item_list,",NULL,(void **)(&listptr));
  listiter = ListIterateInit(listptr);
  while (listiter != NULL)
   {
    dictptr = (Dict *)listiter->data;
    DictLookup(dictptr,"value",NULL,(void **)&valptr);
    if (valptr != NULL)
     { memcpy(iterval, valptr, sizeof(value)); }
    else
     {
      DictLookup(dictptr,"string",NULL,(void **)&strptr);
      iterval->string = strptr;
     }
    chainiter = chain;
    status = loop_execute(&chainiter, IterLevel);
    if (status) break;
    listiter = ListIterate(listiter, NULL);
   }
  return status;
 }

// Handles a single clause in an if ... else if ... else ... structure
int directive_ifelse(Dict *command, int state, int IterLevel) // state = 0 (don't do this, haven't worked), = 1 (do do this), = 2 (have already worked)
 {
  int        bracegot=0; // becomes one after we parse opening {
  int        bracelevel=0;
  int        status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  double    *criterion; // The value of the if (...) criterion
  char      *cptr;
  Dict      *cmd2;
  cmd_chain  chain   = NULL;
  cmd_chain *cmd_put = NULL;
  cmd_put = &chain;

  // Check whether if statement had { and/or first command on same line
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("if ... > ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error("Error: Unterminated if clause."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if (status == 1)
   {
    ppl_error("Error: if statement should be followed by { ... }."); return 1;
   }

  // See whether final line has an else clause on it
  if (status == -1)
   {
    cmd2 = parse(cptr);
    if (cmd2 == NULL) return 1; // Parser has already thrown an error, we assume
    DictLookup(cmd2,"directive",NULL,(void **)(&cptr));
    if (strcmp(cptr,"else")!=0) { ppl_error("Error: only the statement 'else' can be placed after a } here."); return 1; }
    DictLookup(cmd2,"if",NULL,(void **)(&cptr));
    if (cptr == NULL)
     {
      directive_ifelse(cmd2, state==0, IterLevel+1); // We have an else clause
     } else {
      DictLookup(cmd2,"criterion",NULL,(void **)(&criterion)); // We have an 'else if' clause
      if (ppl_units_DblEqual(*criterion, 0.0)) directive_ifelse(cmd2, (state>0)*2, IterLevel);
      else                                     directive_ifelse(cmd2, (state>0)+1, IterLevel);
     }
   }

  // If we were going to do this if ( ) codeblock, execute it now
  status = 0;
  if (state == 1) status = loop_execute(&chain, IterLevel);
  return status;
 }

// Main entry point for the if statement
int directive_if(Dict *command, int IterLevel)
 {
  double *criterion;
  int     status=0;

  DictLookup(command,"criterion",NULL,(void **)(&criterion));
  if (ppl_units_DblEqual(*criterion, 0.0)) status = directive_ifelse(command, 0, IterLevel);
  else                                     status = directive_ifelse(command, 1, IterLevel);
  return status;
 }

