#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include "StreamException.h"
const char* StreamException::what() const
{
	char buf[500];
	sprintf(buf, "%s | Error Code %d\n", msg, errorCode);
	return buf;
}
