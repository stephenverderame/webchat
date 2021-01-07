#define _CRT_SECURE_NO_WARNINGS
#include <type_traits>
#include "StreamableImpl.h"
#include <algorithm>
#include "Connection.h"
#undef min
#undef max
int Streamable::getError(int code) const
{
	return nvi_error(code);
}
Streamable::Streamable() : std::iostream(this), pimpl(std::make_unique<impl>())
{
	pimpl->obuffer.resize(2048);
	setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(0), &pimpl->ibuffer->at(1));
	setp(&pimpl->obuffer[0], &pimpl->obuffer[1]);
};
Streamable::~Streamable() = default;

int Streamable::overflow(int ch)
{
	if (pimpl->o == pimpl->obuffer.size()) {
		if (pimpl->obuffer.size() < pimpl->maxOutputSize) pimpl->obuffer.resize(std::min((std::streamsize)pimpl->obuffer.size() * 2, pimpl->maxOutputSize));
		else if (syncOutputBuffer() != 0) return std::streambuf::traits_type::eof();
	}
	pimpl->obuffer[pimpl->o++] = (char)ch;
	setp(&pimpl->obuffer[0], &pimpl->obuffer[0] + pimpl->o);
	return std::streambuf::traits_type::to_int_type(ch);
}

int Streamable::underflow()
{
	if (gptr() < egptr())
		return std::streambuf::traits_type::to_int_type(*gptr());
	else {
		auto i = gptr() - eback();
		if (pimpl->iend == pimpl->ibuffer->size())
			pimpl->ibuffer->resize(pimpl->ibuffer->size() * 2);
		int r;
		if ((r = nvi_read(&pimpl->ibuffer->at(pimpl->iend), pimpl->ibuffer->size() - pimpl->iend)) <= 0) return std::streambuf::traits_type::eof();
		pimpl->iend += r;
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(i), &pimpl->ibuffer->at(0) + pimpl->iend);
		return std::streambuf::traits_type::to_int_type(pimpl->ibuffer->at(i));
	}
}

std::streamsize Streamable::xsputn(const char * s, std::streamsize n)
{
	std::streamsize extra = n - (pimpl->obuffer.size() - pimpl->o);
	std::streamsize moveSize = std::min(pimpl->obuffer.size() - pimpl->o, n);
	memcpy(&pimpl->obuffer[pimpl->o], s, moveSize);
	pimpl->o += moveSize;
	setp(&pimpl->obuffer[0], &pimpl->obuffer[0] + pimpl->o);
	if (extra > 0) {
		if (pimpl->obuffer.size() + extra < pimpl->maxOutputSize) {
			pimpl->obuffer.resize(std::min(std::max(pimpl->obuffer.size() + extra, (std::streamsize)pimpl->obuffer.size() * 2), pimpl->maxOutputSize));
			memcpy(&pimpl->obuffer[pimpl->o], s + moveSize, extra);
			pimpl->o += extra;
			moveSize += extra;
		}
		else {
			syncOutputBuffer();
			std::streamsize count = 0, ret = 0;
			do {
				ret = nvi_write(s + moveSize + count, n - moveSize - count);
				if (ret <= 0) break;
				count += ret;
			} while (true);
			moveSize += count;
		}
	}
	return moveSize;

}

std::streampos Streamable::seekpos(std::streampos pos, std::ios_base::openmode streamType)
{
	if (streamType & std::ios_base::in) {
		auto i = checkSeekBounds(pos, in);
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(i), &pimpl->ibuffer->at(0) + pimpl->iend);
		return i;
	}
	if (streamType & std::ios_base::out) {
		pos = checkSeekBounds(pos, out);
		setp(&pimpl->obuffer[pos], &pimpl->obuffer[0] + pimpl->o);
		return pos;
	}
	return std::streambuf::pos_type(std::streambuf::off_type(-1));
}

std::streampos Streamable::seekoff(std::streamoff offset, std::ios_base::seekdir way, std::ios_base::openmode mode)
{
	if (mode & std::ios_base::out) {
		std::streamsize p = way == std::ios_base::beg ? 0 : (way == std::ios_base::end ? pimpl->o : pptr() - &pimpl->obuffer[0]);
		p = checkSeekBounds(p, std::ios_base::out);
		setp(&pimpl->obuffer[0], &pimpl->obuffer[0] + pimpl->o);
		return std::streampos(std::streamoff(p));
	}
	else {
		std::streamsize p = way == std::ios_base::beg ? 0 : (way == std::ios_base::end ? pimpl->iend : gptr() - &pimpl->ibuffer->at(0));
		auto i = checkSeekBounds(p + offset, std::ios_base::in);
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(i), &pimpl->ibuffer->at(0) + pimpl->iend);
		return std::streampos(std::streamoff(i));
	}
}

int Streamable::sync()
{
	int ret = 0;
	ret |= syncOutputBuffer();
	ret |= syncInputBuffer();
	return ret;
}

std::streamsize Streamable::checkSeekBounds(std::streamsize index, std::ios_base::openmode streamType)
{ 
	if (index < 0) return 0;
	if (streamType == std::ios_base::in) {
		if (index >= pimpl->iend) {
			return pimpl->iend;
		}		
	}
	else if (index >= pimpl->o) return pimpl->o;
	return index;
}

std::streamsize Streamable::showmanyc()
{
	return egptr() - gptr();
}

void Streamable::setOutputBufferSize(std::streamsize s)
{
	pimpl->maxOutputSize = s;
}

void Streamable::setgetp(char * begin, char * next, char * end)
{
	setg(begin, next, end);
//	setgp(begin);
}

int Streamable::syncOutputBuffer() throw(StreamException)
{
	int count = 0, ret = 0;
	do {
		ret = nvi_write(&pimpl->obuffer[count], pimpl->o - count);
		count += ret;
	} while (ret > 0 && count < pimpl->o);
	pimpl->o = 0;
	setp(&pimpl->obuffer[0], &pimpl->obuffer[1]);
	return 0;
}

int Streamable::syncInputBuffer() throw(StreamException)
{
	long ret = 0;
	long long read = 0;
	auto pos = gptr() - eback();
	do {
		if (pimpl->iend == pimpl->ibuffer->size())
			pimpl->ibuffer->resize(pimpl->ibuffer->size() * 2);
		ret = nvi_read(&pimpl->ibuffer->at(pimpl->iend), pimpl->ibuffer->size() - pimpl->iend);
		if (ret >= 0) {
			read += ret;
			pimpl->iend += ret;
		}
	} while (ret > 0);
	setg(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(pos), &pimpl->ibuffer->at(0) + pimpl->iend);
	return read;
}

std::streamsize Streamable::getBufferSize(std::ios_base::openmode type) const
{
	if (type == std::ios::in)
		return pimpl->iend;
	else
		return pimpl->o;
}

StreamView Streamable::getSharedView(std::streamoff origin, std::streamsize start, std::streamsize end) const
{
	start = origin == std::ios::beg ? start : origin == std::ios::cur ? start + icur() : start + (egptr() - eback());
	return StreamView(pimpl->ibuffer, start, end == -1 ? pimpl->iend : end);
}

StreamView Streamable::view(std::streamsize start, std::streamsize end, std::ios::openmode channel) const
{
	if (channel == std::ios::in) {
		end = end == -1 ? pimpl->iend : end;
		return StreamView(&pimpl->ibuffer->at(0), start, end);
	}
	else {
		end = end == -1 ? (epptr() - pbase()) : end;
		return StreamView(&pimpl->obuffer[0], start, end);
	}
}

void Streamable::write(const Streamable & other, std::ios_base::openmode buffer) throw(StreamException)
{
	if (buffer == std::ios::in) {
		xsputn(other.icurrent_c(), other.iend_c() - other.icurrent_c());
	}
	else {
		xsputn(other.pimpl->obuffer.data(), other.pimpl->o);
	}
}

void Streamable::remove(std::streamsize start, std::streamsize end, std::ios::openmode buffer)
{
	if (buffer == std::ios::in) {
		end = end == -1 ? pimpl->iend : end;
		std::streamsize cur = icur() > start ? icur() - (end - start) : icur();		
		pimpl->ibuffer->erase(pimpl->ibuffer->begin() + start, pimpl->ibuffer->begin() + end);
		pimpl->iend -= (end - start);
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(cur), &pimpl->ibuffer->at(0) + pimpl->iend);
	}
}

std::streamsize Streamable::fetchUntil(const char* delim, std::streamsize packetSize, bool quitOnEmpty) throw(StreamException)
{
	int ret = 0;
	auto i = gptr() - eback();
	std::streamsize index = StreamView::INVALID;
	do {
		if (pimpl->iend == pimpl->ibuffer->size())
			pimpl->ibuffer->resize(pimpl->ibuffer->size() * 2);
		ret = nvi_read(&pimpl->ibuffer->at(pimpl->iend), std::min(pimpl->ibuffer->size() - pimpl->iend, packetSize));
		if (ret >= 0) {
			StreamView view(&pimpl->ibuffer->at(0), std::max(pimpl->iend - strlen(delim), 0LL), pimpl->iend + ret);
			index = view.rel2abs(view.find(delim));
			pimpl->iend += ret;
		}
	} while ((quitOnEmpty && ret > 0 && index == StreamView::INVALID) || (ret >= 0 && index == StreamView::INVALID));
	setg(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(i), &pimpl->ibuffer->at(0) + pimpl->iend);
	return index;
}

std::streamsize Streamable::fetchFor(std::streamsize size, bool quitOnEmpty)
{
	int ret = 0;
	long long read = 0;
	auto i = gptr() - eback();
	do {
		if (pimpl->iend == pimpl->ibuffer->size())
			pimpl->ibuffer->resize(pimpl->ibuffer->size() * 2);
		ret = nvi_read(&pimpl->ibuffer->at(pimpl->iend), std::min(pimpl->ibuffer->size() - pimpl->iend, size - read));
		if (ret >= 0) {
			read += ret;
			pimpl->iend += ret;
		}
	} while ((quitOnEmpty && ret > 0 && read < size) || (ret >= 0 && read < size));
	setg(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(i), &pimpl->ibuffer->at(0) + pimpl->iend);
	return read;
}


bool Streamable::available()
{
	return gptr() < egptr() || nvi_available();
}

const char * Streamable::ibegin_c() const
{
	return eback();
}

const char * Streamable::iend_c() const
{
	return egptr();
}

const char * Streamable::icurrent_c() const
{
	return gptr();
}

char * Streamable::ibegin()
{
	return eback();
}

char * Streamable::iend()
{
	return egptr();
}

char * Streamable::icurrent()
{
	return gptr();
}

std::streamsize Streamable::icur() const
{
	return gptr() - eback();
}

const char* Streamable::obegin_c() const
{
	return pbase();
}

const char* Streamable::oend_c() const
{
	return epptr();
}

void Streamable::purge()
{
	pimpl->iend = 0;
	pimpl->o = 0;
	if(pimpl->ibuffer.use_count() > 1)
		pimpl->ibuffer = std::make_shared<std::vector<char>>(4096);
	setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(0), &pimpl->ibuffer->at(1));
	setp(&pimpl->obuffer[0], &pimpl->obuffer[1]);
}

#include "Socket.h"
#include "File.h"
#include "SSLSocket.h"

std::unique_ptr<Streamable> make_stream(const char* uri)
{
	char scheme[50];
	size_t schemeLen = strchr(uri, ':') - uri;
	if (schemeLen >= strlen(uri)) throw StreamException(10, "Invalid uri");
	memcpy(scheme, uri, schemeLen);
	scheme[schemeLen] = '\0';
	const char* port = strrchr(uri, ':') + 1;
	bool usePort = strchr(uri, ':') != strrchr(uri, ':');
	char host[300];
	strcpy(host, &uri[schemeLen] + 1);
	char * end = strchr(host, ':');
	end = end == NULL ? end = strrchr(host, '/') : end;
	if (end != NULL && end != strstr(host, "//") + 1) host[end - host] = '\0';
	if (strcmp(scheme, "http") == 0) {		
		Connection con(Address(host + 2, usePort ? atoi(port) : 80), FDMethod::TCP);
		return std::make_unique<Socket>(std::move(con));
	}
	else if (strcmp(scheme, "https") == 0) {
		Connection con(Address(host + 2, usePort ? atoi(port) : 443), FDMethod::TCP);
		return std::make_unique<SSLSocket>(std::move(con));
	}
	else if(strcmp(scheme, "file") == 0) {
		char fm[7];
		if(usePort) strcpy(fm, port);
		return std::make_unique<File>(host, usePort ? FileMode::make(fm) : FileMode::readWrite);
	}
	throw StreamException(50, "Unknown Stream Type");
}

std::ostream& operator<<(std::ostream& strm, const Streamable& other) throw(StreamException)
{
	strm.write(other.icurrent_c(), other.iend_c() - other.icurrent_c());
	return strm;
}
