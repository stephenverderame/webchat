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
	return errorCode;
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
	const char* in = str != nullptr ? str->icurrent_c() : &pimpl->obuffer[0];
	std::streamsize sizeToProcess = str != nullptr ? str->iend_c() - str->icurrent_c() : pimpl->o;
	int bufferMultiplier = 1;
	int err = 0;
	std::streamsize processedOutput = 0, processedInput = 0;
	err = deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);
	if (err < Z_OK) return err;
	pimpl->ibuffer->resize(deflateBound(&zs, sizeToProcess));
	do {
		processedOutput = zs.total_out;
		processedInput = zs.total_in;
		if (err == Z_MEM_ERROR || err == Z_BUF_ERROR) {
			bufferMultiplier *= 2;
			pimpl->ibuffer->resize(sizeToProcess * bufferMultiplier);
		}
		zs.avail_in = sizeToProcess - processedInput;
		zs.next_in = (Bytef*)in + processedInput;
		zs.next_out = (Bytef*)pimpl->ibuffer->data() + processedOutput;
		zs.avail_out = pimpl->ibuffer->size() - processedOutput;
		err = deflate(&zs, Z_FINISH);
	} while (err == Z_MEM_ERROR || err == Z_BUF_ERROR || (processedInput < sizeToProcess && err > 0));
	processedOutput = zs.total_out;
	int e = deflateEnd(&zs);
	if (err < Z_OK) return err;
	pimpl->iend = processedOutput;
	setg(pimpl->ibuffer->data(), pimpl->ibuffer->data(), pimpl->ibuffer->data() + pimpl->iend);
	return zs.total_out;
}

int GzipCoder::nvi_decode()
{
	z_stream zs;
	memset(&zs, Z_NULL, sizeof(z_stream));
	const char* in = str != nullptr ? str->icurrent_c() : &pimpl->obuffer[0];
	std::streamsize sizeToProcess = str != nullptr ? str->iend_c() - str->icurrent_c() : pimpl->o;
	int bufferMultiplier = 1;
	int err = 0;
	std::streamsize processedOutput = 0, processedInput = 0;
	err = inflateInit2(&zs, 16 + MAX_WBITS);
	if (err < Z_OK) return err;
	pimpl->ibuffer->resize(sizeToProcess * 5);
	do {
		processedOutput = zs.total_out;
		processedInput = zs.total_in;
		if (err == Z_MEM_ERROR || err == Z_BUF_ERROR) {
			bufferMultiplier *= 2;
			pimpl->ibuffer->resize(sizeToProcess * bufferMultiplier);
		}
		zs.avail_in = sizeToProcess - processedInput;
		zs.next_in = (Bytef*)in + processedInput;
		zs.next_out = (Bytef*)pimpl->ibuffer->data() + processedOutput;
		zs.avail_out = pimpl->ibuffer->size() - processedOutput;
		err = inflate(&zs, Z_FINISH);
	} while (err == Z_MEM_ERROR || err == Z_BUF_ERROR || (processedInput < sizeToProcess && err > 0));
	processedOutput = zs.total_out;
	int e = inflateEnd(&zs);
	if (err < Z_OK) return err;
	pimpl->iend = processedOutput;
	setg(pimpl->ibuffer->data(), pimpl->ibuffer->data(), pimpl->ibuffer->data() + pimpl->iend);
	return zs.total_out;
}
