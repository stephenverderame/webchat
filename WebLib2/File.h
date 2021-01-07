#pragma once
#include "Streamable.h"
class FileMode {
private:
	const char* const mode;
	FileMode(const char* mode) : mode(mode) {}
public:
	const static FileMode read, write, append, openReadWrite, readWrite, readAppend,
		read_b, write_b, append_b, openReadWrite_b, readWrite_b, readAppend_b;

	inline const char* const getMode() const noexcept { return mode; }

	/**
	 * Creates a fileMode from the given string if it is valid
	 * 
	 * @param mode the String representing a file mode
	 * @return a FileMode representing the specified mode
	 * @throw StreamException on error
	 */
	static const FileMode make(const char* mode);
};
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

public:
	File(const char * file, FileMode mode = FileMode::readWrite);

	///Syncs output buffer and closes file stream
	~File();
};


