#pragma once
#define WIN32_LEAN_AND_MEAN //excludes rarely used headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include "Streamable.h"
#include <functional>
using socket_t = unsigned int;
using port_t = unsigned short;

#define HTTP_PORT 80
#define HTTPS_PORT 443
enum class FDMethod {
	TCP = SOCK_STREAM,
	UDP = SOCK_DGRAM
};
class Address {
private:
	in_addr address; //this is just a long
public:
	Address(const char * str);
	inline in_addr get() { return address; }
	inline operator in_addr() { return address; }
};
class Socket : public Streamable {
private:
	socket_t sock;
	sockaddr_in data;
	bool blocking;
protected:
	int nvi_write(const char * data, size_t len) override;
	int nvi_read(char * data, size_t amt) const override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;
public:
	Socket();
	Socket(Address addr, FDMethod meth, port_t p);
	~Socket();
	int open(Address addr, FDMethod meth, port_t p);
	void shouldBlock(bool blocking);
	void close();
};
//Custom RAII class
class Context {
private:
	std::function<void(void)> dtor;
public:
	Context(std::function<void(void)> init, std::function<void(void)> deinit) : dtor(deinit) { init(); }
	~Context() { dtor(); }
};
#define WinsockContext \
Context winsock_ctx([]() { \
	WSAData d; \
	WSAStartup(MAKEWORD(2, 1), &d); \
	}, []() {WSACleanup();})


