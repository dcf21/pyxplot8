// ppl_shell.c
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

#define _PPL_SHELL_C 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <glob.h>
#include <setjmp.h>
#include <readline/history.h>
#include <sys/select.h>
#include <sys/wait.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_memory.h"

#include "pyxplot.h"
#include "ppl_canvasitems.h"
#include "ppl_children.h"
#include "ppl_eqnsolve.h"
#include "ppl_error.h"
#include "ppl_flowctrl.h"
#include "ppl_help.h"
#include "ppl_input.h"
#include "ppl_interpolation.h"
#include "ppl_parser.h"
#include "ppl_passwd.h"
#include "ppl_setshow.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_userspace.h"

int PPL_SHELL_EXITING;

void InteractiveSession()
 {
  int   linenumber = 1;
  char *line_ptr;

  if (DEBUG) ppl_log("Starting an interactive session.");

  // Set up SIGINT handler
  if (sigsetjmp(sigjmp_ToInteractive, 1) == 0)
   {
    sigjmp_FromSigInt = &sigjmp_ToInteractive;

    if ((isatty(STDIN_FILENO) == 1) && (settings_session_default.splash == SW_ONOFF_ON)) ppl_report(txt_init);

    PPL_SHELL_EXITING = 0;
    ClearInputSource();
    while ((PPL_SHELL_EXITING == 0) && (PPL_FLOWCTRL_BROKEN == 0) && (PPL_FLOWCTRL_CONTINUED == 0))
     {
      CheckForGvOutput();
      if (isatty(STDIN_FILENO) == 1) SetInputSourceReadline(&linenumber);
      else                           SetInputSourcePipe(&linenumber, "piped input");
      line_ptr = FetchInputStatement("pyxplot> ",".......> ");
      if (line_ptr == NULL) break;
      ProcessDirective(line_ptr, isatty(STDIN_FILENO), 0);
      ppl_error_setstreaminfo(-1, "");
     }

    if (isatty(STDIN_FILENO) == 1) 
     {
      if (settings_session_default.splash == SW_ONOFF_ON) ppl_report("\nGoodbye. Have a nice day.");
      else                                                ppl_report(""); // Make a new line
     }
   } else {
    ppl_error("\nReceived CTRL-C. Terminating session."); // SIGINT longjmps return here
    if (chdir(settings_session_default.cwd) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into cwd failed."); } // chdir into temporary directory
   }
  PPL_SHELL_EXITING = 0;
  ClearInputSource();

  sigjmp_FromSigInt = &sigjmp_ToMain; // SIGINT now drops back through to main().
  return;
 }

void ProcessPyXPlotScript(char *input, int IterLevel)
 {
  int  linenumber = 1;
  int  status;
  int  ProcessedALine = 0;
  char full_filename[FNAME_LENGTH];
  char filename_description[FNAME_LENGTH];
  char *line_ptr;
  FILE *infile;

  sprintf(temp_err_string, "Processing input from the script file '%s'.", input); ppl_log(temp_err_string);
  UnixExpandUserHomeDir(input , settings_session_default.cwd, full_filename);
  sprintf(filename_description, "file '%s'", input);
  if ((infile=fopen(full_filename,"r")) == NULL)
   {
    sprintf(temp_err_string, "PyXPlot Error: Could not find command file '%s'\nSkipping on to next command file", full_filename); ppl_error(temp_err_string);
    return;
   }

  ClearInputSource();
  PPL_SHELL_EXITING = 0;
  while ((PPL_SHELL_EXITING == 0) && (PPL_FLOWCTRL_BROKEN == 0) && (PPL_FLOWCTRL_CONTINUED == 0))
   {
    SetInputSourceFile(infile, &linenumber, filename_description);
    line_ptr = FetchInputStatement("","");
    if (line_ptr == NULL) break;
    if (StrStrip(line_ptr,line_ptr)[0] != '\0')
     {
      status = ProcessDirective(line_ptr, 0, IterLevel);
      ppl_error_setstreaminfo(-1, "");
      if ((ProcessedALine==0) && (status>0)) // If an error occurs on the first line of a script, aborted processing it
       {
        ppl_error("Error on first line of commandfile: Is this is valid script?\nAborting");
        break;
       }
      if (status==0) ProcessedALine = 1;
     }
   }
  PPL_SHELL_EXITING = 0;
  ClearInputSource();

  fclose(infile);
  CheckForGvOutput();
  return;
 }

int ProcessDirective(char *in, int interactive, int IterLevel)
 {
  int   memcontext, i, is, j, l;
  int   status=0;
  char  QuoteChar='\0';
  static char *DirectiveLinebuffer = NULL;
  Dict *command;
  FILE *SubstPipe;

  static int interactive_last=0;
  if (interactive < 0) interactive      = interactive_last;
  else                 interactive_last = interactive;

  if (DirectiveLinebuffer != NULL) { free(DirectiveLinebuffer); DirectiveLinebuffer=NULL; }

  // If this line is blank, ignore it
  for (i=0; in[i]!='\0'; i++); for (; ((i>0)&&(in[i]<=' ')); i--);
  if ((i==0) && (in[i]<=' ')) return -1;

  memcontext = lt_DescendIntoNewContext();
  if ((interactive==0) || (IterLevel > 0) || (sigsetjmp(sigjmp_ToDirective, 1) == 0))  // Set up SIGINT handler, but only if this is an interactive session
   {
    if ((interactive!=0) && (IterLevel == 0)) sigjmp_FromSigInt = &sigjmp_ToDirective;

    // Do `` substitution
    l = strlen(in+1);
    DirectiveLinebuffer = (char *)malloc(l*sizeof(char));
    for (i=0, j=0; in[i]!='\0'; i++)
     {
      if      ((QuoteChar=='\0') && (in[i]=='\'')                   ) QuoteChar = '\'';
      else if ((QuoteChar=='\0') && (in[i]=='\"')                   ) QuoteChar = '\"';
      else if ((QuoteChar=='\'') && (in[i]=='\'') && (in[i-1]!='\\')) QuoteChar = '\0';
      else if ((QuoteChar=='\"') && (in[i]=='\"') && (in[i-1]!='\\')) QuoteChar = '\0';

      if      ((QuoteChar=='\0') && (in[i]=='`' )                   )
       {
        is=++i;
        for ( ; ((in[i]!='\0')&&(in[i]!='`')) ; i++);
        if (in[i]!='`') { ppl_error("Mismatched `"); status=1; break; }
        in[i]='\0';
        if (DEBUG) { sprintf(temp_err_string, "Shell substitution with command '%s'.", in+is); ppl_log(temp_err_string); }
        if ((SubstPipe = popen(in+is,"r"))==NULL)
         {
          sprintf(temp_err_string, "Could not spawl shell substitution command '%s'.", in+is); ppl_error(temp_err_string);
          status=1; break;
         }
        while ((!feof(SubstPipe)) && (!ferror(SubstPipe)))
         {
          if (l <= j) DirectiveLinebuffer = (char *)realloc((void *)DirectiveLinebuffer, l=l+1024);
          fscanf(SubstPipe,"%c",DirectiveLinebuffer + j);
          if (DirectiveLinebuffer[j] == '\n') DirectiveLinebuffer[j] = ' ';
          if (DirectiveLinebuffer[j] != '\0') j++;
         }
        status = pclose(SubstPipe);
        if (status != 0) break;
       }
      else
       {
        if (l <= j) DirectiveLinebuffer = (char *)realloc((void *)DirectiveLinebuffer, l=l+1024);
        DirectiveLinebuffer[j++] = in[i];
       }
     }
    if (l <= j) DirectiveLinebuffer = (char *)realloc((void *)DirectiveLinebuffer, l=l+1);
    DirectiveLinebuffer[j] = '\0';

    // Parse and execute command
    if (status==0)
     {
      command = parse(DirectiveLinebuffer);
      if (command != NULL) status = ProcessDirective2(DirectiveLinebuffer, command, interactive, memcontext, IterLevel);
      else                 status = 1;
      // If command is NULL, we had a syntax error
     }
   } else {
    ppl_error("\nReceived CTRL-C. Terminating command."); // SIGINT longjmps return here
    status = 1;
   }
  if (IterLevel == 0) sigjmp_FromSigInt = &sigjmp_ToMain; // SIGINT now drops back through to main().
  lt_AscendOutOfContext(memcontext);
  if (chdir(settings_session_default.cwd) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into cwd failed."); } // chdir into temporary directory
  if (DirectiveLinebuffer != NULL) { free(DirectiveLinebuffer); DirectiveLinebuffer=NULL; }
  return status;
 }

int ProcessDirective2(char *in, Dict *command, int interactive, int memcontext, int IterLevel)
 {
  char  *directive, *varname, *varstrval;
  value *varnumval;
  glob_t GlobData;
  char   buffer[LSTR_LENGTH]="";
  int    i;

  if (DEBUG) { sprintf(temp_err_string, "Received command:\n%s", in); ppl_log(temp_err_string); }

  if (IterLevel > 100) { ppl_error("Maximum recursion depth exceeded."); return 1; }

  DictLookup(command,"directive",NULL,(void **)(&directive));

  if      (strcmp(directive, "pling")==0)
   {
    DictLookup(command,"cmd",NULL,(void **)(&directive));
    system(directive);
   }
  else if (strcmp(directive, "var_set")==0)
   {
    DictLookup(command,"varname"     ,NULL,(void **)(&varname));
    DictLookup(command,"string_value",NULL,(void **)(&varstrval));
    if (varstrval != NULL) { ppl_UserSpace_SetVarStr(varname, varstrval, 1); return 0; }
    DictLookup(command,"numeric_value",NULL,(void **)(&varnumval));
    if (varnumval != NULL) { ppl_UserSpace_SetVarNumeric(varname, varnumval, 1); return 0; }
    ppl_UserSpace_UnsetVar(varname);
   }
  else if (strcmp(directive, "func_set")==0)
   {
    i=-1;
    ppl_UserSpace_SetFunc(in, 1, &i, buffer);
    if (i >= 0) ppl_error(buffer);
   }
  else if (strcmp(directive, "var_set_regex")==0)
   return directive_regex(command);
  else if (strcmp(directive, "arrow")==0)
   directive_arrow(command, interactive);
  else if (strcmp(directive, "break")==0)
   {
    if (PPL_FLOWCTRL_BREAKABLE) PPL_FLOWCTRL_BROKEN=1;
    else                        ppl_error("The break statement can only be placed inside a loop structure.");
   }
  else if (strcmp(directive, "cd")==0)
   directive_cd(command);
  else if (strcmp(directive, "clear")==0)
   { directive_clear(); SendCommandToCSP("A"); }
  else if (strcmp(directive, "continue")==0)
   {
    if (PPL_FLOWCTRL_BREAKABLE) PPL_FLOWCTRL_CONTINUED=1;
    else                        ppl_error("The continue statement can only be placed inside a loop structure.");
   }
  else if (strcmp(directive, "do")==0)
   return directive_do(command, IterLevel+1);
  else if (strcmp(directive, "for")==0)
   return directive_for(command, IterLevel+1);
  else if (strcmp(directive, "foreach")==0)
   return directive_foreach(command, IterLevel+1);
  else if (strcmp(directive, "else")==0)
   ppl_error("This else statement does not match any earlier if statement.");
  else if (strcmp(directive, "exec")==0)
   return directive_exec(command, IterLevel+1);
  else if (strcmp(directive, "help")==0)
   directive_help(command, interactive);
  else if (strcmp(directive, "history")==0)
   directive_history(command);
  else if (strcmp(directive, "if")==0)
   return directive_if(command, IterLevel+1);
  else if (strcmp(directive, "jpeg")==0)
   directive_jpeg(command, interactive);
  else if (strcmp(directive, "linear")==0)
   return directive_interpolate(command,INTERP_LINEAR);
  else if (strcmp(directive, "load")==0)
   {
    DictLookup(command,"filename",NULL,(void **)(&varstrval));
    if (glob(varstrval, 0, NULL, &GlobData) != 0) { ppl_error("Could not glob this filename."); return 1; }
    lt_AscendOutOfContext(memcontext); command = NULL;
    for (i=0; i<GlobData.gl_pathc; i++) ProcessPyXPlotScript(GlobData.gl_pathv[i], IterLevel+1);
    globfree(&GlobData);
    return 0;
   }
  else if (strcmp(directive, "loglinear")==0)
   return directive_interpolate(command,INTERP_LOGLIN);
  else if (strcmp(directive, "maximise")==0)
   directive_maximise(command);
  else if (strcmp(directive, "minimise")==0)
   directive_minimise(command);
  else if (strcmp(directive, "polynomial")==0)
   return directive_interpolate(command,INTERP_POLYN);
  else if (strcmp(directive, "print")==0)
   directive_print(command);
  else if (strcmp(directive, "pwd")==0)
   ppl_report(settings_session_default.cwd);
  else if (strcmp(directive, "quit")==0)
   PPL_SHELL_EXITING = 1;
  else if (strcmp(directive, "reset")==0)
   {
    settings_term_current  = settings_term_default;
    settings_graph_current = settings_graph_default;
    for (i=0; i<PALETTE_LENGTH; i++) settings_palette_current[i] = settings_palette_default[i];
    directive_clear();
    SendCommandToCSP("A");
   }
  else if (strcmp(directive, "set")==0)
   directive_set(command);
  else if (strcmp(directive, "set_error")==0)
   directive_seterror(command, interactive);
  else if (strcmp(directive, "show")==0)
   directive_show(command, interactive);
  else if (strcmp(directive, "solve")==0)
   directive_solve(command);
  else if (strcmp(directive, "spline")==0)
   return directive_interpolate(command,INTERP_SPLINE);
  else if (strcmp(directive, "text")==0)
   directive_text(command, interactive);
  else if (strcmp(directive, "unset")==0)
   directive_set(command);
  else if (strcmp(directive, "unset_error")==0)
   directive_unseterror(command, interactive);
  else if (strcmp(directive, "while")==0)
   {
    DictLookup(command,"close_brace",NULL,(void **)(&varstrval));
    if (varstrval == NULL) return directive_while(command, IterLevel+1);
    else                   ppl_error("This while statement does not match any earlier do statement.");
   }
  else if (strcmp(directive, "unrecognised")==0)
   {
    sprintf(temp_err_string, txt_invalid, in);
    ppl_error(temp_err_string);
    return 1;
   }
  else
   ppl_report(DictPrint(command, buffer, LSTR_LENGTH));

  //SendCommandToCSP("2/home/dcf21/pyxplot/pyxplot/doc/figures/pyx_colours.eps\n");

  //if (chdir(settings_session_default.tempdir < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into temporary directory failed."); } // chdir into temporary directory

  //command = DictInit();
  //DictAppendString(command, "topic", 0, "c set");
  //directive_help(command,interactive);

  //directive_show2("settings",interactive);

  return 0;
 }

void directive_cd(Dict *command)
 {
  List         *DirList;
  Dict         *DirNameDict;
  char         *DirName;
  ListIterator *CDIterate;

  DictLookup(command,"path",NULL,(void **)(&DirList));
  CDIterate = ListIterateInit(DirList);
  while (CDIterate != NULL)
   {
    CDIterate = ListIterate(CDIterate , (void **)&DirNameDict);
    DictLookup(DirNameDict,"directory",NULL,(void **)&DirName);
    if (chdir(DirName) < 0)
     {
      sprintf(temp_err_string, "Could not change into directory '%s'.", DirName);
      ppl_error(temp_err_string);
      break;
     }
   }
  if (getcwd( settings_session_default.cwd , FNAME_LENGTH ) == NULL) { ppl_fatal(__FILE__,__LINE__,"Could not read current working directory."); } // Store cwd
  return;
 }

int directive_exec(Dict *command, int IterLevel)
 {
  int   status=0;
  char *strval, *line_ptr;
  int   i=0;

  DictLookup(command,"command",NULL,(void **)(&strval));
  SetInputSourceString(strval, &i);
  ClearInputSource();
  PPL_SHELL_EXITING = 0;
  while ((PPL_SHELL_EXITING == 0) && (PPL_FLOWCTRL_BROKEN == 0) && (PPL_FLOWCTRL_CONTINUED == 0))
   {
    SetInputSourceString(strval, &i);
    line_ptr = FetchInputStatement("","");
    if (line_ptr == NULL) break;
    if (StrStrip(line_ptr,line_ptr)[0] != '\0')
     {
      status = ProcessDirective(line_ptr, 0, IterLevel);
      ppl_error_setstreaminfo(-1, "");
      if (status>0) break; // If an error occurs, aborted processing exec statements
     }
   }
  PPL_SHELL_EXITING = 0;
  ClearInputSource();
  return status;
 }

void directive_history(Dict *command)
 {
  int start=0,endpos,k,*Nlines;
  HIST_ENTRY **history_data;

  endpos       = where_history();
  history_data = history_list();

  DictLookup(command,"number_lines",NULL,(void **)&Nlines);
  if (Nlines != NULL) start = endpos - *Nlines;
  if (start < 0) start=0;

  for (k=start; k<endpos; k++) ppl_report(history_data[k]->line);
  return;
 }

void directive_print(Dict *command)
 {
  List         *ItemList;
  Dict         *ItemSubDict;
  char         *item_str;
  value        *item_val;
  ListIterator *ItemIterate;
  char          PrintString[LSTR_LENGTH] = "\0";
  int           i=0;

  DictLookup(command,"print_list,",NULL,(void **)(&ItemList));
  if (ItemList != NULL) ItemIterate = ListIterateInit(ItemList);
  else                  ItemIterate = NULL;
  while (ItemIterate != NULL)
   {
    ItemIterate = ListIterate(ItemIterate , (void **)&ItemSubDict);
    DictLookup(ItemSubDict,"string",NULL,(void **)&item_str);
    if (item_str != NULL) { strcpy(PrintString+i, item_str); i+=strlen(PrintString+i); }
    DictLookup(ItemSubDict,"expression",NULL,(void **)&item_val);
    if (item_val != NULL) { strcpy(PrintString+i, ppl_units_NumericDisplay(item_val, 0, 0)); i+=strlen(PrintString+i); }
   }
  ppl_report(PrintString);
  return;
 }

int directive_regex(Dict *command)
 {
  char  *varname;
  char   cmd[LSTR_LENGTH];
  value *varnumval;
  int    i, j, fstdin, fstdout, pid, status;
  char   ci;
  struct timespec waitperiod; // A time.h timespec specifier for a wait of zero seconds
  fd_set          readable;

  // Extract the name of the variable we're going to perform regular expression upon
  DictLookup(command, "varname", NULL, (void **)(&varname));
  DictLookup(_ppl_UserSpace_Vars, varname, NULL, (void **)(&varnumval));
  if (varnumval == NULL)
   {
    sprintf(temp_err_string, "No such variable as '%s'.", varname);
    ppl_error(temp_err_string);
    return 1;
   }
  if (varnumval->string == NULL) // ... which must be a string variable
   {
    sprintf(temp_err_string, "Variable '%s' is not a string variable; regular expressions cannot be applied to it.", varname); 
    ppl_error(temp_err_string);
    return 1;
   }

  // Copy the regular expression we're going to use into cmd, adding s onto the front
  DictLookup(command, "regex", NULL, (void **)(&varname));
  strcpy(cmd, "s"); i=strlen(cmd);
  for (j=0; (ci=varname[j])!='\0'; j++) cmd[i++] = ci;
  cmd[i++] = '\0';

  // Fork a child sed process with the regular expression on the command line, and send variable contents to it
  ForkSed(cmd, &pid, &fstdin, &fstdout);
  if (write(fstdin, varnumval->string, strlen(varnumval->string)) != strlen(varnumval->string)) ppl_fatal(__FILE__,__LINE__,"Could not write to pipe to sed");
  close(fstdin);

  // Wait for sed process's stdout to become readable. Get bored if this takes more than a second.
  waitperiod.tv_sec  = 1; waitperiod.tv_nsec = 0;
  FD_ZERO(&readable); FD_SET(fstdout, &readable);
  pselect(fstdout+1, &readable, NULL, NULL, &waitperiod, NULL);
  if (!FD_ISSET(fstdout , &readable)) { ppl_error("Error: Got bored waiting for sed to return data."); return 0; }

  // Read data back from sed process
  if ((i = read(fstdout, cmd, LSTR_LENGTH)) < 0) { ppl_error("Error: Could not read from pipe to sed."); return 0; }
  cmd[i] = '\0';
  close(fstdout);

  // Get exit status back from sed process and check that it is zero
  if (waitpid(pid, &status, 0) <= 0) { ppl_error("Error: Could not read exit status from sed."); return 0; }
  if ((!WIFEXITED(status)) || (WEXITSTATUS(status)!=0)) { return 0; } // Error message will get sent to stderr, we assume

  // Copy string into string variable
  varnumval->string = (char *)lt_malloc_incontext(strlen(cmd)+1, _ppl_UserSpace_Vars->memory_context);
  strcpy(varnumval->string, cmd);
  return 0;
 }

