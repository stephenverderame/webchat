#include <type_traits>
#include "StreamableImpl.h"
#include <algorithm>
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
		pimpl->i = gptr() - eback();
		if (pimpl->iend == pimpl->ibuffer->capacity())
			pimpl->ibuffer->resize(pimpl->ibuffer->capacity() * 2);
		int r;
		if ((r = nvi_read(&pimpl->ibuffer->at(pimpl->iend), pimpl->ibuffer->capacity() - pimpl->iend)) <= 0) return std::streambuf::traits_type::eof();
		pimpl->iend += r;
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(pimpl->i), &pimpl->ibuffer->at(0) + pimpl->iend);
		return std::streambuf::traits_type::to_int_type(pimpl->ibuffer->at(pimpl->i));
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
		pimpl->i = checkSeekBounds(pos, in);
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(pimpl->i), &pimpl->ibuffer->at(0) + pimpl->iend);
		return pimpl->i;
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
		pimpl->i = checkSeekBounds(p + offset, std::ios_base::in);
		setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(pimpl->i), &pimpl->ibuffer->at(0) + pimpl->iend);
		return std::streampos(std::streamoff(pimpl->i));
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
	return (eback() + pimpl->iend) - gptr();
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

int Streamable::syncOutputBuffer()
{
	int count = 0, ret = 0;
	do {
		ret = nvi_write(&pimpl->obuffer[count], pimpl->o - count);
		if (ret < 0) return -1;
		count += ret;
	} while (ret > 0 && count < pimpl->o);
	pimpl->o = 0;
	setp(&pimpl->obuffer[0], &pimpl->obuffer[1]);
	return 0;
}

int Streamable::syncInputBuffer()
{
	return fetch(std::numeric_limits<std::streamsize>().max()); //fetch as much data as there is
}

std::streamsize Streamable::getBufferSize(std::ios_base::openmode type) const
{
	if (type == std::ios::in)
		return pimpl->iend;
	else
		return pimpl->o;
}

StreamView Streamable::getStreamView(std::streamsize start, std::streamsize end) const
{
	return StreamView(pimpl->ibuffer, start, end == -1 ? pimpl->iend : end);
}

StreamView Streamable::view(std::streamsize start, std::streamsize end) const
{
	end = end == -1 ? pimpl->iend : end;
	return StreamView(&pimpl->ibuffer->at(0), start, end);
}

void Streamable::write(const Streamable & other, std::ios_base::openmode buffer)
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
		pimpl->ibuffer->erase(pimpl->ibuffer->begin() + start, pimpl->ibuffer->begin() + end);
		pimpl->iend -= (end - start);
	}
}

int Streamable::fetch(std::streamsize amount, bool hardAmount, const char * delim, bool hardDelim)
{
	std::streamsize oldPos = gptr() - &pimpl->ibuffer->at(0);
	std::streamsize read = 0;
	int ret;
	std::unique_ptr<StreamView> sv;
	if (delim != nullptr) sv = std::make_unique<StreamView>();
	bool foundDelim = delim == nullptr;
	do {
		if (pimpl->iend == pimpl->ibuffer->capacity())
			pimpl->ibuffer->resize(pimpl->ibuffer->capacity() * 2);
		ret = nvi_read(&pimpl->ibuffer->at(pimpl->iend), std::min(pimpl->ibuffer->capacity() - pimpl->iend, amount - read));
		if (ret < 0) return ret;
		if (delim != nullptr) {
			sv->assign(&pimpl->ibuffer->at(0), pimpl->iend, ret);
			std::streamsize ss;
			if ((ss = sv->find_mt(delim)) != StreamView::INVALID) {
				foundDelim = true;
				if (!hardAmount || (hardDelim && read + ret >= amount)) {
					pimpl->iend += ret;
					return ss;
				}
			}
		}
		pimpl->iend += ret;
		read += ret;
	} while ((hardDelim && !foundDelim) || (hardAmount && read < amount) || (ret > 0 && !foundDelim && read < amount));
	setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(oldPos), &pimpl->ibuffer->at(0) + pimpl->iend);
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

void Streamable::purge()
{
	pimpl->i = 0;
	pimpl->iend = 0;
	pimpl->o = 0;
	if(pimpl->ibuffer.use_count() > 1)
		pimpl->ibuffer = std::make_shared<std::vector<char>>(4096);
	setgetp(&pimpl->ibuffer->at(0), &pimpl->ibuffer->at(0), &pimpl->ibuffer->at(1));
	setp(&pimpl->obuffer[0], &pimpl->obuffer[1]);
}