// ppl_help.c
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
#include <unistd.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "StringTools/asciidouble.h"
#include "StringTools/str_constants.h"

#include "ListTools/lt_dict.h"
#include "ListTools/lt_list.h"
#include "ListTools/lt_StringProc.h"

#include "ppl_constants.h"
#include "ppl_error.h"
#include "ppl_settings.h"

#define MAX_HELP_DEPTH 16 // The maximum possible depth of a help hierarchy
#define MAX_HELP_HITS  12 // The maximum possible number of help topics which match the same autocomplete strings

void help_PagerDisplay(char *PageName, xmlNode *node, int interactive)
 {
  xmlNode *cur_node;
  char     TextBuffer      [LSTR_LENGTH];
  char     TextBuffer2     [LSTR_LENGTH];
  char     DefaultPagerName[] = "less";
  char    *PagerName;
  char    *Ncolumns_text;
  int      i, Ncolumns, Nchildren;
  FILE    *PagerHandle;

  sprintf(TextBuffer,"\\\\**** Help Topic: %s****\\\\%s\\\\\\\\\\\\", PageName, node->content);
  i = strlen(TextBuffer);

  // Insert information about children
  Nchildren = 0;
  for (cur_node = node->children; cur_node; cur_node = cur_node->next) // Loop over children
   {
    if (Nchildren == 0) { sprintf(TextBuffer+i,"This help page has the following subtopics:\\\\\\\\"); i += strlen(TextBuffer+i); }
    else                { sprintf(TextBuffer+i,", ");                                                  i += strlen(TextBuffer+i); }
    sprintf(TextBuffer+i,"%s",cur_node->name); i += strlen(TextBuffer+i);
    Nchildren++;
   }
  if (Nchildren == 0) { sprintf(TextBuffer+i,"This help page has no subtopics.\\\\\\\\"); i += strlen(TextBuffer+i); }
  else                { sprintf(TextBuffer+i,".\\\\\\\\");                                i += strlen(TextBuffer+i); }

  // If interactive, tell user how to quit, and also work out column width of display
  if (interactive!=0)
   {
    sprintf(TextBuffer+i,"Press the 'Q' key to exit this help page.\\\\");
    Ncolumns_text = getenv("COLUMNS");
    if (!((Ncolumns_text != NULL) && (Ncolumns = GetFloat(Ncolumns_text, &i), i==strlen(Ncolumns_text)))) Ncolumns = 80;
    PagerName = getenv("PAGER");
    if (PagerName == NULL) PagerName = DefaultPagerName;
   } else {
    Ncolumns = 80;
    PagerName = DefaultPagerName;
   }

  // Word wrap the text we have, and send it to a pager
  StrWordWrap(TextBuffer, TextBuffer2, Ncolumns);
  if (interactive == 0)
   {
    ppl_report(TextBuffer2); // In non-interactive sessions, we just send text to stdout
   }
  else
   {
    if ((PagerHandle = popen(PagerName,"w"))==NULL) { ppl_error("Cannot open pipe to pager application."); }
    else
     {
      fprintf(PagerHandle, "%s", TextBuffer2);
      pclose(PagerHandle);
     }
   }
  return;
 }

void help_TopicPrint(char *out, char **words, int Nwords)
 {
  int i,j;
  for (i=0,j=0; i<=Nwords; i++)
   {
    strcpy(out+j, words[i]);
    j += strlen(out+j);
    out[j++]=' ';
   }
  out[j]='\0';
 }

void help_MatchFound(xmlNode *node, xmlNode **MatchNode, List *TopicWords, int *MatchTextPos, int *Nmatches, int *ambiguous, char **HelpPosition, char **HelpTexts)
 {
  int   abbreviation,i;
  List *PrevMatchPosition;

  if ((*Nmatches)==MAX_HELP_HITS) { ppl_error("Error whilst parsing ppl_help.xml. Need to increase MAX_HELP_HITS."); return; }
  help_TopicPrint(HelpTexts[*Nmatches], HelpPosition, ListLen(TopicWords));
  if (*ambiguous == 0)
   {
    if (*MatchNode == NULL)
     {
      *MatchTextPos = 0;
      *MatchNode    = node; // This is the first match we've found
     } else {
      PrevMatchPosition = StrSplit( HelpTexts[*MatchTextPos] );

      abbreviation = 1; // If previous match is an autocomplete shortening of current match, let it stand
      for (i=0; i<=ListLen(TopicWords); i++) if (StrAutocomplete( (char *)ListGetItem(PrevMatchPosition,i) , HelpPosition[i] , 1)==-1) {abbreviation=0; break;}
      if (abbreviation!=1)
       {
        abbreviation = 1; // If current match is an autocomplete shortening of previous match, the current match wins
        for (i=0; i<=ListLen(TopicWords); i++) if (StrAutocomplete( HelpPosition[i] , (char *)ListGetItem(PrevMatchPosition,i) , 1)==-1) {abbreviation=0; break;}
        if (abbreviation==1)
         {
          *MatchTextPos = *Nmatches;
          *MatchNode    = node;
         } else {
          *ambiguous = 1; // We have multiple ambiguous matches
         }
       }
     }
   }
  (*Nmatches)++;
  return;
 }

void help_explore(xmlNode *node, xmlNode **MatchNode, List *TopicWords, int *MatchTextPos, int *Nmatches, int *ambiguous, char **HelpPosition, char **HelpTexts, int depth)
 {
  xmlNode *cur_node = NULL;
  int match,i;

  if (depth>ListLen(TopicWords)) return;
  if (depth==MAX_HELP_DEPTH) { ppl_error("Error whilst parsing ppl_help.xml. Need to increase MAX_HELP_DEPTH."); return; }

  for (cur_node = node; cur_node; cur_node = cur_node->next) // Loop over siblings
   {
    match=1;
    if (cur_node->type == XML_ELEMENT_NODE)
     {
      sprintf(HelpPosition[depth], "%s", cur_node->name); // Converted signedness of chars
      for (i=1;i<=depth;i++) if (StrAutocomplete( (char *)ListGetItem(TopicWords,i-1) , HelpPosition[i], 1)==-1) {match=0; break;}
      if ((match==1) && (depth==ListLen(TopicWords))) help_MatchFound(cur_node,MatchNode,TopicWords,MatchTextPos,Nmatches,ambiguous,HelpPosition,HelpTexts);
     }
    if (match==1) help_explore(cur_node->children,MatchNode,TopicWords,MatchTextPos,Nmatches,ambiguous,HelpPosition,HelpTexts,depth+1);
   }
  return;
 }

void directive_help(Dict *command, int interactive)
 {
  List    *TopicWords   = NULL; // A list of the help topic words supplied by the user
  char    *TopicString  = NULL; // The string of help topic words supplied by the user
  char     filename[FNAME_LENGTH]; // The filename of ppl_help.xml
  char    *HelpPosition[MAX_HELP_DEPTH]; // A list of xml tags, used to keep track of our position as we traverse the xml hierarchy
  char    *HelpTexts   [MAX_HELP_HITS ]; // A list of all of the help topics which have matched the user's request
  xmlDoc  *doc          = NULL; // The XML document ppl_help.xml
  xmlNode *root_element = NULL; // The root element of the above
  xmlNode *MatchNode    = NULL; // The XML node which best fits the user's request
  int      MatchTextPos = -1; // The position within HelpTexts of the node pointed to by MatchNode
  int      ambiguous    = 0; // Becomes one if we find that the user's request matches multiple help pages
  int      Nmatches     = 0; // Counts the number of pages the user's request matches
  int      i;

  DictLookup(command, "topic", NULL, NULL, (void **)&TopicString); // Make a list of the requested topic words in TopicWords
  TopicWords = StrSplit( TopicString );

  sprintf(filename, "%s%s%s", SRCDIR, PATHLINK, "ppl_help.xml"); // Find ppl_help.xml
  LIBXML_TEST_VERSION

  doc = xmlReadFile(filename, NULL, 0);

  if (doc==NULL)
   {
    sprintf(temp_err_string, "Help command cannot find help data in expected location of '%s'.", filename);
    ppl_error(temp_err_string);
    return;
   }

  for (i=0;i<MAX_HELP_DEPTH;i++) HelpPosition[i]=(char *)malloc(SSTR_LENGTH*sizeof(char));
  for (i=0;i<MAX_HELP_HITS ;i++) HelpTexts   [i]=(char *)malloc(SSTR_LENGTH*sizeof(char));

  root_element = xmlDocGetRootElement(doc);
  help_explore(root_element, &MatchNode, TopicWords, &MatchTextPos, &Nmatches, &ambiguous, HelpPosition, HelpTexts, 0);

  if (ambiguous == 1)
   {
    ppl_report("Ambiguous help request. The following help topics were matched:");
    for(i=0;i<Nmatches;i++) ppl_report( HelpTexts[i] );
    ppl_report("Please make your help request more specific, and try again.");
   }
  else if (MatchNode == NULL)
   {
    ppl_report("Please make your help request more specific, and try again.");
   }
  else
   {
    help_PagerDisplay( HelpTexts[MatchTextPos] , MatchNode , interactive );
   }

  for (i=0;i<MAX_HELP_DEPTH;i++) free(HelpPosition[i]);
  for (i=0;i<MAX_HELP_HITS ;i++) free(HelpTexts   [i]);

  xmlFreeDoc(doc); // Tidy up
  xmlCleanupParser();
  return;
 }

