#pragma once
using hash_t = unsigned long;
namespace util{
	static constexpr hash_t BASE = 127, MOD = (1UL << 30UL) - 35UL;
	static constexpr long strlen_c(const char* str)
	{
		int i = 0;
		while (str[i] != '\0') ++i;
		return i;
	}
	static constexpr char toupper_c(char c)
	{
		return c >= 'a' && c <= 'z' ? c - 'a' + 'A' : c;
	}
	static constexpr hash_t HASH(const char* str, size_t len = ~0) {
		if (len == ~0) len = strlen_c(str);
		hash_t r = 0;
		for (int i = 0; i < len; ++i) {
			r = (r * BASE + str[i]) % MOD;
		}
		return r;
	}
	static constexpr hash_t HASH_NO_CASE(const char* str, size_t len = ~0) {
		if (len == ~0) len = strlen_c(str);
		hash_t r = 0;
		for (int i = 0; i < len; ++i) {
			r = (r * BASE + toupper_c(str[i])) % MOD;
		}
		return r;
	}
	static constexpr hash_t knuth(hash_t h) {
		return (h * 2654435769ULL) >> 16;
	}
}