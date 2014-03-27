#ifndef STR_UTIL_H
#define STR_UTIL_H

#include <stdio.h>
#include <string.h>

#include "data_storage.h"

using namespace std;

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

#endif // STR_UTIL_H
