#pragma once
#include "StreamView.h"
#include <vector>
enum class JSONType : char {
	object,
	array,
	field,
	null
};
struct JSONObjData {
	JSONType type;
	long hash;
};
class JSONArray;
class JSONObject
{
	friend std::ostream& operator<<(std::ostream&, const JSONObject&);
private:
	StreamView txt;
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	JSONObject(const StreamView& txt);
	JSONObject(StreamView&& txt);
	JSONObject(const JSONObject& other);
	JSONObject(JSONObject&& other);
	JSONObject& operator=(const JSONObject& other);
	JSONObject& operator=(JSONObject&& other);
	JSONObject();
	~JSONObject();

	//return 0 length StreamView on failure
	StreamView get(long hash) const;
	StreamView get(const char* name) const;
	//return nullptr on failure
	JSONObject* getObj(long hash) const;
	JSONObject* getObj(const char* name) const;
	JSONArray* getArray(long hash) const;
	JSONArray* getArray(const char* name) const;

	JSONType typeof(long hash) const;
	JSONType typeof(const char* name) const;

	std::vector<long> keyList() const;
	StreamView nameof(long hash, JSONType type) const;



	//name and data must outlive the JSONObject
	//if name already exists, value is overriden
	void put(const char* name, const char* data);
	void put(const char* name, const StreamView& data);
	void put(const StreamView& name, const StreamView& data);
	void put(const char* name, StreamView&& data);
	void put(const char* name, const JSONObject& obj);
	void put(const char* name, const JSONArray& arr);

	inline void assign(StreamView&& txt);
	inline void assign(const StreamView& txt);

private:
	void init();
};

class JSONArray {
	friend std::ostream& operator<<(std::ostream& strm, const JSONArray& json);
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
	StreamView txt;
public:
	JSONArray(const StreamView& sv);
	JSONArray(StreamView&& sv);
	JSONArray& operator=(const JSONArray& other);
	JSONArray(const JSONArray& other);
	JSONArray();
	~JSONArray();

	int size() const;
	StreamView operator[](int index);

	JSONArray* getArray(int index);
	JSONObject* getObj(int index);
	StreamView get(int index);

	inline void assign(StreamView&& txt);
	inline void assign(const StreamView& txt);

	void put(const char* element);
private:
	void init();
};
std::ostream& operator<<(std::ostream& strm, const JSONObject& json);
Streamable& operator>>(Streamable& strm, JSONObject& json);

std::ostream& operator<<(std::ostream& strm, const JSONArray& json);
Streamable& operator>>(Streamable& strm, JSONArray& json);

inline void JSONObject::assign(const StreamView& txt)
{
	this->txt = txt;
	init();
}
inline void JSONObject::assign(StreamView&& txt)
{
	this->txt = std::move(txt);
	init();
}

inline void JSONArray::assign(const StreamView& txt)
{
	this->txt = txt;
	init();
}
inline void JSONArray::assign(StreamView&& txt)
{
	this->txt = std::move(txt);
	init();
}
