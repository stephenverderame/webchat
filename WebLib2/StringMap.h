#pragma once
#include "Hash.h"
#include <vector>
#include <list>
#include <string>
#include <string_view>
#include "StreamView.h"
using sm_uid = unsigned long long; // hash << 32 | index
static const constexpr sm_uid BAD_ID = ~0; //invalid sm_uid
/** 
 * Class for string key-value pairs without copying
 * Supports strings, StreamView, and string_view
 * Allows comparisons without creating a new object for strings, char *, StreamView, and string_views
 */
template<typename K, typename V>
class StringMap
{
public:
private:
	std::vector<std::list<std::pair<K, V>>> map;
	bool useCase; //True if map is case-sensitive
	size_t elements;
public:
	StringMap(long initSize = 100, bool needCase = true){
		map.resize(initSize);
		useCase = needCase;
		elements = 0;
	}
	sm_uid put(K&& key, V&& val) {
		const hash_t h = hash(key.data(), key.size());
		std::list<std::pair<K, V>>& li = map[h];
		size_t index = 0;
		for(auto& p : li) {
			if(p.first == key) {
				p.second = val;
				return (unsigned long long)h << 32ULL | index;
			}
			++index;
		}
		li.push_back(std::make_pair(key, val));
		++elements;
		return (unsigned long long)h << 32ULL | index;
	}
	sm_uid put(K&& key, V& val) {
		const hash_t h = hash(key.data(), key.size());
		std::list<std::pair<K, V>>& li = map[h];
		size_t index = 0;
		for (auto& p : li) {
			if (p.first == key) {
				p.second = val;
				return (unsigned long long)h << 32ULL | index;
			}
			++index;
		}
		li.push_back(std::make_pair(key, val));
		++elements;
		return (unsigned long long)h << 32ULL | index;
	}
	/**
	 * Gets the element with the given key
	 * If not found, a newly created pair at the corresponding index
	 * @param key key
	 * @return an existing or new pair with the given key
	 */
	inline std::pair<K, V>& operator[](const char * key) {
		return get(key, strlen(key));
	}
	inline std::pair<K, V>& operator[](const std::string& key) {
		return get(key.data(), key.size());
	}
	inline std::pair<K, V>& operator[](const std::string&& key) {
		return get(key.data(), key.size());
	}
	inline std::pair<K, V>& operator[](const StreamView& key) {
		return get(key.data(), key.size());
	}
	inline std::pair<K, V>& operator[](const StreamView&& key) {
		return get(key.data(), key.size());
	}
	inline std::pair<K, V>& operator[](const std::string_view&& key) {
		return get(key.data(), key.size());
	}
	inline std::pair<K, V>& operator[](const std::string_view& key) {
		return get(key.data(), key.size());
	}
	/**
	 * Determines if the key is in the map. If so gets its uid, otherwise BAD_ID is returned
	 * @param key the key to look for
	 * @return the uid of the pair or BAD_ID if it was not found
	 */
	inline sm_uid find(const char * key) const {
		return _find(key, strlen(key));
	}
	inline sm_uid find(const std::string& key) const {
		return _find(key.data(), key.size());
	}
	inline sm_uid find(const std::string&& key) const {
		return _find(key.data(), key.size());
	}
	inline sm_uid find(const std::string_view& key) const {
		return _find(key.data(), key.size());
	}
	inline sm_uid find(const std::string_view&& key) const {
		return _find(key.data(), key.size());
	}
	inline sm_uid find(const StreamView& key) const {
		return _find(key.data(), key.size());
	}
	inline sm_uid find(const StreamView&& key) const {
		return _find(key.data(), key.size());
	}
	/**
	 * Requires uid be valid (come from this StringMap)
	 * @param uid 
	 * @return 
	 */
	std::pair<K, V>& operator[](sm_uid uid) {
		hash_t h = uid >> 32;
		size_t index = uid & 0xFFFFFFFF;
		return nthNode(map[h], index);
	}
	inline size_t size() const {
		return elements;
	}
	std::vector<sm_uid> uidList() {
		std::vector<sm_uid> list;
		hash_t h = 0;
		for(auto& l : map) {
			size_t index = 0;
			for(auto& p : l) {
				list.push_back((h << 32ULL) | index++);
			}
			++h;
		}
		return list;
	}
private:
	inline void checkLoad() {
		if (elements / (double)map.size() > 1)
			elements.resize(elements.size() * 2);
	}
	inline hash_t hash(const char* s, size_t len = ~0) const {
		hash_t h = useCase ? util::HASH(s, len) : util::HASH_NO_CASE(s, len);
		return util::knuth(h) % map.size();

	}
	std::pair<K, V>& get(const char * s, size_t len = ~0) {
		std::list<std::pair<K, V>>& li = map[hash(s, len)];
		for (auto& p : li) {
			if (p.first == s) {
				return p;
			}
		}
		std::pair<K, V> p = std::make_pair(K(), V());
		li.push_back(p);
		++elements;
		return li.back();
	}
	sm_uid _find(const char * s, size_t len = ~0) const {
		const hash_t h = hash(s, len);
		const auto li = map[h];
		size_t index = 0;
		for (auto& p : li) {
			if (p.first == s) {
				return h << 32ULL | index;
			}
			++index;
		}
		return BAD_ID;
	}
	/**
	 * Requires index < list.size()
	 * @param list 
	 * @param index 
	 * @return node in list at position index
	 */
	std::pair<K, V>& nthNode(std::list<std::pair<K, V>> & list, size_t index) {
		size_t i = 0;
		for(auto& p : list){
			if (i++ == index) return p;
		}
	}

};
