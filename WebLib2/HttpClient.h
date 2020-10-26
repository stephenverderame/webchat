#pragma once
#include "Client.h"
#include "StreamView.h"
namespace HTTP {
	namespace Resp {
		constexpr const char* OK = "200 OK";
		constexpr const char* CREATED = "201 Created";
		constexpr const char* BAD = "400 Bad Request";
		constexpr const char* FORBIDDEN = "403 Forbidden";
		constexpr const char* UNAUTH = "401 Unauthorized";
		constexpr const char* NOT_FOUND = "404 Not Found";
		constexpr const char* NOT_ALLOW = "405 Method Not Allowed";
		constexpr const char* NOT_IMPLEMENT = "501 Not Implemented";
		constexpr const char* SWITCH = "101 Switching Protocols";
	}
	namespace RespCode {
		constexpr const int ok = 200;
		constexpr const int created = 201;
		constexpr const int bad = 400;
		constexpr const int forbidden = 403;
		constexpr const int unauth = 401;
		constexpr const int not_found = 404;
		constexpr const int not_allow = 405;
		constexpr const int not_implement = 501;
		constexpr const int switch_proto = 101;
	}
	namespace Req {
		constexpr const char* GET = "GET";
		constexpr const char* POST = "POST";
		constexpr const char* PUT = "PUT";
	}
}

class HttpHeaders {
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	//returns a streamview with a zero length on failure
	StreamView& operator[](const char * key);
	StreamView& operator[](hash_t hashKey);

	void put(StreamView&& key, StreamView&& val);
	void put(const StreamView& key, const StreamView& val);
	void put(StreamView&& key, const StreamView& val);
	void put(const StreamView& key, StreamView&& val);
	HttpHeaders();
	HttpHeaders(HttpHeaders && h);
	HttpHeaders& operator=(HttpHeaders && h);
	~HttpHeaders();
	bool find(const char * key);
	bool find(hash_t hashKey);
	void for_each(std::function<void(StreamView &, StreamView &)> pred);
};
struct HttpResponse {
	int responseCode = 0;
	StreamView responseMessage;
	HttpHeaders headers;
	StreamView content;
};
enum class AutoHttpHeaders {
	AcceptedEncodings
};
class HttpClient : public Client
{
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	HttpClient & path(const char * path);
	HttpClient & put(const char * header, const char * value);
	HttpClient & put(AutoHttpHeaders header);
	HttpClient & method(const char * method);
	HttpClient & response(const char * response);
	Streamable & getStream();
	//Buffers headers into input stream. Must be called before buffering in content
	void bufferHeaders();
	void send();
	/*
	Syncs input and output buffers (writes all, reads all)
	Then reads through the stream until it reaches an empty line (end of headers)
	Copies header data and returns it in the HttpResponse
	Stream is left in a state where it points to the content
	*/
	HttpResponse getResponse();
	HttpClient(std::unique_ptr<Streamable> && stream);
	HttpClient(HttpClient && client);
	HttpClient& operator=(HttpClient && c);
	~HttpClient();
};

