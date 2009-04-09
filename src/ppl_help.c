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

void directive_help(Dict *command, int interactive)
 {
  List    *TopicWords   = NULL;
  char    *TopicString  = NULL;
  char     filename[FNAME_LENGTH];
  char     HelpPosition[10][SSTR_LENGTH];
  xmlDoc  *doc          = NULL;
  xmlNode *root_element = NULL;
  int      depth;


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

  root_element = xmlDocGetRootElement(doc);
  sprintf(temp_err_string, "Root element name %s.", root_element->name); ppl_report(temp_err_string);

  xmlFreeDoc(doc); // Tidy up
  xmlCleanupParser();
  return;
 }

