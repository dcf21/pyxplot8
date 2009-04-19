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
#include <ctype.h>
#include <readline/readline.h>

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

ParserNode SeparatorNode = {PN_TYPE_ITEM, NULL, 1, NULL, NULL, NULL, NULL};

void parse_descend(ParserNode *node, char *line, int *linepos, int *start, int *number, char *expecting, int *ExpectingPos, char *AlgebraError, int *AlgebraLinepos, Dict *output, int *match, int *success);

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

  while (((*i)>0) && (DefnStack[(*i)-1]->type == PN_TYPE_SEQ)) (*i)--; // Automatically close SEQs; they don't have closing grammar

  if ((*i)<=0)
   { sprintf(temp_err_string, "Incorrect nesting of types in command specification -- attempt to close an unopened bracket."); ppl_fatal(__FILE__,__LINE__, temp_err_string); }
  else if (DefnStack[(*i)-1]->type != type)
   { sprintf(temp_err_string, "Incorrect nesting of types in command specification -- expected %d, but got %d.", type, DefnStack[(*i)-1]->type); ppl_fatal(__FILE__,__LINE__, temp_err_string); }

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
        while ((ppl_commands[InputPos] > ' ') && (ppl_commands[InputPos] !='@')) InputPos++;
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
    ListAppendPtr(PplParserCmdList, DefnStack[0], sizeof(ParserNode), 0);
   }
  return;
 }

// --------------------------------------------------------------------------
// PART II: PARSE A LINE OF USER INPUT

// PARSE(): Top-level interface. Parses a commandline "line" from the user.
// It expects that ; and `` have already been dealt with by pyxplot.py

Dict *parse(char *line)
 {
  ListIterator *CmdIterator;
  ParserNode   *CmdDescriptor;
  Dict         *output;
  int           match, success, AlgebraLinepos, linepos, ExpectingPos, ErrPos, i;
  char          expecting   [LSTR_LENGTH];
  char          ErrText     [LSTR_LENGTH];
  char          AlgebraError[LSTR_LENGTH];

  CmdIterator = ListIterateInit(PplParserCmdList);
  while (CmdIterator != NULL)
   {
    CmdIterator     = ListIterate(CmdIterator, (void **)&CmdDescriptor);
    match           = 0;
    success         = 1;
    linepos         = 0;
    AlgebraLinepos  =-1;
    ExpectingPos    = 0;
    expecting[0]    = '\0';
    ErrText[0]      = '\0';
    AlgebraError[0] = '\0';
    output          = DictInit();

    parse_descend(CmdDescriptor, line, &linepos, NULL, NULL, expecting, &ExpectingPos, AlgebraError, &AlgebraLinepos, output, &match, &success);

    if (match==0) continue; // This command did not even begin to match

    while ((line[linepos]!='\0') && (line[linepos]<=' ')) linepos++; // FFW over trailing spaces

    if ((success==0) || (line[linepos]!='\0'))
     {
      if (AlgebraLinepos >= 0) strcpy(ErrText, "\nSyntax Error: ");
      else                     strcpy(ErrText, "\nAt this point, was ");
      ErrPos = strlen(ErrText);
      if (success==0) { sprintf(ErrText+ErrPos, "expecting %s.\n",expecting); ErrPos += strlen(ErrText+ErrPos); }
      else
       {
        if (expecting[0]=='\0') { sprintf(ErrText+ErrPos, "unexpected trailing matter at the end of command.\n"); ErrPos += strlen(ErrText+ErrPos); }
        else                    { sprintf(ErrText+ErrPos, "expecting %s or end of command.\n",expecting); ErrPos += strlen(ErrText+ErrPos); }
       }
      for (i=0;i<linepos;i++) ErrText[ErrPos++] = ' ';
      strcpy(ErrText+ErrPos, " |\n");                    ErrPos += strlen(ErrText+ErrPos);
      for (i=0;i<linepos;i++) ErrText[ErrPos++] = ' ';
      sprintf(ErrText+ErrPos, "\\|/\n %s", line);        ErrPos += strlen(ErrText+ErrPos);
      if (AlgebraLinepos >= 0)
       {
        ErrText[ErrPos++] = '\n';
        for (i=0;i<linepos;i++) ErrText[ErrPos++] = ' ';
        strcpy(ErrText+ErrPos, "/|\\\n");                ErrPos += strlen(ErrText+ErrPos);
        for (i=0;i<linepos;i++) ErrText[ErrPos++] = ' ';
        sprintf(ErrText+ErrPos, " |\n%s", AlgebraError); ErrPos += strlen(ErrText+ErrPos);
       }
      strcpy(ErrText+ErrPos, "\n"); ErrPos += strlen(ErrText+ErrPos);
      ppl_error(ErrText);
      return NULL;
     }
    return output;
   }
  output = DictInit();
  DictAppendString(output, "directive", 0, "unrecognised");
  return output;
 }

// PARSE_AUTOCOMPLETE(): Make suggestion for words which could come next. number gives the nth possible word.


char *ppl_dummy_completer(const char *line, int status) // Because readline is a fucking piece of shit, and you can't turn off the filename autocompleter
 {                                                      // without putting something else in its place
  return NULL;  // Dear author of readline. Please do the gene pool a service and shoot yourself.
 }

char *parse_autocomplete(const char *LineConst, int status)
 {
  static int number, start;

  ListIterator *CmdIterator;
  ParserNode   *CmdDescriptor;
  int           match, success, AlgebraLinepos, linepos, ExpectingPos, NumberCpy;
  static char   expecting   [SSTR_LENGTH];
  char          ErrText     [LSTR_LENGTH];
  char          AlgebraError[LSTR_LENGTH];
  char         *output, *line;

  line = (char *)LineConst;

  if (status<0)
   {
    start = -status-1; number = -1;
   }

  NumberCpy = number++;
  if (NumberCpy<0) NumberCpy=0; // Return first item twice
  CmdIterator = ListIterateInit(PplParserCmdList);
  while (CmdIterator != NULL)
   {
    CmdIterator     = ListIterate(CmdIterator, (void **)&CmdDescriptor);
    match           = 0;
    success         = 1;
    linepos         = 0;
    AlgebraLinepos  = -1;
    expecting[0]    = '\0';
    ErrText[0]      = '\0';
    AlgebraError[0] = '\0';

    parse_descend(CmdDescriptor, line, &linepos, &start, &NumberCpy, expecting, &ExpectingPos, AlgebraError, &AlgebraLinepos, NULL, &match, &success);

    if (expecting[0] == '\n')
     {
      output = (char *)malloc((strlen(expecting)+1)*sizeof(char)); // Special case: use Readline's filename tab completion
      strcpy(output, expecting);
      return output;
     }
    if (expecting[0] != '\0')
     {
      output = (char *)malloc((strlen(expecting)+1)*sizeof(char));
      strcpy(output, expecting);
      return output;
     }
   }
  return NULL; // No more matches
 }

#define TABCOMPLETES_MAX 1024

char **ppl_rl_completion(const char *text, int start, int end)
 {
  char **matches;
  char  *FirstItem;

  FirstItem = parse_autocomplete(text, -1-start); // Setup parse_autocomplete

  if ((FirstItem!=NULL) && (FirstItem[0]=='\n')) // Trigger filename completion
   {
    free(FirstItem);
    rl_completion_entry_function = NULL;
    return NULL;
   }
  else
   free(FirstItem);

  matches = rl_completion_matches(text, parse_autocomplete);
  rl_completion_entry_function = ppl_dummy_completer;
  return matches;
 }

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

void parse_descend(ParserNode *node, char *line, int *linepos, int *start, int *number, char *expecting, int *ExpectingPos,
                   char *AlgebraError, int *AlgebraLinepos, Dict *output, int *match, int *success)
 {
  unsigned char repeating=0, first=0;
  int MatchType=0, LinePosOld=-1, excluded[PER_MAXSIZE], i;
  union {double _dbl; int _int; char *_str; } MatchVal;
  char varname[SSTR_LENGTH], SeparatorString[4];
  Dict *OutputOld, *DictBaby;
  List *DictBabyList;
  ParserNode *NodeIter;

  while ((line[*linepos]!='\0') && (line[*linepos]<=' ')) (*linepos)++; // FFW over spaces
  *success = 1; // We are successful until proven otherwise

  if      (node->type == PN_TYPE_ITEM)
   {
    if ((start != NULL) && ((*start) <  (*linepos))) {(*success)=0; return;} // We have overshot...
    if ((start != NULL) && ((*start) == (*linepos)))
     {
      if ((*number)!=0) {(*success)=0; (*number)--; return;}
      if (node->MatchString[0]=='%')
       {
        if ((node->VarName != NULL) && ((strcmp(node->VarName,"filename")==0)||(strcmp(node->VarName,"directory"))))
         { // Expecting a filename
          (*success)=2; strcpy(expecting, "\n"); (*number)--; return;
         }
        (*success)=0; return; // Expecting a float or string, which we don't tab complete... move along and look for something else
       }
      for (i=0; ((line[*linepos+i]>' ') && (node->MatchString[i]>' ')); i++)
       if (toupper(line[*linepos+i])!=toupper(node->MatchString[i]))
        {
         (*success)=0; return; // We don't match the beginning of this string
        }
      (*success)=2;
      strcpy(expecting, node->MatchString); // Matchstring should match itself
      (*number)--;
      return;
     }

    *success = 0; // PLACEHOLDER

    if (start == NULL)
     {
      if (*success != 0)
       {
        expecting[0]   = '\0'; *ExpectingPos  =0;
        AlgebraError[0]= '\0'; *AlgebraLinepos=0;
        if ((node->VarName     != NULL) && (node->VarName[0]    != '\0'))
         {
          if ((node->VarSetVal != NULL) && (node->VarSetVal[0]  != '\0'))  DictAppendString(output , node->VarName , 0 , node->VarSetVal);
          else
           {
            if      (MatchType == DATATYPE_INT)    DictAppendInt   (output , node->VarName , 0 , MatchVal._int);
            else if (MatchType == DATATYPE_FLOAT)  DictAppendFloat (output , node->VarName , 0 , MatchVal._dbl);
            else if (MatchType == DATATYPE_STRING) DictAppendString(output , node->VarName , 0 , MatchVal._str);
           }
         }
       }
      else
       {
        if (*ExpectingPos != 0) { strcpy(expecting+*ExpectingPos, ", or "); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        if ((node->VarName != NULL) && (node->VarName[0] != '\0'))  sprintf(varname, " (%s)", node->VarName);
        else                                                        varname[0]='\0';

        if       (strcmp(node->MatchString, "%a")==0)
         { sprintf(expecting+*ExpectingPos, "an axis name%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if  (strcmp(node->MatchString, "%d")==0)
         { sprintf(expecting+*ExpectingPos, "an integer value or expression%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%e")==0) || (strcmp(node->MatchString, "%E")==0))
         { sprintf(expecting+*ExpectingPos, "an algebraic expression%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if  (strcmp(node->MatchString, "%f")==0)
         { sprintf(expecting+*ExpectingPos, "a numeric value or expression%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%s")==0) || (strcmp(node->MatchString, "%S")==0) || (strcmp(node->MatchString, "%r")==0))
         { sprintf(expecting+*ExpectingPos, "a string%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if  (strcmp(node->MatchString, "%q")==0)
         { sprintf(expecting+*ExpectingPos, "a quoted string%s", varname); (*ExpectingPos)+=strlen(expecting+*ExpectingPos); }
        else if ((strcmp(node->MatchString, "%Q")==0) || (strcmp(node->MatchString, "%v")==0))
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
      parse_descend(NodeIter, line, linepos, start, number, expecting, ExpectingPos, AlgebraError, AlgebraLinepos, output, match, success);
      if (*success==2) return;
      NodeIter = NodeIter->NextSibling;
      if (*success==0) break;
     }
   }
  else if (node->type == PN_TYPE_REP)
   {
    repeating = 1; first = 1; DictBabyList = ListInit();
    SeparatorString[0] = node->VarName[strlen(node->VarName)-1];
    if ((SeparatorString[0]!=',')&&(SeparatorString[0]!=':')) SeparatorString[0]='\0';
    SeparatorString[1]='\0';
    SeparatorNode.MatchString=SeparatorString;

    while (repeating != 0)
     {
      DictBaby = DictInit();
      LinePosOld = *linepos;
      if (output != NULL) OutputOld = DictCopy(output,1);
      if ((first==0)&&(SeparatorString[0]!='\0'))
       {
        parse_descend(&SeparatorNode, line, linepos, start, number, expecting, ExpectingPos, AlgebraError, AlgebraLinepos, output, match, success);
        if (*success==2) return;
       }
      first = 0;
      if (*success!=0)
       {
        NodeIter = node->FirstChild;
        while (NodeIter != NULL)
         {
          parse_descend(NodeIter, line, linepos, start, number, expecting, ExpectingPos, AlgebraError, AlgebraLinepos, output, match, success);
          if (*success==2) return;
          if (*success==0)
           {
            if ((output != NULL)&&(*linepos!=LinePosOld)) *output = *OutputOld; // Don't need to do another deepcopy; just overwrite top Dict struct
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
      if (repeating!=0) ListAppendDict(DictBabyList, DictBaby);
     }
    DictAppendList(output , node->VarName , 0 , DictBabyList);
   }
  else if (node->type == PN_TYPE_OPT)
   {
    LinePosOld = *linepos;
    if (output != NULL) OutputOld = DictCopy(output,1);
    NodeIter = node->FirstChild;
    while (NodeIter != NULL)
     {
      parse_descend(NodeIter, line, linepos, start, number, expecting, ExpectingPos, AlgebraError, AlgebraLinepos, output, match, success);
      if (*success==2) return;
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
          parse_descend(NodeIter, line, linepos, start, number, expecting, ExpectingPos, AlgebraError, AlgebraLinepos, output, match, success);
          if (*success==2) return;
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
      parse_descend(NodeIter, line, linepos, start, number, expecting, ExpectingPos, AlgebraError, AlgebraLinepos, output, match, success);
      if (*success==2) return;
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
   ppl_fatal(__FILE__,__LINE__,"Hit an unexpected node type.");
  return;
 }

