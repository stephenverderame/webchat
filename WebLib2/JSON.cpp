#include "JSON.h"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <execution>
inline bool ignore(char c)
{
	return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
StreamView getField(StreamView& parent, std::streamsize& p)
{
	std::streamsize end;
	if (parent[p] == '"') {
		p++;
		end = parent.find('"', p);
	}
	else {
		end = parent.find(',', p);
		if (end == StreamView::INVALID) end = parent.getSize() - 1;
		while (ignore(parent[end - 1])) --end;
	}
	auto out = parent.sub(p, end);
	p = end;
	return out;
}
StreamView getObject(StreamView& parent, std::streamsize& p)
{
	auto end = p + 1;
	int braces = 1;
	for (; braces > 0 && end < parent.getSize(); ++end) {
		if (parent[end] == '{' || parent[end] == '[') ++braces;
		else if (parent[end] == '}' || parent[end] == ']') --braces;
	}
	auto e = parent.sub(p, end);
	p = end;
	return e;
}
struct JSONObject::impl {
	std::unordered_map<long, std::pair<StreamView, StreamView>> fields;
	std::unordered_map<long, std::pair<StreamView, JSONObject>> objs;
	std::unordered_map<long, std::pair<StreamView, JSONArray>> arrs;
};

struct JSONArray::impl {
	std::vector<StreamView> elements;
	std::unordered_map<size_t, JSONArray> arrs;
	std::unordered_map<size_t, JSONObject> objs;
};
JSONObject::JSONObject() : pimpl(std::make_unique<impl>())
{

}
JSONObject::JSONObject(const StreamView& txt) : JSONObject()
{
	this->txt = txt;
	init();
}

JSONObject::JSONObject(StreamView&& txt) : JSONObject()
{
	this->txt = std::move(txt);
	init();
}

JSONObject::JSONObject(const JSONObject& other) : JSONObject()
{
	txt = other.txt;
	pimpl->fields = other.pimpl->fields;
	pimpl->objs = other.pimpl->objs;
	pimpl->arrs = other.pimpl->arrs;

}

JSONObject::JSONObject(JSONObject&& other)
{
	txt = std::move(other.txt);
	pimpl = std::move(other.pimpl);
}

JSONObject& JSONObject::operator=(const JSONObject& other)
{
	txt = other.txt;
	pimpl->fields = other.pimpl->fields;
	pimpl->objs = other.pimpl->objs;
	pimpl->arrs = other.pimpl->arrs;
	return *this;
}

JSONObject& JSONObject::operator=(JSONObject&& other)
{
	txt = std::move(other.txt);
	pimpl = std::move(other.pimpl);
	return *this;
}


JSONObject::~JSONObject() = default;

StreamView JSONObject::get(long hash) const
{
	if (pimpl->fields.find(hash) != pimpl->fields.end())
		return pimpl->fields.at(hash).second;
	return StreamView();
}

StreamView JSONObject::get(const char* name) const
{
	return get(StreamView::hash(name));
}

JSONObject* JSONObject::getObj(long hash) const
{
	if (pimpl->objs.find(hash) != pimpl->objs.end())
		return &pimpl->objs.at(hash).second;
	return nullptr;
}

JSONObject* JSONObject::getObj(const char* name) const
{
	return getObj(StreamView::hash(name));
}

JSONArray* JSONObject::getArray(long hash) const
{
	if (pimpl->arrs.find(hash) != pimpl->arrs.end())
		return &pimpl->arrs.at(hash).second;
	return nullptr;
}

JSONArray* JSONObject::getArray(const char* name) const
{
	return getArray(StreamView::hash(name));
}

JSONType JSONObject::typeof(long hash) const
{
	if (pimpl->fields.find(hash) != pimpl->fields.end()) return JSONType::field;
	if (pimpl->objs.find(hash) != pimpl->objs.end()) return JSONType::object;
	if (pimpl->arrs.find(hash) != pimpl->arrs.end()) return JSONType::array;
	return JSONType::null;
}

JSONType JSONObject::typeof(const char* name) const
{
	return typeof(StreamView::hash(name));
}

std::vector<long> JSONObject::keyList() const
{
	std::vector<long> vec;
	vec.reserve(pimpl->fields.size() + pimpl->arrs.size() + pimpl->objs.size());
	for (auto it : pimpl->fields)
		vec.push_back(it.first);
	for (auto it : pimpl->arrs)
		vec.push_back(it.first);
	for (auto it : pimpl->objs)
		vec.push_back(it.first);
	return vec;
}

StreamView JSONObject::nameof(long hash, JSONType type) const
{
	switch (type) {
	case JSONType::array:
		return pimpl->arrs[hash].first;
		break;
	case JSONType::field:
		return pimpl->fields[hash].first;
		break;
	case JSONType::object:
		return pimpl->objs[hash].first;
		break;
	default:
		return StreamView();
	}
}

void JSONObject::put(const char* name, const char* data)
{
	pimpl->fields[StreamView::hash(name)] = std::make_pair(StreamView(name, 0, strlen(name)), StreamView(data, 0, strlen(data)));
}

void JSONObject::put(const char* name, const StreamView& data)
{
	pimpl->fields[StreamView::hash(name)] = std::make_pair(StreamView(name, 0, strlen(name)), data);
}

void JSONObject::put(const StreamView& name, const StreamView& data)
{
	pimpl->fields[name.hash()] = std::make_pair(name, data);
}

void JSONObject::put(const char* name, StreamView&& data)
{
	pimpl->fields[StreamView::hash(name)] = std::make_pair(StreamView(name, 0, strlen(name)), data);
}

void JSONObject::put(const char* name, const JSONObject& obj)
{
	pimpl->objs[StreamView::hash(name)] = std::make_pair(StreamView(name, 0, strlen(name)), obj);
}

void JSONObject::put(const char* name, const JSONArray& arr)
{
	pimpl->arrs[StreamView::hash(name)] = std::make_pair(StreamView(name, 0, strlen(name)), arr);
}

void JSONObject::init()
{
	this->txt;
	txt.advance();
	std::streamsize p = txt.tell();
	std::streamsize lastP = txt.tell();
	while ((p = txt.find(':', p)) != StreamView::INVALID && lastP != StreamView::INVALID) {
		for (; lastP < txt.getSize() && ignore(txt[lastP]); ++lastP);
		++lastP;
		StreamView name = txt.sub(lastP, txt.find('"', lastP));
		for (++p; p < txt.getSize() && ignore(txt[p]); ++p);
		if (txt[p] == '{' || txt[p] == '[') {
			if(txt[p] == '{') pimpl->objs[name.hash()] = std::make_pair(name, getObject(txt, p));
			else pimpl->arrs[name.hash()] = std::make_pair(name, getObject(txt, p));
		}
		else 
			pimpl->fields[name.hash()] = std::make_pair(name, getField(txt, p));
		lastP = txt.find(',', p) + 1;
			
	}
}

std::ostream& operator<<(std::ostream& strm, const JSONObject& json)
{
	strm << "{";
	int amount = 1;
	for (auto& p : json.pimpl->fields) {
		strm << "\"" << p.second.first << "\": " << "\"" << p.second.second << "\"";
		if (amount++ < json.pimpl->fields.size() + json.pimpl->objs.size() + json.pimpl->arrs.size()) strm << ',';
	}
	for (auto& o : json.pimpl->objs) {
		strm << "\"" << o.second.first << "\": " << o.second.second;
		if (amount++ < json.pimpl->fields.size() + json.pimpl->objs.size() + json.pimpl->arrs.size()) strm << ',';
	}
	for (auto& o : json.pimpl->arrs) {
		strm << "\"" << o.second.first << "\": " << o.second.second;
		if (amount++ < json.pimpl->fields.size() + json.pimpl->objs.size() + json.pimpl->arrs.size()) strm << ',';
	}
	strm << "}";
	return strm;
}

Streamable& operator>>(Streamable& strm, JSONObject& json)
{
	auto start = strm.tellg();
	if (strm.peek() == '{') {
		int braces = 1;
		strm.get();
		char c;
		while (braces > 0) {
			c = strm.get();
			if (c == '{' || c == '[') ++braces;
			else if (c == '}' || c == ']') --braces;
		}
		json.assign(strm.getStreamView(std::ios::beg, start, strm.tellg()));
	}
	return strm;
}

std::ostream& operator<<(std::ostream& strm, const JSONArray& json)
{
	strm << "[";
	size_t i = 1;
	for (auto& s : json.pimpl->elements) {
		strm << "\"" << s << "\"";
		if (i++ < json.size()) strm << ",";
	}
	strm << "]";
	return strm;

}

Streamable& operator>>(Streamable& strm, JSONArray& json)
{
	auto start = strm.tellg();
	if (strm.peek() == '[') {
		int braces = 1;
		strm.get();
		char c;
		while (braces > 0) {
			c = strm.get();
			if (c == '{' || c == '[') ++braces;
			else if (c == '}' || c == ']') --braces;
		}
		json.assign(strm.getStreamView(std::ios::beg, start, strm.tellg()));
	}
	return strm;
}

JSONArray::JSONArray(const StreamView& sv) : JSONArray()
{
	txt = sv;
	init();
}

JSONArray::JSONArray(StreamView&& sv) : JSONArray()
{
	txt = std::move(sv);
	init();
}

JSONArray& JSONArray::operator=(const JSONArray& other)
{
	txt = other.txt;
	pimpl->elements = other.pimpl->elements;
	pimpl->arrs = other.pimpl->arrs;
	pimpl->objs = other.pimpl->objs;
	return *this;
}

JSONArray::JSONArray(const JSONArray& other) : JSONArray()
{
	this->txt = other.txt;
	pimpl->elements = other.pimpl->elements;
	pimpl->arrs = other.pimpl->arrs;
	pimpl->objs = other.pimpl->objs;
	init();
}

JSONArray::JSONArray() : pimpl(std::make_unique<impl>())
{
}

JSONArray::~JSONArray() = default;
int JSONArray::size() const
{
	return pimpl->elements.size();;
}

StreamView JSONArray::operator[](int index)
{
	return pimpl->elements[index];
}

JSONArray* JSONArray::getArray(int index)
{
	if (pimpl->arrs.find(index) != pimpl->arrs.end())
		return &pimpl->arrs.at(index);
	return nullptr;
}

JSONObject* JSONArray::getObj(int index)
{
	if (pimpl->objs.find(index) != pimpl->objs.end())
		return &pimpl->objs.at(index);
	return nullptr;
}

StreamView JSONArray::get(int index)
{
	return pimpl->elements[index];
}

void JSONArray::put(const char* element)
{
	pimpl->elements.push_back(StreamView(element, 0, strlen(element)));
}

void JSONArray::init()
{
	std::streamsize p = txt.tell();
	do {
		++p;
		for (; ignore(txt[p]); ++p);
		if (txt[p] == '[' || txt[p] == '{') {
			bool isObj = txt[p] == '{';
			auto e = getObject(txt, p);
			pimpl->elements.push_back(e);
			if (isObj) pimpl->objs.insert_or_assign(pimpl->elements.size() - 1, e);
			else pimpl->arrs.insert_or_assign(pimpl->elements.size() - 1, e);
		}
		else {
			pimpl->elements.push_back(getField(txt, p));
		}
	} while ((p = txt.find(',', p)) != StreamView::INVALID);
}
