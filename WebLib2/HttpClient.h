#pragma once
#include "Client.h"
#include "StreamView.h"
#include "StringMap.h"
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

struct HttpResponse {
	int responseCode = 0;
	StreamView responseMessage;
	StringMap<StreamView, StreamView> headers;
	StreamView content;
	HttpResponse();
};
enum class AutoHttpHeaders {
	AcceptedEncodings
};
class HttpHeaderBuilder {
protected:
	struct impl;
	std::unique_ptr<impl> pimpl;
public:
	/**
	 * Buffers a header
	 * Strings must live until build() is called
	 * @param header key
	 * @param value value
	 * @return this for chaning
	 */
	HttpHeaderBuilder& put(const char* header, const char* value);
	/**
	 * Buffers in an automatic header
	 * @param header
	 * @return this for chaining
	 */
	HttpHeaderBuilder& put(AutoHttpHeaders header);


	virtual std::stringstream build() const = 0;

	virtual ~HttpHeaderBuilder();
	HttpHeaderBuilder();
};
class HttpRequestBuilder : public HttpHeaderBuilder {
private:
	const char* requestMethod, * requestPath;
public:
	/**
	 * Buffers in the path of the resource to fetch
	 * String must live until build() is called
	 * @param path
	 * @return this for chaining
	 */
	HttpRequestBuilder& path(const char* path);

	/**
	 * Buffers in the HTTP method (ie. GET or POST)
	 * @param method the method, must live until build() is called
	 * @return this for chaining
	 */
	HttpRequestBuilder& method(const char* method);

	std::stringstream build() const override;

	~HttpRequestBuilder();
	HttpRequestBuilder();
};

class HttpResponseBuilder : public HttpHeaderBuilder {
private:
	const char* respCode;
public:
	/**
	 * Buffers in an HTTP response code
	 * String must live until build()
	 * @param response the response code
	 * @return this -> for chaining
	 */
	HttpResponseBuilder& response(const char* response);

	std::stringstream build() const override;

	~HttpResponseBuilder();
	HttpResponseBuilder();
};
class HttpClient : public Client
{
private:
	std::unique_ptr<Streamable> stream;
public:
	Streamable & getStream();
	//Buffers headers into input stream. Must be called before buffering in content
	void bufferHeaders(const HttpHeaderBuilder& builder);
	/** 
	 * Sends any unbuffered data
	 */
	void send();
	/**
	* Syncs input and output buffers (writes all, reads all)
	* Then reads through the stream until it reaches an empty line (end of headers)
	* Copies header data and returns it in the HttpResponse
	* Stream is left in a state where it points to the content
	*/
	HttpResponse getResponse();
	/**
	 * Constructs an HTTP client over the given stream
	 * @param stream 
	 */
	HttpClient(std::unique_ptr<Streamable> && stream);
	HttpClient(HttpClient && client);
	HttpClient& operator=(HttpClient && c);
	~HttpClient();
};

