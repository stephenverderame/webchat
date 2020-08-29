#pragma once
#include "Streamable.h"
#include <functional>
#include <vector>
//class to manipulate data without copying
class StreamView
{
private:
	const char * parent_p;
	std::shared_ptr<std::vector<char>> parent;
	std::streamsize start, size;
	mutable std::streamsize ptr;
public:
	static constexpr long BASE = 256, MOD = 217500000;
public:
	static constexpr std::streamsize INVALID = -1;
	//start inclusive, end exclusive
	StreamView(const char * parent, std::streamsize start, std::streamsize end);
	StreamView(const std::shared_ptr<std::vector<char>> & parent, std::streamsize start, std::streamsize end);
	StreamView();
	StreamView(StreamView&& other);
	StreamView(const StreamView& other);
	StreamView& operator=(const StreamView& other);
	StreamView& operator=(StreamView&& other);
	~StreamView();

	std::streamsize find(char k, std::streamsize start = 0) const;
	std::streamsize find(const char * k, std::streamsize start = 0, size_t len = -1) const;

	//find multithreaded
	std::streamsize find_mt(const char * k, std::streamsize start = 0, size_t len = -1) const;
	
	//start inclusive, end exclusive
	StreamView sub(std::streamsize start, std::streamsize end = -1) const;
	std::string sub_cpy(std::streamsize start, std::streamsize end = -1) const;

	void put(std::ostream & stream) const;
	std::string copy() const;

	template<typename T = int>
	T parse(std::streamsize start = 0, std::streamsize end = -1, int base = 10);

	//continues to get the next substring delimeted by the delim, increasing the internal seek pointer until no more references of the delim are found
	//returns true when a delim is found, false otherwise
	bool getsub(StreamView & view, char delim) const;
	bool getsub(StreamView & view, const char * delim) const;

	//operations of the internal pointer
	char get() const;
	std::streamsize tell() const;
	void seek(std::streamsize offset, std::ios::seekdir reference = std::ios::beg) const;
	void advance(std::streamsize offset = 1) const;

	long hash() const;

	bool operator==(const StreamView & other) const;
	bool operator==(const char * other) const;

	std::streamsize getSize() const;

	inline std::streamsize rel2abs(std::streamsize relative);


	//Does not take owenership
	void assign(const char * mem, std::streamsize start, std::streamsize end);
public:
	//case-insensitive
	static long hash(const char * str, size_t len = -1);
	static constexpr long HASH(const char * str, size_t len = -1);
protected:
	//Factors start into pointer
	const char * _gptr() const;

};

std::ostream & operator<<(std::ostream & s, const StreamView & str);

template<typename T>
inline T StreamView::parse(std::streamsize start, std::streamsize end, int base)
{
	static_assert(std::is_integral<T>::value);
	if (end == -1) end = size;
	char back = _gptr()[end];
	const_cast<char*>(_gptr())[end] = '\0';
	T val;
	if (std::is_floating_point<T>::value) {
		if (sizeof(T) == sizeof(double))
			val = strtod(&_gptr()[start], NULL);
		else if (sizeof(T) == sizeof(long double))
			val = strtold(&_gptr()[start], NULL);
		else
			val = strtof(&_gptr()[start], NULL);
	}
	else if (std::is_unsigned<T>::value) {
		if (sizeof(T) == sizeof(long long))
			val = strtoull(&_gptr()[start], NULL, base);
		else
			val = strtoul(&_gptr()[start], NULL, base);
	}
	else {
		switch (sizeof(T)) {
			case sizeof(long long) :
				val = strtoll(&_gptr()[start], NULL, base);
				break;
			default :
				val = strtol(&_gptr()[start], NULL, base);
				break;
		}
	}
	const_cast<char*>(_gptr())[end] = back;
	return val;
}

inline constexpr long StreamView::HASH(const char * str, size_t len) {
	if (len == -1) len = strlen(str);
	long r = 0;
	for (int i = 0; i < len; ++i) {
		r = r * BASE + toupper(str[i]);
		r %= MOD;
	}
	return r;
}
inline std::streamsize StreamView::rel2abs(std::streamsize relativeOffset) {
	return relativeOffset + start;
}


