#pragma once
#include <exception>
/** 
 * Represents some kind of IO exception that occurred when using Streams
 */
class StreamException : public std::exception {
private:
	int errorCode;
	const char* msg;
public:
	StreamException(int errorCode, const char* msg) : errorCode(errorCode), msg(msg) {}
	inline int code() { return errorCode; }
	const char* what() const override;
};