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
#include <setjmp.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_memory.h"

#include "pyxplot.h"
#include "ppl_children.h"
#include "ppl_error.h"
#include "ppl_help.h"
#include "ppl_parser.h"
#include "ppl_passwd.h"
#include "ppl_setshow.h"
#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_userspace.h"

int PPL_SHELL_EXITING;

char *DirectiveLinebuffer  = NULL;
char *DirectiveLinebuffer2 = NULL;

void InteractiveSession()
 {
  int   memcontext;
  int   linenumber = 1;
  char *line_ptr;
  char  linebuffer[LSTR_LENGTH];

  memcontext = lt_DescendIntoNewContext();
  PPL_SHELL_EXITING = 0;
  if (DirectiveLinebuffer  != NULL) { free(DirectiveLinebuffer ); DirectiveLinebuffer =NULL; }
  if (DirectiveLinebuffer2 != NULL) { free(DirectiveLinebuffer2); DirectiveLinebuffer2=NULL; }
  ppl_log("Starting an interactive session.");

  // Set up SIGINT handler
  if (sigsetjmp(sigjmp_ToInteractive, 1) == 0)
   {
    sigjmp_FromSigInt = &sigjmp_ToInteractive;

    if ((isatty(STDIN_FILENO) == 1) && (settings_session_default.splash == SW_ONOFF_ON)) ppl_report(txt_init);

    while (PPL_SHELL_EXITING == 0)
     {
      CheckForGvOutput();
      if (isatty(STDIN_FILENO) == 1)
       {
        if (DirectiveLinebuffer == NULL) line_ptr = readline("pyxplot> ");
        else                             line_ptr = readline(".......> ");
        if (line_ptr == NULL) { PPL_SHELL_EXITING = 1; continue; }
        add_history(line_ptr);
        strcpy(linebuffer, line_ptr) ; free(line_ptr);
        ProcessDirective(linebuffer, 1);
       } else {
        ppl_error_setstreaminfo(linenumber, "piped input");
        file_readline(stdin, linebuffer);
        ProcessDirective(linebuffer, 0);
        ppl_error_setstreaminfo(0, "");
        linenumber++;
        if (feof(stdin) || ferror(stdin)) PPL_SHELL_EXITING = 1;
       }
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
  if (DirectiveLinebuffer  != NULL) { free(DirectiveLinebuffer ); DirectiveLinebuffer =NULL; }
  if (DirectiveLinebuffer2 != NULL) { free(DirectiveLinebuffer2); DirectiveLinebuffer2=NULL; }
  lt_AscendOutOfContext(memcontext);
  sigjmp_FromSigInt = &sigjmp_ToMain; // SIGINT now drops back through to main().
  return;
 }

void ProcessPyXPlotScript(char *input)
 {
  int  memcontext;
  int  linenumber = 1;
  int  status;
  int  ProcessedALine = 0;
  char full_filename[FNAME_LENGTH];
  char filename_description[FNAME_LENGTH];
  char linebuffer[LSTR_LENGTH];
  FILE *infile;

  memcontext = lt_DescendIntoNewContext();
  PPL_SHELL_EXITING = 0;
  if (DirectiveLinebuffer  != NULL) { free(DirectiveLinebuffer ); DirectiveLinebuffer =NULL; }
  if (DirectiveLinebuffer2 != NULL) { free(DirectiveLinebuffer2); DirectiveLinebuffer2=NULL; }
  sprintf(temp_err_string, "Processing input from the script file '%s'.", input); ppl_log(temp_err_string);
  UnixExpandUserHomeDir(input , settings_session_default.cwd, full_filename);
  sprintf(filename_description, "file '%s'", input);
  if ((infile=fopen(full_filename,"r")) == NULL)
   {
    sprintf(temp_err_string, "PyXPlot Error: Could not find command file '%s'\nSkipping on to next command file", full_filename); ppl_error(temp_err_string);
    lt_AscendOutOfContext(memcontext);
    return;
   }

  while ((PPL_SHELL_EXITING == 0) && (!feof(infile)) && (!ferror(infile)))
   {
    file_readline(infile, linebuffer);
    if (StrStrip(linebuffer,linebuffer)[0] != '\0')
     {
      ppl_error_setstreaminfo(linenumber, filename_description); 
      status = ProcessDirective(linebuffer, 0);
      ppl_error_setstreaminfo(0, "");
      if ((ProcessedALine==0) && (status!=0)) // If an error occurs on the first line of a script, aborted processing it
       {
        ppl_error("Error on first line of commandfile: Is this is valid script?\nAborting");
        break;
       }
      ProcessedALine = 1;
     }
    linenumber++;
   }
  if (DirectiveLinebuffer  != NULL) { free(DirectiveLinebuffer ); DirectiveLinebuffer =NULL; }
  if (DirectiveLinebuffer2 != NULL) { free(DirectiveLinebuffer2); DirectiveLinebuffer2=NULL; }
  fclose(infile);
  CheckForGvOutput();
  lt_AscendOutOfContext(memcontext);
  return;
 }

int ProcessDirective(char *in, int interactive)
 {
  int   i, j, status;
  char *line = NULL;
  char  QuoteChar = '\0';

  // Join together lines that end in backslashes
  for (i=0; in[i]!='\0'; i++); for (; ((i>0)&&(in[i]<=' ')); i--);
  if (in[i]=='\\')
   {
    if (DirectiveLinebuffer==NULL)
     {
      DirectiveLinebuffer = (char *)malloc((i+2)*sizeof(char));
      strncpy(DirectiveLinebuffer, in, i);
      DirectiveLinebuffer[i+1]='\0';
     } else {
      j = strlen(DirectiveLinebuffer);
      DirectiveLinebuffer = (char *)realloc((void *)DirectiveLinebuffer, (j+i+2)*sizeof(char));
      strncpy(DirectiveLinebuffer+j, in, i);
      DirectiveLinebuffer[j+i+1]='\0';
     }
    return 0;
   }

  // Add previous backslashed lines to the beginning of this one
  if (DirectiveLinebuffer!=NULL)
   {
    j = strlen(DirectiveLinebuffer);
    line = (char *)realloc((void *)DirectiveLinebuffer, (j+i+2)*sizeof(char));
    DirectiveLinebuffer = NULL;
    strncpy(line+j, in, i+1);
    line[j+i+1]='\0';
    in = line;
   }

  // Cut comments off the ends of lines and split it on semicolons
  for (i=0, j=0; in[i]!='\0'; i++)
   {
    if      ((QuoteChar=='\0') && (in[i]=='\'')                   ) QuoteChar = '\'';
    else if ((QuoteChar=='\0') && (in[i]=='\"')                   ) QuoteChar = '\"';
    else if ((QuoteChar=='\'') && (in[i]=='\'') && (in[i-1]!='\\')) QuoteChar = '\0';
    else if ((QuoteChar=='\"') && (in[i]=='\"') && (in[i-1]!='\\')) QuoteChar = '\0';
    else if ((QuoteChar=='\0') && (in[i]==';' )                   )
     {
      in[i]='\0';
      status = ProcessDirective2(in+j,interactive);
      if (status!=0) { if (line != NULL) free(line); return status; }
      j=i+1;
     }
    else if ((QuoteChar=='\0') && (in[i]=='#' )                   ) break;
   }
  in[i] = '\0';
  status = ProcessDirective2(in+j,interactive);
  if (line != NULL) free(line);
  return status;
 }

int ProcessDirective2(char *in, int interactive)
 {
  int   memcontext, i, is, j, l;
  int   status=0;
  char  QuoteChar='\0';
  Dict *command;
  FILE *SubstPipe;

  // If this line is blank, ignore it
  for (i=0; in[i]!='\0'; i++); for (; ((i>0)&&(in[i]<=' ')); i--);
  if ((i==0) && (in[i]<=' ')) return 0;

  memcontext = lt_DescendIntoNewContext();
  if ((interactive==0) || (sigsetjmp(sigjmp_ToDirective, 1) == 0))  // Set up SIGINT handler, but only if this is an interactive session
   {
    if (interactive!=0) sigjmp_FromSigInt = &sigjmp_ToDirective;

    // Do `` substitution
    l = strlen(in+1);
    DirectiveLinebuffer2 = (char *)malloc(l*sizeof(char));
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
          if (l <= j) DirectiveLinebuffer2 = (char *)realloc((void *)DirectiveLinebuffer2, l=l+1024);
          fscanf(SubstPipe,"%c",DirectiveLinebuffer2 + j);
          if (DirectiveLinebuffer2[j] == '\n') DirectiveLinebuffer2[j] = ' ';
          if (DirectiveLinebuffer2[j] != '\0') j++;
         }
        status = pclose(SubstPipe);
        if (status != 0) break;
       }
      else
       {
        if (l <= j) DirectiveLinebuffer2 = (char *)realloc((void *)DirectiveLinebuffer2, l=l+1024);
        DirectiveLinebuffer2[j++] = in[i];
       }
     }
    if (l <= j) DirectiveLinebuffer2 = (char *)realloc((void *)DirectiveLinebuffer2, l=l+1);
    DirectiveLinebuffer2[j] = '\0';

    // Parse and execute command
    if (status==0)
     {
      command = parse(DirectiveLinebuffer2);
      if (command != NULL) status  = ProcessDirective3(DirectiveLinebuffer2, command, interactive); // If command is NULL, we had a syntax error
     }
   } else {
    ppl_error("\nReceived CTRL-C. Terminating command."); // SIGINT longjmps return here
    status = 1;
   }
  sigjmp_FromSigInt = &sigjmp_ToMain; // SIGINT now drops back through to main().
  lt_AscendOutOfContext(memcontext);
  if (chdir(settings_session_default.cwd) < 0) { ppl_fatal(__FILE__,__LINE__,"chdir into cwd failed."); } // chdir into temporary directory
  if (DirectiveLinebuffer2 != NULL) { free(DirectiveLinebuffer2); DirectiveLinebuffer2=NULL; }
  return status;
 }

int ProcessDirective3(char *in, Dict *command, int interactive)
 {
  char *directive;
  char buffer[LSTR_LENGTH]="";
  if (DEBUG) { sprintf(temp_err_string, "Received command:\n%s", in); ppl_log(temp_err_string); }

  DictLookup(command,"directive",NULL,NULL,(void **)(&directive));

  if      (strcmp(directive, "pling")==0)
   {
    DictLookup(command,"cmd",NULL,NULL,(void **)(&directive));
    system(directive);
   }
  else if (strcmp(directive, "cd")==0)
   directive_cd(command);
  else if (strcmp(directive, "help")==0)
   directive_help(command, interactive);
  else if (strcmp(directive, "print")==0)
   directive_print(command);
  else if (strcmp(directive, "pwd")==0)
   ppl_report(settings_session_default.cwd);
  else if (strcmp(directive, "quit")==0)
   PPL_SHELL_EXITING = 1;
  else if (strcmp(directive, "show")==0)
   directive_show(command, interactive);
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

  DictLookup(command,"path",NULL,NULL,(void **)(&DirList));
  CDIterate = ListIterateInit(DirList);
  while (CDIterate != NULL)
   {
    CDIterate = ListIterate(CDIterate , (void **)&DirNameDict);
    DictLookup(DirNameDict,"directory",NULL,NULL,(void **)&DirName);
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

void directive_print(Dict *command)
 {
  List         *ItemList;
  Dict         *ItemSubDict;
  char         *item_str;
  double       *item_dbl;
  ListIterator *ItemIterate;
  char          PrintString[LSTR_LENGTH] = "\0";
  int           i=0;

  DictLookup(command,"print_list,",NULL,NULL,(void **)(&ItemList));
  if (ItemList != NULL) ItemIterate = ListIterateInit(ItemList);
  else                  ItemIterate = NULL;
  while (ItemIterate != NULL)
   {
    ItemIterate = ListIterate(ItemIterate , (void **)&ItemSubDict);
    DictLookup(ItemSubDict,"string",NULL,NULL,(void **)&item_str);
    if (item_str != NULL) { strcpy(PrintString+i, item_str); i+=strlen(PrintString+i); }
    DictLookup(ItemSubDict,"expression",NULL,NULL,(void **)&item_dbl);
    if (item_dbl != NULL) { strcpy(PrintString+i, NumericDisplay(*item_dbl, LSTR_LENGTH-i)); i+=strlen(PrintString+i); }
   }
  ppl_report(PrintString);
  return;
 }

