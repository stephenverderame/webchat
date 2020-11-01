#include "HttpClient.h"
#include <vector>
#include <string>
#include "StreamView.h"
#include <array>
#include "Coder.h"
#include "File.h"
struct HttpClient::impl
{
	std::vector<std::pair<const char *, const char *>> headers;
	const char * method = nullptr, * path = "", * resp = nullptr;	
};
HttpClient & HttpClient::path(const char * path)
{
	pimpl->path = path;
	return *this;
}
HttpClient & HttpClient::put(const char * header, const char * value)
{
	pimpl->headers.emplace_back(header, value);
	return *this;
}
HttpClient & HttpClient::put(AutoHttpHeaders header)
{
	switch (header) {
	case AutoHttpHeaders::AcceptedEncodings:
		put("Accept-Encoding", "gzip, chunked, identity");
		break;
	}
	return *this;
}
HttpClient & HttpClient::method(const char * method)
{
	pimpl->method = method;
	return *this;
}
HttpClient & HttpClient::response(const char * response)
{
	pimpl->resp = response;
	return *this;
}
Streamable & HttpClient::getStream()
{
	return *stream.get();
}
void HttpClient::bufferHeaders()
{
	Streamable & str = *stream.get();
	if (pimpl->resp != nullptr) {
		str << "HTTP/1.1 " << pimpl->resp;
		for (auto& p : pimpl->headers)
			str << p.first << ": " << p.second << "\r\n";
		str << (pimpl->headers.empty() ? "\r\n\r\n" : "\r\n");
	}
	else if (pimpl->method != nullptr) {
		str << pimpl->method << " /" << pimpl->path << " HTTP/1.1\r\n";
		for (auto& p : pimpl->headers)
			str << p.first << ": " << p.second << "\r\n";
		str << (pimpl->headers.empty() ? "\r\n\r\n" : "\r\n");
	}
}
void HttpClient::send()
{
	stream->pubsync();
}
HttpResponse HttpClient::getResponse()
{
	stream->syncOutputBuffer();
	auto headerEndIndex = stream->fetchUntil("\r\n\r\n", 500);
	HttpResponse resp;
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
					std::cout << sv << ": " << line.sub(pos + 2) << "\n";
					if (sv == "content-encoding") {
						std::cout << sv.hashCaseInsensitive() << " : " << util::HASH_NO_CASE("Content-Encoding") << std::endl;
					}
				}
				else {					
					break;
				}
			}
			stream->seekg(headerEndIndex + 4);
			StreamView & transferEncoding = resp.headers[util::HASH_NO_CASE("Transfer-Encoding")];
			StreamView & contentEncoding = resp.headers[util::HASH_NO_CASE("Content-Encoding")];
			if (transferEncoding.size() > 0 && transferEncoding == "chunked") {
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
				printf("%d\n", headerEndIndex);
			}
			else if (resp.headers.find(util::HASH_NO_CASE("Content-Length"))) {
				std::streampos initial = s.tellg();
				std::streampos len = resp.headers[util::HASH_NO_CASE("Content-Length")].parse();
				while ((std::streamoff)initial + len >= s.getBufferSize()) {
					if (s.syncInputBuffer() != 0) break;
				}
				stream->seekg(headerEndIndex + 4);
			}
			std::cout << contentEncoding << "\n";
			if (contentEncoding.size() > 0 && contentEncoding == "gzip") {
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
HttpClient::HttpClient(std::unique_ptr<Streamable> && stream) : pimpl(std::make_unique<impl>()) {
	attach(std::move(stream));
}

HttpClient::HttpClient(HttpClient && client) : pimpl(std::move(client.pimpl))
{
}

HttpClient & HttpClient::operator=(HttpClient && c)
{
	pimpl.swap(c.pimpl);
	return *this;
}

HttpClient::~HttpClient() = default;

struct HttpHeaders::impl {
	std::unordered_map<hash_t, std::pair<StreamView, StreamView>> data;
	StreamView emptyView;
};
StreamView& HttpHeaders::operator[](const char * key)
{
	return operator[](util::HASH_NO_CASE(key));
}
StreamView& HttpHeaders::operator[](hash_t hashKey)
{
	auto it = pimpl->data.find(hashKey);
	if (it != pimpl->data.end())
		return it->second.second;
	return pimpl->emptyView;
}
void HttpHeaders::put(StreamView && key, StreamView && val)
{
	pimpl->data[key.hashCaseInsensitive()] = std::make_pair(key, val);
}
void HttpHeaders::put(const StreamView& key, const StreamView& val)
{

	pimpl->data[key.hashCaseInsensitive()] = std::make_pair(key, val);
}
void HttpHeaders::put(StreamView && key, const StreamView & val)
{
	pimpl->data[key.hashCaseInsensitive()] = std::make_pair(key, val);
}
void HttpHeaders::put(const StreamView & key, StreamView && val)
{
	pimpl->data[key.hashCaseInsensitive()] = std::make_pair(key, val);
}
HttpHeaders::HttpHeaders() : pimpl(std::make_unique<impl>())
{
}

HttpHeaders::HttpHeaders(HttpHeaders && h) : pimpl(std::move(h.pimpl))
{
	
}

HttpHeaders & HttpHeaders::operator=(HttpHeaders && h)
{
	pimpl.swap(h.pimpl);
	return *this;
}

HttpHeaders::~HttpHeaders() = default;

bool HttpHeaders::find(const char * key)
{
	return (*this)[key].size() > 0;
}

bool HttpHeaders::find(hash_t hashKey)
{
	return (*this)[hashKey].size() > 0;
}

void HttpHeaders::for_each(std::function<void(StreamView &, StreamView &)> pred)
{
	for (auto header : pimpl->data) {
		pred(header.second.first, header.second.second);
	}
}
