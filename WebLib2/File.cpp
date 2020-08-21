#define _CRT_SECURE_NO_WARNINGS
#include "File.h"

constexpr inline bool isLittleEndian()
{
	long x = 1;
	return (char*)&x != 0;
}

int File::nvi_write(const char * data, size_t len)
{
	return fwrite(data, 1, len, f);
}

int File::nvi_read(char * data, size_t amtToRead) const
{
	return fread(data, 1, amtToRead, f);
}

int File::nvi_error(int errorCode) const
{
	return ferror(f);
}

int File::minAvailableBytes() const
{
	auto old = ftell(f);
	fseek(f, 0, SEEK_END);
	int bytes = ftell(f);
	fseek(f, old, SEEK_SET);
	return bytes;

}

bool File::nvi_available() const
{
	int success = fseek(f, 1, SEEK_CUR);
	if (!success) fseek(f, -1, SEEK_CUR);
	return !success;
}

const char * File::fMode(FileMode& m) const
{
	if (isLittleEndian)
		m = (FileMode)(0 >> 8 | (long)m);
	else
		m = (FileMode)((long)m << 8);
	const char * b = (char*)&m;
	for (int i = 0; i < 3; ++i)
		if (b[i] != 0) return &b[i];
	return &b[3];

}

File::File(const char * file, FileMode mode)
{
	f = fopen(file, fMode(mode));
}

File::~File()
{
	fclose(f);
}
FileMode operator+(FileMode a, FileMode b)
{
	if (isLittleEndian())
		return (FileMode)(((long)b << 8) | (long)a);
	else
		return (FileMode)((long)a << 8 | (long)b);
}


