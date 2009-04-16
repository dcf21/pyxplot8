// ppl_parser.c
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_list.h"
#include "ListTools/lt_dict.h"
#include "ListTools/lt_memory.h"

#include "ppl_error.h"
#include "ppl_parser.h"
#include "pyxplot.h"

#define PN_TYPE_SEQ  21000
#define PN_TYPE_OPT  21001
#define PN_TYPE_REP  21002
#define PN_TYPE_PER  21003
#define PN_TYPE_ORA  21004
#define PN_TYPE_ITEM 21005

typedef struct ParserNode {
  int   type;
  char *MatchString; // ITEMs only
  int   ACLevel;     // ITEMs only
  char *VarName;
  char *VarSetVal;
  struct ParserNode *FirstChild;
  struct ParserNode *NextSibling;
 } ParserNode;

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

  if (PplCommandsText[j]==':') j++;
  if (PplCommandsText[j]> ' ')
   {
    VarNameBegin = j;
    while (PplCommandsText[j]> ' ') j++; // FFW over variable name
    VarNameEnd   = j;
   }
  while ((PplCommandsText[j]!='\0') && (PplCommandsText[j]<' ')) j++; // FFW over trailing whitespace

  if (VarNameBegin > -1)
   {
    DefnStack[(*i)-1]->VarName = (char *)lt_malloc((VarNameEnd-VarNameBegin+1)*sizeof(char));
    strncpy( DefnStack[(*i)-1]->VarName , PplCommandsText+VarNameBegin , VarNameEnd-VarNameBegin );
    DefnStack[(*i)-1]->VarName[VarNameEnd - VarNameBegin] = '\0';
   }
  DefnStack[(*i)-1]->VarSetVal = "\0"; // ITEM structures can do "exit:directive:quit" to store "quit" into variable "directive", but []s never do that.

  while (((*i)>0) && (DefnStack[(*i)-1]->type == PN_TYPE_SEQ)) (*i)--; // Automatically close SEQs; they don't have closing grammar

  if ((*i)<=0)
   { sprintf(temp_err_string, "Incorrect nesting of types in command specification -- attempt to close an unopened bracket."); ppl_fatal(__FILE__,__LINE__, temp_err_string); }
  else if (DefnStack[(*i)-1]->type != type)
   { sprintf(temp_err_string, "Incorrect nesting of types in command specification -- expected %d, but got %d.", type, DefnStack[(*i)-1]->type); ppl_fatal(__FILE__,__LINE__, temp_err_string); }

  (*i)--; // Close structure of type "type" and roll back stack.
  *NcharsTaken = j;
  return;
 }

// ppl_commands_read(): This runs on startup to populate a list,
// PplParserCmdList, with list-based definitions of PyXPlot's commands, parsed
// using the functions above from plp_commands

static List *PplParserCmdList;

void ppl_commands_read()
 {
  int         InputPos = 0;
  ParserNode *DefnStack[25];
  int         StackPos;
  int         i,j,k,l,m,n,N;
  ParserNode *NewNode;
  ParserNode **target = NULL;

  PplParserCmdList = ListInit();
  while (ppl_commands[InputPos] != '\0')
   {
    while ((ppl_commands[InputPos] != '\0') && (ppl_commands[InputPos] <= ' ')) InputPos++; // Ignore whitespace
    if (ppl_commands[InputPos] == '\0') continue;
    StackPos = 0;
    StartNewStructure(DefnStack, &StackPos, PN_TYPE_SEQ);

    while (1)
     {
      while ((ppl_commands[InputPos] != '\0') && (ppl_commands[InputPos] <= ' ')) InputPos++; // Ignore whitespace
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
        k=l=m=n=N=-1;
        i=InputPos;
        while (ppl_commands[InputPos] > ' ') InputPos++;
        j=InputPos;
        if (j==i) ppl_fatal(__FILE__,__LINE__,"ppl_parser found a word of zero length. This should never happen.");
        if (ppl_commands[InputPos] == '@')
         {
          InputPos++;
          N = (int)GetFloat(ppl_commands+InputPos , &k);
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
        NewNode          = (ParserNode *)lt_malloc(sizeof(ParserNode));
        NewNode->type    = PN_TYPE_ITEM;
        NewNode->ACLevel = N;
                   NewNode->MatchString = (char *)lt_malloc((j-i+1)*sizeof(char)); strncpy(NewNode->MatchString, ppl_commands + i, j-i); NewNode->MatchString[j-i]='\0';
        if (k>0) { NewNode->VarName     = (char *)lt_malloc((l-k+1)*sizeof(char)); strncpy(NewNode->VarName    , ppl_commands + k, l-k); NewNode->VarName    [l-k]='\0'; }
        else       NewNode->VarName     = NULL;
        if (m>0) { NewNode->VarSetVal   = (char *)lt_malloc((n-m+1)*sizeof(char)); strncpy(NewNode->VarSetVal  , ppl_commands + m, n-m); NewNode->VarSetVal  [n-m]='\0'; }
        else       NewNode->VarSetVal   = NULL;
        NewNode->FirstChild = NewNode->NextSibling = NULL;
        target = &(DefnStack[StackPos-1]->FirstChild);
        while (*target != NULL) target = &((*target)->NextSibling);
        *target = NewNode; // Add new node into hierarchy
       }
     }
    ListAppendPtr(PplParserCmdList, DefnStack[0]);
   }
  return;
 }

// --------------------------------------------------------------------------
// PART II: PARSE A LINE OF USER INPUT

// PARSE(): Top-level interface. Parses a commandline "line" from the user.
// It expects that ; and `` have already been dealt with by pyxplot.py

Dict *parse(char *line)
 {
  return NULL;
 }

// PARSE_DESCEND(): We go through command definition structure, recursively descending into sub-structures
//   line -- input line from user
//   vars -- PyXPlot's user-defined variables; used for evaluating expressions
//   linepos -- how far through line have we got with our parsing efforts so far?
//   expecting -- used to build up a list of all possible match items which could be used for next word. Used for intelligent syntax errors.
//   algebra_linepos -- if we encounter an error evaluating an expression, we store the position in the line of the error here.
//   algebra_error -- if we encounter an error evaluating an expression, we store the error message here.
//   command -- the command definition structure into which we are descending
//   match -- we set this to true when we've got match to be sure this was the command that the user wanted, even if he made a syntax error.
//   dict -- we populate this dictionary with settings from the user's input

void parse_descend(char *line, int linepos, char *expecting, int algebra_linepos, char *algebra_error, List *command, int match, Dict *dict)
 {
  return;
 }

