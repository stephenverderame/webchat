#pragma once
#include "Client.h"
#include "StreamView.h"
#define HTTP_OK "200 OK"
#define HTTP_CREATED "201 Created"
#define HTTP_BAD "400 Bad Request"
#define HTTP_FORBIDDEN "403 Forbidden"
#define HTTP_UNAUTH "401 Unauthorized"
#define HTTP_NFOUND "404 Not Found"
#define HTTP_NALLOW "405 Method Not Allowed"
#define HTTP_NIMPLEMENT "501 Not Implemented"
#define HTTP_SWITCH "101 Switching Protocols"

#define HTTP_GET "GET"
#define HTTP_POST "POST"
#define HTTP_PUT "PUT"

class HttpHeaders {
private:
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	//returns a streamview with a zero length on failure
	StreamView& operator[](const char * key);
	StreamView& operator[](long hashKey);
	void put(StreamView&& key, StreamView&& val);
	void put(const StreamView& key, const StreamView& val);
	void put(StreamView&& key, const StreamView& val);
	void put(const StreamView& key, StreamView&& val);
	HttpHeaders();
	HttpHeaders(HttpHeaders && h);
	HttpHeaders& operator=(HttpHeaders && h);
	~HttpHeaders();
	bool find(const char * key);
	bool find(long hashKey);
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

