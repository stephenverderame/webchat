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
	JSONObject(StreamView txt);
	JSONObject(const JSONObject& other);
	JSONObject(JSONObject&& other);
	JSONObject& operator=(const JSONObject& other);
	JSONObject& operator=(JSONObject&& other);
	JSONObject();
	~JSONObject();

	/**
	* This function will fail if the value you are trying to receive is an Object or Array
	* @see typeof()
	* @return 0 length StreamView on failure, else the content of the field
	*/
	///@{
	StreamView get(const char* name) const;
	StreamView get(StreamView& name) const;
	StreamView get(StreamView&& name) const;
	///@}
	///return nullptr on failure
	///@{
	JSONObject* getObj(const char* name) const;
	JSONObject* getObj(StreamView& name) const;
	JSONObject* getObj(StreamView&& name) const;
	JSONArray* getArray(const char* name) const;
	JSONArray* getArray(StreamView& name) const;
	JSONArray* getArray(StreamView&& name) const;
	///@}

	JSONType typeof(const char* name) const;
	JSONType typeof(StreamView& name) const;
	JSONType typeof(StreamView&& name) const;

	/**
	* @return a list of hashes of all fields of this object
	* Hashes can be passed to any function and can be treated as a unique identifier fo each field in the Object
	*/
	std::vector<StreamView> keyList() const;



	///name and data must outlive the JSONObject
	///if name already exists, value is overriden
	///@{
	void put(const char* name, const char* data);
	void put(const char* name, const StreamView& data);
	void put(const StreamView& name, const StreamView& data);
	void put(const char* name, StreamView&& data);
	void put(const char* name, const JSONObject& obj);
	void put(const char* name, const JSONArray& arr);
	///@}

	/// Assigns a new stream view for this JSONObject
	/// @{
	inline void assign(StreamView&& txt);
	inline void assign(const StreamView& txt);
	///@}

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
	JSONArray(StreamView sv);
	JSONArray& operator=(const JSONArray& other);
	JSONArray(const JSONArray& other);
	JSONArray();
	~JSONArray();

	int size() const;
	StreamView operator[](int index);

	/**
	* If an element is known to be an array or an object, returns the given reference instead of having to reconstruct one each time from its text representation
	*/
	///@{
	JSONArray* getArray(int index);
	JSONObject* getObj(int index);
	/// @}

	StreamView get(int index);

	inline void assign(StreamView&& txt);
	inline void assign(const StreamView& txt);

	void put(const char* element);
	void put(StreamView element);
	void put(JSONObject& element);
	void put(JSONArray& element);
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
