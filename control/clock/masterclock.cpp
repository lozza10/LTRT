/*
 *
 * masterclock v1.01
 * LT 15/02/2013
 *
 * Compile with: g++ -o masterclock masterclock.cpp
 */ 

#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string>
#include <unistd.h>
#include <time.h>
#include <ctime>

using namespace std;

double lstdeg;
float glong=151.098826; //LTRT longitude E, Marsfield NSW
float glat=33.773749;   //LTRT latitude  S, Marsfield NSW
//float glong=151.086179; //LTRT longitude E, Hornsby NSW
//float glat=33.701816;   //LTRT latitude  S, Hornsby NSW
char xlabel[80], tunix[80], tlocal[80], tutc[80], tdate[80];
time_t ltime;
string jd, lst;

double round(double x) {

  const double sd = 1000;
  return int(x*sd + (x<0? -0.5 : 0.5))/sd;

}

double split_float(double num) {

  int intpart = (int)num;
  double decpart = num - intpart;
  return decpart;

}

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

  } else if ((hh > 0) && (hh < 10)) {

    sprintf(strhh, "0%d", hh);

  } else if (hh == 0) {

    sprintf(strhh, "00");

  } else {

    sprintf(strhh, "%d", hh);

  }

  if ((mm > 0) && (mm < 10)) {

    sprintf(strmm, "0%d", mm);

  } else if (mm == 0) {

    sprintf(strmm, "00");

  } else {

    sprintf(strmm, "%d", mm);

  } 
  
  if ((ss > 0) && (ss < 10)) {

    sprintf(strss, "0%d", ss);

  } else if (ss == 0) {

    sprintf(strss, "00");

  } else {

    sprintf(strss, "%d", ss);

  }
 
    sprintf(str, "%s:%s:%s", strhh,strmm,strss);
  
  return string(str);

}

string get_jd() {

  char strjd[80];
  time_t tt;
  double unixts=time(NULL);
  float s=86400.0f;
  float jplus=2440587.5f;

  sprintf(strjd, "%.6f\n", unixts/s+jplus);
 
  return string(strjd);

}

double get_lstdeg(string jd) {

  double jdflt=atof(jd.c_str());
  double tt=jdflt-2451545.0;
  double aa=280.46061837+360.98564736629*tt;
  double bb=fmod(aa,360);
  double lstdeg=bb+glong;

  if (lstdeg >= 360) {
    lstdeg=lstdeg-360;
  }

  return lstdeg;

}

int main ()
{

  while(1) {

  system("/usr/bin/tput clear");
  system("/usr/bin/tput cup 0 0");

  // Get Julian Day
  jd=get_jd();

  // Get LST in degrees
  lstdeg=get_lstdeg(jd);

  // Convert LST from degrees to hh:mm:ss
  lst=get_lst(lstdeg);

  // Get Local time and UTC
  time(&ltime);
  tm *ltm = localtime(&ltime);
  sprintf(tunix, "%lu", time(NULL));
  sprintf(tlocal, "%s", asctime(localtime(&ltime)));
  sprintf(tutc, "%s", asctime(gmtime(&ltime)));
  sprintf(tdate, "%d/%d/%d", ltm->tm_mday, 1+ltm->tm_mon, 1900+ltm->tm_year);

  // Print to stdout
  cout << endl;
  cout << "           -- MasterClock v1.01 --          " << endl;
  cout << " *******************************************" << endl;
  cout << "  Location (deg):  " << glong << " E, " << glat << " S" << endl;
  cout << "   Unix Time (s):  " << tunix << endl;
  cout << "      Local Time:  " << tlocal;
  cout << "             UTC:  " << tutc;
  cout << "      Julian Day:  " << jd;
  cout << "       LST (deg):  " << lstdeg << endl;
  cout << "  LST (hh:mm:ss):  " << lst << endl;
  cout << " *******************************************" << endl;
  cout << endl;

  system("/usr/bin/tput rc");
  sleep(1);

  }

  return 0;

}
