#include "Socket.h"
#include <array>

int Socket::nvi_write(const char * data, size_t len)
{
	return con.send(data, len);
}

int Socket::nvi_read(char * data, size_t amt) const
{
	return con.recv(data, amt);
}

int Socket::nvi_error(int errorCode) const
{
	return con.errorCode();
}

int Socket::minAvailableBytes() const
{
	return con.available();
}

bool Socket::nvi_available() const
{
	return minAvailableBytes() > 0;
}


Socket::Socket(Connection&& s)
{
	con = std::move(s);
}

void Socket::open() throw(StreamException)
{
	con.connectSocket();
}


