// dcftime.c
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

// This file is imported from within dcfmath.c, so that is can use the macros therein defined

#define _DCFMATH_C 1

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#include "ListTools/lt_memory.h"

#include "ppl_settings.h"
#include "ppl_units.h"

void SwitchOverCalDate(double *LastJulian, double *FirstGregorian)
 {
  switch (settings_term_current.CalendarIn)
   {
    case SW_CALENDAR_GREGORIAN: { *LastJulian = -HUGE_VAL;  *FirstGregorian = -HUGE_VAL; return; }
    case SW_CALENDAR_JULIAN   : { *LastJulian =  HUGE_VAL;  *FirstGregorian =  HUGE_VAL; return; }
    case SW_CALENDAR_BRITISH  : { *LastJulian = 17520902.0; *FirstGregorian = 17520914.0; return; }
    case SW_CALENDAR_FRENCH   : { *LastJulian = 15821209.0; *FirstGregorian = 15821220.0; return; }
    case SW_CALENDAR_CATHOLIC : { *LastJulian = 15821004.0; *FirstGregorian = 15821015.0; return; }
    case SW_CALENDAR_RUSSIAN  : { *LastJulian = 19180131.0; *FirstGregorian = 19180214.0; return; }
    case SW_CALENDAR_GREEK    : { *LastJulian = 19230215.0; *FirstGregorian = 19230301.0; return; }
    default                   : ppl_fatal(__FILE__,__LINE__,"Internal Error: Calendar option is set to an illegal setting.");
   }
 }

double SwitchOverJD()
 {
  switch (settings_term_current.CalendarOut)
   {
    case SW_CALENDAR_GREGORIAN: return -HUGE_VAL;
    case SW_CALENDAR_JULIAN   : return  HUGE_VAL;
    case SW_CALENDAR_BRITISH  : return 2361222.0;
    case SW_CALENDAR_FRENCH   : return 2299227.0;
    case SW_CALENDAR_CATHOLIC : return 2299161.0;
    case SW_CALENDAR_RUSSIAN  : return 2421639.0;
    case SW_CALENDAR_GREEK    : return 2423480.0;
    default                   : ppl_fatal(__FILE__,__LINE__,"Internal Error: Calendar option is set to an illegal setting.");
   }
  return 0; // Never gets here
 }

double JulianDate(int year, int month, int day, int hour, int min, int sec, int *status, char *errtext)
 {
  double JD, DayFraction, LastJulian, FirstGregorian, ReqDate;
  int b;

  SwitchOverCalDate(&LastJulian, &FirstGregorian);
  ReqDate = 10000.0*year + 100*month + day;

  if (month<=2) { month+=12; year--; }

  if (ReqDate <= LastJulian)
   { b = -2 + ((year+4716)/4) - 1179; } // Julian calendar
  else if (ReqDate >= FirstGregorian)
   { b = (year/400) - (year/100) + (year/4); } // Gregorian calendar
  else
   { *status=1; sprintf(errtext, "The requested date never happened in the %s calendar: it was lost in the transition from the Julian to the Gregorian calendar.", (char *)FetchSettingName(settings_term_current.CalendarIn, SW_CALENDAR_INT, (void **)SW_CALENDAR_STR )); }

  JD = 365.0*year - 679004.0 + 2400000.5 + b + floor(30.6001*(month+1)) + day;

  DayFraction = (fabs(hour) + fabs(min)/60.0 + fabs(sec)/3600.0) / 24.0;

  return JD + DayFraction;
 }

void InvJulianDate(double JD, int *year, int *month, int *day, int *hour, int *min, double *sec)
 {
  long a,b,c,d,e,f;
  double DayFraction;
  int temp;
  if (month == NULL) month = &temp; // Dummy placeholder, since we need month later in the calculation

  a = JD + 0.5; // Number of whole Julian days. b = Number of centuries since the Council of Nicaea. c = Julian Date as if century leap years happened.
  if (a < SwitchOverJD())
   { b=0; c=a+1524; } // Julian calendar
  else
   { b=(a-1867216.25)/36524.25; c=a+b-(b/4)+1525; } // Gregorian calendar
  d = (c-122.1)/365.25;   // Number of 365.25 periods, starting the year at the end of February
  e = 365*d + d/4; // Number of days accounted for by these
  f = (c-e)/30.6001;      // Number of 30.6001 days periods (a.k.a. months) in remainder
  if (day  != NULL) *day   = (int)floor(c-e-(int)(30.6001*f));
                    *month = (int)floor(f-1-12*(f>=14));
  if (year != NULL) *year  = (int)floor(d-4715-(*month>=3));

  DayFraction = (JD+0.5) - floor(JD+0.5);
  if (hour != NULL) *hour = (int)floor(        24*DayFraction      );
  if (min  != NULL) *min  = (int)floor(fmod( 1440*DayFraction , 60));
  if (sec  != NULL) *sec  =            fmod(86400*DayFraction , 60) ;
 }

// Wrappers for importing into PyXPlot's function table

void dcftime_juliandate(value *in1, value *in2, value *in3, value *in4, value *in5, value *in6, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_juliandate(year,month,day,hour,min,sec)";
  CHECK_6NOTNAN;
  CHECK_6INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function's first input (year) must be an integer");
  CHECK_NEEDINT(in2, "function's second input (month) must be an integer");
  CHECK_NEEDINT(in3, "function's third input (day) must be an integer");
  CHECK_NEEDINT(in4, "function's fourth input (hours) must be an integer");
  CHECK_NEEDINT(in5, "function's fifth input (minutes) must be an integer");
  CHECK_NEEDINT(in6, "function's sixth input (seconds) must be an integer");
  IF_6COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = JulianDate((int)in1->real, (int)in2->real, (int)in3->real, (int)in4->real, (int)in5->real, (int)in6->real, status, errtext); }
  ENDIF
 }

void dcftime_now(value *output, int *status, char *errtext)
 {
  time_t timer;
  struct tm timeinfo;
  timer = time(NULL);
  gmtime_r(&timer , &timeinfo);
  WRAPPER_INIT;
  output->real = JulianDate(1900+timeinfo.tm_year, 1+timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, status, errtext);
 }

void dcftime_year(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_year(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in->real, &i, NULL, NULL, NULL, NULL, NULL); }
  ENDIF
  output->real = (double)i;
 }

void dcftime_monthnum(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_monthnum(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in->real, NULL, &i, NULL, NULL, NULL, NULL); }
  ENDIF
  output->real = (double)i;
 }

void dcftime_monthname(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_monthname(JD,length)";
  int i;
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in2, "function's second input (length) must be an integer");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in1->real, NULL, &i, NULL, NULL, NULL, NULL); }
  ENDIF
  output->string = lt_malloc(16);
  output->string[0] = '\0';
  switch (i)
   {
    case  1: strcpy(output->string, "January");   break;
    case  2: strcpy(output->string, "February");  break;
    case  3: strcpy(output->string, "March");     break;
    case  4: strcpy(output->string, "April");     break;
    case  5: strcpy(output->string, "May");       break;
    case  6: strcpy(output->string, "June");      break;
    case  7: strcpy(output->string, "July");      break;
    case  8: strcpy(output->string, "August");    break;
    case  9: strcpy(output->string, "September"); break;
    case 10: strcpy(output->string, "October");   break;
    case 11: strcpy(output->string, "November");  break;
    case 12: strcpy(output->string, "December");  break;
   }
  if ((in2->real>0) && (in2->real<15)) output->string[(int)in2->real] = '\0';
 }

void dcftime_daymonth(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_daymonth(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in->real, NULL, NULL, &i, NULL, NULL, NULL); }
  ENDIF
  output->real = (double)i;
 }

void dcftime_dayweeknum(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_dayweeknum(JD)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor( fmod(in->real+0.5 , 7) + 1); }
  ENDIF
 }

void dcftime_dayweekname(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_dayweekname(JD,length)";
  int i;
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in2, "function's second input (length) must be an integer");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { i = floor( fmod(in1->real+0.5 , 7) ); }
  ENDIF
  output->string = lt_malloc(16);
  output->string[0] = '\0';
  switch (i)
   {
    case  0: strcpy(output->string, "Monday");    break;
    case  1: strcpy(output->string, "Tuesday");   break;
    case  2: strcpy(output->string, "Wednesday"); break;
    case  3: strcpy(output->string, "Thursday");  break;
    case  4: strcpy(output->string, "Friday");    break;
    case  5: strcpy(output->string, "Saturday");  break;
    case  6: strcpy(output->string, "Sunday");    break;
   }
  if ((in2->real>0) && (in2->real<15)) output->string[(int)in2->real] = '\0';
 }

void dcftime_hour(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_hour(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in->real, NULL, NULL, NULL, &i, NULL, NULL); }
  ENDIF
  output->real = (double)i;
 }

void dcftime_min(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_min(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in->real, NULL, NULL, NULL, NULL, &i, NULL); }
  ENDIF
  output->real = (double)i;
 }

void dcftime_sec(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_sec(JD)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDate(in->real, NULL, NULL, NULL, NULL, NULL, &(output->real)); }
  ENDIF
 }

void dcftimediff_years(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "timediff_years(JD1,JD2)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor((in2->real - in1->real) / 365); }
  ENDIF
 }

void dcftimediff_days(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "timediff_days(JD1,JD2,wrap)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor(in2->real - in1->real); }
  ENDIF
  if (!ppl_units_DblEqual(in3->real, 0)) output->real = fmod(output->real, 365);
 }

void dcftimediff_hours(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "timediff_hours(JD1,JD2,wrap)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor((in2->real - in1->real)*24); }
  ENDIF
  if (!ppl_units_DblEqual(in3->real, 0)) output->real = fmod(output->real, 24);
 }

void dcftimediff_minutes(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "timediff_mins(JD1,JD2,wrap)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor((in2->real - in1->real)*1440); }
  ENDIF
  if (!ppl_units_DblEqual(in3->real, 0)) output->real = fmod(output->real, 60);
 }

void dcftimediff_seconds(value *in1, value *in2, value *in3, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "timediff_secs(JD1,JD2,wrap)";
  CHECK_3NOTNAN;
  CHECK_3INPUT_DIMLESS;
  IF_3COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor((in2->real - in1->real)*86400); }
  ENDIF
  if (!ppl_units_DblEqual(in3->real, 0)) output->real = fmod(output->real, 60);
 }

void dcftime_ordinal(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "ordinal(number)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  CHECK_NEEDINT(in, "function's input must be an integer");
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL
   {
    i = (int)in->real;
    output->string = lt_malloc(16);
    if      (((i%100)<21) && ((i%100)>3)) sprintf(output->string, "%dth", i);
    else if  ((i% 10)==1)                 sprintf(output->string, "%dst", i);
    else if  ((i% 10)==2)                 sprintf(output->string, "%dnd", i);
    else if  ((i% 10)==3)                 sprintf(output->string, "%drd", i);
    else                                  sprintf(output->string, "%dth", i);
   }
  ENDIF
 }

