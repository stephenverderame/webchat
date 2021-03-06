#pragma once
#include "Streamable.h"
#include "Hash.h"
#include <functional>
#include <vector>
///class to manipulate data without copying
class StreamView
{
	///Offsets used by internal functions are relative from the start of this StreamView
private:
	const char * parent_p;
	std::shared_ptr<std::vector<char>> parent;
	std::streamsize start, _size;
	mutable std::streamsize ptr;
public:
	static constexpr std::streamsize INVALID = -1;
	///start inclusive, end exclusive
	StreamView(const char * parent, std::streamsize start, std::streamsize end);
	StreamView(const char* parent, std::streamsize len);

	/**
	* Creates a StreamView with shared buffer ownership of parent from [start, end) (absolute)
	* Use if you want the StreamView to persist past the lifetime of the buffer's owner
	*/
	///@{
	StreamView(const std::shared_ptr<std::vector<char>> & parent, std::streamsize start, std::streamsize end);
	StreamView(const std::shared_ptr<std::vector<char>>&& parent, std::streamsize start, std::streamsize end);
	///@}
	
	StreamView();
	StreamView(StreamView&& other);
	StreamView(const StreamView& other);
	StreamView& operator=(const StreamView& other);
	StreamView& operator=(StreamView&& other);
	~StreamView();

	std::streamsize find(char k, std::streamsize start = 0) const;
	std::streamsize find(const char * k, std::streamsize start = 0, size_t len = -1) const;

	///find multithreaded
	std::streamsize find_mt(const char * k, std::streamsize start = 0, size_t len = -1) const;
	
	///start relative inclusive, end relative exclusive
	///@{
	StreamView sub(std::streamsize start, std::streamsize end = -1) const;
	std::string sub_cpy(std::streamsize start, std::streamsize end = -1) const;
	///@}

	void put(std::ostream & stream) const;
	std::string copy() const;

	/**
	* Pareses this StreamView for the specified integral type of base base
	* @param <T> the integral type ie double, float, long
	* @param start relative offset of start
	* @param end relative offset of the place to stop parsing
	* @param base the numerical base to parse
	*/
	template<typename T = int>
	T parse(std::streamsize start = 0, std::streamsize end = -1, int base = 10);

	//continues to get the next substring delimeted by the delim, increasing the internal seek pointer until no more references of the delim are found
	//returns true when a delim is found, false otherwise
	bool getsub(StreamView & view, char delim) const;
	bool getsub(StreamView & view, const char * delim) const;

	char operator[](std::streamsize index) const;

	///operations of the internal pointer
	/// @{
	char get() const;
	std::streamsize tell() const;
	void seek(std::streamsize offset, std::ios::seekdir reference = std::ios::beg) const;
	void advance(std::streamsize offset = 1) const;
	///@}
	
	
	hash_t hash() const;
	hash_t hashCaseInsensitive() const;

	bool operator==(const StreamView & other) const;
	bool operator==(const char * other) const;

	std::streamsize getSize() const;

	/**
	* Takes a relative pointer (returned by find(), passed to sub() etc) and converts it into an absolute offset from the start of the buffer
	* @param relative relative offset
	* @return absolute offset
	*/
	inline std::streamsize rel2abs(std::streamsize relative);


	/**
	* Assigns this streamview to mem from start to end
	* Does not take ownership
	* @param mem the new buffer
	* @param start the absolute index to start at in this buffer
	* @param end the absolute end index of the buffer
	*/
	void assign(const char * mem, std::streamsize start, std::streamsize end);
	/**
	* Adjusts the StreamView to view a new area in the same buffer
	* @param start the new start index (absolute)
	* @param end the new absolute end index
	*/
	void adjust(std::streamsize start, std::streamsize end);

	const char* data() const;
	std::streamsize size() const;
protected:
	///Factors start into pointer
	const char * _gptr() const;
public:
	/**
	 * Constructs a StreamView by copying the data
	 * Use to ensure lifetime of the StreamView
	 * @param s 
	 * @return 
	 */
	static StreamView make(std::string&& s);
	static StreamView make(std::vector<char>&& b);

};

std::ostream & operator<<(std::ostream & s, const StreamView & str);

template<typename T>
inline T StreamView::parse(std::streamsize start, std::streamsize end, int base)
{
	static_assert(std::is_integral<T>::value);
	if (end == -1) end = _size;
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

inline std::streamsize StreamView::rel2abs(std::streamsize relativeOffset) {
	return relativeOffset + start;
}


