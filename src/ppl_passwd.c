// ppl_passwd.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-8 Dominic Ford <coders@pyxplot.org.uk>
//               2008   Ross Church
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

// Functions for getting information out of /etc/passwd

#include <unistd.h>
#include <pwd.h>

#include "ppl_error.h"

struct passwd *UnixGetPwEntry()
 {
  int uid;
  struct passwd *ptr;

  uid = getuid();
  setpwent();
  while ((ptr = getpwent()) != NULL)
   if (ptr->pw_uid == uid) break;
  endpwent();
  return(ptr);
 }

char *UnixGetHomeDir()
 {
  struct passwd *ptr;
  ptr = UnixGetPwEntry();
  if (ptr==NULL) ppl_fatal(__FILE__,__LINE__,"Could not fild user's entry in /etc/passwd file.");
  return ptr->pw_dir;
 }

char *UnixGetUserIRLName()
 {
  struct passwd *ptr;
  ptr = UnixGetPwEntry();
  if (ptr==NULL) ppl_fatal(__FILE__,__LINE__,"Could not fild user's entry in /etc/passwd file.");
  return ptr->pw_gecos;
 }

