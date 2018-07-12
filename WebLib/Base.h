#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>
#include <memory>
#define STD_HTTP_PORT 80
#define STD_HTTPS_PORT 443
#define STD_FTP_PORT 20
#define STD_FTPS_PORT 990
#define STD_SMTP_PORT 25
#define STD_POP_PORT 110
#define STD_IMAP_PORT 143
#define ENABLE 1
#define DISABLE 0
#define EOL "\r\n"
typedef unsigned long long fsize_t;
typedef long long num_t;
class Winsock {
public:
	Winsock();
	~Winsock();
	inline int lastError() { return WSAGetLastError(); }
};
class FD;
class Socket {
	friend class FD;
protected:
	SOCKET sock;
	SOCKADDR_IN addrData;
protected:
	int send_s(char * msg, num_t size = -1);
	int recv_s(char ** msg, num_t size);
public:
	void addToFD(FD_SET * fd);
	bool isInFD(FD_SET * fd);
	void nagle(int enable);
	void nonblocking(int enable);
	int getErrorCode();
	std::string getIpAddress();
	virtual void close();
	SOCKET & sock_t() { return sock; }
};
class Client : public Socket {
protected:
	bool closed;
public:
	Client() : closed(false) {};
	Client(SOCKET s);
	virtual void close();
	bool isClosed() { return closed; }
};
class Listener : public Socket{
public:
	Listener(unsigned short port);
};
class FD {
private:
	FD_SET set;
public:
	void clear() { FD_ZERO(&set); }
	void add(Socket * s) { FD_SET(s->sock, &set); }
	bool inSet(Socket * s) { return FD_ISSET(s->sock, &set); }
public:
	static void wait(FD * read, FD * write = nullptr, FD * except = nullptr);
	static void waitUntil(timeval * t, FD * read, FD * write = nullptr, FD * except = nullptr);
};