#pragma once
#include "Base.h"
#include <cstdio>
#include <regex>
#include <string>
#include <map>
#include <vector>
#include <sstream>

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
class HttpListener;
class HttpClient;

struct HttpFrame {
	std::string data;
	std::string protocol;
	std::string file;
	std::string responseCode;
	std::map<std::string, std::string> headers;
	std::string content;
	void load(char * frame);
	void composeResponse();
	void composeRequest();
};

class HtmlFile {
	friend class HttpClient;
private:
	char * data;
	size_t size;
public:
	~HtmlFile() { if(size > 0) delete[] data; }
	void loadFile(const char * filename);
	HtmlFile() {};
	HtmlFile(const char * filename) { loadFile(filename); }
	HtmlFile(HttpFrame frame);
	void saveFile(const char * filename);
};
class HttpListener : public Listener {
public:
	HttpListener(unsigned short port = STD_HTTP_PORT) : Listener(port) {};
	HttpClient accept(int & error);

};
class HttpClient : public Client {
public:
	HttpClient(SOCKET s) : Client(s) {};
	HttpClient() : Client() {};
	HttpClient(char * hostname, unsigned short port = STD_HTTP_PORT);
	int getMessage(HttpFrame & frame);
	int sendHtmlFile(HtmlFile file);
	int sendMessage(HttpFrame msg);
	int sendMessage(char * rawMsg);
	void changeHost(char * hostname, unsigned short port = STD_HTTP_PORT);
	int connectClient();

};