// dcftime.c
//
// The code in this file is part of PyXPlot
// <http://www.pyxplot.org.uk>
//
// Copyright (C) 2006-2010 Dominic Ford <coders@pyxplot.org.uk>
//               2008-2010 Ross Church
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

#include "StringTools/str_constants.h"

#include "ppl_settings.h"
#include "ppl_setting_types.h"
#include "ppl_units.h"
#include "ppl_units_fns.h"
#include "ppl_userspace.h"

#include "dcftime.h"

#include "dcfmath_macros.h"

// Routines for looking up the dates when the transition between the Julian calendar and the Gregorian calendar occurred

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

// Functions for looking up the names of the Nth calendar month and the Nth day of the week

char *GetMonthName(int i)
 {
  switch (settings_term_current.CalendarOut)
   {
    case SW_CALENDAR_HEBREW:
    switch (i)
     {
      case  1: return "Tishri";
      case  2: return "Heshvan";
      case  3: return "Kislev";
      case  4: return "Tevet";
      case  5: return "Shevat";
      case  6: return "Adar";
      case  7: return "Veadar";
      case  8: return "Nisan";
      case  9: return "Iyar";
      case 10: return "Sivan";
      case 11: return "Tammuz";
      case 12: return "Av";
      case 13: return "Elul";
      default: return "???";
     }
    case SW_CALENDAR_ISLAMIC:
    switch (i)
     {
      case  1: return "Muharram";
      case  2: return "Safar";
      case  3: return "Rabi'al-Awwal";
      case  4: return "Rabi'ath-Thani";
      case  5: return "Jumada l-Ula";
      case  6: return "Jumada t-Tania";
      case  7: return "Rajab";
      case  8: return "Sha'ban";
      case  9: return "Ramadan";
      case 10: return "Shawwal";
      case 11: return "Dhu l-Qa'da";
      case 12: return "Dhu l-Hijja";
      default: return "???";
     }
    default:
    switch (i)
     {
      case  1: return "January";
      case  2: return "February";
      case  3: return "March";
      case  4: return "April";
      case  5: return "May";
      case  6: return "June";
      case  7: return "July";
      case  8: return "August";
      case  9: return "September";
      case 10: return "October";
      case 11: return "November";
      case 12: return "December";
      default: return "???";
     }
   }
  return "???";
 }

char *GetWeekDayName(int i)
 {
  switch (i)
   {
    case  0: return "Monday";
    case  1: return "Tuesday";
    case  2: return "Wednesday";
    case  3: return "Thursday";
    case  4: return "Friday";
    case  5: return "Saturday";
    case  6: return "Sunday";
   }
  return "???";
 }

// Routines for converting between Julian Day numbers and the Hebrew Calendar

const int HebrewMonthLengths[6][13] = { {30,29,29,29,30,29, 0,30,29,30,29,30,29},    // Deficient Common Year
                                        {30,29,30,29,30,29, 0,30,29,30,29,30,29},    // Regular   Common Year
                                        {30,30,30,29,30,29, 0,30,29,30,29,30,29},    // Complete  Common Year
                                        {30,29,29,29,30,30,29,30,29,30,29,30,29},    // Deficient Embolismic Year
                                        {30,29,30,29,30,30,29,30,29,30,29,30,29},    // Regular   Embolismic Year
                                        {30,30,30,29,30,30,29,30,29,30,29,30,29} };  // Complete  Embolismic Year

void GetHebrewNewYears(int GregYear, int *YearNumbers, double *JDs, int *YearTypes)
 {
  int i;
  long X,C,S,A,a,b,j,D,MONTH=3;
  double Q, JD, r;

  for (i=0; i<3; i++) // Return THREE new year numbers and Julian Day numbers, and YearTypes for the first two.
   {
    X = GregYear-1+i;
    C = X/100;
    S = (3*C-5)/4;
    A = X + 3760;
    a = (12*X+12)%19; // Work out the date of the passover in the Hebrew year A
    b = X%4;
    Q = -1.904412361576 + 1.554241796621*a + 0.25*b - 0.003177794022*X + S;
    j = (((long)Q) + 3*X + 5*b + 2 - S)%7;
    r = Q-((long)Q);

    if      ((j==2)||(j==4)||(j==6)          ) D = ((long)Q)+23;
    else if ((j==1)&&(a> 6)&&(r>=0.632870370)) D = ((long)Q)+24;
    else if ((j==0)&&(a>11)&&(r>=0.897723765)) D = ((long)Q)+23;
    else                                       D = ((long)Q)+22; // D is day when 15 Nisan falls after 1st March in the Gregorian year X

    b = (X/400) - (X/100) + (X/4); // Gregorian calendar
    JD = 365.0*X - 679004.0 + 2400000.5 + b + floor(30.6001*(MONTH+1)) + D;

    YearNumbers[i] = A  +   1;
    JDs        [i] = JD + 163; // 1 Tishri (New Year) for next year falls 163 days after 15 Nisan

    a = (A+1)%19;
    if ((a==0)||(a==3)||(a==6)||(a==8)||(a==11)||(a==14)||(a==17)) YearTypes[i] = 3;
    else                                                           YearTypes[i] = 0;
   }

  // We've so far worked out whether years are Common or Embolismic. Now need to work out how many days there will be in each year.
  for (i=0; i<2; i++)
   {
    if (YearTypes[i]==0) YearTypes[i] =     JDs[i+1]-JDs[i]-353;
    else                 YearTypes[i] = 3 + JDs[i+1]-JDs[i]-383;

    if ((YearTypes[i]<0)||(YearTypes[i]>6)) YearTypes[i]=0; // Something very bad has happened.
   }
  return;
 }

double JulianDayHebrew(int year, int month, int day, int hour, int min, int sec, int *status, char *errtext)
 {
  int    i;
  int    YearNumbers[3], YearTypes[3];
  double JD, DayFraction, JDs[3];

  if ((month<1)||(month>13)) { *status=1; sprintf(errtext, "Supplied month number should be in the range 1-13."); return 0.0; }
  if ((year <1)            ) { *status=1; sprintf(errtext, "Supplied year number must be positive for the Hebrew calendar; the calendar is undefined prior to 4760 BC, corresponding to Hebrew year AM 1."); return 0.0; }

  GetHebrewNewYears(year-3760, YearNumbers, JDs, YearTypes);
  JD = JDs[0];
  for (i=0; i<month-1; i++) JD += HebrewMonthLengths[ YearTypes[0] ][ i ];
  if (day>HebrewMonthLengths[ YearTypes[0] ][ i ]) { *status=1; sprintf(errtext, "Supplied day number in the Hebrew month of %s in the year AM %d must be in the range 1-%d.", GetMonthName(month), year, HebrewMonthLengths[ YearTypes[1] ][ i ]); return 0.0; } 
  JD += day-1;
  DayFraction = (fabs(hour) + fabs(min)/60.0 + fabs(sec)/3600.0) / 24.0;
  return JD + DayFraction;
 }

void InvJulianDayHebrew(double JD, int *year, int *month, int *day, int *hour, int *min, double *sec, int *status, char *errtext)
 {
  long   a,b,c,d,e,f;
  int    i,j;
  int    JulDay, JulMon, JulYr;
  int    YearNumbers[3], YearTypes[3];
  double JDs[3];

  // First work out date in Julian calendar
  a = JD + 0.5; // Number of whole Julian days. b = Number of centuries since the Council of Nicaea. c = Julian Day number as if century leap years happened.
  b=0; c=a+1524;
  d = (c-122.1)/365.25;   // Number of 365.25 periods, starting the year at the end of February
  e = 365*d + d/4; // Number of days accounted for by these
  f = (c-e)/30.6001;      // Number of 30.6001 days periods (a.k.a. months) in remainder
  JulDay = (int)floor(c-e-(int)(30.6001*f));
  JulMon = (int)floor(f-1-12*(f>=14));
  JulYr  = (int)floor(d-4715-(JulMon>=3));

  GetHebrewNewYears(JulYr, YearNumbers, JDs, YearTypes);
  i = (JD<JDs[1]) ? 0 : 1;
  JD -= JDs[i];
  for (j=0;j<13;j++) if (JD>=HebrewMonthLengths[ YearTypes[i] ][ j ]) JD -= HebrewMonthLengths[ YearTypes[i] ][ j ]; else break;
  if (year != NULL) *year  = YearNumbers[i];
  if (month!= NULL) *month = j+1;
  if (day  != NULL) *day   = ((int)JD+1);
  return;
 }

// Routines for converting between Julian Day numbers and the Islamic Calendar

double JulianDayIslamic(int year, int month, int day, int hour, int min, int sec, int *status, char *errtext)
 {
  long   N,Q,R,A,W,Q1,Q2,G,K,E,J,X,JD;
  double DayFraction;

  if ((month<1)||(month>12)) { *status=1; sprintf(errtext, "Supplied month number should be in the range 1-12."); return 0.0; }
  if ((year <1)            ) { *status=1; sprintf(errtext, "Supplied year number must be positive for the Islamic calendar; the calendar is undefined prior to AD 622 Jul 18, corresponding to AH 1 Muh 1."); return 0.0; }

  N  = day + (long)(29.5001*(month-1)+0.99);
  Q  = year/30;
  R  = year%30;
  A  = (long)((11*R+3)/30);
  W  = 404*Q+354*R+208+A;
  Q1 = W/1461;
  Q2 = W%1461;
  G  = 621 + 4*((long)(7*Q+Q1));
  K  = Q2/365.2422;
  E  = 365.2422*K;
  J  = Q2-E+N-1;
  X  = G+K;

  if      ((J>366) && (X%4==0)) { J-=366; X++; }
  else if ((J>365) && (X%4 >0)) { J-=365; X++; }

  JD = 365.25*(X-1) + 1721423 + J;

  DayFraction = (fabs(hour) + fabs(min)/60.0 + fabs(sec)/3600.0) / 24.0;

  return JD + DayFraction - 0.5;
 }

void InvJulianDayIslamic(double JD, int *year, int *month, int *day, int *hour, int *min, double *sec, int *status, char *errtext)
 {
  long a,b,c,d,e,f;
  int  JulDay, JulMon, JulYr;
  long alpha,beta,W,N,A,B,C,C2,D,Q,R,J,K,O,H,JJ,CL,DL,S,m;
  double C1;

  if (JD<1948439.5) { *status=1; sprintf(errtext, "Supplied year number must be positive for the Islamic calendar; the calendar is undefined prior to AD 622 Jul 18, corresponding to AH 1 Muh 1."); return; }

  // First work out date in Julian calendar
  a = JD + 0.5; // Number of whole Julian days. b = Number of centuries since the Council of Nicaea. c = Julian Day number as if century leap years happened.
  b=0; c=a+1524;
  d = (c-122.1)/365.25;   // Number of 365.25 periods, starting the year at the end of February
  e = 365*d + d/4; // Number of days accounted for by these
  f = (c-e)/30.6001;      // Number of 30.6001 days periods (a.k.a. months) in remainder
  JulDay = (int)floor(c-e-(int)(30.6001*f));
  JulMon = (int)floor(f-1-12*(f>=14));
  JulYr  = (int)floor(d-4715-(JulMon>=3));

  alpha = (JD-1867216.25)/36524.25; // See pages 75-76 of "Astronomical Algorithms", by Jean Meeus

  if (JD<2299161)  beta = (JD+0.5);
  else             beta = (JD+0.5) + 1 + alpha - ((long)(alpha/4));

  c  = (b-122.1)/365.25;
  d  = 365.25*c;
  e  = (b-d)/30.6001;

  W  = 2-(JulYr%4==0);
  N  = ((long)((275*JulMon)/9)) - W*((long)((JulMon+9)/12)) + JulDay - 30;
  A  = JulYr-623;
  B  = A/4;
  C  = A%4;
  C1 = 365.2501*C;
  C2 = C1;

  if (C1-C2>0.5) C2++;

  D  = 1461*B+170+C2;
  Q  = D/10631;
  R  = D%10631;
  J  = R/354;
  K  = R%354;
  O  = (11*J+14)/30;
  H  = 30*Q+J+1;
  JJ = K-O+N-1;

  if (JJ>354)
   {
    CL = H%30;
    DL = (11*CL+3)%30;
    if      (DL<19) { JJ-=354; H++; }
    else if (DL>18) { JJ-=355; H++; }
    if (JJ==0) { JJ=355; H--; }
   }

  S = (JJ-1)/29.5;
  m = 1+S;
  d = JJ-29.5*S;

  if (JJ==355) { m=12; d=30; }

  if (year !=NULL) *year  = (int)H;
  if (month!=NULL) *month = (int)m;
  if (day  !=NULL) *day   = (int)d;
  return;
 }

// Routines for converting between Julian Day numbers and Calendar Dates in Gregorian and Julian calendars

double JulianDay(int year, int month, int day, int hour, int min, int sec, int *status, char *errtext)
 {
  double JD, DayFraction, LastJulian, FirstGregorian, ReqDate;
  int b;

  if ((year<-1e6)||(year>1e6)||(!gsl_finite(year))) { *status=1; sprintf(errtext, "Supplied year is too big."); return 0.0; }
  if ((day  <1)||(day  >31)) { *status=1; sprintf(errtext, "Supplied day number should be in the range 1-31."); return 0.0; }
  if ((hour <0)||(hour >23)) { *status=1; sprintf(errtext, "Supplied hour number should be in the range 0-23."); return 0.0; }
  if ((min  <0)||(min  >59)) { *status=1; sprintf(errtext, "Supplied minute number should be in the range 0-59."); return 0.0; }
  if ((sec  <0)||(sec  >59)) { *status=1; sprintf(errtext, "Supplied second number should be in the range 0-59."); return 0.0; }

  if      (settings_term_current.CalendarIn == SW_CALENDAR_HEBREW ) return JulianDayHebrew (year, month, day, hour, min, sec, status, errtext);
  else if (settings_term_current.CalendarIn == SW_CALENDAR_ISLAMIC) return JulianDayIslamic(year, month, day, hour, min, sec, status, errtext);

  if ((month<1)||(month>12)) { *status=1; sprintf(errtext, "Supplied month number should be in the range 1-12."); return 0.0; }

  SwitchOverCalDate(&LastJulian, &FirstGregorian);
  ReqDate = 10000.0*year + 100*month + day;

  if (month<=2) { month+=12; year--; }

  if (ReqDate <= LastJulian)
   { b = -2 + ((year+4716)/4) - 1179; } // Julian calendar
  else if (ReqDate >= FirstGregorian)
   { b = (year/400) - (year/100) + (year/4); } // Gregorian calendar
  else
   { *status=1; sprintf(errtext, "The requested date never happened in the %s calendar: it was lost in the transition from the Julian to the Gregorian calendar.", *(char **)FetchSettingName(settings_term_current.CalendarIn, SW_CALENDAR_INT, (void *)SW_CALENDAR_STR, sizeof(char *))); return 0.0; }

  JD = 365.0*year - 679004.0 + 2400000.5 + b + floor(30.6001*(month+1)) + day;

  DayFraction = (fabs(hour) + fabs(min)/60.0 + fabs(sec)/3600.0) / 24.0;

  return JD + DayFraction;
 }

void InvJulianDay(double JD, int *year, int *month, int *day, int *hour, int *min, double *sec, int *status, char *errtext)
 {
  long a,b,c,d,e,f;
  double DayFraction;
  int temp;
  if (month == NULL) month = &temp; // Dummy placeholder, since we need month later in the calculation

  if ((JD<-1e8)||(JD>1e8)||(!gsl_finite(JD))) { *status=1; sprintf(errtext, "Supplied Julian Day number is too big."); return; }

  // Work out hours, minutes and seconds
  DayFraction = (JD+0.5) - floor(JD+0.5);
  if (hour != NULL) *hour = (int)floor(        24*DayFraction      );
  if (min  != NULL) *min  = (int)floor(fmod( 1440*DayFraction , 60));
  if (sec  != NULL) *sec  =            fmod(86400*DayFraction , 60) ;

  // Now work out calendar date
  if      (settings_term_current.CalendarOut == SW_CALENDAR_HEBREW ) return InvJulianDayHebrew (JD, year, month, day, hour, min, sec, status, errtext);
  else if (settings_term_current.CalendarOut == SW_CALENDAR_ISLAMIC) return InvJulianDayIslamic(JD, year, month, day, hour, min, sec, status, errtext);

  a = JD + 0.5; // Number of whole Julian days. b = Number of centuries since the Council of Nicaea. c = Julian Day number as if century leap years happened.
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

  return;
 }

// Wrappers for importing into PyXPlot's function table

void dcftime_julianday(value *in1, value *in2, value *in3, value *in4, value *in5, value *in6, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_julianday(year,month,day,hour,min,sec)";
  CHECK_6NOTNAN;
  CHECK_6INPUT_DIMLESS;
  CHECK_NEEDINT(in1, "function's first input (year) must be an integer");
  CHECK_NEEDINT(in2, "function's second input (month) must be an integer");
  CHECK_NEEDINT(in3, "function's third input (day) must be an integer");
  CHECK_NEEDINT(in4, "function's fourth input (hours) must be an integer");
  CHECK_NEEDINT(in5, "function's fifth input (minutes) must be an integer");
  CHECK_NEEDINT(in6, "function's sixth input (seconds) must be an integer");
  IF_6COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = JulianDay((int)in1->real, (int)in2->real, (int)in3->real, (int)in4->real, (int)in5->real, (int)in6->real, status, errtext); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcftime_now(value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_now()";
  time_t timer;
  timer = time(NULL);
  WRAPPER_INIT;
  output->real = 2440587.5 + ((double)timer)/86400;
  CHECK_OUTPUT_OKAY;
 }

void dcftime_year(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_year(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in->real, &i, NULL, NULL, NULL, NULL, NULL, status, errtext); }
  ENDIF
  output->real = (double)i;
  CHECK_OUTPUT_OKAY;
 }

void dcftime_monthnum(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_monthnum(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in->real, NULL, &i, NULL, NULL, NULL, NULL, status, errtext); }
  ENDIF
  output->real = (double)i;
  CHECK_OUTPUT_OKAY;
 }

void dcftime_monthname(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_monthname(JD,length)";
  int i;
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  CHECK_NEEDINT(in2, "function's second input (length) must be an integer");
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in1->real, NULL, &i, NULL, NULL, NULL, NULL, status, errtext); }
  ENDIF
  output->string = lt_malloc(16);
  output->string[0] = '\0';
  strcpy(output->string, GetMonthName(i));
  if ((in2->real>0) && (in2->real<15)) output->string[(int)in2->real] = '\0';
 }

void dcftime_daymonth(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_daymonth(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in->real, NULL, NULL, &i, NULL, NULL, NULL, status, errtext); }
  ENDIF
  output->real = (double)i;
  CHECK_OUTPUT_OKAY;
 }

void dcftime_dayweeknum(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_dayweeknum(JD)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = floor( fmod(in->real+0.5 , 7) + 1); }
  ENDIF
  CHECK_OUTPUT_OKAY;
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
  strcpy(output->string, GetWeekDayName(i));
  if ((in2->real>0) && (in2->real<15)) output->string[(int)in2->real] = '\0';
 }

void dcftime_hour(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_hour(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in->real, NULL, NULL, NULL, &i, NULL, NULL, status, errtext); }
  ENDIF
  output->real = (double)i;
  CHECK_OUTPUT_OKAY;
 }

void dcftime_min(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_min(JD)";
  int i;
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in->real, NULL, NULL, NULL, NULL, &i, NULL, status, errtext); }
  ENDIF
  output->real = (double)i;
  CHECK_OUTPUT_OKAY;
 }

void dcftime_sec(value *in, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "time_sec(JD)";
  CHECK_1NOTNAN;
  CHECK_1INPUT_DIMLESS;
  IF_1COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { InvJulianDay(in->real, NULL, NULL, NULL, NULL, NULL, &(output->real), status, errtext); }
  ENDIF
  CHECK_OUTPUT_OKAY;
 }

void dcftime_string(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  value   JD1;
  char   *FormatString;
  int     i=0,j=0,k=0;
  int     year, month, day, hour, min;
  double  sec;

  *status=-1;
  ppl_units_zero(output);
  inlen--; // Make inlen point to last character
  while ((in[i]!='\0')&&(in[i]<=' ')) i++; // Strip spaces off front

  // Fetch input Julian Day number
  j=-1;
  ppl_EvaluateAlgebra(in+i, &JD1, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1);
  if (*status >= 0) { (*status) += i; return; }
  i+=j;
  while ((in[i]>'\0')&&(in[i]<=' ')) i++;

  // Fetch format string
  if (in[i] != ',')
   {
    if (in[i] ==')') { FormatString = "%a %Y %b %d %H:%M:%S"; }
    else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; }
   } else {
    i++; j=-1;
    FormatString = lt_malloc(LSTR_LENGTH);
    ppl_GetQuotedString(in+i, FormatString, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1);
    if (*status >= 0) { (*status) += i; return; }
    i+=j;
    while ((in[i]>'\0')&&(in[i]<=' ')) i++;
   }

  // Check for closing bracket
  if (in[i] != ')')
   {
    *status = i;
    if (in[i] == ',') { strcpy(errtext,"Syntax Error: Too many arguments supplied to function."); }
    else              { strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); }
    return;
   }

  // Check that input is real
  if (JD1.FlagComplex) { *status=0; strcpy(errtext,"The time_string() function can only act upon real times. The supplied Julian Day number is complex."); return; }
  if (!JD1.dimensionless) { *status=0; sprintf(errtext,"The time_string() function can only act upon dimensionless times. The supplied Julian Day number has units of <%s>.", ppl_units_GetUnitStr(&JD1,NULL,NULL,0,0)); return; }

  *status=0;
  InvJulianDay(JD1.real, &year, &month, &day, &hour, &min, &sec, status, errtext);
  if (*status) { *status=0; return; }
  *status=-1;

  // Loop through format string making substitutions
  output->string = lt_malloc(LSTR_LENGTH);
  for (j=0; FormatString[j]!='\0'; j++)
   {
    if (FormatString[j]!='%') { output->string[k++] = FormatString[j]; continue; }
    switch (FormatString[j+1])
     {
      case '%': sprintf(output->string+k, "%%"); break;
      case 'a': sprintf(output->string+k, "%s", GetWeekDayName( floor( fmod(JD1.real+0.5 , 7) ))); output->string[k+3]='\0'; break;
      case 'A': sprintf(output->string+k, "%s" , GetWeekDayName( floor( fmod(JD1.real+0.5 , 7) ))); break;
      case 'b': sprintf(output->string+k, "%s", GetMonthName(month)); output->string[k+3]='\0'; break;
      case 'B': sprintf(output->string+k, "%s" , GetMonthName(month)); break;
      case 'C': sprintf(output->string+k, "%d", (year/100)+1); break;
      case 'd': sprintf(output->string+k, "%d", day); break;
      case 'H': sprintf(output->string+k, "%02d", hour); break;
      case 'I': sprintf(output->string+k, "%02d", ((hour-1)%12)+1); break;
      case 'k': sprintf(output->string+k, "%d", hour); break;
      case 'l': sprintf(output->string+k, "%d", ((hour-1)%12)+1); break;
      case 'm': sprintf(output->string+k, "%02d", month); break;
      case 'M': sprintf(output->string+k, "%02d", min); break;
      case 'p': sprintf(output->string+k, "%s", (hour<12)?"am":"pm"); break;
      case 'S': sprintf(output->string+k, "%02d", (int)sec); break;
      case 'y': sprintf(output->string+k, "%d", year%100); break;
      case 'Y': sprintf(output->string+k, "%d", year); break;
      default: { *status=0; sprintf(errtext,"Format string supplied to time_string() function contains unrecognised substitution token '%%%c'.",FormatString[j+1]); return; }
     }
    j++;
    k += strlen(output->string + k);
   }
  output->string[k]='\0'; // Null terminate string
 }

void dcftime_diff_string(char *in, int inlen, value *output, unsigned char DollarAllowed, int RecursionDepth, int *status, char *errtext)
 {
  value JD1,JD2;
  char *FormatString;
  int   i=0,j=0,k=0;
  long  GapYears, GapDays, GapHours, GapMinutes, GapSeconds;

  *status=-1;
  ppl_units_zero(output);
  inlen--; // Make inlen point to last character
  while ((in[i]!='\0')&&(in[i]<=' ')) i++; // Strip spaces off front

  // Fetch first input Julian Day number
  j=-1;
  ppl_EvaluateAlgebra(in+i, &JD1, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1);
  if (*status >= 0) { (*status) += i; return; }
  i+=j;
  while ((in[i]>'\0')&&(in[i]<=' ')) i++;

  // Fetch second input Julian Day number
  if (in[i] != ',')
   {
    if (in[i] ==')') { *status = i; strcpy(errtext,"Syntax Error: Too few arguments supplied to function."); return; }
    else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; }
   }
  i++; j=-1;
  ppl_EvaluateAlgebra(in+i, &JD2, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1);
  if (*status >= 0) { (*status) += i; return; }
  i+=j;
  while ((in[i]>'\0')&&(in[i]<=' ')) i++;

  // Fetch format string
  if (in[i] != ',')
   {
    if (in[i] ==')') { FormatString = "%Y years %d days %h hours %m minutes and %s seconds"; }
    else             { *status = i; strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); return; }
   } else {
    i++; j=-1;
    FormatString = lt_malloc(LSTR_LENGTH);
    ppl_GetQuotedString(in+i, FormatString, 0, &j, DollarAllowed, status, errtext, RecursionDepth+1);
    if (*status >= 0) { (*status) += i; return; }
    i+=j;
    while ((in[i]>'\0')&&(in[i]<=' ')) i++;
   }

  // Check for closing bracket
  if (in[i] != ')')
   {
    *status = i;
    if (in[i] == ',') { strcpy(errtext,"Syntax Error: Too many arguments supplied to function."); }
    else              { strcpy(errtext,"Syntax Error: Unexpected trailing matter after argument to function."); }
    return;
   }

  // Check that inputs are real
  if (JD1.FlagComplex) { *status=0; strcpy(errtext,"The time_diff_string() function can only act upon real times. The first supplied Julian Day number is complex."); return; }
  if (!JD1.dimensionless) { *status=0; sprintf(errtext,"The time_diff_string() function can only act upon dimensionless times. The first supplied Julian Day number has units of <%s>.", ppl_units_GetUnitStr(&JD1,NULL,NULL,0,0)); return; }
  if (JD2.FlagComplex) { *status=0; strcpy(errtext,"The time_diff_string() function can only act upon real times. The second supplied Julian Day number is complex."); return; }
  if (!JD2.dimensionless) { *status=0; sprintf(errtext,"The time_diff_string() function can only act upon dimensionless times. The second supplied Julian Day number has units of <%s>.", ppl_units_GetUnitStr(&JD1,NULL,NULL,0,0)); return; }

  GapYears   = (JD2.real - JD1.real) / 365;
  GapDays    = (JD2.real - JD1.real);
  GapHours   = (JD2.real - JD1.real) * 24;
  GapMinutes = (JD2.real - JD1.real) * 24 * 60;
  GapSeconds = (JD2.real - JD1.real) * 24 * 3600;

  // Loop through format string making substitutions
  output->string = lt_malloc(LSTR_LENGTH);
  for (j=0; FormatString[j]!='\0'; j++)
   {
    if (FormatString[j]!='%') { output->string[k++] = FormatString[j]; continue; }
    switch (FormatString[j+1])
     {
      case '%': sprintf(output->string+k, "%%"); break;
      case 'Y': sprintf(output->string+k, "%ld", GapYears); break;
      case 'D': sprintf(output->string+k, "%ld", GapDays); break;
      case 'd': sprintf(output->string+k, "%ld", GapDays%365); break;
      case 'H': sprintf(output->string+k, "%ld", GapHours); break;
      case 'h': sprintf(output->string+k, "%ld", GapHours%24); break;
      case 'M': sprintf(output->string+k, "%ld", GapMinutes); break;
      case 'm': sprintf(output->string+k, "%ld", GapMinutes%60); break;
      case 'S': sprintf(output->string+k, "%ld", GapSeconds); break;
      case 's': sprintf(output->string+k, "%ld", GapSeconds%60); break;
      default: { *status=0; sprintf(errtext,"Format string supplied to time_diff_string() function contains unrecognised substitution token '%%%c'.",FormatString[j+1]); return; }
     }
    j++;
    k += strlen(output->string + k);
   }
  output->string[k]='\0'; // Null terminate string
  return;
 }

void dcftime_diff(value *in1, value *in2, value *output, int *status, char *errtext)
 {
  char *FunctionDescription = "timediff(JD1,JD2)";
  CHECK_2NOTNAN;
  CHECK_2INPUT_DIMLESS;
  IF_2COMPLEX { QUERY_MUST_BE_REAL }
  ELSE_REAL   { output->real = (in2->real - in1->real)*86400; }
  ENDIF
  CLEANUP_APPLYUNIT(UNIT_TIME);
  CHECK_OUTPUT_OKAY;
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

