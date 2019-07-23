/*
 analogueCapture.cpp
 Version: 1.03
 Capture Arduino output from LTRT on serial port over USB
 at sample rate of ~1khz
 LT June 2013

 Complile on psrVM:
 g++ -o analogueCapture analogueCapture.cpp acFunctions.cpp

 Compile on psr2:
 g++ -o analogueCapture analogueCapture.cpp acFunctions.cpp -L/home/lozza/docs/latest/lib -L/home/lozza/docs/latest/pgplot_build -lcpgplot -lpgplot -lpng -L/usr/lib/X11 -lX11 -lpthread -L/usr/lib/gcc/i686-redhat-linux/3.4.6 -L/usr/lib/gcc/i686-redhat-linux/3.4.6/../../.. -lfrtbegin -lg2c -lm
*/


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/ioctl.h>
#include <csignal>
#include <cstdlib>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include "analogueCapture.h"
#include <pthread.h>

using namespace std;


/* Define some variables */

int fd, ttunix, tstop;
int tlevel=30.0;  // Read time for level-setting (s)
float glong=151.007831; // LTRT longitude E, Glenorie NSW
float glat=33.595944;   // LTRT latitude  S, Glenorie NSW
//const char* serial_port = "/dev/ttyACM0"; // Serial port
const char* serial_port = "/dev/ttyS0"; // Serial port
const char* data_dir = "/home/pulsar/LTRT/control/data/"; // Data directory
const char* version = "v1.03 June 2013"; // Code version
char outfname[80], outsname[80], file_suffix[80];
ofstream outfile;


/* Function declarations */

int open_port(const char* serial_port);
int init_port_params(int fd);
int read_port(int fd, char* outfname, int tstop);
void set_levels(int t);
void close_port(int fd);
void sigint_handler(int x);
void end_obs();


/* SIGINT handler - CTRL+D interrupt */

void sigint_handler(int fd) {

  cout << endl << " analogueCapture :: Received terminating signal...exiting now!" << endl;
  remove(outfname);
  close_port(fd);

}


/* Open serial port */

int open_port(const char* serial_port) {

  int fd;

  fd = open(serial_port, O_RDWR | O_NOCTTY | O_NDELAY);

  switch(fd) {

    case -1:
      cerr << " analogueCapture :: Unable to open serial port " << serial_port << "...exiting now!" << endl;
      exit(1);
      break;

    default:
      cout << " analogueCapture :: Serial port " << serial_port << " opened" << endl;

      // Set file descriptor status
      fcntl(fd, F_SETFL, 0);

      // Flush existing buffer contents
      cout << " analogueCapture :: Flushing serial port buffer" << endl;
      sleep(3);
      tcflush(fd, TCIOFLUSH);

      return(fd);
      break;

  }

}


/* Set port parameters */

int init_port_params(int fd) {

  struct termios options;

  // Get current port options
  tcgetattr(fd, &options);

  // Set required baud rate
  cfsetispeed(&options, B115200);
  cfsetospeed(&options, B115200);

  // Enable reciever and set local mode
  options.c_cflag |= (CLOCAL | CREAD);
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  
  // Set new port options
  tcsetattr(fd, TCSANOW, &options);

  return 1;

}


/* Read serial port buffer */

int read_port(int fd, char* outfname, int tstop) {

  #define MAXBUFFER 128
  int nbytes;
  string x; 
  char buffer[MAXBUFFER];
  ofstream outfile(outfname, ios::app);

  // Get buffer contents
  while(ttunix < tstop) {

    read(fd, &buffer[0], sizeof(char));
    char* bufstr = &buffer[0];
    string str(&buffer[0]);
    x = str[0];

    // Write to file
    outfile << x;
      
    // Check tstop against Unix time
    string ttunix_str=get_time(1);
    ttunix=atoi(ttunix_str.c_str());

  }

  return 0;

}


/* Set receiver levels */

void set_levels(int t) {

  int tstop;
  char outlname[80];
  sprintf(outlname, "%sac_levels.dmp", data_dir);
  ofstream loutfile(outlname);

  cout << " analogueCapture :: Setting receiver levels" << endl;

  // Open port
  fd = open_port(serial_port);

  // Set port params
  init_port_params(fd);

  // Check tstop against Unix time
  string ttunix_str=get_time(1);
  ttunix=atoi(ttunix_str.c_str());
  tstop=ttunix+t;

  // Read port input buffer and dump t seconds to file
  cout << " analogueCapture :: Reading " << t << " seconds of data" << endl;
  read_port(fd, outlname, tstop);
  loutfile.close();

  // Remove file and close port
  remove(outlname);
  close(fd);
  cout << " analogueCapture :: Level-setting complete" << endl;
  cout << " analogueCapture :: Serial port closed" << endl << endl;

  // Allow time for serial port to recover
  sleep(1);

}


/* Terminate observation and close port */

void close_port(int fd) {

  outfile.close();
  close(fd);
  exit(fd);

}


/* Close file at observation end */

void end_obs() {

  outfile.close();
  cout << " analogueCapture :: Observation complete" << endl;

}


/* Usage */

void usage() {

  cout << endl;
  cout << "   analogueCapture: LTRT data capture" << endl;
  cout << "                     " << version << endl << endl;
  cout << "              Usage: analogueCapture <args>" << endl;
  cout << " Accepted arguments: -mode <mode> [search mode def: Drift scan]" << endl;
  cout << "                     -G <gain> [manually adjusted receiver Gain (dB)]" << endl;
  cout << "                     -tobs <tobs> [observation length (s)]" << endl;
  cout << "                     -nobs <nobs> [no. of cycles (0=loop)]" << endl;
  cout << "                     -dec <dec> [source declination (+/-dd:mm)]" << endl;
  cout << "                     -src <src> [source name]" << endl;
  cout << "                     -pos <pos> [actuator position index]" << endl;
  cout << endl;

}


int main(int argc, char* argv[]) {

  char* mode;
  char* src;
  char* dec;
  char* pos;
  int tobs, nobs, i;
  float gain;
  int loop=0;

  // Take user input params
  if ( argc < 2) {

    usage();
    cout << " Note: analogueCapture must take the following arguments:" << endl;
    cout << "       -mode -G -tobs -nobs -dec -src -pos" << endl << endl;
    exit(0);

  } else {

    for (int i=1; i<argc; i++) {
      if (strcmp(argv[i],"-mode")==0) {
         mode=argv[i + 1];                
      } else if (strcmp(argv[i],"-G")==0) {
         gain=atoi(argv[i + 1]);
      } else if (strcmp(argv[i],"-tobs")==0) {
         tobs=atoi(argv[i + 1]);
      } else if (strcmp(argv[i],"-nobs")==0) {
         nobs=atoi(argv[i + 1]);
         if (nobs==0) {
           loop=1;
         }
      } else if (strcmp(argv[i],"-dec")==0) {
         dec=argv[i + 1];
      } else if (strcmp(argv[i],"-src")==0) {
         src=argv[i + 1];
      } else if (strcmp(argv[i],"-pos")==0) {
         pos=argv[i + 1];
      } else {
          // Cannot get this to work!
          //cout << "ERROR: Invalid arguments, please try again." << endl << endl;
          //exit(0);
      }

    }

  }

  // Catch CTRL+D interrupt
  signal(SIGINT, sigint_handler);

  // Check smoothing script is running
  char ps_buffer[1024];
  char buff[512];
  FILE *in;
  sprintf(ps_buffer, "ps -C smoothData.sh | awk '{if (NR==2) {print}}'");
  if ((in = popen(ps_buffer, "r"))==NULL) {
    cout << "Cannot run " << ps_buffer;
    return 1;
  }
  if (fgets(buff, sizeof(buff), in)==NULL) {
    cout << endl << " analogueCapture :: [smoothData.sh] no such process...exiting" << endl;
    return 1;
  } else {
    cout << endl << " *****************************************************" << endl;
    cout << "          analogueCapture " << version << endl;
    cout << " *****************************************************" << endl << endl;
    cout << " analogueCapture :: Ok, smoothing script running" << endl;
  }
  pclose(in);

  // Set receiver levels
  set_levels(tlevel);

  // Create directory structure
  char obs_dir[80];
  string lst=get_time(0);
  string lst_strip=strip_chars(lst, ':');
  string ttdate_str=get_time(3);
  string ttdate_strip=strip_chars(ttdate_str, '/');
  sprintf(obs_dir, "%spos%s_%s_%s", data_dir, pos, ttdate_strip.c_str(), lst_strip.c_str());
  mkdir(obs_dir, S_IRWXU);

  // Main loop
  int j, k;

  if (loop==1) {
    j=0;
    k=1;
  } else {
    j=1;
  }

  while(j<=nobs) {

    if (loop==1) {
      cout << endl << " analogueCapture :: Starting observation " << k << endl;
    } else {
      cout << endl << " analogueCapture :: Starting observation cycle " << j << "/" << nobs << endl;
    }

    // Open port
    fd = open_port(serial_port);

    // Get LST, Unix time, UTC, Date, Local time
    lst=get_time(0);
    string ttunix_str=get_time(1);
    string ttutc=get_time(2);
    ttdate_str=get_time(3);
    string ttlocal=get_time(5);
    ttunix=atoi(ttunix_str.c_str());

    // Create directory structure, raw and smoothed files
    lst_strip=strip_chars(lst, ':');
    string dec_strip=strip_chars(dec, ':');
    ttdate_strip=strip_chars(ttdate_str, '/');
    sprintf(file_suffix, "ac_%s_%s%s", ttdate_strip.c_str(), lst_strip.c_str(), dec_strip.c_str());
    sprintf(outfname, "%s/%s.tmp", obs_dir, file_suffix);
    sprintf(outsname, "%s/%s.dat", obs_dir, file_suffix);
    ofstream outfile(outfname);
    ofstream soutfile(outsname, ios::app);

    // Print to stdout
    cout << endl;
    cout << " *****************************************************" << endl;
    cout << "               Telescope:  LTRT" << endl;
    cout << "          Location (deg):  " << glong << " E, " << glat << " S" << endl;
    cout << "                    Mode:  " << mode << endl;
    cout << "  Centre frequency (MHz):  3700" << endl;
    cout << "        Local time start:  " << ttlocal << endl;
    cout << "               UTC start:  " << ttutc << endl;
    cout << "               LST start:  " << lst << endl;
    cout << "                  Source:  " << src << endl;
    cout << "     Declination (dd:mm):  " << dec << endl;
    cout << " Actuator position index:  " << pos << endl;
    cout << "  Observation length (s):  " << tobs << endl;
    if (loop==1) {
      cout << "                   Cycle:  Continuous loop" << endl;
    } else {
      cout << "                   Cycle:  " << j << "/" << nobs<< endl;
    }
    cout << "                Raw file:  " << file_suffix << ".tmp" << endl;
    cout << "           Smoothed file:  " << file_suffix << ".dat" << endl;
    cout << " *****************************************************" << endl;
    cout << endl;

    // Write headers
    soutfile << "# Telescope: LTRT" << endl;
    soutfile << "# Description: 2.4m parabolic C-Band antenna" << endl;
    soutfile << "# Location: Glenorie NSW Australia" << endl;
    soutfile << "# Coordinates: " << glong << "E " << glat << "S" << endl;
    soutfile << "# Mode: " << mode << endl;
    soutfile << "# Centre_frequency (MHz): 3700" << endl;
    soutfile << "# Sample_time (ms): 1.0" << endl;
    soutfile << "# Receiver_gain (dB): " << gain << endl;
    soutfile << "# File: " << file_suffix << ".dat" << endl;
    soutfile << "# Observation_length (s): " << tobs << endl;
    soutfile << "# LocalTime_start: " << ttlocal << endl;
    soutfile << "# UTC_start: " << ttutc << endl;
    soutfile << "# LST_start: " << lst << endl;
    soutfile << "# Source: " << src << endl;
    soutfile << "# Declination (dd:mm): " << dec << endl;
    soutfile << "# Actuator_index: " << pos << endl;

    // Set observation stop time
    tstop=ttunix+tobs;
    char tstop_buf[80];
    time_t rawtime;
    struct tm * tstop_local;
    time_t ttstop=time(&rawtime)+tobs;
    tstop_local=localtime(&ttstop);
    strftime(tstop_buf,80,"%T",tstop_local);
    
    // Main loop to take data
    cout << " analogueCapture :: Cycle " << j << " ends at " << tstop_buf << " local time" << endl;
    cout << " analogueCapture :: Taking " << tobs << " seconds of data" << endl;

    // Read port input buffer and write data to file
    read_port(fd, outfname, tstop);

    // Clean up
    //remove(outfname);
    close(fd);
    cout << " analogueCapture :: Observation complete" << endl;
    cout << " analogueCapture :: Serial port closed" << endl << endl;

    if (loop==1) {
      j=0;
      k++;
    } else {
      j++;
    }

  }

  // Terminate
  close_port(fd);
  return 0;
  
}
