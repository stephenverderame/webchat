#include "Socket.h"
#include <array>
#include <ws2tcpip.h>

int Socket::nvi_write(const char * data, size_t len)
{
	return send(sock, data, len, 0);
}

int Socket::nvi_read(char * data, size_t amt) const
{
	return recv(sock, data, amt, 0);
}

int Socket::nvi_error(int errorCode) const
{
	int len = sizeof(errorCode);
	getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&errorCode, &len);
	return errorCode;
}

int Socket::minAvailableBytes() const
{
	unsigned long arg;
	ioctlsocket(sock, FIONREAD, &arg);
	return arg;
}

bool Socket::nvi_available() const
{
	return minAvailableBytes() > 0;
}

Socket::Socket() : sock(INVALID_SOCKET), blocking(true)
{
	
}

Socket::Socket(Address addr, FDMethod meth, port_t p) : Socket()
{
	open(addr, meth, p);
}

Socket::~Socket()
{
	close();
}

int Socket::open(Address addr, FDMethod meth, port_t p)
{
	data.sin_addr = addr.get();
	data.sin_family = AF_INET;
	data.sin_port = htons(p);
	sock = socket(AF_INET, (int)meth, 0);
	if (sock == INVALID_SOCKET) return -1;
	return connect(sock, (sockaddr*)&data, sizeof(data));
}

void Socket::shouldBlock(bool blocking)
{
	this->blocking = blocking;
	unsigned long enableNonBlock = !blocking;
	ioctlsocket(sock, FIONBIO, &enableNonBlock);
}

void Socket::close()
{
	if (sock != INVALID_SOCKET) closesocket(sock);
}

Address::Address(const char * str)
{
	if (isalpha(str[0])) {
		hostent * host = gethostbyname(str);
		address = *((in_addr*)*host->h_addr_list);
	}
	else {
		inet_pton(AF_INET, str, (void*)&address);
	}
}

