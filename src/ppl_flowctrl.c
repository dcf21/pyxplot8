// ppl_flowctrl.c
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

#define _PPL_FLOWCTRL_C 1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <wordexp.h>
#include <glob.h>

#include <gsl/gsl_math.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_memory.h"
#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"

#include "pyxplot.h"
#include "ppl_datafile.h"
#include "ppl_error.h"
#include "ppl_input.h"
#include "ppl_parser.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

Dict *PPL_SUBROUTINES = NULL;

char *PPL_FLOWCTRL_LOOPNAME[MAX_ITERLEVEL_DEPTH+1];
int   PPL_FLOWCTRL_BREAKABLE  = 0;
int   PPL_FLOWCTRL_BREAKLEVEL = -1;
int   PPL_FLOWCTRL_BROKEN     = 0;
int   PPL_FLOWCTRL_CONTINUED  = 0;

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
  if (( **cmd_put = (cmd_chain_item *)lt_malloc(sizeof(cmd_chain_item)) )==NULL) return; // Make a new chain element
  if (( (**cmd_put)->line = (char *)lt_malloc(strlen(line+i)+1)         )==NULL) return; // Write command line
  strcpy( (**cmd_put)->line , line+i );
  if (k==NULL) (**cmd_put)->linenumber  = -1; // Write source line number
  else         (**cmd_put)->linenumber  = *k;
  (**cmd_put)->next        = NULL; // Put null next tag in linked list
  if (desc == NULL)
   { (**cmd_put)->description = NULL; } // Write source filename description
  else
   {
    if (( (**cmd_put)->description = (char *)lt_malloc(strlen(desc)+1) )==NULL) return;
    strcpy((**cmd_put)->description, desc);
   }
  *cmd_put                 = &((**cmd_put)->next); // Update where we're going to write the next line of looped commandline
  return;
 }

// Execute a loop chain once through
int loop_execute(cmd_chain *chain, int breakable, int IterLevel)
 {
  int   status=0, OldBreakable;
  char *OldLB, *OldLBP, *OldLBA;
  char *line_ptr;

  OldBreakable = PPL_FLOWCTRL_BREAKABLE;
  PPL_FLOWCTRL_BREAKLEVEL = -1;
  PPL_FLOWCTRL_BROKEN     = 0;
  PPL_FLOWCTRL_CONTINUED  = 0;
  ClearInputSource(NULL,NULL,NULL,&OldLB,&OldLBP,&OldLBA);
  while ((!status) && (PPL_SHELL_EXITING == 0) && (PPL_FLOWCTRL_BROKEN == 0) && (PPL_FLOWCTRL_CONTINUED == 0))
   {
    PPL_FLOWCTRL_BREAKABLE = (breakable || OldBreakable);
    SetInputSourceLoop(chain);
    line_ptr = FetchInputStatement("","");
    if (line_ptr == NULL) break;
    if (StrStrip(line_ptr,line_ptr)[0] != '\0') status = ProcessDirective(line_ptr, 0, IterLevel);
   }
  PPL_FLOWCTRL_BREAKABLE = OldBreakable;
  if ((breakable) && ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel))) PPL_FLOWCTRL_CONTINUED = 0;
  ClearInputSource(OldLB,OldLBP,OldLBA,NULL,NULL,NULL);
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
    else            { ppl_error(ERR_SYNTAX, "Unterminated do clause."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error(ERR_SYNTAX, "do statement should be followed by { ... }."); return 1; }
  else if (status == -2) { ppl_error(ERR_SYNTAX, "do clause should be terminated with a while statement."); return 1; }

  // Check that final line has a while clause on it
  cmd2 = parse(cptr);
  if (cmd2 == NULL) return 1; // Parser has already thrown an error, we assume
  DictLookup(cmd2,"directive",NULL,(void **)(&cptr));
  if (strcmp(cptr,"while")!=0) { ppl_error(ERR_SYNTAX, "Only the statement 'while' can be placed after a } here."); return 1; }
  DictLookup(cmd2,"criterion",NULL,(void **)(&criterion));

  // Set loop name, if we have one
  DictLookup(command,"loopname",NULL,(void **)(&cptr));
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = cptr;

  // Execute this do loop repeatedly
  do
   {
    chainiter = chain;
    status = loop_execute(&chainiter, 1, IterLevel);
    if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; break; }
    if (status) break;
    i=-1; j=-1;
    ppl_EvaluateAlgebra(criterion, &criterion_val, 0, &i, 0, &j, temp_err_string, 0);
    if (j>=0) { ppl_error(ERR_GENERAL, temp_err_string); PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL; return 1; }
    if (!criterion_val.dimensionless) { sprintf(temp_err_string,"while (...) criterion should be a dimensionless quantity, but instead has units of <%s>.",ppl_units_GetUnitStr(&criterion_val, NULL, NULL, 1, 0)); ppl_error(ERR_NUMERIC, temp_err_string); PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL; return 1; }
   }
  while ((!status) && ((!ppl_units_DblEqual(criterion_val.real,0.0)) || (!ppl_units_DblEqual(criterion_val.imag,0.0))));
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;
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
    else            { ppl_error(ERR_SYNTAX, "Unterminated while loop."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error(ERR_SYNTAX, "while statement should be followed by { ... }."); return 1; }
  else if (status == -1) { ppl_error(ERR_SYNTAX, "while clause should be terminated with a }."); return 1; }

  // Set loop name, if we have one
  DictLookup(command,"loopname",NULL,(void **)(&cptr));
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = cptr;

  // Execute this while loop repeatedly
  do
   {
    i=-1; j=-1; status=0;
    ppl_EvaluateAlgebra(criterion, &criterion_val, 0, &i, 0, &j, temp_err_string, 0);
    if (j>=0) { ppl_error(ERR_GENERAL, temp_err_string); PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL; return 1; }
    if (!criterion_val.dimensionless) { sprintf(temp_err_string,"while (...) criterion should be a dimensionless quantity, but instead has units of <%s>.",ppl_units_GetUnitStr(&criterion_val, NULL, NULL, 1, 0)); ppl_error(ERR_NUMERIC, temp_err_string); PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL; return 1; }
    if (ppl_units_DblEqual(criterion_val.real,0.0) && ppl_units_DblEqual(criterion_val.imag,0.0)) break;
    chainiter = chain;
    status = loop_execute(&chainiter, 1, IterLevel);
    if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; break; }
   }
  while (!status);
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;
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

  if (!ppl_units_DimEqual(start, end)) { sprintf(temp_err_string, "The start and end values in this for loop are not dimensionally compatible. The start value has units of <%s>, while the end value has units of <%s>.", ppl_units_GetUnitStr(start, NULL, NULL, 0, 0), ppl_units_GetUnitStr(end, NULL, NULL, 1, 0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }
  if (!ppl_units_DimEqual(start, step)) { sprintf(temp_err_string, "The start value and step size  in this for loop are not dimensionally compatible. The start value has units of <%s>, while the step size has units of <%s>.", ppl_units_GetUnitStr(start, NULL, NULL, 0, 0), ppl_units_GetUnitStr(step, NULL, NULL, 1, 0)); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }

  if (start->real < end->real) backwards=0;
  else                         backwards=1;

  if (((!backwards) && (step->real<=0)) || ((backwards) && (step->real>=0))) { sprintf(temp_err_string, "The projected number of steps in this for loop is infinite."); ppl_error(ERR_NUMERIC, temp_err_string); return 1; }

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("for... > ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error(ERR_SYNTAX, "Unterminated for loop."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error(ERR_SYNTAX, "for statement should be followed by { ... }."); return 1; }
  else if (status == -1) { ppl_error(ERR_SYNTAX, "for loop should be terminated with a }."); return 1; }

  DictAppendValue(_ppl_UserSpace_Vars , loopvar , *start);
  DictLookup     (_ppl_UserSpace_Vars , loopvar , NULL, (void *)&iterval);

  // Set loop name, if we have one
  DictLookup(command,"loopname",NULL,(void **)(&cptr));
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = cptr;

  // Execute this for loop repeatedly
  status = 0;
  while (((!backwards)&&(iterval->real < end->real)) || ((backwards)&&(iterval->real > end->real)))
   {
    chainiter = chain;
    status = loop_execute(&chainiter, 1, IterLevel);
    if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; break; }
    if (status) break;
    iterval->real += step->real;
   }
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;
  return status;
 }

// Main entry point for the foreach statement
int directive_foreach(Dict *command, int IterLevel)
 {
  unsigned char foreachdatum;
  int        bracegot=0; // becomes one after we parse opening {
  int        bracelevel=0;
  int        i,j,status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
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
  wordexp_t  WordExp;
  glob_t     GlobData;
  unsigned char WordExpNull=1,GlobDataNull=1;

  cmd_put = &chain;

  // Check whether if statement had { and/or first command on same line
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
  DictLookup(command,"df",NULL,(void **)(&cptr));
  if (cptr==NULL) { foreachdatum=0; } else { foreachdatum=1; } // See whether we're looping over filenames, or data within files

  if (!foreachdatum) DictLookup(command,"var_name",NULL,(void **)(&loopvar)); // Have one loop variable if not looping over a datafile

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("for... > ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error(ERR_SYNTAX, "Unterminated for loop."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error(ERR_SYNTAX, "for statement should be followed by { ... }."); return 1; }
  else if (status == -1) { ppl_error(ERR_SYNTAX, "for loop should be terminated with a }."); return 1; }

  if (!foreachdatum)
   {
    ppl_units_zero(&dummy);
    DictAppendValue(_ppl_UserSpace_Vars , loopvar , dummy);
    DictLookup     (_ppl_UserSpace_Vars , loopvar , NULL, (void *)&iterval); // Have one loop variable if not looping over a datafile
   }

  // Set loop name, if we have one
  DictLookup(command,"loopname",NULL,(void **)(&cptr));
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = cptr;

  // See if we're iterating over a globbed filename
  DictLookup     (command,"filename_list",NULL,(void **)(&listptr));
  if (listptr != NULL)
   {
    listiter = ListIterateInit(listptr);
    while (listiter != NULL)
     {
      dictptr = (Dict *)listiter->data;
      DictLookup(dictptr,"filename",NULL,(void **)(&cptr));
      status=0;
      if ((wordexp(cptr, &WordExp, 0) != 0) || (WordExp.we_wordc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", cptr); ppl_error(ERR_FILE, temp_err_string); listiter = ListIterate(listiter, NULL); continue; }
      WordExpNull = 0;
      for (j=0; j<WordExp.we_wordc; j++)
       {
        if ((glob(WordExp.we_wordv[j], 0, NULL, &GlobData) != 0) || (GlobData.gl_pathc <= 0)) { sprintf(temp_err_string, "Could not glob filename '%s'.", WordExp.we_wordv[j]); ppl_error(ERR_FILE, temp_err_string); GlobDataNull=1; continue; }
        GlobDataNull=0;
        for (i=0; i<GlobData.gl_pathc; i++)
         {
          if (foreachdatum) { directive_foreach_LoopOverData(command, GlobData.gl_pathv[i], &chain, &chainiter, IterLevel, &status); } // Looping over data
          else
           {
            iterval->string = GlobData.gl_pathv[i]; // Looping over filenames
            chainiter = chain;
            status = loop_execute(&chainiter, 1, IterLevel);
            if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; break; }
           }
          if (status) break;
         }
        if (status) break;
       }
      if (!WordExpNull ) { wordfree(&WordExp ); WordExpNull =1; }
      if (!foreachdatum) ppl_units_zero(iterval);
      if (!GlobDataNull) { globfree(&GlobData); GlobDataNull=1; }
      if (status) break;
      listiter = ListIterate(listiter, NULL);
     }
    PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;
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
    status = loop_execute(&chainiter, 1, IterLevel);
    if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; break; }
    if (status) break;
    listiter = ListIterate(listiter, NULL);
   }
  if (!foreachdatum) ppl_units_zero(iterval);
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;
  return status;
 }

// Part of directive_foreach... loops over data in a datafile
void directive_foreach_LoopOverData(Dict *command, char *filename, cmd_chain *chain, cmd_chain *chainiter, int IterLevel, int *status)
 {
  DataTable *data;
  DataBlock *blk;
  long int   i, j, k;
  int        ContextDataTab, index=-1, *indexptr, rowcol=DATAFILE_COL, ErrCount=DATAFILE_NERRS;
  char       errtext[LSTR_LENGTH], *ReadVars[USING_ITEMS_MAX], *tempstr=NULL, *SelectCrit=NULL;
  List      *UsingList=NULL, *EveryList=NULL;
  value     *min[USING_ITEMS_MAX], *max[USING_ITEMS_MAX], *outval[USING_ITEMS_MAX];
  value     *DummyVar, DummyTemp;
  double     val;
  unsigned char InRange;

  List         *RangeList, *VarList;
  ListIterator *ListIter;
  Dict         *TempDict;

  if (DEBUG) ppl_log("Beginning to read data in for 'foreach datum' command");

  // Initialise DummyTemp
  ppl_units_zero(&DummyTemp);
  DummyTemp.real    = 1.0;

  DictLookup(command, "variables," , NULL, (void **)&VarList);
  i = ListLen(VarList);
  if ((i<0) || (i>USING_ITEMS_MAX)) { sprintf(temp_err_string,"The 'foreach ... in datafile' construct must be supplied a list of between %d and %d variables to read.", 1, USING_ITEMS_MAX); ppl_error(ERR_SYNTAX, temp_err_string); *status=1; return; }
  ListIter = ListIterateInit(VarList);
  for (j=0; j<i; j++)
   {
    TempDict = (Dict *)ListIter->data;
    DictLookup(TempDict,"variable",NULL,(void **)(ReadVars+j)); // Read variable name into ReadVars[j]

    // Look up variable in user space and get pointer to its value
    DictLookup(_ppl_UserSpace_Vars, ReadVars[j], NULL, (void **)&DummyVar);
    if (DummyVar!=NULL)
     {
      if ((DummyVar->string != NULL) || ((DummyVar->FlagComplex) && (settings_term_current.ComplexNumbers == SW_ONOFF_OFF)) || (!gsl_finite(DummyVar->real)) || (!gsl_finite(DummyVar->imag))) { ppl_units_zero(DummyVar); DummyVar->real=1.0; } // Turn string variables into floats
      outval[j] = DummyVar;
     }
    else
     {
      DictAppendValue(_ppl_UserSpace_Vars, ReadVars[j], DummyTemp);
      DictLookup(_ppl_UserSpace_Vars, ReadVars[j], NULL, (void **)&DummyVar);
      outval[j] = DummyVar;
     }
    ListIter = ListIterate(ListIter, NULL);
   }

  DictLookup(command, "index"      , NULL, (void **)&indexptr);   if (indexptr == NULL) indexptr = &index;
  DictLookup(command, "use_rows"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_ROW;
  DictLookup(command, "use_cols"   , NULL, (void **)&tempstr);    if (tempstr  != NULL) rowcol=DATAFILE_COL;
  DictLookup(command, "using_list:", NULL, (void **)&UsingList);
  DictLookup(command, "every_list:", NULL, (void **)&EveryList);
  DictLookup(command, "select_criterion", NULL, (void **)&SelectCrit);

  DictLookup(command, "range_list", NULL, (void **)&RangeList);
  ListIter = ListIterateInit(RangeList);
  for (j=0; j<i; j++)
   if (ListIter == NULL) { min[j]=NULL; max[j]=NULL; }
   else
    {
     TempDict = (Dict *)ListIter->data;
     DictLookup(TempDict,"min",NULL,(void **)(min+j));
     DictLookup(TempDict,"max",NULL,(void **)(max+j));
     if ((min[j]!=NULL)&&(max[j]!=NULL)&&(!ppl_units_DimEqual(min[j],max[j]))) { sprintf(temp_err_string, "The minimum and maximum limits specified for fitting variable %ld (%s) in the 'foreach ... in datafile' construct have conflicting physical dimensions. The former has units of <%s>, whilst the latter has units of <%s>.", j+1, ReadVars[j], ppl_units_GetUnitStr(min[j],NULL,NULL,0,0), ppl_units_GetUnitStr(max[j],NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *status=1; return; }
     ListIter = ListIterate(ListIter, NULL);
    }
   if (ListIter != NULL) { sprintf(temp_err_string, "Too many ranges supplied to the 'foreach ... in datafile' construct. %d ranges were supplied, even though only %ld variables are being read.", ListLen(RangeList), i); ppl_error(ERR_SYNTAX, temp_err_string); *status=1; return; }

  ContextDataTab = lt_DescendIntoNewContext();
  DataFile_read(&data, status, errtext, filename, *indexptr, rowcol, UsingList, EveryList, NULL, i, SelectCrit, DATAFILE_CONTINUOUS, &ErrCount);
  if (*status) { ppl_error(ERR_GENERAL, errtext); *status=1; return; }

  // Check that the FirstEntries above have the same units as any supplied ranges
  for (j=0; j<i; j++)
   if (min[j] != NULL)
    {
     if (!ppl_units_DimEqual(min[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the 'foreach ... in datafile' construct for variable %ld (%s) have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ReadVars[j], ppl_units_GetUnitStr(min[j],NULL,NULL,0,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *status=1; return; }
    }
   else if (max[j] != NULL)
    {
     if (!ppl_units_DimEqual(max[j],data->FirstEntries+j)) { sprintf(temp_err_string, "The minimum and maximum limits specified in the 'foreach ... in datafile' construct for variable %ld (%s) have conflicting physical dimensions with the data returned from the data file. The limits have units of <%s>, whilst the data have units of <%s>.", j+1, ReadVars[j], ppl_units_GetUnitStr(max[j],NULL,NULL,0,0), ppl_units_GetUnitStr(data->FirstEntries+j,NULL,NULL,1,0)); ppl_error(ERR_NUMERIC, temp_err_string); *status=1; return; }
    }

  // Begin looping over input data
  blk = data->first;
  while (blk != NULL)
   {
    for (j=0; j<blk->BlockPosition; j++)
     {
      InRange=1;
      for (k=0; k<i; k++)
       {
        val = blk->data_real[k + i*j];
        if ( ((min[k]!=NULL)&&(val<min[k]->real)) || ((max[k]!=NULL)&&(val>max[k]->real)) ) { InRange=0; break; } // Check that value is within range
        ppl_units_zero(outval[k]);
        ppl_units_DimCpy(outval[k] , data->FirstEntries+k); // Copy physical dimensions from data file to output variables
        outval[k]->real = val;
        outval[k]->imag = 0.0;
        outval[k]->FlagComplex = 0;
       }
      if (InRange) // Only run looped script if this data point is within supplied range
       {
       *chainiter = *chain;
       *status = loop_execute(chainiter, 1, IterLevel);
       if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; break; }
       if (*status) break;
      }
     }
    blk=blk->next;
    if (*status) break;
   }

  // Free data grid read from data file
  lt_AscendOutOfContext(ContextDataTab);
  return;
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

  // if clauses have no loop names
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;

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
    else            { ppl_error(ERR_SYNTAX, "Unterminated if clause."); return 1; }
   }

  // Check whether we found a statement before we found a {
  if (status == 1)
   {
    ppl_error(ERR_SYNTAX, "if statement should be followed by { ... }."); return 1;
   }

  // See whether final line has an else clause on it
  if (status == -1)
   {
    cmd2 = parse(cptr);
    if (cmd2 == NULL) return 1; // Parser has already thrown an error, we assume
    DictLookup(cmd2,"directive",NULL,(void **)(&cptr));
    if (strcmp(cptr,"else")!=0) { ppl_error(ERR_SYNTAX, "Only the statement 'else' can be placed after a } here."); return 1; }
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
  if (state == 1) status = loop_execute(&chain, 0, IterLevel);
  return status;
 }

// Main entry point for the if statement
int directive_if(Dict *command, int IterLevel)
 {
  double *criterion;
  int     status=0;

  // if clauses have no loop names
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;

  DictLookup(command,"criterion",NULL,(void **)(&criterion));
  if (ppl_units_DblEqual(*criterion, 0.0)) status = directive_ifelse(command, 0, IterLevel);
  else                                     status = directive_ifelse(command, 1, IterLevel);
  return status;
 }

// -----------------------------------------------
// PROCEDURES FOR DEFINING AND CALLING SUBROUTINES
// -----------------------------------------------

int directive_subroutine(Dict *command, int IterLevel)
 {
  int                   bracegot=0; // becomes one after we parse opening {
  int                   bracelevel=0;
  int                   status=0; // status =-2 (found }), =-1 (found }...), =0 (still reading), =1 (didn't find {)
  int                   i, j, NArgs, MemContext;
  char                 *cptr, *name;
  List                 *ArgList;
  ListIterator         *ListIter;
  Dict                 *TempDict;
  SubroutineDescriptor *NewSub;
  cmd_chain             chain     = NULL;
  cmd_chain            *cmd_put   = NULL;
  cmd_put = &chain;

  // Define subroutine command loop in memory context 0
  MemContext = lt_GetMemContext();
  _lt_SetMemContext(0);

  // Read arguments of subroutine
  DictLookup(command,"argument_list,",NULL,(void **)(&ArgList));
  NArgs = ListLen(ArgList);
  ListIter = ListIterateInit(ArgList);
  for (i=0,j=0; i<NArgs; i++)
   {
    TempDict = (Dict *)ListIter->data;
    DictLookup(TempDict,"argument_name" ,NULL,(void **)(&cptr));
    strcpy(temp_err_string+j,cptr);
    j += strlen(temp_err_string+j)+1;
    ListIter = ListIterate(ListIter, NULL);
   }

  // Malloc subroutine descriptor
  NewSub = (SubroutineDescriptor *)lt_malloc(sizeof(SubroutineDescriptor));
  if (NewSub==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); _lt_SetMemContext(MemContext); return 1; }
  NewSub->ArgList = (char *)lt_malloc(j);
  if (NewSub->ArgList==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); _lt_SetMemContext(MemContext); return 1; }
  memcpy(NewSub->ArgList,temp_err_string,j);
  NewSub->NumberArguments = NArgs;
  NewSub->commands = chain;

  // Check whether subroutine statement had { and/or first command on same line
  DictLookup(command,"subroutine_name",NULL,(void **)(&name));
  DictLookup(command,"brace",NULL,(void **)(&cptr));
  if (cptr!=NULL) bracegot = 1;
  DictLookup(command,"command",NULL,(void **)(&cptr));
  if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);

  // Fetch lines and add them into loop chain until we get a }
  while (status==0)
   {
    cptr = FetchInputStatement("subrtne> ",".......> ");
    if (cptr!=NULL) loopaddline(&cmd_put, cptr, &bracegot, &bracelevel, &status);
    else            { ppl_error(ERR_SYNTAX, "Unterminated subroutine definition."); _lt_SetMemContext(MemContext); return 1; }
   }

  // Check whether we found a statement before we found a {
  if      (status ==  1) { ppl_error(ERR_SYNTAX, "subroutine statement should be followed by { ... }."); _lt_SetMemContext(MemContext); return 1; }
  else if (status == -1) { ppl_error(ERR_SYNTAX, "subroutine clause should be terminated with a }."); _lt_SetMemContext(MemContext); return 1; }

  // Add subroutine to subroutine dictionary
  NewSub->commands = chain;
  DictAppendPtr(PPL_SUBROUTINES, name, (void *)NewSub, sizeof(SubroutineDescriptor), 0, DATATYPE_VOID);
  _lt_SetMemContext(MemContext);
  return 0;
 }

int directive_call(Dict *command, int IterLevel)
 {
  int           j, k, NArgs, status=0;
  char         *name, *cptr;
  SubroutineDescriptor *sd;
  List         *ArgList;
  ListIterator *ListIter;
  Dict         *TempDict;
  value     *VarData, *InputValue, *ValueBuffer, TempValue;
  cmd_chain  chainiter = NULL;

  ppl_units_zero(&TempValue);

  // Look up subroutine name
  DictLookup(command,"subroutine_name",NULL,(void **)(&name));
  DictLookup(PPL_SUBROUTINES,name,NULL,(void **)(&sd));
  if (sd==NULL) { sprintf(temp_err_string,"No subroutine defined with name '%s'.",name); ppl_error(ERR_GENERAL,temp_err_string); return 1; }

  // Check that we have the right number of arguments
  NArgs = sd->NumberArguments;
  DictLookup(command,"argument_list,",NULL,(void **)(&ArgList));
  if (ListLen(ArgList)!=NArgs) { sprintf(temp_err_string,"Subroutine '%s' takes %d arguments, but %d have been supplied.",name,sd->NumberArguments,ListLen(ArgList)); ppl_error(ERR_GENERAL,temp_err_string); return 1; }

  // Malloc temporary buffer for holding the values of the variables which we overwrite
  ValueBuffer = (value *)lt_malloc(NArgs * sizeof(value));
  if (ValueBuffer==NULL) { ppl_error(ERR_MEMORY, "Out of memory."); return 1; }

  // Substitute arguments into user's variable dictionary
  DictLookup(command,"argument_list,",NULL,(void **)(&ArgList));
  ListIter = ListIterateInit(ArgList);
  for (j=k=0; k<NArgs; k++) // Swap new arguments for old in global dictionary
   {
    TempDict = (Dict *)ListIter->data;
    DictLookup(TempDict,"argument",NULL,(void **)(&InputValue));
    if (InputValue==NULL)
     {
      DictLookup(TempDict,"string_argument",NULL,(void **)(&cptr));
      InputValue = &TempValue;
      TempValue.string = cptr;
     }

    DictLookup(_ppl_UserSpace_Vars, sd->ArgList+j, NULL, (void **)&VarData);
    if (VarData!=NULL)
     {
      memcpy(ValueBuffer+k, VarData, sizeof(value));
      memcpy(VarData, InputValue, sizeof(value));
     }
    else
     {
      ppl_units_zero(ValueBuffer+k);
      ValueBuffer[k].modified=2;
      DictAppendValue(_ppl_UserSpace_Vars, sd->ArgList+j, *InputValue);
     }
    j += strlen(sd->ArgList+j)+1;
    ListIter = ListIterate(ListIter, NULL);
   }

  // Loop through command loop
  PPL_FLOWCTRL_LOOPNAME[IterLevel] = NULL;
  chainiter = sd->commands;
  status = loop_execute(&chainiter, 1, IterLevel);
  if (PPL_FLOWCTRL_BROKEN) { if ((PPL_FLOWCTRL_BREAKLEVEL<0)||(PPL_FLOWCTRL_BREAKLEVEL==IterLevel)) PPL_FLOWCTRL_BROKEN=0; }

  // Return arguments to their original values
  for (j=k=0; k<NArgs; k++) // Swap new arguments for old in global dictionary
   {
    DictLookup(_ppl_UserSpace_Vars, sd->ArgList+j, NULL, (void **)&VarData);
    memcpy(VarData, ValueBuffer+k, sizeof(value));
    j += strlen(sd->ArgList+j)+1;
   }

  return status;
 }

