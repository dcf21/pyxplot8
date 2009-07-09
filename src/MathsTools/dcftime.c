double JulianDate(int year, int month, int day, int hour, int min, int sec)
 {
  double sign, JD, DayFraction;
  int b;

  if (month<=2) { month+=12; year--; }

  if ( (10000.0*year + 100*month + day) <= 15821004.0 )
   b = -2 + ((year+4716)/4) - 1179; // Julian
  else
   b = (year/400) - (year/100) + (year/4); // Gregorian

  JD = 365.0*year - 679004.0 + 2400000.5 + b + floor(30.6001*(month+1)) + day;

  if ((day<0) || (min<0) || (sec<0)) sign=-1; else sign = 1.0;
  DayFraction = sign * (fabs(day) + fabs(min)/60.0 + fabs(sec)/3600.0);

  return JD + DayFraction;
 }

void InvJulianDate(double JD, ... )
 {
  double a,b,c,d,e,f;
  double DayFraction;

  a = JD + 0.5;
  if (a < 2299161)
   { b=0; c=a+1524; } // Julian
  else
   { b=(a-1867216.25)/26524.25; c=a+b-(b/4)+1525; }
  d = (c-122.1)/365.25;
  e = 365*d + d/4;
  f = (c-e)/30.6001;
  day = c-e-floor(30.6001*f);
  month = f-1-12*(f/14);
  year  = d-4715-((7+month)/10);
  DayFraction = JD - floor(JD);
  hour = 24*DayFraction; // Fix to include minutes and seconds
 }
