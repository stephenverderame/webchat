#include "HttpClient.h"
#include <vector>
#include <string>
#include "StreamView.h"
#include <array>
#include "Coder.h"
#include "File.h"
#include <sstream>
struct HttpHeaderBuilder::impl
{
	std::vector<std::pair<const char *, const char *>> headers;
};
HttpRequestBuilder & HttpRequestBuilder::path(const char * path)
{
	this->requestPath = path;
	return *this;
}
HttpHeaderBuilder & HttpHeaderBuilder::put(const char * header, const char * value)
{
	pimpl->headers.emplace_back(header, value);
	return *this;
}
HttpHeaderBuilder & HttpHeaderBuilder::put(AutoHttpHeaders header)
{
	switch (header) {
	case AutoHttpHeaders::AcceptedEncodings:
		put("Accept-Encoding", "gzip, chunked, identity");
		break;
	}
	return *this;
}
HttpRequestBuilder & HttpRequestBuilder::method(const char * method)
{
	this->requestMethod = method;
	return *this;
}
HttpResponseBuilder & HttpResponseBuilder::response(const char * response)
{
	this->respCode = response;
	return *this;
}
HttpHeaderBuilder::~HttpHeaderBuilder() = default;
HttpHeaderBuilder::HttpHeaderBuilder() : pimpl(std::make_unique<impl>()) {};
Streamable & HttpClient::getStream()
{
	return *stream.get();
}
void HttpClient::bufferHeaders(const HttpHeaderBuilder& builder)
{
	(*stream) << builder.build().str();
}
std::stringstream HttpResponseBuilder::build() const
{
	std::stringstream str;
	str << "HTTP/1.1 " << this->respCode;
	for (auto& p : pimpl->headers)
		str << p.first << ": " << p.second << "\r\n";
	str << (pimpl->headers.empty() ? "\r\n\r\n" : "\r\n");
	return str;
}
HttpResponseBuilder::~HttpResponseBuilder() = default;
HttpResponseBuilder::HttpResponseBuilder() : respCode(nullptr) {};
std::stringstream HttpRequestBuilder::build() const
{
	std::stringstream str;
	str << this->requestMethod << " /" << this->requestPath << " HTTP/1.1\r\n";
	for (auto& p : pimpl->headers)
		str << p.first << ": " << p.second << "\r\n";
	str << (pimpl->headers.empty() ? "\r\n\r\n" : "\r\n");
	return str;
}
HttpRequestBuilder::~HttpRequestBuilder() = default;
HttpRequestBuilder::HttpRequestBuilder() : requestMethod(nullptr), requestPath("") {};
void HttpClient::send()
{
	stream->pubsync();
}
HttpResponse HttpClient::getResponse()
{
	stream->syncOutputBuffer();
	auto headerEndIndex = stream->fetchUntil("\r\n\r\n", 500);
	HttpResponse resp;
	resp.headers.setCaseSensitive(false);
	Streamable & s = *stream.get();
	StreamView buffer = stream->getSharedView();
	StreamView line;
	if (buffer.getsub(line, "\r\n")) {
		std::streamsize firstSpace = line.find(' ');
		if (firstSpace != StreamView::INVALID) {
			resp.responseCode = line.sub(firstSpace + 1, firstSpace + 4).parse();
			resp.responseMessage = line.sub(line.find(' ', firstSpace + 1) + 1);
			std::streamsize lastHeaderPart = buffer.tell();
			while (buffer.getsub(line, "\r\n") && line.size() > 0) {
				lastHeaderPart = buffer.tell();
				std::streamsize pos = line.find(':');
				if (pos != StreamView::INVALID) {
					resp.headers.put(line.sub(0, pos), line.sub(pos + 2));
					StreamView sv = line.sub(0, pos);
//					std::cout << sv << ": " << line.sub(pos + 2) << "\n";
/*					if (sv == "content-encoding") {
						if (!resp.headers.find(sv) || !resp.headers.find("Content-Encoding")) std::cout << "UHAHDHDHD" << "\n";
						std::cout << sv.hashCaseInsensitive() << " : " << util::HASH_NO_CASE("Content-Encoding") << std::endl;
					}*/
				}
				else {					
					break;
				}
			}
			stream->seekg(headerEndIndex + 4);
			bool transferEncoding = resp.headers.find("Transfer-Encoding");
			bool contentEncoding = resp.headers.find("Content-Encoding");
			if (transferEncoding && resp.headers["Transfer-Encoding"].second == "chunked") {
				/*
				* Chunk format:
				*   size in hex\r\n
				*   chunk of size\r\n
				* ... next chunk
				*/

				std::streamsize size;
				do {
					if (stream->view().find("\r\n", stream->icur()) == StreamView::INVALID) {
						stream->fetchUntil("\r\n");
					}
					std::streampos initial = s.tellg();
					s >> std::hex >> size;
					s.pubseekoff(2, std::ios::cur, std::ios::in); //one more \r\n
					auto now = s.tellg();
					stream->remove(initial, now);
					now = s.tellg();
					if (size > 0) {
						stream->fetchFor(size + 2LL - (stream->getBufferSize() - now));						
					}
					s.pubseekoff(size + 2LL, std::ios::cur, std::ios::in);
					now = s.tellg();
					stream->remove(now - std::streamoff(2), now); //remove ending \r\n
				} while (size > 0);
				stream->seekg(headerEndIndex + 4);
//				printf("%d\n", headerEndIndex);
			}
			else if (resp.headers.find("Content-Length")) {
				std::streampos initial = s.tellg();
				std::streampos len = resp.headers["Content-Length"].second.parse();
				while ((std::streamoff)initial + len >= s.getBufferSize()) {
					if (s.syncInputBuffer() != 0) break;
				}
				stream->seekg(headerEndIndex + 4);
			}
			if (contentEncoding && resp.headers["Content-Encoding"].second == "gzip") {
//				File test("test.gz", FileMode::write + FileMode::binary);
//				test << stream;
				GzipCoder * gzip = new GzipCoder(stream.get());
				int err = gzip->decode();
				printf("Gzip error: %d %d\n", err, gzip->getError(err));
				stream = std::unique_ptr<Streamable>(gzip);
			}
			resp.content = stream->getSharedView(std::ios::cur);
		}
	}
	return std::move(resp);
	
}
HttpClient::HttpClient(std::unique_ptr<Streamable> && stream) : stream(std::move(stream)) {
}

HttpClient::HttpClient(HttpClient && client) : stream(std::move(client.stream))
{
}

HttpClient & HttpClient::operator=(HttpClient && c)
{
	stream.swap(c.stream);
	return *this;
}

HttpClient::~HttpClient() = default;


HttpResponse::HttpResponse()
{
	headers.setCaseSensitive(false);
}
