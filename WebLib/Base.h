#ifndef BASE_H
#define BASE_H
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

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <string>
#include <memory>
class Winsock {
public:
	Winsock();
	~Winsock();
	inline int lastError() { return WSAGetLastError(); }
};
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <string>
#include <memory.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
typedef unsigned int SOCKET;
typedef sockaddr SOCKADDR;
typedef sockaddr_in SOCKADDR_IN;
typedef fd_set FD_SET;
typedef in_addr * LPIN_ADDR;
typedef SOCKADDR* PSOCKADDR;
#define closesocket(sock) shutdown(sock, SHUT_RDWR); close(sock);
#define SOCKET_ERROR -1
#define INVALID_SOCKET (~0)
#define ioctlsocket(s, cmd, arg) ioctl(s, cmd, arg)
#define WSAGetLastError() 1
#define sprintf_s(buffer, size, format, ...) sprintf(buffer, format, __VA_ARGS__)
#define memcpy_s(dest, destSize, source, sourceSize) memcpy(dest, source, sourceSize)
inline long long ntohll(long long a);
inline int fopen_s(FILE ** stream, const char * filename, const char * mode);
#define htonll(a) ntohll(a)
#endif //_WIN32
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
	virtual void closeConnection();
	SOCKET & sock_t() { return sock; }
};
class Client : public Socket {
protected:
	bool closed;
public:
	Client() : closed(false) {};
	Client(SOCKET s);
	virtual void closeConnection();
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

#endif /*BASE_H*/
