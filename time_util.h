#ifndef TIME_UTIL_H
#define TIME_UTIL_H

#include <sys/time.h>

unsigned long getmsofday() {
   struct timeval tv;
   gettimeofday(&tv, NULL);
   return (long long)tv.tv_sec*1000 + tv.tv_usec/1000;
}

#endif // TIME_UTIL_H
