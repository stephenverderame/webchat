#include "JSON.h"
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <execution>
#include "StringMap.h"
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
		if (end == StreamView::INVALID) end = parent.size() - 1;
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
	for (; braces > 0 && end < parent.size(); ++end) {
		if (parent[end] == '{' || parent[end] == '[') ++braces;
		else if (parent[end] == '}' || parent[end] == ']') --braces;
	}
	auto e = parent.sub(p, end);
	p = end;
	return e;
}
struct JSONObject::impl {
	StringMap<StreamView, StreamView> fields;
	StringMap<StreamView, JSONObject> objs;
	StringMap<StreamView, JSONArray> arrs;
};

struct JSONArray::impl {
	std::vector<StreamView> elements;
	std::unordered_map<size_t, JSONArray> arrs;
	std::unordered_map<size_t, JSONObject> objs;
};
JSONObject::JSONObject() : pimpl(std::make_unique<impl>())
{

}

JSONObject::JSONObject(StreamView txt) : JSONObject()
{
	this->txt = txt;
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


StreamView JSONObject::get(const char* name) const
{
	return pimpl->fields[name].second;
}

StreamView JSONObject::get(StreamView& name) const
{
	return pimpl->fields[name].second;
}

StreamView JSONObject::get(StreamView&& name) const
{
	return pimpl->fields[name].second;
}

JSONObject* JSONObject::getObj(const char* name) const
{
	return &pimpl->objs[name].second;
}

JSONObject* JSONObject::getObj(StreamView& name) const
{
	return &pimpl->objs[name].second;
}

JSONObject* JSONObject::getObj(StreamView&& name) const
{
	return &pimpl->objs[name].second;
}

JSONArray* JSONObject::getArray(const char* name) const
{
	return &pimpl->arrs[name].second;
}

JSONArray* JSONObject::getArray(StreamView& name) const
{
	return &pimpl->arrs[name].second;
}

JSONArray* JSONObject::getArray(StreamView&& name) const
{
	return &pimpl->arrs[name].second;
}

JSONType JSONObject::typeof(const char* name) const
{
	if (pimpl->fields.find(name)) return JSONType::field;
	if (pimpl->objs.find(name)) return JSONType::object;
	if (pimpl->arrs.find(name)) return JSONType::array;
	return JSONType::null;
	
}

JSONType JSONObject::typeof(StreamView& name) const
{
	if (pimpl->fields.find(name)) return JSONType::field;
	if (pimpl->objs.find(name)) return JSONType::object;
	if (pimpl->arrs.find(name)) return JSONType::array;
	return JSONType::null;;
}

JSONType JSONObject::typeof(StreamView&& name) const
{
	if (pimpl->fields.find(name)) return JSONType::field;
	if (pimpl->objs.find(name)) return JSONType::object;
	if (pimpl->arrs.find(name)) return JSONType::array;
	return JSONType::null;
}

std::vector<StreamView> JSONObject::keyList() const
{
	std::vector<StreamView> list;
	std::vector<StreamView> l = pimpl->fields.keyList();
	list.insert(list.end(), l.begin(), l.end());
	l = pimpl->objs.keyList();
	list.insert(list.end(), l.begin(), l.end());
	l = pimpl->arrs.keyList();
	list.insert(list.end(), l.begin(), l.end());
	return list;
}

void JSONObject::put(const char* name, const char* data)
{
	pimpl->fields[name].second = StreamView(data, 0, strlen(data));
}

void JSONObject::put(const char* name, const StreamView& data)
{
	pimpl->fields[name].second = data;
}

void JSONObject::put(const StreamView& name, const StreamView& data)
{
	pimpl->fields[name].second = data;
}

void JSONObject::put(const char* name, StreamView&& data)
{
	pimpl->fields[name].second = data;
}

void JSONObject::put(const char* name, const JSONObject& obj)
{
	pimpl->objs[name].second = obj;
}

void JSONObject::put(const char* name, const JSONArray& arr)
{
	pimpl->arrs[name].second = arr;
}

void JSONObject::init()
{
	if (txt.size() > 0) {
		this->txt;
		txt.advance();
		std::streamsize p = txt.tell();
		std::streamsize lastP = txt.tell();
		while ((p = txt.find(':', p)) != StreamView::INVALID && lastP != StreamView::INVALID) {
			for (; lastP < txt.size() && ignore(txt[lastP]); ++lastP);
			++lastP;
			StreamView name = txt.sub(lastP, txt.find('"', lastP));
			for (++p; p < txt.size() && ignore(txt[p]); ++p);
			if (txt[p] == '{' || txt[p] == '[') {
				if (txt[p] == '{') pimpl->objs[name].second = getObject(txt, p);
				else pimpl->arrs[name].second = getObject(txt, p);
			}
			else
				pimpl->fields[name].second = getField(txt, p);
			lastP = txt.find(',', p) + 1;

		}
	}
}

std::ostream& operator<<(std::ostream& strm, const JSONObject& json)
{
	strm << "{";
	int amount = 1;
	for (StreamView& p : json.pimpl->fields.keyList()) {
		strm << "\"" << p << "\": " << "\"" << json.pimpl->fields[p].second << "\"";
		if (amount++ < json.pimpl->fields.size() + json.pimpl->objs.size() + json.pimpl->arrs.size()) strm << ',';
	}
	for (StreamView& o : json.pimpl->objs.keyList()) {
		strm << "\"" << o << "\": " << json.pimpl->objs[o].second;
		if (amount++ < json.pimpl->fields.size() + json.pimpl->objs.size() + json.pimpl->arrs.size()) strm << ',';
	}
	for (StreamView& o : json.pimpl->arrs.keyList()) {
		strm << "\"" << o << "\": " << json.pimpl->arrs[o].second;
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
		json.assign(strm.getSharedView(std::ios::beg, start, strm.tellg()));
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
		json.assign(strm.getSharedView(std::ios::beg, start, strm.tellg()));
	}
	return strm;
}

JSONArray::JSONArray(StreamView sv) : JSONArray()
{
	txt = sv;
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
	pimpl->elements.emplace_back(element, 0, strlen(element));
}

void JSONArray::put(StreamView element)
{
	pimpl->elements.push_back(element);
}

void JSONArray::put(JSONObject& element)
{
	pimpl->elements.emplace_back();
	pimpl->objs.insert(std::make_pair(pimpl->elements.size() - 1, element));
}

void JSONArray::put(JSONArray& element)
{
	pimpl->elements.emplace_back();
	pimpl->arrs.insert(std::make_pair(pimpl->elements.size() - 1, element));
}

void JSONArray::init()
{
	if (txt.size() > 0) {
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
}
