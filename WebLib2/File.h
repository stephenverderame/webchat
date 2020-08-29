#pragma once
#include "Streamable.h"
enum class FileMode : long {
	read = 'r',
	write = 'w',
	append = 'a',
	binary = 'b',
	update = '+'
};
FileMode operator+(FileMode a, FileMode b);
inline FileMode& operator+=(FileMode& a, FileMode b);
class File : public Streamable
{
private:
	FILE * f;
protected:
	int nvi_write(const char * data, size_t len) override;
	int nvi_read(char * data, size_t amtToRead) const override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;

	const char * fMode(FileMode& m) const;
public:
	File(const char * file, FileMode mode = FileMode::write + FileMode::update);

	//Syncs output buffer and closes file stream
	~File();
};
inline FileMode & operator+=(FileMode & a, FileMode b)
{
	a = a + b;
	return a;
}


