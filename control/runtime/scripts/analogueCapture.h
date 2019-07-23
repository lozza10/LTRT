/*
 * analogueCapture.h
 * Header file for analogueCapture
 * LT 25/02/2013
 *
 */ 

using namespace std;


/* Function declarations */
string strip_chars(string str, char a);
int get_nlines(char* filename);
double round(double x);
double split_float(double num);
string get_lst(float deg);
string get_jd();
double get_lstdeg(string jd);
string get_date();
string get_time(int opt);
void* plot(void* arg);

struct thread_data {
  char* arg1;
  char* arg2;
  int arg3;
  const char* arg4;
  int arg5;
};

