// ppl_parser.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#ifdef HAVE_READLINE
#include <readline/readline.h>
#endif

#include <gsl/gsl_math.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"
#include "ListTools/lt_memory.h"

#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_input.h"
#include "ppl_parser.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"
#include "pyxplot.h"

#define PN_TYPE_SEQ  21000
#define PN_TYPE_OPT  21001
#define PN_TYPE_REP  21002 // Must have at least one repeat item
#define PN_TYPE_REP2 21003 // Can have zero items
#define PN_TYPE_PER  21004
#define PN_TYPE_ORA  21005
#define PN_TYPE_ITEM 21006

typedef struct ParserNode {
  int   type;
  char *MatchString; // ITEMs only
  int   ACLevel;     // ITEMs only
  char *VarName;
  char *VarSetVal;
  struct ParserNode *FirstChild;
  struct ParserNode *NextSibling;
 } ParserNode;

void parse_descend(ParserNode *node, char *line, int IterLevel, int *linepos, int *start, int *number, char *expecting, int *ExpectingPos, int *ExpectingLinePos, char *AlgebraError, int *AlgebraLinepos, char *AlgebraNewError, int *AlgebraNewLinepos, Dict *output, int *match, int *success);

// --------------------------------------------------------------------------
// PART I: READ SYNTAX SPECIFICATION
//
// The syntaxes of PyXPlot commands are specified in gp_commands.
//
// We make a tree structure -- a hierarchy of ParserNodes
//
// type can be:
//   seq -- match each member of structure in turn.
//   opt -- entire structure is optional. always depends immediately into a new seq structure.
//   rep -- repeat structure as many times as it matches input. again, always contains a single seq structure.
//   per -- members of this structure may be found in any order, but each may only match once.
//   ora -- match any ONE member of this structure.
//  item -- A special case, with no children. MatchString should be matched to ACLevel.
//
// Only "item" and "rep" structures are allowed to return output variables "varname".
// Upon parsing a line of user input, a dictionary is returned, containing the values of all these variables.

// StartNewStructure(): This creates a child ParserNode within the present ParserNode.

// DefnStack is an array: item 0 is the top-level of the tree; item N is the current Nth leaf node that we are populating.

void StartNewStructure(ParserNode **DefnStack, int *i, int type)
 {
  ParserNode **target = NULL;
  ParserNode  *NewNode = (ParserNode *)lt_malloc(sizeof(ParserNode));

  if (NewNode == NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory whilst setting up PyXPlot's command line parser."); exit(1); }

  NewNode->type = type;
  NewNode->MatchString = NULL;
  NewNode->VarName     = NewNode->VarSetVal   = NULL;
  NewNode->FirstChild  = NewNode->NextSibling = NULL;
  NewNode->ACLevel = -1;

  if ((*i)>0)
   {
    if ((DefnStack[(*i)-1]->type != PN_TYPE_SEQ) && (type != PN_TYPE_SEQ)) StartNewStructure(DefnStack, i, PN_TYPE_SEQ); // This new structure is actually the first in a possible sequence
    target = &(DefnStack[(*i)-1]->FirstChild);
    while (*target != NULL) target = &((*target)->NextSibling);
    *target = NewNode; // Add new node into hierarchy
   }
  DefnStack[(*i)++] = NewNode; // ... and onto the stack describing our current position
 }

// RollBack(): The opposite of the above... implements the closing bracket.
// Finishes the present structure, which should be of type "type". Throws an error if it is not.
// Note that "seq" structures are automatically closed without need for closing grammar.

void RollBack(ParserNode **DefnStack, int *i, int type, char *PplCommandsText, int *NcharsTaken)
 {
  int j=1; // First character of PplCommandsText is a closing bracket
  int VarNameBegin=-1, VarNameEnd=-1;

  while (((*i)>0) && (DefnStack[(*i)-1]->type == PN_TYPE_SEQ)) (*i)--; // Automatically close SEQs; they don't have closing grammar

  if ((*i)<=0)
   { sprintf(temp_err_string, "Incorrect nesting of types in command specification -- attempt to close an unopened bracket."); ppl_fatal(__FILE__,__LINE__, temp_err_string); }
  else if (DefnStack[(*i)-1]->type != type)
   { sprintf(temp_err_string, "Incorrect nesting of types in command specification -- expected %d, but got %d.", type, DefnStack[(*i)-1]->type); ppl_fatal(__FILE__,__LINE__, temp_err_string); }

  if (PplCommandsText[j]==':') j++;
  if ((PplCommandsText[j]=='@') && (type==PN_TYPE_REP)) { type=PN_TYPE_REP2; DefnStack[(*i)-1]->type=type; j++; } // A REP2 item
  if (PplCommandsText[j]> ' ')
   {
    VarNameBegin = j;
    while (PplCommandsText[j]> ' ') j++; // FFW over variable name
    VarNameEnd   = j;
   }
  while ((PplCommandsText[j]!='\0') && (PplCommandsText[j]!='\n') && (PplCommandsText[j]<' ')) j++; // FFW over trailing whitespace

  if (VarNameBegin > -1)
   {
    if ((DefnStack[(*i)-1]->VarName = (char *)lt_malloc((VarNameEnd-VarNameBegin+1)*sizeof(char)))==NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory whilst setting up PyXPlot's command line parser."); exit(1); }
    strncpy( DefnStack[(*i)-1]->VarName , PplCommandsText+VarNameBegin , VarNameEnd-VarNameBegin );
    DefnStack[(*i)-1]->VarName[VarNameEnd - VarNameBegin] = '\0';
   }
  DefnStack[(*i)-1]->VarSetVal = "\0"; // ITEM structures can do "exit:directive:quit" to store "quit" into variable "directive", but []s never do that.

  (*i)--; // Close structure of type "type" and roll back stack.
  *NcharsTaken = j;
  return;
 }

// ppl_commands_read(): This runs on startup to populate a list,
// PplParserCmdList, with list-based definitions of PyXPlot's commands, parsed
// using the functions above from plp_commands

static List *PplParserCmdList[27];

void ppl_commands_read()
 {
  int         InputPos = 0;
  ParserNode *DefnStack[25];
  int         StackPos;
  int         i,j,k,l,m,n,N,cln;
  ParserNode *NewNode;
  ParserNode **target = NULL;

  for (i=0;i<27;i++) PplParserCmdList[i] = ListInit();
  while (ppl_commands[InputPos] != '\0')
   {
    while ((ppl_commands[InputPos] != '\0') && (ppl_commands[InputPos] <= ' ')) InputPos++; // Ignore whitespace
    if (ppl_commands[InputPos] == '\0') continue;
    StackPos = 0;
    if ((ppl_commands[InputPos]>='a')&&(ppl_commands[InputPos]<='z')) cln = (int)(ppl_commands[InputPos]-'a');
    else                                                              cln = 26; // Begins with punctuation
    StartNewStructure(DefnStack, &StackPos, PN_TYPE_SEQ);

    while (1)
     {
      while ((ppl_commands[InputPos] != '\0') && (ppl_commands[InputPos] != '\n') && (ppl_commands[InputPos] <= ' ')) InputPos++; // Ignore whitespace
      if ((ppl_commands[InputPos] == '\0') || (ppl_commands[InputPos] == '\n')) break; // Newline
      if      (ppl_commands[InputPos] == '{') // {} grammar indicates an optional series of items, which go into a new structure.
       { StartNewStructure(DefnStack, &StackPos, PN_TYPE_OPT); InputPos++; }
      else if (ppl_commands[InputPos] == '[') // [] grammar indicates a series of items which repeat 0 or more times. These go into a new structure.
       { StartNewStructure(DefnStack, &StackPos, PN_TYPE_REP); InputPos++; }
      else if (ppl_commands[InputPos] == '(') // () grammar indicates items which can appear in any order, but each one not more than once. New structure.
       { StartNewStructure(DefnStack, &StackPos, PN_TYPE_PER); InputPos++; }
      else if (ppl_commands[InputPos] == '<') // <> grammar indicates a list of items of which only one should be matched. New structure.
       { StartNewStructure(DefnStack, &StackPos, PN_TYPE_ORA); InputPos++; }
      else if (ppl_commands[InputPos] == '~') // ~ is used inside () to separate items
       {
        while ((StackPos>=0) && (DefnStack[StackPos-1]->type == PN_TYPE_SEQ)) StackPos--; // Automatically close SEQs; they don't have closing grammar
        if ((StackPos<0) || (DefnStack[StackPos-1]->type != PN_TYPE_PER)) ppl_fatal(__FILE__,__LINE__, "Tilda should be used only in permutation structures.");
        InputPos++;
       }
      else if (ppl_commands[InputPos] == '|') // | is used inside <> for either/or items
       {
        while ((StackPos>=0) && (DefnStack[StackPos-1]->type == PN_TYPE_SEQ)) StackPos--; // Automatically close SEQs; they don't have closing grammar
        if ((StackPos<0) || (DefnStack[StackPos-1]->type != PN_TYPE_ORA)) ppl_fatal(__FILE__,__LINE__, "Pipe alternatives should only be used inside ORA structures.");
        InputPos++;
       }
      else if (ppl_commands[InputPos] == '>') // Match closing brackets for the above types
       { RollBack(DefnStack, &StackPos, PN_TYPE_ORA, ppl_commands+InputPos, &j) ; InputPos+=j; }
      else if (ppl_commands[InputPos] == ')')
       { RollBack(DefnStack, &StackPos, PN_TYPE_PER, ppl_commands+InputPos, &j) ; InputPos+=j; }
      else if (ppl_commands[InputPos] == ']')
       { RollBack(DefnStack, &StackPos, PN_TYPE_REP, ppl_commands+InputPos, &j) ; InputPos+=j; }
      else if (ppl_commands[InputPos] == '}')
       { RollBack(DefnStack, &StackPos, PN_TYPE_OPT, ppl_commands+InputPos, &j) ; InputPos+=j; }
      else
       { // This word is a rare instance of a piece of RE++ syntax which is not grammar
        if (ppl_commands[InputPos] == '\\') InputPos++; // Escape character put at the beginnings of words are ignored; they allow them to begin with punctuation e.g. "["
        if (DefnStack[StackPos-1]->type != PN_TYPE_SEQ) StartNewStructure(DefnStack, &StackPos, PN_TYPE_SEQ); // Match words have to go in sequences, not, e.g. "ora" structures
        // i     j N  k   l  m   n
        // foobar @3: plob : bolp
        k=l=m=n=N=-1; // ACL of -1 means no ACL specified
        i=InputPos;
        while ((ppl_commands[InputPos] > ' ') && (ppl_commands[InputPos] !='@') && ((ppl_commands[InputPos] !=':')||(i==InputPos))) InputPos++;
        j=InputPos;
        if (j==i) ppl_fatal(__FILE__,__LINE__,"ppl_parser found a word of zero length. This should never happen.");
        if (ppl_commands[InputPos] == '@')
         {
          InputPos++;
          if (ppl_commands[InputPos] == 'n') { N=-2 ; k=1; } // ACL of -2 means @n mode
          else N = (int)GetFloat(ppl_commands+InputPos , &k);
          InputPos+=k; k=-1;
         }
        if (ppl_commands[InputPos] == ':')
         {
          k = ++InputPos;
          while ((ppl_commands[InputPos] > ' ') && (ppl_commands[InputPos] != ':')) InputPos++;
          l = InputPos;
         }
        if (ppl_commands[InputPos] == ':')
         {
          m = ++InputPos;
          while ((ppl_commands[InputPos] > ' ') && (ppl_commands[InputPos] != ':')) InputPos++;
          n = InputPos;
         }
        if ((NewNode = (ParserNode *)lt_malloc(sizeof(ParserNode)))==NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory whilst setting up PyXPlot's command line parser."); exit(1); }
        NewNode->type    = PN_TYPE_ITEM;
        NewNode->ACLevel = N;

        if ((NewNode->MatchString = (char *)lt_malloc((j-i+1)*sizeof(char)))==NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory whilst setting up PyXPlot's command line parser."); exit(1); }
        strncpy(NewNode->MatchString, ppl_commands + i, j-i);
        NewNode->MatchString[j-i]='\0';
        if (k>0)
         {
           if ((NewNode->VarName = (char *)lt_malloc((l-k+1)*sizeof(char)))==NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory whilst setting up PyXPlot's command line parser."); exit(1); }
           strncpy(NewNode->VarName, ppl_commands + k, l-k);
           NewNode->VarName[l-k]='\0';
          }
        else
         { NewNode->VarName = NULL; }
        if (m>0)
         {
          if ((NewNode->VarSetVal = (char *)lt_malloc((n-m+1)*sizeof(char)))==NULL) { ppl_fatal(__FILE__,__LINE__,"Out of memory whilst setting up PyXPlot's command line parser."); exit(1); }
          strncpy(NewNode->VarSetVal, ppl_commands + m, n-m);
          NewNode->VarSetVal[n-m]='\0';
         }
        else
         { NewNode->VarSetVal = NULL; }
        NewNode->FirstChild = NewNode->NextSibling = NULL;
        target = &(DefnStack[StackPos-1]->FirstChild);
        while (*target != NULL) target = &((*target)->NextSibling);
        *target = NewNode; // Add new node into hierarchy
       }
     }
    ListAppendPtr(PplParserCmdList[cln], DefnStack[0], sizeof(ParserNode), 0, DATATYPE_VOID);
   }
  return;
 }

// --------------------------------------------------------------------------
// PART II: PARSE A LINE OF USER INPUT

// PARSE(): Top-level interface. Parses a commandline "line" from the user.
// It expects that ; and `` have already been dealt with by pyxplot.py

static int bn=0, bnd=0;

void ppl_parser_bnreset()
 {
  bn=0;
  bnd=0;
  return;
 }

Dict *parse(char *line, int IterLevel)
 {
  ListIterator *CmdIterator;
  ParserNode   *CmdDescriptor;
  Dict         *output;
  int           match, success, AlgebraLinepos, AlgebraNewLinepos, linepos, ExpectingPos, ExpectingLinePos, ErrPos, i, cln;
  int           OutContext, TempContext=0, OwnContext=0, bn_old;
  char         *expecting, *ErrText, *AlgebraError, *AlgebraNewError;

  static char *b1[5] = {NULL,NULL,NULL,NULL,NULL};
  static char *b2[5] = {NULL,NULL,NULL,NULL,NULL};
  static char *b3[5] = {NULL,NULL,NULL,NULL,NULL};
  static char *b4[5] = {NULL,NULL,NULL,NULL,NULL};

  // Check that input commandline won't overflow fixed-length buffers
  if (strlen(line)>LSTR_LENGTH-STR_MARGIN)
   {
    ppl_error(ERR_SYNTAX, -1, -1, "Input commandline is too long.");
    return NULL;
   }

  // Malloc processing buffers
  bn_old = bn;
  if (bn<5)
   {
    if ((b1[bn]==NULL)||(b2[bn]==NULL)||(b3[bn]==NULL)||(b4[bn]==NULL)) { b1[bn]=(char*)lt_malloc_incontext(LSTR_LENGTH,0); b2[bn]=(char*)lt_malloc_incontext(LSTR_LENGTH,0); b3[bn]=(char*)lt_malloc_incontext(LSTR_LENGTH,0); b4[bn]=(char*)lt_malloc_incontext(LSTR_LENGTH,0); }
    if ((b1[bn]==NULL)||(b2[bn]==NULL)||(b3[bn]==NULL)||(b4[bn]==NULL)) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); return NULL; }
    expecting       = b1[bn];
    ErrText         = b2[bn];
    AlgebraError    = b3[bn];
    AlgebraNewError = b4[bn];
    bn              ++;
   }
  else
   {
    OutContext  = lt_GetMemContext();
    if (OutContext < 2)
     {
      TempContext = lt_DescendIntoNewContext();
      _lt_SetMemContext(OutContext);
      OwnContext  = 1;
     }
    expecting       = (char *)lt_malloc(LSTR_LENGTH);
    ErrText         = (char *)lt_malloc(LSTR_LENGTH);
    AlgebraError    = (char *)lt_malloc(LSTR_LENGTH);
    AlgebraNewError = (char *)lt_malloc(LSTR_LENGTH);
    if ((expecting==NULL)||(ErrText==NULL)||(AlgebraError==NULL)||(AlgebraNewError==NULL)) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); if (OwnContext) { _lt_SetMemContext(TempContext); lt_AscendOutOfContext(TempContext); } bn=bn_old; return NULL; }
  }

  // Fetch first non-whitespace character of command string
  for (i=0; ((line[i]>='\0')&&(line[i]<=' ')); i++);
  if      ((line[i]>='a')&&(line[i]<='z')) cln=(int)(line[i]-'a');
  else if ((line[i]>='A')&&(line[i]<='Z')) cln=(int)(line[i]-'A');
  else                                     cln=-1;

  CmdIterator = ListIterateInit(PplParserCmdList[26]);
  while (1)
   {
    if (CmdIterator == NULL)
     {
      if (cln==-1) break;
      CmdIterator = ListIterateInit(PplParserCmdList[cln]);
      cln=-1;
      if (CmdIterator==NULL) break;
     }
    CmdIterator     = ListIterate(CmdIterator, (void *)&CmdDescriptor);
    match           = 0;
    success         = 1;
    linepos         = 0;
    AlgebraLinepos  =-1;
    AlgebraNewLinepos  =-1;
    ExpectingPos    = 0;
    ExpectingLinePos= 0;
    expecting[0]    = '\0';
    ErrText[0]      = '\0';
    AlgebraError[0] = '\0';
    AlgebraNewError[0] = '\0';
    output          = DictInit(HASHSIZE_SMALL);

    parse_descend(CmdDescriptor, line, IterLevel, &linepos, NULL, NULL, expecting, &ExpectingPos, &ExpectingLinePos,
                  AlgebraError, &AlgebraLinepos, AlgebraNewError, &AlgebraNewLinepos, output, &match, &success);

    if (match==0) continue; // This command did not even begin to match

    while ((line[linepos]!='\0') && (line[linepos]<=' ')) linepos++; // FFW over trailing spaces

    if ((success==0) || (line[linepos]!='\0'))
     {
      int HighlightPos1=-1, HighlightPos2=-1;
      if (AlgebraLinepos < 0) strcpy(ErrText, "\nSyntax Error: ");
      else                    strcpy(ErrText, "\nAt this point, was ");
      ErrPos = strlen(ErrText);
      if (success==0) { sprintf(ErrText+ErrPos, "expecting %s.\n",expecting); ErrPos += strlen(ErrText+ErrPos); }
      else
       {
        if (expecting[0]=='\0')
         { sprintf(ErrText+ErrPos, "unexpected trailing matter at the end of command.\n"); ErrPos += strlen(ErrText+ErrPos); ExpectingLinePos=linepos; }
        else
         { sprintf(ErrText+ErrPos, "expecting %s or end of command.\n",expecting); ErrPos += strlen(ErrText+ErrPos); }
       }
      for (i=0;i<ExpectingLinePos;i++) ErrText[ErrPos++] = ' ';
      strcpy(ErrText+ErrPos, " |\n");                    ErrPos += strlen(ErrText+ErrPos);
      for (i=0;i<ExpectingLinePos;i++) ErrText[ErrPos++] = ' ';
      sprintf(ErrText+ErrPos, "\\|/\n %s", line);
      HighlightPos1 = ErrPos + 5 + ExpectingLinePos;
      if (AlgebraLinepos >= 0) HighlightPos2 = ErrPos + 5 + AlgebraLinepos;
      ErrPos += strlen(ErrText+ErrPos);
      if (AlgebraLinepos >= 0)
       {
        ErrText[ErrPos++] = '\n';
        for (i=0;i<AlgebraLinepos;i++) ErrText[ErrPos++] = ' ';
        strcpy(ErrText+ErrPos, "/|\\\n");                ErrPos += strlen(ErrText+ErrPos);
        for (i=0;i<AlgebraLinepos;i++) ErrText[ErrPos++] = ' ';
        sprintf(ErrText+ErrPos, " |\n%s", AlgebraError); ErrPos += strlen(ErrText+ErrPos);
       }
      strcpy(ErrText+ErrPos, "\n"); ErrPos += strlen(ErrText+ErrPos);
      ppl_error(ERR_PREFORMED, HighlightPos1, HighlightPos2, ErrText);
      if (OwnContext) { _lt_SetMemContext(TempContext); lt_AscendOutOfContext(TempContext); }
      bn=bn_old;
      return NULL;
     }
    if (OwnContext) { _lt_SetMemContext(TempContext); lt_AscendOutOfContext(TempContext); }
    bn=bn_old;
    return output;
   }
  output = DictInit(HASHSIZE_SMALL);
  DictAppendString(output, "directive" , "unrecognised");
  if (OwnContext) { _lt_SetMemContext(TempContext); lt_AscendOutOfContext(TempContext); }
  bn=bn_old;
  return output;
 }

// PARSE_AUTOCOMPLETE(): Make suggestion for words which could come next. number gives the nth possible word.

#ifdef HAVE_READLINE
char *parse_autocomplete(const char *LineConst, int status)
 {
  static int   number, start;

  ListIterator *CmdIterator;
  ParserNode   *CmdDescriptor;
  int           i, cln, match, success, AlgebraLinepos, AlgebraNewLinepos, linepos, ExpectingPos, ExpectingLinePos, NumberCpy;
  int           IterLevel=0;
  static char  *line = NULL, *linep = NULL;
  static char   expecting      [SSTR_LENGTH];
  char          ErrText        [LSTR_LENGTH];
  char          AlgebraError   [LSTR_LENGTH];
  char          AlgebraNewError[LSTR_LENGTH];
  char         *output;

  if (status<0)
   {
    start = -status-1; number = -1; // We are called once with negative status to set up static varaibles, before readline calls us with status>=0
    if (line != NULL) {free(line); line=NULL;}
    if (InputLineAddBuffer == NULL) // If we're on the second line of a continued line, add InputLineAddBuffer to beginning of line
     { linep = rl_line_buffer; }
    else
     {
      i = strlen(rl_line_buffer) + strlen(InputLineAddBuffer);
      if ((line = (char *)malloc((i+1)*sizeof(char)))==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst trying to generate tab-completion suggestions."); return NULL; }
      strcpy(line, InputLineAddBuffer);
      strcpy(line+strlen(line) , rl_line_buffer);
      line[i] = '\0';
      linep = line;
      start += strlen(InputLineAddBuffer);
     }
   }

  while (1)
   {
    NumberCpy = number++;
    if (NumberCpy<0) NumberCpy=0; // Return first item twice

    // Fetch first non-whitespace character of command string
    cln=26;

    CmdIterator = ListIterateInit(PplParserCmdList[cln--]);
    while (1)
     {
      if (CmdIterator == NULL)
       {
        if (cln==-1) break;
        CmdIterator = ListIterateInit(PplParserCmdList[cln--]);
        if (CmdIterator==NULL) continue;
       }
      CmdIterator     = ListIterate(CmdIterator, (void *)&CmdDescriptor);
      match           = 0;
      success         = 1;
      linepos         = 0;
      AlgebraLinepos  = -1;
      AlgebraNewLinepos = -1;
      ExpectingPos    = 0;
      ExpectingLinePos= 0;
      expecting[0]    = '\0';
      ErrText[0]      = '\0';
      AlgebraError[0] = '\0';

      parse_descend(CmdDescriptor, linep, IterLevel, &linepos, &start, &NumberCpy, expecting, &ExpectingPos, &ExpectingLinePos,
                    AlgebraError, &AlgebraLinepos, AlgebraNewError, &AlgebraNewLinepos, NULL, &match, &success);

      if (expecting[0] == '\n')
       {
        if (status < 0) // Special case: use Readline's filename tab completion
         {
          if ((output = (char *)malloc((strlen(expecting)+1)*sizeof(char)))==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst trying to generate tab-completion suggestions."); return NULL; }
          strcpy(output, expecting);
          return output;
         } else {
          break;
         }
       }
      if (expecting[0] != '\0') // We have a new completion option; do not iterate through more commands
       {
        if ((output = (char *)malloc((strlen(expecting)+1)*sizeof(char)))==NULL) { ppl_error(ERR_MEMORY, -1, -1, "Out of memory whilst trying to generate tab-completion suggestions."); return NULL; }
        strcpy(output, expecting);
        return output;
       }
     }
    if (expecting[0] == '\n') continue; // We've been asked to match filenames, but have already made other PyXPlot syntax suggestions
    else                      break;
   }
  return NULL; // Tell readline we have no more matches to offer
 }

char **ppl_rl_completion(const char *text, int start, int end)
 {
  char **matches;
  char  *FirstItem;

  if ((start>0)&&((rl_line_buffer[start-1]=='\"')||(rl_line_buffer[start-1]=='\''))) return NULL; // Do filename completion

  FirstItem = parse_autocomplete(text, -1-start); // Setup parse_autocomplete to know what string it's working on

  if ((FirstItem!=NULL) && (FirstItem[0]=='\n'))  // We are trying to match a %s:filename field, so turn on filename completion
   {
    free(FirstItem);
    rl_attempted_completion_over = 1; // NULL means that readline's default filename completer is activated
    return NULL;
   }
  else
   free(FirstItem);

  matches = rl_completion_matches(text, parse_autocomplete);
  rl_attempted_completion_over = 1; // Make sure that filenames are not completion options
  return matches;
 }
#endif

// PARSE_DESCEND(): We go through command definition structure, recursively descending into sub-structures
//   line -- input line from user
//   linepos -- how far we have got through line with our parsing efforts so far
//   expecting -- used to build up a list of all possible match items which could be used for next word. Used for intelligent syntax errors.
//   AlgebraLinepos -- if we encounter an error evaluating an expression, we store the position in the line of the error here.
//   AlgebraError -- if we encounter an error evaluating an expression, we store the error message here.
//   command -- the command definition structure into which we are descending
//   match -- we set this to true when we've got match to be sure this was the command that the user wanted, even if he made a syntax error.
//   output -- we populate this dictionary with settings from the user's input

// If start and number are set, these indicate that expecting should be populated with the nth autocompletion possibility at point start.

#define PER_MAXSIZE 32

void parse_descend(ParserNode *node, char *line, int IterLevel, int *linepos, int *start, int *number, char *expecting, int *ExpectingPos, int *ExpectingLinePos,
                   char *AlgebraError, int *AlgebraLinepos, char *AlgebraNewError, int *AlgebraNewLinepos,
                   Dict *output, int *match, int *success)
 {
  unsigned char repeating=0, first=0;
  int MatchType=0, LinePosOld=-1, excluded[PER_MAXSIZE], i, j, ACLevel, OutContext, TempContext=0, OwnContext=0, bnd_old;
  struct {int _int; char *_str; value _val; } MatchVal;
  char *varname, *TempMatchStr, SeparatorString[4], QuoteType;
  unsigned char DummyStatus[ALGEBRA_MAXLENGTH];
  Dict *OutputOld=NULL, *DictBaby=NULL;
  List *DictBabyList=NULL;
  ParserNode *NodeIter=NULL;

  static char *b1[5] = {NULL,NULL,NULL,NULL,NULL};
  static char *b2[5] = {NULL,NULL,NULL,NULL,NULL};

  static int AtNLastPos=-1;
  if (*linepos<AtNLastPos) AtNLastPos=-1;

  while ((line[*linepos]!='\0') && (line[*linepos]<=' ')) (*linepos)++; // FFW over spaces
  *success = 1; // We are successful until proven otherwise

#define BND_QUIT if (OwnContext) { _lt_SetMemContext(TempContext); lt_AscendOutOfContext(TempContext); } bnd=bnd_old;

  // Malloc processing buffers
  bnd_old = bnd;
  if (bnd<5)
   {
    if ((b1[bnd]==NULL)||(b2[bnd]==NULL)) { b1[bnd]=(char*)lt_malloc_incontext(SSTR_LENGTH,0); b2[bnd]=(char*)lt_malloc_incontext(LSTR_LENGTH,0); }
    if ((b1[bnd]==NULL)||(b2[bnd]==NULL)) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); *success=0; return; }
    varname         = b1[bnd];
    TempMatchStr    = b2[bnd];
    bnd             ++;
   }
  else
   {
    OutContext  = lt_GetMemContext();
    if (OutContext < 2)
     {
      TempContext = lt_DescendIntoNewContext();
      _lt_SetMemContext(OutContext);
      OwnContext  = 1;
     }
    varname         = (char *)lt_malloc(LSTR_LENGTH);
    TempMatchStr    = (char *)lt_malloc(LSTR_LENGTH);
    if ((varname==NULL)||(TempMatchStr==NULL)) { ppl_error(ERR_MEMORY,-1,-1,"Out of memory."); BND_QUIT; *success=0; return; }
  }

  if      (node->type == PN_TYPE_ITEM)
   {
    // IF WE ARE RUNNING IN TAB-COMPLETION MODE, SEE IF WE OUGHT TO MAKE TAB COMPLETION SUGGESTIONS BASED ON THIS ITEM
    if ((start != NULL) && ((*start) <= (*linepos)))
     {
      if (node->MatchString[0]=='%')
       {
        if ((node->VarName != NULL) && ((strcmp(node->VarName,"filename")==0)||(strcmp(node->VarName,"directory")==0)))
         { // Expecting a filename
          if ((*number)!=0) {(*success)=0; (*number)--; BND_QUIT; return;}
          (*success)=2; strcpy(expecting, "\n"); (*number)--; BND_QUIT; return;
         }
        goto NO_TAB_COMPLETION; // Expecting a float or string, which we don't tab complete... move along and look for something else
       }
      if ((*start) <= (*linepos))
       {
        if ((node->MatchString[0]=='=') && (node->MatchString[1]=='\0'))
         {
          (*success)=1; BND_QUIT; return; // Ignore match character
         }

        if ((*linepos>0) && (isalnum(line[(*linepos)-1])) && (AtNLastPos!=*linepos)) goto NO_TAB_COMPLETION; // 'plot a' cannot tab complete 'using' without space

        for (i=0; ((line[*linepos+i]>' ') && (node->MatchString[i]>' ')); i++)
         if (toupper(line[*linepos+i])!=toupper(node->MatchString[i]))
          {
           (*success)=0; BND_QUIT; return; // We don't match the beginning of this string
          }
        if ((node->ACLevel == -2) && (node->MatchString[i]<=' ')) goto NO_TAB_COMPLETION; // We've matched an @n string right to the end... move on
        if ((*number)!=0) {(*success)=0; (*number)--; BND_QUIT; return;}
        (*success)=2;
        for (i=0; i<((*linepos)-(*start)); i++) expecting[i] = line[*start+i];
        strcpy(expecting+i, node->MatchString); // Matchstring should match itself
        (*number)--;
        BND_QUIT; return;
       }
     }
NO_TAB_COMPLETION:

    if (node->ACLevel == -2)
     {
      for (i=0; (node->MatchString[i]>' '); i++)
       if (toupper(line[*linepos+i])!=toupper(node->MatchString[i]))
        {
         (*success)=0; break; // We don't match this string
        }
      if (*success!=0)
       {
        *linepos += i; // We do match this string: advance by i spaces
        AtNLastPos = *linepos;
        MatchType = DATATYPE_STRING;
        MatchVal._str = node->MatchString;
       }
     }
    else
     {
      ACLevel = node->ACLevel;
      if (ACLevel == -1) ACLevel = strlen(node->MatchString);
      if      (strcmp(node->MatchString, "=")==0)
       { *match=1; }
      else if (strcmp(node->MatchString, "%r")==0)
       {
        MatchType = DATATYPE_STRING;
        MatchVal._str = line + *linepos;
        *linepos += strlen(line + *linepos);
       }
      else if (strcmp(node->MatchString, "%s")==0)
       {
        for (i=0; (isalpha(line[*linepos+i]) || (line[*linepos+i]=='_')); i++);
        if (i>0)
         {
          strncpy(TempMatchStr, line + *linepos, i);
          TempMatchStr[i] = '\0';
          MatchType = DATATYPE_STRING;
          MatchVal._str = TempMatchStr;
          *linepos += i;
         }
        else *success=0;
       }
      else if (strcmp(node->MatchString, "%S")==0)
       {
        for (i=0; ((line[*linepos+i]>' ')&&(line[*linepos+i]!='\'')&&(line[*linepos+i]!='\"')); i++);
        if (i>0)
         {
          strncpy(TempMatchStr, line + *linepos, i);
          TempMatchStr[i] = '\0';
          MatchType = DATATYPE_STRING;
          MatchVal._str = TempMatchStr;
          *linepos += i;
         }
        else *success=0;
       }
      else if (strcmp(node->MatchString, "%q")==0)
       {
        i=-1;
        *AlgebraNewLinepos=-1;
        ppl_GetQuotedString(line, TempMatchStr, *linepos, &i, 0, AlgebraNewLinepos, AlgebraNewError, IterLevel);
        if (*AlgebraNewLinepos >= 0)
         {
          *success=0;
          if (*AlgebraNewLinepos > *AlgebraLinepos)
            { *AlgebraLinepos=*AlgebraNewLinepos; strcpy(AlgebraError, AlgebraNewError); }
         }
        else
         {
          MatchType     = DATATYPE_STRING;
          MatchVal._str = TempMatchStr;
          *linepos      = i;
         }
       }
      else if (strcmp(node->MatchString, "%Q")==0)
       {
        for (i=0; ((line[*linepos+i]>'\0') && (line[*linepos+i]<=' ')); i++); // Fast-forward over preceding spaces

        if ((line[*linepos+i]!='\'') && (line[*linepos+i]!='\"')) // If quoted string doesn't start with quote, it must be a string variable name
         {
          if (!isalpha(line[*linepos+i]))
           { *success=0; }
          else
           {
            while (((isalnum(line[*linepos+i]))||(line[*linepos+i]=='_'))) i++; // Fetch a word
            while ((line[*linepos+i]>'\0') && (line[*linepos+i]<=' ')) i++; // Fast-forward over trailing spaces
            if (line[*linepos+i]=='(') // We have a function
             {
              j=-1;
              StrBracketMatch(line + *linepos + i, NULL, NULL, &j, -1);
              if (j<0) { *success=0; }
              else     { i+=j+1; } // We have succeeded in finding a valid function call
             }
            else // We have a variable name, and have nothing more to do
             { }
           }
         }
        else // We have a quoted string
         {
          QuoteType = line[*linepos+(i++)];
          for (; ((line[*linepos+i]!='\0') && (line[*linepos+i]!=QuoteType)); i++)
           {
            if ((line[*linepos+i]=='\\') && (line[*linepos+i+1]=='\\')) i++;
            if ((line[*linepos+i]=='\\') && (line[*linepos+i+1]=='\'')) i++;
            if ((line[*linepos+i]=='\\') && (line[*linepos+i+1]=='\"')) i++;
           }
          if (line[*linepos+i]!=QuoteType)
           { *success=0; }
          else
           {
            i++;
            while ((line[*linepos+i]>'\0') && (line[*linepos+i]<=' ')) i++; // Fast-forward over trailing spaces
            if (line[*linepos+i]=='%') // We have a substitution operator
             {
              i++;
              while ((line[*linepos+i]>'\0') && (line[*linepos+i]<=' ')) i++; // Fast-forward over trailing spaces
              if (line[*linepos+i]!='(') // Need an opening bracket after substitution operator
               { *success=0; }
              else
               {
                j=-1;
                StrBracketMatch(line + *linepos + i, NULL, NULL, &j, -1);
                if (j<0) { *success=0; }
                else     { i+=j+1; } // We have succeeded in finding a valid function call
               }
             }
           }
         }
        if (*success!=0) // If we have succeeded, write output
         {
          strncpy(TempMatchStr, line + *linepos, i);
          TempMatchStr[i] = '\0';
          MatchType = DATATYPE_STRING;
          MatchVal._str = TempMatchStr;
          *linepos += i;
         }
       }
      else if (strcmp(node->MatchString, "%a")==0)
       {
        if      ((line[*linepos]=='x')||(line[*linepos]=='X')) TempMatchStr[0]='x';
        else if ((line[*linepos]=='y')||(line[*linepos]=='Y')) TempMatchStr[0]='y';
        else if ((line[*linepos]=='z')||(line[*linepos]=='Z')) TempMatchStr[0]='z';
        else                                                   *success = 0;

        if (*success!=0)
         {
          (*linepos)++;
          i = -1;
          *AlgebraNewLinepos = -1;
          ppl_EvaluateAlgebra(line, &MatchVal._val, *linepos, &i, 0, AlgebraNewLinepos, AlgebraNewError, IterLevel);
          if (*AlgebraNewLinepos >= 0)
           {
            TempMatchStr[1]='1'; TempMatchStr[2]='\0'; *AlgebraNewLinepos = -1;
           }
          else
           {
            if (MatchVal._val.dimensionless == 0)
             {
              sprintf(AlgebraError, "This value should have been dimensionless, but instead has units of <%s>.", ppl_units_GetUnitStr(&MatchVal._val,NULL,NULL,0,1,0));
              *AlgebraLinepos = *linepos;
              *match          = 1; // Fudge to make sure error is displayed
              *success        = 0;
             }
            else if (MatchVal._val.FlagComplex == 1)
             {
              sprintf(AlgebraError, "This value should have been real, but in fact has an imaginary component.");
              *AlgebraLinepos = *linepos;
              *match          = 1; // Fudge to make sure error is displayed
              *success        = 0;
             }
            else if (!gsl_finite(MatchVal._val.real))
             {
              sprintf(AlgebraError, "Axis numbers must be finite real integers in the range 1 - %d.",MAX_AXES-1);
              *AlgebraLinepos = *linepos;
              *match          = 1; // Fudge to make sure error is displayed
              *success        = 0;
             }
            else if ((MatchVal._val.real>=INT_MAX)||(MatchVal._val.real>=MAX_AXES))
             {
              sprintf(AlgebraError, "A maximum of %d parallel axes are allowed; axis numbers greater than %d are not permitted.",MAX_AXES-1,MAX_AXES-1);
              *AlgebraLinepos = *linepos;
              *match          = 1; // Fudge to make sure error is displayed
              *success        = 0;
             }
            else if ((MatchVal._val.real<1)||(!gsl_finite(MatchVal._val.real)))
             {
              sprintf(AlgebraError, "Axis numbers must be greater than or equal to one.");
              *AlgebraLinepos = *linepos;
              *match          = 1; // Fudge to make sure error is displayed
              *success        = 0;
             }
            else
             {
              sprintf(TempMatchStr+1, "%d", (int)floor(MatchVal._val.real));
              *linepos = i;
             }
           }
          if (*success!=0)
           {
            MatchVal._str = TempMatchStr;
            MatchType = DATATYPE_STRING;
            AtNLastPos = *linepos;
           }
         }
       }
      else if (strcmp(node->MatchString, "%v")==0)
       {
        if (!isalpha(line[*linepos])) *success=0;
        else
         {
          for (i=0; ((isalnum(line[*linepos])) || (line[*linepos]=='_')); i++,(*linepos)++) TempMatchStr[i] = line[*linepos];
          TempMatchStr[i]='\0';
          MatchType     = DATATYPE_STRING;
          MatchVal._str = TempMatchStr;
         }
       }
      else if ((strcmp(node->MatchString, "%e")==0) || (strcmp(node->MatchString, "%E")==0)) // %E allows dollars; %e does not
       {
        i = -1;
        *AlgebraNewLinepos=-1;
        ppl_GetExpression(line+*linepos, &i, (strcmp(node->MatchString, "%E")==0), DummyStatus, NULL, AlgebraNewLinepos, AlgebraNewError);
        if (*AlgebraNewLinepos >= 0)
         {
          *success=0;
          (*AlgebraNewLinepos)+=*linepos;
          if (*AlgebraNewLinepos > *AlgebraLinepos)
            { *AlgebraLinepos=*AlgebraNewLinepos; strcpy(AlgebraError, AlgebraNewError); }
         }
        else
         {
          strncpy(TempMatchStr, line+*linepos, i);
          TempMatchStr[i] = '\0';
          MatchType     = DATATYPE_STRING;
          MatchVal._str = TempMatchStr;
          *linepos     += i;
         }
       }
      else if ((strcmp(node->MatchString, "%f")==0) || (strcmp(node->MatchString, "%fi")==0) || (strcmp(node->MatchString, "%fu")==0) || (strcmp(node->MatchString, "%d")==0))
       {
        i = -1;
        *AlgebraNewLinepos=-1;
        ppl_EvaluateAlgebra(line, &MatchVal._val, *linepos, &i, 0, AlgebraNewLinepos, AlgebraNewError, IterLevel);
        if (*AlgebraNewLinepos >= 0)
         {
          *success=0;
          if (*AlgebraNewLinepos > *AlgebraLinepos)
            { *AlgebraLinepos=*AlgebraNewLinepos; strcpy(AlgebraError, AlgebraNewError); }
         }
        else
         {
          if ((strcmp(node->MatchString, "%fi")!=0) && (strcmp(node->MatchString, "%fu")!=0) && (MatchVal._val.dimensionless == 0))
           {
            sprintf(AlgebraError, "This value should have been dimensionless, but instead has units of <%s>.", ppl_units_GetUnitStr(&MatchVal._val,NULL,NULL,0,1,0));
            *AlgebraLinepos = *linepos;
            *success        = 0;
           }
          else if ((strcmp(node->MatchString, "%fi")!=0) && (MatchVal._val.FlagComplex == 1))
           {
            sprintf(AlgebraError, "This value should have been real, but in fact has an imaginary component.");
            *AlgebraLinepos = *linepos;
            *success        = 0;
           }
          else if (strcmp(node->MatchString, "%d")==0)
           {
            if (!gsl_finite(MatchVal._val.real))
             {
              sprintf(AlgebraError, "This integer value is not finite and real.");
              *AlgebraLinepos = *linepos;
              *success        = 0;
             }
            else if ((MatchVal._val.real<=INT_MIN)||(MatchVal._val.real>=INT_MAX))
             {
              sprintf(AlgebraError, "This integer value is too large.");
              *AlgebraLinepos = *linepos;
              *success        = 0;
             }
            else
             {
              MatchVal._int = (int)floor(MatchVal._val.real);
              MatchType     = DATATYPE_INT;
             }
           }
          else if (strcmp(node->MatchString, "%f")==0)
           {
            MatchType    = DATATYPE_FLOAT;
           }
          else
           {
            MatchType    = DATATYPE_VALUE;
           }
          if (*success!=0) *linepos = i;
         }
       }
      else // Anything else matches itself
       {
        i = StrAutocomplete(line + *linepos, node->MatchString, ACLevel);
        if (i<0)
         { *success=0; }
        else
         {
          *linepos += i;
          MatchType = DATATYPE_STRING;
          MatchVal._str = node->MatchString;
         }
       }
     }

    // IF WE'RE NOT RUNNING IN TAB COMPLETION MODE, WE MAY WANT TO SET SOME OUTPUT VARIABLES NOW
    if (start == NULL)
     {
      if (*success != 0)
       {
        expecting[0]   = '\0'; *ExpectingPos  =0;
        AlgebraError[0]= '\0'; *AlgebraLinepos=-1;
        if ((node->VarName     != NULL) && (node->VarName[0]    != '\0'))
         {
          if ((node->VarSetVal != NULL) && (node->VarSetVal[0]  != '\0'))  DictAppendString(output , node->VarName , node->VarSetVal);
          else
           {
            if      (MatchType == DATATYPE_INT)    DictAppendInt   (output , node->VarName , MatchVal._int     );
            else if (MatchType == DATATYPE_FLOAT)  DictAppendFloat (output , node->VarName , MatchVal._val.real);
            else if (MatchType == DATATYPE_VALUE)  DictAppendValue (output , node->VarName , MatchVal._val     );
            else if (MatchType == DATATYPE_STRING) DictAppendString(output , node->VarName , MatchVal._str     );
           }
         }
       }
      else if (*ExpectingLinePos <= *linepos) // If we failed to match this item, add something to 'expecting' about what the user could have typed here
       {
        if ((*ExpectingLinePos) < (*linepos)) { *ExpectingLinePos = *linepos; *ExpectingPos = 0; expecting[0] = '\0'; } // Wipe old expecting string
        if (*ExpectingPos != 0) { strcpy(expecting+*ExpectingPos, ", or "); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        if ((node->VarName != NULL) && (node->VarName[0] != '\0'))  sprintf(varname, " (%s)", node->VarName);
        else                                                        varname[0]='\0';

        if       (strcmp(node->MatchString, "%a")==0)
         { sprintf(expecting+*ExpectingPos, "an axis name%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if  (strcmp(node->MatchString, "%d")==0)
         { sprintf(expecting+*ExpectingPos, "an integer value or expression%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%e")==0) || (strcmp(node->MatchString, "%E")==0))
         { sprintf(expecting+*ExpectingPos, "an algebraic expression%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%f")==0) || (strcmp(node->MatchString, "%fi")==0) || (strcmp(node->MatchString, "%fu")==0))
         { sprintf(expecting+*ExpectingPos, "a numeric value or expression%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%s")==0) || (strcmp(node->MatchString, "%S")==0) || (strcmp(node->MatchString, "%r")==0))
         { sprintf(expecting+*ExpectingPos, "a string%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%q")==0) || (strcmp(node->MatchString, "%Q")==0))
         { sprintf(expecting+*ExpectingPos, "a quoted string%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if  (strcmp(node->MatchString, "%v")==0)
         { sprintf(expecting+*ExpectingPos, "a variable name%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else
         { sprintf(expecting+*ExpectingPos, "\"%s\"", node->MatchString); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
       }
     }
   }
  else if (node->type == PN_TYPE_SEQ)
   {
    NodeIter = node->FirstChild;
    while (NodeIter != NULL)
     {
      parse_descend(NodeIter, line, IterLevel, linepos, start, number, expecting, ExpectingPos, ExpectingLinePos,
                    AlgebraError, AlgebraLinepos, AlgebraNewError, AlgebraNewLinepos, output, match, success);
      if (*success==2) { BND_QUIT; return; }
      NodeIter = NodeIter->NextSibling;
      if (*success==0) break;
     }
   }
  else if ((node->type == PN_TYPE_REP) || (node->type == PN_TYPE_REP2))
   {
    repeating = 1; first = 1;
    if (output != NULL) DictBabyList = ListInit();
    SeparatorString[0] = node->VarName[strlen(node->VarName)-1];
    if ((SeparatorString[0]!=',')&&(SeparatorString[0]!=':')) SeparatorString[0]='\0';
    SeparatorString[1] = '\0';

    while (repeating != 0)
     {
      ParserNode SeparatorNode;
      if (output != NULL) DictBaby = DictInit(HASHSIZE_SMALL);
      LinePosOld = *linepos;
      SeparatorNode.type        = PN_TYPE_ITEM;
      SeparatorNode.ACLevel     = -2;
      SeparatorNode.MatchString = SeparatorString;
      SeparatorNode.VarName     = SeparatorNode.VarSetVal = NULL;
      SeparatorNode.FirstChild  = SeparatorNode.NextSibling = NULL;
      if ((first==0)&&(SeparatorString[0]!='\0'))
       {
        parse_descend(&SeparatorNode, line, IterLevel, linepos, start, number, expecting, ExpectingPos, ExpectingLinePos,
                      AlgebraError, AlgebraLinepos, AlgebraNewError, AlgebraNewLinepos, DictBaby, match, success);
        if (*success==2) { BND_QUIT; return; }
       }
      if (*success!=0)
       {
        NodeIter = node->FirstChild;
        while (NodeIter != NULL)
         {
          parse_descend(NodeIter, line, IterLevel, linepos, start, number, expecting, ExpectingPos, ExpectingLinePos,
                        AlgebraError, AlgebraLinepos, AlgebraNewError, AlgebraNewLinepos, DictBaby, match, success);
          if (*success==2) { BND_QUIT; return; }
          if (*success==0)
           {
            *linepos = LinePosOld;
            *success = 1; // Optional items are allowed to fail
            repeating= 0; // But we don't do any more work after we get one
            break;
           }
          NodeIter = NodeIter->NextSibling;
         }
       }
      else
       {
        *success = 1; repeating = 0; // We didn't get a repeat separator
       }
      if  (repeating!=0)                      { first = 0; }
      if ((repeating!=0) && (output != NULL)) { ListAppendDict(DictBabyList, DictBaby); }
     }
    if (first==0) { if (output != NULL) DictAppendList(output , node->VarName , DictBabyList); } // Only append list if we matched at least once
    else if (node->type == PN_TYPE_REP) *success=0; // We needed at least one item, but got none
   }
  else if (node->type == PN_TYPE_OPT)
   {
    LinePosOld = *linepos;
    if (output != NULL) OutputOld = DictCopy(output,1);
    NodeIter = node->FirstChild;
    while (NodeIter != NULL)
     {
      parse_descend(NodeIter, line, IterLevel, linepos, start, number, expecting, ExpectingPos, ExpectingLinePos,
                    AlgebraError, AlgebraLinepos, AlgebraNewError, AlgebraNewLinepos, output, match, success);
      if (*success==2) { BND_QUIT; return; }
      if (*success==0)
       {
        if ((output != NULL)&&(*linepos!=LinePosOld)) *output = *OutputOld; // Don't need to do another deepcopy; just overwrite top Dict struct
        *linepos = LinePosOld;
        *success = 1; // Optional items are allowed to fail
        break;
       }
      NodeIter = NodeIter->NextSibling;
     }
   }
  else if (node->type == PN_TYPE_PER)
   {
    repeating = 1;
    for (i=0; i<PER_MAXSIZE; i++) excluded[i]=0; // Reset all excluded flags; we haven't had any PER items yet
    while (repeating != 0)
     {
      repeating  = 0; // Unless we find an item on the list which succeeds, we don't want to loop again
      LinePosOld = *linepos;
      if (output != NULL) OutputOld = DictCopy(output,1);
      NodeIter = node->FirstChild; i=0;
      while (NodeIter != NULL)
       {
        if (excluded[i]==0)
         {
          parse_descend(NodeIter, line, IterLevel, linepos, start, number, expecting, ExpectingPos, ExpectingLinePos,
                        AlgebraError, AlgebraLinepos, AlgebraNewError, AlgebraNewLinepos, output, match, success);
          if (*success==2) { BND_QUIT; return; }
          if (*success!=0)
           { excluded[i]=1; repeating=1; break; } // This item worked; flag it and loop again
          else
           {
            if ((output != NULL)&&(*linepos!=LinePosOld)) // We failed, so reset output and linepos
             {
              *output   = *OutputOld; // Don't need to do another deepcopy; just overwrite top Dict struct
              OutputOld = DictCopy(output,1);
             }
            *linepos = LinePosOld;
            *success = 1; // The failure of one item doesn't matter... all PER items are optional.
           }
         }
        NodeIter = NodeIter->NextSibling; i++;
       }
     }
   }
  else if (node->type == PN_TYPE_ORA)
   {
    LinePosOld = *linepos;
    if (output != NULL) OutputOld = DictCopy(output,1);
    NodeIter = node->FirstChild;
    while (NodeIter != NULL)
     {
      *success = 1;
      parse_descend(NodeIter, line, IterLevel, linepos, start, number, expecting, ExpectingPos, ExpectingLinePos,
                    AlgebraError, AlgebraLinepos, AlgebraNewError, AlgebraNewLinepos, output, match, success);
      if (*success==2) { BND_QUIT; return; }
      if (*success==0) // Reset output and try the next ORA item
       {
        if ((output != NULL)&&(*linepos!=LinePosOld))
         {
          *output   = *OutputOld; // Don't need to do another deepcopy; just overwrite top Dict struct
          OutputOld = DictCopy(output,1);
         }
        *linepos = LinePosOld;
       }
      else break; // We've found an ORA item which worked
      NodeIter = NodeIter->NextSibling;
     }
   }
  else
   {
    sprintf(temp_err_string, "Hit an unexpected node type %d.", node->type);
    ppl_fatal(__FILE__,__LINE__,temp_err_string);
   }
  BND_QUIT;
  return;
 }

