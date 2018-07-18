#include "stdafx.hpp"
#include "base_object.hpp"

#ifdef _WINDOWS
#include <windows.h>
#else
#include <sys/time.h>
#endif

using namespace std;

#define TIME_BUF_SIZE 15


BaseObject::BaseObject(const string &tag) {
    if (tag.size() > 0) {
        tag_ = tag + " ";
    }
}

void BaseObject::log(const char *format, ...) {
	char timeBuf[TIME_BUF_SIZE];
#ifdef _WINDOWS
	SYSTEMTIME st;
	GetLocalTime(&st);
	snprintf(timeBuf, TIME_BUF_SIZE, "%02d:%02d:%02d.%03d ", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
	timeval curTime;
    gettimeofday(&curTime, NULL);
    int ms = curTime.tv_usec / 1000;
    strftime(timeBuf, TIME_BUF_SIZE, "%H:%M:%S", localtime(&curTime.tv_sec));
    int i = 8;
    int n = TIME_BUF_SIZE - i;
    snprintf(timeBuf + i, n, ".%03d ", ms);
#endif

    stringstream pattern;
    pattern << "-- " << timeBuf << tag_ << format << endl;
    
    va_list args;
    va_start(args, format);
    vprintf(pattern.str().c_str(), args);
    va_end(args);
}
