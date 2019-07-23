/*
 *
 * acFunctions.cpp
 * Functions for analogueCapture
 *
 * LT 25/02/2013
 *
 */ 

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <ctime>
#include "analogueCapture.h"

using namespace std;

float gglong=151.007831; //LTRT longitude E, Glenorie NSW


/* Strip characters from string */
string strip_chars(string str, char a) {

  string::size_type k=0;
  while((k=str.find(a,k))!=str.npos) {
    str.erase(k, 1);
  }
  return str;

}


/* Get nlines in file */
int get_nlines(char* filename) {

  int nlines=0;
  string line;

  ifstream inFile(filename);

  while (getline(inFile, line)) {
    if (line.find("#")==0) { // Skip header
       continue;
    }
    ++nlines;
  }

  inFile.close();
  return nlines;

}


/* Round */
double round(double x) {

  const double sd = 1000;
  return int(x*sd + (x<0? -0.5 : 0.5))/sd;

}


/* Split a float */
double split_float(double num) {

  int intpart = (int)num;
  double decpart = num - intpart;
  return decpart;

}


/* Get LST in hh:mm:ss */
string get_lst(float deg) {

  char str[80], strhh[80], strmm[80], strss[80];
  int hh=round(deg/15);
  double hhdec=split_float(deg/15);
  double mmdbl=hhdec*60;
  int mm=round(mmdbl);
  double mmdec=split_float(mmdbl);
  int ss=round(mmdec*60);
 
  if (hh >= 24) {

    sprintf(strhh, "0%d", hh-24);

  } else if ((hh >= 0) && (hh < 10)) {

    sprintf(strhh, "0%d", hh);

  } else {

    sprintf(strhh, "%d", hh);

  }

  if ((mm >= 0) && (mm < 10)) {

    sprintf(strmm, "0%d", mm);

  } else {

    sprintf(strmm, "%d", mm);

  } 
  
  if ((ss >= 0) && (ss < 10)) {

    sprintf(strss, "0%d", ss);

  } else {

    sprintf(strss, "%d", ss);

  }
 
    sprintf(str, "%s:%s:%s", strhh,strmm,strss);
  
  return string(str);

}


/* Get LST in degrees */
double get_lstdeg(string jd) {

  double jdflt=atof(jd.c_str());
  double tt=jdflt-2451545.0;
  double aa=280.46061837+360.98564736629*tt;
  double bb=fmod(aa,360);
  double lstdeg=bb+gglong;

  return lstdeg;

}


/* Get Julian Day */
string get_jd() {

  char strjd[80];
  unsigned long int unixts=time(NULL);
  float s=86400.0f;
  float jplus=2440587.5f;

  sprintf(strjd, "%.6f\n", unixts/s+jplus);
  return string(strjd);

}


/* Get date */
string get_date() {

  char tdate[80], tdate_d[80], tdate_m[80];
  time_t ltime;
  time(&ltime);
  tm *ltm = localtime(&ltime);
  int day=ltm->tm_mday;
  int mnth=1+ltm->tm_mon;
  int year=1900+ltm->tm_year;

  if ((day > 0) && (day < 10)) {
    sprintf(tdate_d, "0%d", day);
  } else {
    sprintf(tdate_d, "%d", day);
  }

  if ((mnth > 0) && (mnth < 10)) {
    sprintf(tdate_m, "0%d", mnth);
  } else {
    sprintf(tdate_m, "%d", mnth);
  }

  sprintf(tdate, "%s/%s/%d", tdate_d, tdate_m, year);
  return string(tdate);

}


/* Get time in various formats */
string get_time(int opt) {

  switch(opt) {

    // LST
    case 0:
      {
        string t;
        string jd=get_jd();
        double lstdeg=get_lstdeg(jd);
        t=get_lst(lstdeg);
        return t;
      }
      break;

    // Unix time
    case 1:
      {
        char t[80];
        sprintf(t, "%u", time(NULL));
        return t;
      }
      break;

    // UTC as (HH:MM:SS)
    case 2:
      {
        struct tm * timeinfo;
        char ttutc[80];
        time_t ltime;

        time(&ltime);
        timeinfo=gmtime(&ltime);
        strftime(ttutc,80,"%T %d/%m/%Y",timeinfo);
        return ttutc;
      }
      break;

    // Date in dd/mm/yyyy format
    case 3:
      {
        string ttdate;
        ttdate=get_date();
        return ttdate;
      }
      break;

   // Local time as (Www Mmm dd hh:mm:ss yyyy)
   case 4:
       {
         char ttlocal[80];
         time_t ltime;

         time(&ltime);
         sprintf(ttlocal, "%s", asctime(localtime(&ltime)));
         return ttlocal;
       }
       break;

   // Local time as (HH:MM:SS)
   case 5:
       {
         struct tm * timeinfo;
         char ttlocal[80];
         time_t ltime;

         time(&ltime);
         timeinfo=localtime(&ltime);
         strftime(ttlocal,80,"%T %d/%m/%Y",timeinfo);
         return ttlocal;
       }
       break;
  }

} 
