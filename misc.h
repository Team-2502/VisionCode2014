#ifndef MISC_H
#define MISC_H

#include <raspicam/raspivid.h>
#include <sys/time.h>
#include <string.h>
#include <iostream>
#include <iomanip>
#include <cmath>

using namespace std;

#define PI 3.1415926535
#define STATUS_INITIALIZING 1
#define STATUS_RUNNING 2
#define STATUS_STOPPED 4
#define STATUS_CAPTURING 8
#define STATUS_CALIBRATING 16

unsigned long getmsofday() {
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
}

int str2int(const char * a) {
	//stringstream ss; ss << a; int i = 0; ss >> i;
	int i = 0;
	sscanf(a, "%d", &i);
	return i;
	//return i;
}

char * int2str(char * buf, int i) {
	//stringstream ss; ss << i; char * a; ss >> a;
	sprintf(buf, "%d", i);
	return buf;
}

char * float2str(char * buf, float f) {
	//stringstream ss; ss << f; char * a; ss >> a;
	//return a;
	sprintf(buf, "%.3f", f);
	return buf;
}

int str2int(string s) {
	return str2int(s.c_str());
}

float str2float(const char * a) {
	//stringstream ss; ss << a; float f = 0; ss >> f;
	return atof(a);
	//return f;
}

string strtolower(string str) {
	string ret = "";
	for (int i = 0; i < str.length(); i++) {
		ret.push_back(tolower(str[i]));
	}
	return ret;
}

/*
 * Global Variables
 */

DataStorage globalStorage;
USERDATA * data;

#endif // MISC_H
