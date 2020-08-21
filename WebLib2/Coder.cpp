#include "Coder.h"
#include <zlib.h>
#include <vector>
#include <array>
#include "StreamableImpl.h"

int Coder::encode()
{
	return nvi_encode();
}

int Coder::decode()
{
	return nvi_decode();
}

Coder::Coder(const Streamable * input) : str(input)
{
	pimpl->maxOutputSize = std::numeric_limits<std::streamsize>().max(); //no limit
}

Coder::Coder() : str(nullptr)
{
}

//Uses parent buffer, so when that runs out there is no more data
int GzipCoder::nvi_write(const char * data, size_t len)
{
	return 0;
}
int GzipCoder::nvi_read(char * data, size_t amt) const
{
	return 0;
}

int GzipCoder::nvi_error(int errorCode) const
{
	return 0;
}

int GzipCoder::minAvailableBytes() const
{
	return egptr() - gptr();
}

bool GzipCoder::nvi_available() const
{
	//no extra data besides what is in the parent buffer
	return false;
}

int GzipCoder::nvi_encode()
{
	z_stream zs;
	memset(&zs, Z_NULL, sizeof(z_stream));
	if (str != nullptr) {
		std::streamsize sizeToProcess = (str->iend_c() - str->icurrent_c() + 1);
//		result.resize(sizeToProcess * 2);
		zs.avail_in = sizeToProcess;
		zs.next_in = (Bytef*)str->icurrent_c();
//		zs.avail_out = result.size();		
		int err = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15 | 16, 8, Z_DEFAULT_STRATEGY);
		if (err < Z_OK) return err;
		result.resize(deflateBound(&zs, sizeToProcess));
		zs.avail_out = result.size();
		zs.next_out = (Bytef*)result.data();
		err = deflate(&zs, Z_FINISH);
		if (err < Z_OK) return err;
		err = deflateEnd(&zs);
		if (err < Z_OK) return err;
		pimpl->i = 0;
		pimpl->iend = zs.total_out - 1;
		setg(result.data(), result.data() + 1, result.data() + zs.total_out);
	}
	else {
		std::streamsize sizeToProcess = pimpl->o;
		zs.avail_in = sizeToProcess;
		zs.next_in = (Bytef*)&pimpl->obuffer[0];		
		int err = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 | MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
		if (err < Z_OK) return err;
		result.resize(deflateBound(&zs, sizeToProcess));
		zs.avail_out = result.size();
		zs.next_out = (Bytef*)result.data();
		err = deflate(&zs, Z_FINISH);
		if (err < Z_OK) return err;
		err = deflateEnd(&zs);
		if (err < Z_OK) return err;
		pimpl->i = 0;
		pimpl->iend = zs.total_out - 1;
		setg(result.data(), result.data() + 1, result.data() + zs.total_out);
		syncOutputBuffer();
	}
	return zs.total_out;
}

int GzipCoder::nvi_decode()
{
	z_stream zs;
	memset(&zs, Z_NULL, sizeof(z_stream));
	if (str != nullptr) {
		std::streamsize sizeToProcess = (str->iend_c() - str->icurrent_c() + 1);
		result.resize(sizeToProcess * 5);
		zs.avail_in = sizeToProcess;
		zs.next_in = (Bytef*)str->icurrent_c();
		zs.next_out = (Bytef*)result.data();
		zs.avail_out = result.size();
		int err = inflateInit2(&zs, 16 + MAX_WBITS);
		if (err < Z_OK) return err;
		err = inflate(&zs, Z_FINISH);
		if (err < Z_OK) return err;
		err = inflateEnd(&zs);
		if (err < Z_OK) return err;
		pimpl->i = 0;
		pimpl->iend = zs.total_out - 1;
		setg(result.data(), result.data() + 1, result.data() + zs.total_out);
	}
	else {
		std::streamsize sizeToProcess = pimpl->o;
		result.resize(sizeToProcess * 5);
		zs.avail_in = sizeToProcess;
		zs.next_in = (Bytef*)&pimpl->obuffer[0];
		zs.next_out = (Bytef*)result.data();
		zs.avail_out = result.size();
		int err = inflateInit2(&zs, 16 + MAX_WBITS);
		if (err < Z_OK) return err;
		err = inflate(&zs, Z_FINISH);
		if (err < Z_OK) return err;
		err = inflateEnd(&zs);
		if (err < Z_OK) return err;
		pimpl->i = 0;
		pimpl->iend = zs.total_out - 1;
		setg(result.data(), result.data() + 1, result.data() + zs.total_out);
		syncOutputBuffer();
	}
	return zs.total_out;
}
