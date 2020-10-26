#pragma once
#include <iostream>
// Check windows
#if _WIN32 || _WIN64
#if _WIN64
#define ENV_64
#else
#define ENV_32
#endif
#endif

// Check GCC
#if __GNUC__
#if __x86_64__ || __ppc64__
#define ENV_64
#else
#define ENV_32
#endif
#endif

#ifdef ENV_32
using st_size = unsigned long;
#else
using st_size = unsigned long long;
#endif
constexpr st_size STR_INVALID = ~0;
class StreamView;
class Streamable : public std::streambuf, public std::iostream
{
protected:
	struct impl;
	std::unique_ptr<impl> pimpl;
protected:
	/*
	 Writes data to the underlying object
	 @return amount of bytes written or < 0 for error
	*/
	virtual int nvi_write(const char * data, size_t len) = 0;
	/*
	 Reads data from the underlying object
	 @return amount of bytes written or < 0 for error
	*/
	virtual int nvi_read(char * data, size_t amtToRead) const = 0;
	/*
	 @param errorCode. The return value from another function (ie. nvi_write)
	 @return an extended information error code to get more information
	*/
	virtual int nvi_error(int errorCode) const = 0;
	virtual int minAvailableBytes() const = 0;
	//@return true if there is data available on underlying object
	virtual bool nvi_available() const = 0;

	//appends character to output buffer
	int overflow(int ch) override;
	//gets character from input buffer
	int underflow() override;
	//puts a block of characters into the input buffer
	std::streamsize xsputn(const char * s, std::streamsize n) override;
//	std::streamsize xsgetn(char * s, std::streamsize n) override;
	//seeks through buffer
	std::streampos seekpos(std::streampos pos, std::ios_base::openmode streamType = std::ios_base::in) override;
	std::streampos seekoff(std::streamoff offset, std::ios_base::seekdir way, std::ios_base::openmode mode = std::ios_base::in) override;

	//writes entire buffer and reads everything
	int sync() override;

	//makes sure index is within the stream buffer.
	std::streamsize checkSeekBounds(std::streamsize index, std::ios_base::openmode streamType);

	std::streamsize showmanyc() override;

	//pass -1 to enable dynamic buffer resizing (works the same way as the input) - requires a manual sync to call nvi_write
	//default is 2048
	void setOutputBufferSize(std::streamsize s);

	//function to set getptr for both streambuf and viewable interface
	void setgetp(char * begin, char * next, char * end);

public:

	int getError(int code) const;

	Streamable();
	~Streamable();

	bool available();

	//iterators to look through input buffer
	const char * ibegin_c() const;
	const char * iend_c() const;
	const char * icurrent_c() const;

	char * ibegin();
	char * iend();
	char * icurrent();

	std::streamsize icur() const;

	//iterators for output buffer
	const char* obegin_c() const;
	const char* oend_c() const;

	//indicates that the data in the buffer is no longer needed
	void purge();

	//gets/writes data until there is no more (0 return) or error (non zero return)
	int syncOutputBuffer();
	int syncInputBuffer();

	std::streamsize getBufferSize(std::ios_base::openmode type = std::ios::in) const;

	//returns a streamview that has shared ownership of the buffer
	StreamView getSharedView(std::streamoff origin = std::ios::beg, std::streamsize start = 0, std::streamsize end = -1) const;

	//returns a streamview that does not own the buffer, and must have a shorter lifetime than the stream
	StreamView view(std::streamsize start = 0, std::streamsize end = -1, std::ios_base::openmode channel = std::ios::in) const;

	//Writes the specified buffer from the other stream to this streams output buffer
	//Starting with the streams current position
	void write(const Streamable & other, std::ios_base::openmode buffer);

	//Warning: extremely expensive. Best to do at the end of the buffer
	void remove(std::streamsize start, std::streamsize end = -1, std::ios::openmode buffer = std::ios::in);


	/**
	* Controlled reading of input stream to buffer until a delimeter is reached
	* Fetchs packets of size packetSize, then searches for the delimeter
	* @param delim the delimeter to search for
	* @param packetSize the size of each new packet read from the input
	* @param quitOnEmpty true to exit if no data is read, false to only exit on error or delim
	* @return the index of the delimeter
	*/
	std::streamsize fetchUntil(const char* delim, std::streamsize packetSize = 10, bool quitOnEmpty = false);
	/**
	* Controlled reading of input stream to buffer
	* Reads a specified amount of data
	* @param size the amount to read
	* @param quitOnEmpty true to quit if no more data is read, false to wait until size 
	* @return amount actually read or < 0 if error
	*/
	std::streamsize fetchFor(std::streamsize size, bool quitOnEmpty = false);


};
/*
pbase()	Beginning of the buffered part of the output sequence
pptr()	Current position in the output sequence ("put pointer")
epptr()	End of the buffered part of the output sequence

eback()	Beginning of the buffered part of the input sequence
gptr()	Current position in the input sequence ("get pointer")
egptr()	End of the buffered part of the input sequence
*/

/**
* @param uri    uri in the form of scheme:[//]host[:port][/path] [] Denote optional components
* @return       Streamable object that interfaces with the desired stream or nullptr on failure
*/
std::unique_ptr<Streamable> make_stream(const char* uri);

std::ostream& operator<<(std::ostream& strm, const Streamable& other);
