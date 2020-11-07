#pragma once
#include "Hash.h"
#include <vector>
#include <list>
#include <string>
#include <string_view>
#include "StreamView.h"
#include <iterator>

template<typename T, typename V = void>
struct is_defaultConstructable : std::false_type {};
template<typename T>
struct is_defaultConstructable<T, std::void_t<decltype(T())>> : std::true_type {};

template<typename T, typename = void, typename = void>
struct is_stringMappable : std::false_type {};
template<typename T>
struct is_stringMappable<T, std::void_t<decltype(std::declval<T>().data())>, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};
/** 
 * Class for string keys without copying
 * Supports strings, StreamView, and string_view as keys
 * Allows comparisons without creating a new object for strings, char *, StreamView, and string_views
 */
template<typename K, typename V>
class StringMap
{	
	class Iterator;
public:
private:
	std::vector<std::list<std::pair<K, V>>> map;
	bool useCase; //True if map is case-sensitive
	size_t elements;
public:
	StringMap(long initSize = 100, bool needCase = true){
		static_assert(is_defaultConstructable<V>::value, "Value for StringMap must have a no parameter constructor");
		static_assert(is_stringMappable<K>::value, "Key for StringMap must be a string type");
		map.resize(initSize);
		useCase = needCase;
		elements = 0;
	}
	StringMap(const StringMap& other) = default;
	StringMap(StringMap&& other) = default;
	StringMap& operator=(const StringMap& other) = default;
	StringMap& operator=(StringMap&& other) = default;
	StringMap& put(K&& key, V&& val) {
		const hash_t h = hash(key.data(), key.size());
		std::list<std::pair<K, V>>& li = map[h];
		for(auto& p : li) {
			if(p.first == key) {
				p.second = val;
				return *this;
			}
		}
		li.push_back(std::make_pair(key, val));
		++elements;
		return *this;
	}
	StringMap& put(K&& key, V& val) {
		const hash_t h = hash(key.data(), key.size());
		std::list<std::pair<K, V>>& li = map[h];
		for (auto& p : li) {
			if (p.first == key) {
				p.second = val;
				return *this;
			}
		}
		li.push_back(std::make_pair(key, val));
		++elements;
		return *this;
	}
	StringMap& put(K& key, V&& val) {
		const hash_t h = hash(key.data(), key.size());
		std::list<std::pair<K, V>>& li = map[h];
		for (auto& p : li) {
			if (p.first == key) {
				p.second = val;
				return *this;
			}
		}
		li.push_back(std::make_pair(key, val));
		++elements;
		return *this;
	}
	StringMap& put(K& key, V& val) {
		const hash_t h = hash(key.data(), key.size());
		std::list<std::pair<K, V>>& li = map[h];
		for (auto& p : li) {
			if (p.first == key) {
				p.second = val;
				return *this;
			}
		}
		li.push_back(std::make_pair(key, val));
		++elements;
		return *this;
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
		std::list<std::pair<K, V>>& li = map[hash(key.data(), key.size())];
		std::pair<K, V>* p = _getHelper(li, key.data(), key.size());
		if (p != nullptr) return *p;
		li.emplace_back(key, V());
		++elements;
		return li.back();
	}
	inline std::pair<K, V>& operator[](const StreamView&& key) {
		std::list<std::pair<K, V>>& li = map[hash(key.data(), key.size())];
		std::pair<K, V>* p = _getHelper(li, key.data(), key.size());
		if (p != nullptr) return *p;
		li.emplace_back(key, V());
		++elements;
		return li.back();
	}
	inline std::pair<K, V>& operator[](const std::string_view&& key) {
		return get(key.data(), key.size());
	}
	inline std::pair<K, V>& operator[](const std::string_view& key) {
		return get(key.data(), key.size());
	}
	/**
	 * Determines if the key is in the map. If so gets its uid, otherwise false is returned
	 * @param key the key to look for
	 * @return true if the param was found
	 */
	inline bool find(const char * key) const {
		return _find(key, strlen(key));
	}
	inline bool find(const std::string& key) const {
		return _find(key.data(), key.size());
	}
	inline bool find(const std::string&& key) const {
		return _find(key.data(), key.size());
	}
	inline bool find(const std::string_view& key) const {
		return _find(key.data(), key.size());
	}
	inline bool find(const std::string_view&& key) const {
		return _find(key.data(), key.size());
	}
	inline bool find(const StreamView& key) const {
		return _find(key.data(), key.size());
	}
	inline bool find(const StreamView&& key) const {
		return _find(key.data(), key.size());
	}
	inline size_t size() const {
		return elements;
	}
	std::vector<K> keyList() {
		std::vector<K> list;
		for(std::list<std::pair<K, V>>& l : map) {
			for(std::pair<K, V>& p : l) {
				list.push_back(p.first);
			}
		}
		return list;
	}
	/**
	* Removes the pair with the given key from the map
	* @param key the key to look for
	* @return true if the pair is removed, false otherwise
	*/
	inline bool remove(const char* key) {
		return _remove(key, strlen(key));
	}
	inline bool remove(const std::string& key) {
		return _remove(key.data(), key.size());
	}
	inline bool remove(const std::string&& key) {
		return _remove(key.data(), key.size());
	}
	inline bool remove(const std::string_view& key) {
		return _remove(key.data(), key.size());
	}
	inline bool remove(const std::string_view&& key) {
		return _remove(key.data(), key.size());
	}
	inline bool remove(const StreamView& key) {
		return _remove(key.data(), key.size());
	}
	inline bool remove(const StreamView&& key) {
		return _remove(key.data(), key.size());
	}
	Iterator begin() {
		if(elements > 0) {
			for(int i = 0; i < map.size(); ++i) {
				if (!map[i].empty()) return Iterator(i, map, map[i].begin());
			}
		}
		return end();
	}
	inline Iterator end() {
		return Iterator();
	}
	inline void setCaseSensitive(bool c) {
		useCase = c;
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
	/**
	 * Gets the element with the given key
	 * If no such element with the key exists, adds a new pair with a default value
	 * @param s 
	 * @param len 
	 * @return 
	 */
	std::pair<K, V>& get(const char * s, size_t len = ~0) {
		len = len == ~0 ? strlen(s) : len;
		std::list<std::pair<K, V>>& li = map[hash(s, len)];
		std::pair<K, V> *p = _getHelper(li, s, len);
		if (p != nullptr) return *p;
		li.emplace_back(K(), V());
		++elements;
		return li.back();
	}
	bool _find(const char * s, size_t len = ~0) const {
		len = len == ~0 ? strlen(s) : len;
		const std::list<std::pair<K, V>>& li = map[hash(s, len)];
		return _getHelper(li, s, len) != nullptr;
	}
	bool _remove(const char * s, size_t len = ~0) {
		len = len == ~0 ? strlen(s) : len;
		std::list<std::pair<K, V>>& li = map[hash(s, len)];
		std::pair<K, V>* p = _getHelper(li, s, len);
		if(p != nullptr) {
			li.remove(*p);
			--elements;
			return true;
		}
		return false;

		
	}
	/**
	 * Gets the pair with the given key in the given list
	 * returns nullptr if not found
	 * @param list the list to search for the pair in
	 * @param s the c str of the key
	 * @param len length of the key
	 * @return pair with a matching key or nullptr if not found
	 */
	std::pair<K, V>* _getHelper(std::list<std::pair<K, V>> & list, const char * s, size_t len) {
		for (std::pair<K, V>& p : list) {
			if (p.first.size() == len) {
				bool match = true;
				for (int i = 0; i < len; ++i) {
					if (useCase && p.first.data()[i] != s[i] || !useCase && toupper(p.first.data()[i]) != toupper(s[i])) {
						match = false;
						break;
					}
				}
				if (match) return &p;
			}
		}
		return nullptr;
	}
	/**
	 * Gets the pair with the given key in the given list
	 * returns nullptr if not found
	 * @param list the list to search for the pair in
	 * @param s the c str of the key
	 * @param len length of the key
	 * @return pair with a matching key or nullptr if not found
	 */
	const std::pair<K, V>* _getHelper(const std::list<std::pair<K, V>>& list, const char* s, size_t len) const {
		for (auto& p : list) {
			if (p.first.size() == len) {
				bool match = true;
				for (int i = 0; i < len; ++i) {
					if (useCase && p.first.data()[i] != s[i] || !useCase && toupper(p.first.data()[i]) != toupper(s[i])) {
						match = false;
						break;
					}

				}
				if (match) return &p;
			}
		}
		return nullptr;
	}

};
template<typename K, typename V>
class StringMap<K, V>::Iterator {
	friend class StringMap<K, V>;
private:
	int bucket;
	//bucket is a number [0, map.size()) or ~0 if this iterator is invalid
	typename std::list<std::pair<K, V>>::iterator it;
	//iterator is always valid if bucket != ~0
	std::vector<std::list<std::pair<K, V>>>* map;
private:
	/**
	 * Creates an invalid iterator
	 * @param bucket 
	 */
	Iterator() : bucket(~0) {}
	/**
	 * Creates an iterator from the given bucket and element
	 * @param bucket bucket of the map
	 * @param map the parent of this iterator
	 * @param element the specific index in the specified bucket
	 */
	Iterator(int bucket, std::vector<std::list<std::pair<K, V>>>& map, typename std::list<std::pair<K, V>>::iterator element) : map(&map), bucket(bucket), it(element) {}
public:
	std::pair<K, V>& operator*() {
		if (bucket != ~0) return *it;
		throw std::exception("No elements");
	}
	bool operator==(Iterator& it) const {
		return (bucket == it.bucket && bucket == ~0) || (bucket == it.bucket && this->it == it.it);
	}
	bool operator!=(Iterator& it) const {
		return !(*this == it);
	}
	Iterator operator++() {
		if (++it == map->at(bucket).end()) {
			while (++bucket < map->size()) {
				if ((it = map->at(bucket).begin()) != map->at(bucket).end()) return *this;
			}
			bucket = ~0;
			return Iterator();
		}
		return *this;
	}
	Iterator operator++(int) {
		Iterator ret = *this;
		if (++it == map->at(bucket).end()) {
			while (++bucket < map->size()) {
				if ((it = map->at(bucket).begin()) != map->at(bucket).end());
			}
			bucket = ~0;
		}
		return ret;
	}
};
