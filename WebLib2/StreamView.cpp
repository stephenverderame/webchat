#include "StreamView.h"
#include <atomic>
#include <future>
constexpr st_size MIN_SEARCH_ELEMENTS = 100;
std::streamsize find_mt_helper_hash(long needle, long power, const char * haystack, size_t start, size_t hlen, size_t nlen, std::atomic<bool> & flag, bool end) {
	if (hlen <= MIN_SEARCH_ELEMENTS) {
		long h2 = 0;
		auto it = haystack + start;
		for (it; it < haystack + start + nlen && !flag.load(); ++it) {
			h2 = h2 * StreamView::BASE + *it;
			h2 %= StreamView::MOD;
		}
		const char * endP = end ? haystack + start + hlen - nlen : haystack + start + hlen;
		for (it; it < endP && !flag.load(); ++it) {
			h2 = h2 * StreamView::BASE + *it;
			h2 %= StreamView::MOD;
			h2 -= power * *(it - nlen) % StreamView::MOD;
			if (h2 < 0) h2 += StreamView::MOD;
			if (h2 == needle) {
				flag.store(true);
				return it - haystack;
			}
		}
		return StreamView::INVALID;
	}
	else {
		st_size mdpt = hlen / 2;
		auto f1 = std::async(&find_mt_helper_hash, needle, power, haystack, start, mdpt, nlen, std::ref(flag), false);
		auto f2 = find_mt_helper_hash(needle, power, haystack, start + mdpt, hlen - mdpt, nlen, std::ref(flag), end);
		return f2 == StreamView::INVALID ? f1.get() : f2;
	}
}
std::streamsize find_mt_helper(const char * needle, const char * haystack, size_t start, size_t hlen, size_t nlen, std::atomic<bool> & flag, bool end) {
	if (hlen <= MIN_SEARCH_ELEMENTS) {
		st_size endP = end ? hlen - nlen : hlen;
		for (st_size i = 0; i < endP && !flag.load(); ++i) {
			if (memcmp(needle, &haystack[start + i], nlen) == 0) {
				flag.store(true);
				return start + i;
			}
		}
		return StreamView::INVALID;
	}
	else {
		st_size mdpt = hlen / 2;
		auto f1 = std::async(&find_mt_helper, needle, haystack, start, mdpt, nlen, std::ref(flag), false);
		auto res = find_mt_helper(needle, haystack, start + mdpt, hlen - mdpt, nlen, std::ref(flag), end);
		return res == StreamView::INVALID ? f1.get() : res;
	}
}
std::streamsize find_mt_helper_char(char needle, const char * haystack, size_t start, size_t hlen, std::atomic<bool> & flag) {
	if (hlen <= MIN_SEARCH_ELEMENTS) {
		for (size_t i = 0; i < hlen && !flag.load(); ++i) {
			if (haystack[start + i] == needle) {
				flag.store(true);
				return start + i;
			}
		}
		return StreamView::INVALID;
	}
	else {
		st_size mdpt = hlen / 2;
		auto f1 = std::async(&find_mt_helper_char, needle, haystack, start, mdpt, std::ref(flag));
		auto res = find_mt_helper_char(needle, haystack, start + mdpt, hlen - mdpt, std::ref(flag));
		return res == StreamView::INVALID ? f1.get() : res;
	}
}
StreamView::StreamView(const char * parent, std::streamsize start, std::streamsize end) : parent_p(parent), start(start), size(end - start)
{
}
StreamView::StreamView(const std::shared_ptr<std::vector<char>>& parent, std::streamsize start, std::streamsize end) : parent_p(nullptr), parent(parent), start(start), size(end - start), ptr(start)
{
}
StreamView::StreamView() : parent_p(nullptr), parent(nullptr), start(0), size(0), ptr(0) {}
StreamView::StreamView(StreamView&& other) : parent_p(other.parent_p), parent(other.parent), start(other.start), size(other.size), ptr(other.ptr) {}
StreamView::StreamView(const StreamView & other) : parent_p(other.parent_p), parent(other.parent), start(other.start), size(other.size), ptr(other.ptr) {}
StreamView & StreamView::operator=(const StreamView & other) = default;
StreamView & StreamView::operator=(StreamView && other) = default;
StreamView::~StreamView() = default;
const char * StreamView::_gptr() const
{
	if (parent_p != nullptr) return parent_p + start;
	else if (parent == nullptr) throw std::exception("StreamView does not reference any data");
	else return &parent->at(start);
}
std::streamsize StreamView::find(char k, std::streamsize start) const
{
	auto it = _gptr() + start;
	for (; it < _gptr() + size; ++it) {
		if (*it == k) return it - _gptr();
	}
	return INVALID;
}
std::streamsize StreamView::find(const char * k, std::streamsize start, size_t len) const
{
	if (len == -1) len = strlen(k);
	if (size >= len) {
		auto it = _gptr() + start;
		if (len <= 8) {
			for (it; it <= _gptr() + size - len; ++it) {
				if (memcmp(it, k, len) == 0) return it - _gptr();
			}
		}
		else {
			const long key = hash(k, len);
			long h2 = 0;
			long power = 1;
			for (size_t i = 0; i < len; ++i)
				power = (power * BASE) % MOD;			
			const auto sp = it;
			//initialize hash
			for (it; it < sp + len; ++it) {
				h2 = h2 * BASE + *it;
				h2 %= MOD;
			}
			for (it; it < _gptr() + size; ++it) {
				h2 = h2 * BASE + *it;
				h2 %= MOD;
				h2 -= power * *(it - len) % MOD;
				if (h2 < 0) h2 += MOD;
				if (h2 == key) return it - _gptr();
			}
		}
	}
	return INVALID;
}

std::streamsize StreamView::find_mt(const char * k, std::streamsize start, size_t len) const
{
	len = len == -1 ? strlen(k) : len;
	std::atomic<bool> flag(false);
	if (len == 1) {
		return find_mt_helper_char(*k, _gptr(), start, size, flag);
	}
	if (len <= 8) {
		return find_mt_helper(k, _gptr(), start, size, len, flag, true);
	}
	else {
		const long key = hash(k, len);
		long power = 1;
		for (size_t i = 0; i < len; ++i)
			power = (power * BASE) % MOD;
		return find_mt_helper_hash(key, power, _gptr(), start, size, len, flag, true);
	}
	
}

StreamView StreamView::sub(std::streamsize start, std::streamsize end) const
{
	if (end == -1) end = size;
	if (parent_p != nullptr)
		return StreamView(parent_p, this->start + start, this->start + end);
	if (parent != nullptr)
		return StreamView(parent, this->start + start, this->start + end);
}

std::string StreamView::sub_cpy(std::streamsize start, std::streamsize end) const
{
	end = end == -1 ? size : end;
	return std::string(_gptr(), _gptr() + end + 1);
}

void StreamView::put(std::ostream & stream) const
{
	stream.write(_gptr(), size);
}

std::string StreamView::copy() const
{
	return std::string(_gptr(), _gptr() + size);
}

bool StreamView::getsub(StreamView & view, char delim) const
{
	std::streamsize index;
	if ((index = find(delim, ptr)) != INVALID) {
		view.start = ptr;
		view.size = index - ptr;
		view.parent_p = parent_p;
		if (view.parent != parent) view.parent = parent;
		ptr = index + 1;
		return true;
	}
	return false;
}

bool StreamView::getsub(StreamView & view, const char * delim) const
{
	std::streamsize index;
	if ((index = find(delim, ptr)) != INVALID) {
		view.start = ptr;
		view.size = index - ptr;	
		view.parent_p = parent_p;
		if(view.parent != parent) view.parent = parent;
		ptr = index + strlen(delim);
		return true;
	}
	return false;
}

char StreamView::operator[](std::streamsize index) const
{
	return _gptr()[index];
}

char StreamView::get() const
{
	return _gptr()[ptr];
}

std::streamsize StreamView::tell() const
{
	return ptr - start;
}

void StreamView::seek(std::streamsize offset, std::ios::seekdir reference) const
{
	std::streamsize origin = reference == std::ios::beg ? 0 : (reference == std::ios::end ? size : ptr);
	ptr = origin + offset;
}

void StreamView::advance(std::streamsize offset) const
{
	ptr += offset;
}

long StreamView::hash() const
{
	return hash(_gptr(), size);
}

bool StreamView::operator==(const StreamView & other) const
{
	if (size == other.size) {
		for (int i = 0; i < size; ++i) {
			seek(std::ios::beg, i);
			other.seek(std::ios::beg, i);
			if (get() != other.get()) return false;
		}
		return true;
	}
	return false;
}

bool StreamView::operator==(const char * other) const
{
	if (size == strlen(other)) {
		for (int i = 0; i < size; ++i) {
			if (*(_gptr() + i) != other[i]) return false;
		}
		return true;
	}
	return false;
}

std::streamsize StreamView::getSize() const
{
	return size;
}

void StreamView::assign(const char * mem, std::streamsize start, std::streamsize end)
{
	parent_p = mem;
	this->start = start;
	size = end - start;
}

long StreamView::hash(const char * str, size_t len)
{
	if (len == -1) len = strlen(str);
	long r = 0;
	for (int i = 0; i < len; ++i) {
		r = r * BASE + toupper(str[i]);
		r %= MOD;
	}
	return r;
}

std::ostream & operator<<(std::ostream & s, const StreamView & str)
{
	str.put(s);
	return s;
}


