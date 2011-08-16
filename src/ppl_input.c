// ppl_input.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2011 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2011 Ross Church
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
#include <string.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ppl_error.h"
#include "ppl_flowctrl.h"
#include "ppl_userspace.h"
#include "pyxplot.h"

#define INPUT_PIPE       101
#define INPUT_READLINE   102
#define INPUT_FILE       103
#define INPUT_LOOPCHAIN  104
#define INPUT_STRING     105

static FILE      *infile = NULL;
static cmd_chain *inchain = NULL;
static int        mode = -1;
static int       *linenumber = NULL;
static char      *instr = NULL;
static int       *inpos = NULL;
static char      *filename_description = NULL;

void SetInputSourcePipe(int *linenumber_, char *filename_description_)
 {
  infile               = stdin;
  inchain              = NULL;
  mode                 = INPUT_PIPE;
  linenumber           = linenumber_;
  instr                = NULL;
  inpos                = NULL;
  filename_description = filename_description_;
  return;
 }

void SetInputSourceReadline(int *linenumber_)
 {
  infile               = NULL;
  inchain              = NULL;
  mode                 = INPUT_READLINE;
  linenumber           = linenumber_;
  instr                = NULL;
  inpos                = NULL;
  filename_description = NULL;
  return;
 }

void SetInputSourceFile(FILE *infile_, int *linenumber_, char *filename_description_)
 {
  infile               = infile_;
  inchain              = NULL;
  mode                 = INPUT_FILE;
  linenumber           = linenumber_;
  instr                = NULL;
  inpos                = NULL;
  filename_description = filename_description_;
  return;
 }

void SetInputSourceLoop(cmd_chain *inchain_)
 {
  infile               = NULL;
  inchain              = inchain_;
  mode                 = INPUT_LOOPCHAIN;
  linenumber           = NULL;
  instr                = NULL;
  inpos                = NULL;
  filename_description = NULL;
  return;
 }

void SetInputSourceString(char *instr_, int *inpos_)
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
  *descr  = filename_description;
  return;
 }

char *FetchInputLine(char *output, char *prompt)
 {
  int i;

  if (mode == INPUT_READLINE)
   {
    ppl_error_setstreaminfo(-1, "");
#ifdef HAVE_READLINE
    {
     char *line_ptr;
     line_ptr = readline(prompt);
     if (line_ptr==NULL) return NULL;
     add_history(line_ptr);
     history_NLinesWritten++;
     strcpy(output, line_ptr);
     free(line_ptr);
    }
#else
    printf("%s",prompt);
    if (fgets(output,LSTR_LENGTH,stdin)==NULL) return NULL;
#endif
    return output;
   }
  else if ((mode == INPUT_PIPE) || (mode == INPUT_FILE))
   {
    ppl_error_setstreaminfo(*linenumber, filename_description);
    if ((feof(infile)) || (ferror(infile))) return NULL;
    file_readline(infile, output, LSTR_LENGTH);
    (*linenumber)++;
    return output;
   }
  else if (mode == INPUT_LOOPCHAIN)
   {
    if ((inchain == NULL) || (*inchain == NULL)) return NULL;
    if ((*inchain)->description != NULL) ppl_error_setstreaminfo((*inchain)->linenumber , (*inchain)->description);
    else                                 ppl_error_setstreaminfo(-1                     , ""                     );
    strcpy(output, (*inchain)->line);
    *inchain = (*inchain)->next;
    return output;
   }
  else if (mode == INPUT_STRING)
   {
    if (instr[*inpos]=='\0') return NULL;
    for (i=0; ((instr[*inpos]!='\0')&&(instr[*inpos]!='\n')&&(instr[*inpos]!=EOF)); i++,(*inpos)++) output[i]=instr[*inpos];
    output[i++]='\0';
    while ((instr[*inpos]=='\n')||(instr[*inpos]==EOF)) (*inpos)++;
    return output;
   }
  else
   ppl_fatal(__FILE__,__LINE__,"Illegal setting for input mode.");
  return NULL;
 }

// Higher level routines for fetching input with ; and \ characters already used to split/join lines

char *InputLineBuffer    = NULL;
char *InputLineAddBuffer = NULL;
char *InputLineBufferPos = NULL;

void ClearInputSource(char *New, char *NewPos, char *NewAdd, char **Old, char **OldPos, char **OldAdd)
 {
  if (Old    != NULL) { *Old    = InputLineBuffer;    } else if (InputLineBuffer    != NULL) { free(InputLineBuffer); }
  if (OldPos != NULL) { *OldPos = InputLineBufferPos; }
  if (OldAdd != NULL) { *OldAdd = InputLineAddBuffer; } else if (InputLineAddBuffer != NULL) { free(InputLineAddBuffer); }
  InputLineBuffer    = New;
  InputLineBufferPos = NewPos;
  InputLineAddBuffer = NewAdd;
  if (InputLineBuffer    == NULL) InputLineBuffer = (char *)malloc(LSTR_LENGTH);
  if (InputLineBuffer    == NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory error whilst trying to allocate input buffer."); exit(1); }
  return;
 }

char *FetchInputStatement(char *prompt1, char *prompt2, int MacroSubst)
 {
  int   i, j;
  char *line = NULL;
  char  QuoteChar = '\0';
  static char b[LSTR_LENGTH];

  if (InputLineBufferPos==NULL)
   {
    if (InputLineAddBuffer != NULL) { free(InputLineAddBuffer); InputLineAddBuffer=NULL; }

    // Join together lines that end in backslashes
    while (1)
     {
      if (InputLineAddBuffer == NULL) line = FetchInputLine(InputLineBuffer, prompt1);
      else                            line = FetchInputLine(InputLineBuffer, prompt2);
      if (line == NULL) { i=0; InputLineBuffer[0]='\0'; break; }
      for (i=0; InputLineBuffer[i]!='\0'; i++); for (; ((i>0)&&(InputLineBuffer[i]<=' ')); i--);
      if (InputLineBuffer[i]!='\\') break;
       {
        if (InputLineAddBuffer==NULL)
         {
          InputLineAddBuffer = (char *)malloc(i+1);
          if (InputLineAddBuffer == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst trying to combine input lines."); return NULL; }
          strncpy(InputLineAddBuffer, InputLineBuffer, i);
          InputLineAddBuffer[i]='\0';
         } else {
          j = strlen(InputLineAddBuffer);
          InputLineAddBuffer = (char *)realloc((void *)InputLineAddBuffer, j+i+1);
          if (InputLineAddBuffer == NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst trying to combine input lines."); return NULL; }
          strncpy(InputLineAddBuffer+j, InputLineBuffer, i);
          InputLineAddBuffer[j+i]='\0';
         }
       }
     }

    // Add previous backslashed lines to the beginning of this one
    if (InputLineAddBuffer!=NULL)
     {
      j = strlen(InputLineAddBuffer);
      InputLineAddBuffer = (char *)realloc((void *)InputLineAddBuffer, j+i+2);
      strncpy(InputLineAddBuffer+j, InputLineBuffer, i+1);
      InputLineAddBuffer[j+i+1]='\0';
      InputLineBufferPos = InputLineAddBuffer;
     }
    else
     {
      if (line == NULL) return NULL;
      InputLineBufferPos = InputLineBuffer;
     }
   }

#define LOOP_OVER_LINE \
  for (i=0; line[i]!='\0'; i++) \
   { \
    if      ((QuoteChar=='\0') && (line[i]=='\'')                     ) QuoteChar = '\''; \
    else if ((QuoteChar=='\0') && (line[i]=='\"')                     ) QuoteChar = '\"'; \
    else if ((QuoteChar=='\'') && (line[i]=='\'') && (line[i-1]!='\\')) QuoteChar = '\0'; \
    else if ((QuoteChar=='\"') && (line[i]=='\"') && (line[i-1]!='\\')) QuoteChar = '\0';

#define LOOP_END }

  line = InputLineBufferPos;

  // Cut comments off the ends of lines
LOOP_OVER_LINE
    else if ((QuoteChar=='\0') && (line[i]=='`')                      ) QuoteChar = '`';
    else if ((QuoteChar=='`' ) && (line[i]=='`' ) && (line[i-1]!='\\')) QuoteChar = '\0';
    else if ((QuoteChar=='\0') && (line[i]=='#' )                     ) break;
LOOP_END
  line[i] = '\0';

  // Do ` ` substitution
  if (MacroSubst)
   {
LOOP_OVER_LINE
    else if ((QuoteChar=='\0') && (line[i]=='`'))
     {
      int is=++i, l=0, status;
      FILE *SubstPipe;
      for ( ; ((line[i]!='\0')&&(line[i]!='`')) ; i++);
      if (line[i]!='`') { ppl_error(ERR_SYNTAX, -1, -1, "Mismatched `"); InputLineBufferPos = NULL; return NULL; }
      line[i]='\0';
      if (DEBUG) { sprintf(temp_err_string, "Shell substitution with command '%s'.", line+is); ppl_log(temp_err_string); }
      if ((SubstPipe = popen(line+is,"r"))==NULL)
       {
        sprintf(temp_err_string, "Could not spawl shell substitution command '%s'.", line+is); ppl_error(ERR_GENERAL, -1, -1, temp_err_string);
        InputLineBufferPos = NULL; return NULL;
       }
      while ((!feof(SubstPipe)) && (!ferror(SubstPipe)))
       {
        if (l >= LSTR_LENGTH) { sprintf(temp_err_string, "Input line length overflow during ` ` substitution."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); InputLineBufferPos = NULL; return NULL; }
        if (fscanf(SubstPipe,"%c",b+l) == EOF) break;
        if (b[l]=='\n') b[l] = ' ';
        if (b[l]!='\0') l++;
       }
      status = pclose(SubstPipe);
      if (status != 0) { sprintf(temp_err_string, "Command failure during ` ` substitution."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); InputLineBufferPos = NULL; return NULL; }
      for (i++ ; line[i]!='\0' ; )
       {
        b[l++] = line[i++];
        if (l >= LSTR_LENGTH-is) { sprintf(temp_err_string, "Input line length overflow during ` ` substitution."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); InputLineBufferPos = NULL; return NULL; }
       }
      b[l]='\0';
      for (i=is-1, l=0 ; b[l]!='\0'; ) line[i++] = b[l++];
      line[i]='\0';
      i = is-1;
     }
LOOP_END
   }

  // Substitute for macros
  if (MacroSubst)
   {
LOOP_OVER_LINE
    else if ((QuoteChar=='\0') && (line[i]=='@'))
     {
      char end;
      int is=++i,l=0;
      value *VarData;
      for ( ; (isalnum(line[i])) ; i++);
      end=line[i] ; line[i]='\0';
      DictLookup(_ppl_UserSpace_Vars, line+is, NULL, (void *)&VarData);
      if (VarData == NULL) { sprintf(temp_err_string, "Undefined macro, \"%s\".", line+is); ppl_warning(ERR_SYNTAX, temp_err_string); line[i]=end; i=is; continue; }
      if (VarData->string == NULL) { sprintf(temp_err_string, "Attempt to expand a macro, \"%s\", which is a numerical variable not a string.", line+is); ppl_warning(ERR_SYNTAX, temp_err_string); line[i]=end; i=is; continue; }
      line[i]=end;
      if ((l=strlen(VarData->string)) > LSTR_LENGTH-strlen(line)) { sprintf(temp_err_string, "Input line length overflow during macro substitution."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); InputLineBufferPos = NULL; return NULL; }
      strcpy(b, VarData->string);
      for ( ; line[i]!='\0' ; )
       {
        b[l++] = line[i++];
        if (l >= LSTR_LENGTH-is) { sprintf(temp_err_string, "Input line length overflow during ` ` substitution."); ppl_error(ERR_GENERAL, -1, -1, temp_err_string); InputLineBufferPos = NULL; return NULL; }
       }
      b[l]='\0';
      for (i=is-1, l=0 ; b[l]!='\0'; ) line[i++] = b[l++];
      line[i]='\0';
      i = is-1;
     }
LOOP_END
   }

  // Split line on semicolons
LOOP_OVER_LINE
    else if ((QuoteChar=='\0') && (line[i]=='`')                      ) QuoteChar = '`';
    else if ((QuoteChar=='`' ) && (line[i]=='`' ) && (line[i-1]!='\\')) QuoteChar = '\0';
    else if ((QuoteChar=='\0') && (line[i]==';' )                     )
     {
      line[i]='\0';
      if (line[i+1]=='\0') InputLineBufferPos = NULL;
      else                 InputLineBufferPos = line+i+1;
      return line; // Return semi-colon separated portion of line; save rest for later
     }
LOOP_END

  // Return final semi-colon separated portion of line
  InputLineBufferPos = NULL;
  return line;
 }

