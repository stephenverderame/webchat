#define _CRT_SECURE_NO_WARNINGS
#include "File.h"

const FileMode FileMode::read("r"), FileMode::write("w"), FileMode::append("a"), 
FileMode::openReadWrite("r+"), FileMode::readWrite("w+"), FileMode::readAppend("a+"), FileMode::read_b("rb"), 
FileMode::write_b("wb"), FileMode::append_b("ab"), FileMode::openReadWrite_b("rb+"), 
FileMode::readWrite_b("wb+"), FileMode::readAppend_b("ab+");

int File::nvi_write(const char * data, size_t len)
{
	int w = fwrite(data, 1, len, f);
	int err;
	if (w != len && (err = ferror(f)) != 0) throw StreamException(err, "Error writing to file");
	return w;
}

int File::nvi_read(char * data, size_t amtToRead) const
{
	int r = fread(data, 1, amtToRead, f);
	int err;
	if (r != amtToRead && (err = ferror(f)) != 0) throw StreamException(err, "Error reading from file");
	return r;
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

File::File(const char * file, FileMode mode)
{
	if((f = fopen(file, mode.getMode())) == NULL) throw StreamException(-1, "Cannot open file");
}

File::~File()
{
	try {
		syncOutputBuffer();
	}
	catch (StreamException&) {};
	fclose(f);
}

const FileMode FileMode::make(const char* mode)
{
	const auto len = strlen(mode);
	if (len > 3) throw StreamException(20, "Invalid filemode");
	char count = 0;
	for (decltype(strlen(0)) i = 0; i < len; ++i) {
		if (mode[i] == 'r' || mode[i] == 'w') count += 100;
		else if (mode[i] == 'b') count += 10;
		else if (mode[i] == '+') ++count;
		else throw StreamException(10, "Invalid filemode");
	}
	if (count % 10 <= 1 && (count / 10) % 10 <= 1 && count <= 111)
		return FileMode(mode);
	else 
		throw StreamException(2, "Invalid filemode");

}
