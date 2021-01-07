#pragma once
#define WIN32_LEAN_AND_MEAN //excludes rarely used headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#undef min
#undef max
#include <WinSock2.h>
#include <string>
#include <functional>
#include "StreamException.h"
#include "RAIIContext.h"
using socket_t = unsigned int;
using port_t = unsigned short;
#define HTTP_PORT 80
#define HTTPS_PORT 443
enum class FDMethod : char {
	TCP = SOCK_STREAM,
	UDP = SOCK_DGRAM
};
enum class FDType : char {
	read, write, exception
};
/** 
 * An IPV4 address
 */
class Connection;
class FD;
class Address {
	friend class Connection;
private:
	static void winsockinit();
	static void winsockdeinit();
private:
	sockaddr_in address;
	static RAIIContext<winsockinit, winsockdeinit> ctx;
public:
	Address() = default;
	Address(sockaddr_in data);
	/**
	 * Constructs an address open for accepting on the port
	 * @param port the port to accept clients on
	 */
	Address(port_t port);
	/**
	 * Constructs an address with the given address and port
	 * @param address 
	 * @param port 
	 */
	Address(in_addr address, port_t port);
	/**
	 * Constructs an address with the given ip, given in a resolvable hostname or x.x.x.x format
	 * @param str 
	 */
	Address(const char* str);
	/**
	 * Constructs an address with the given ip, given in a resolvable hostname or x.x.x.x format and port
	 * @param str
	 */
	Address(const char* str, port_t port);
	/**
	 * Gets the string of the ip
	 * @return 
	 */
	std::wstring toString() const;
	inline in_addr getAddr() { return address.sin_addr; }
	inline port_t getPort() { return ntohs(address.sin_port); }
};
/** 
 * Object for basic socket operations
 */
class Connection {
	friend class SSLSocket; ///< need to pass sock to SSL methods
	friend class SSLListener;
	friend class FD;
private:
	static FD readFd;
	socket_t sock;
	///< Invariant: sock is INVALID_SOCKET if it is not open
	Address addr;
	FDMethod method;

	///Invariant: nullptr if it is not set
	std::function<void(Connection&)> onclose;
public:
	Connection();
	Connection(Address& addr, FDMethod method);
	Connection(Address&& addr, FDMethod method);
	~Connection();

	Connection& operator=(const Connection&) = delete;
	Connection(const Connection&) = delete;

	Connection(Connection&& o);
	Connection& operator=(Connection&& o);
	
	/**
	 * Gets the address of peer
	 * @return 
	 */
	Address peername() const;
	inline void close();

	/** 
	 * Binds the socket on the specified port and opens it for listening
	 * @throw ConnectionException on exception
	 */
	inline void bindSocket() throw(StreamException);
	/**
	 * Accepts a socket 
	 * Requires that this connection be bound and listening
	 * @return an incoming socket connection
	 * @throw ConnectionException on exception
	 */
	inline Connection acceptSocket() throw(StreamException);
	/**
	 * Determines if the socket is valid
	 * @return true if this is a valid and open socket
	 * @return false if not
	 */
	inline bool isValid() const noexcept;

	/**
	 * Connects the socket
	 * @throw ConnectionException on exception
	 */
	inline void connectSocket() throw(StreamException);

	inline void nonblock(bool enable);

	inline Address getAddr() const;

	/**
	 * Gets the amount of bytes that can be read in a single call to read
	 * @return the amount of bytes available
	 * @throw ConnectionException on failure
	 */
	inline int available() const throw(StreamException);

	/**
	 * Gets the socket operator of param
	 * @param param integer operation to get
	 * @return the reutn of getsockopt
	 * @throw ConnectionException on failure
	 */
	inline int getSockOpt(int param) const throw(StreamException);

	/**
	 * Gets the last error code and clears the error state
	 * @return last error code
	 * @throw ConnectionException on failure
	 */
	inline int errorCode() const throw(StreamException);

	/**
	 * Sets the socket operator to val
	 * @param param the socket operator to set
	 * @param val the value to set it to
	 * @throw ConnectionException on failure
	 */
	inline int setSockOpt(int param, int val) throw(StreamException);

	/**
	 * Gets the onclose callback function
	 * The callback is called right before the socket is closed
	 * @return 
	 */
	inline std::function<void(Connection&)>& onClose() { return onclose; }

	/**
	 * Sends a message on the socket
	 * @param buf buffer
	 * @param len length
	 * @return the amount of data actually sent
	 * @throw StreamException on error
	 */
	inline int send(const char* buf, size_t len) const throw(StreamException);

	/**
	 * Receives length amount of bytes at buf
	 * @param buf place to store the data
	 * @param len amount to read
	 * @return the amount read
	 * @throw StreamException on error
	 */
	inline int recv(char* buf, size_t len) const throw(StreamException);

	/** 
	 * Gets a ReadFDSET for all Connections
	 * Connections are automatically added and removed to this FD as they are created/destroyed
	 */
	inline const class FD& getReadFD() const noexcept;


};
inline void Connection::close()
{
	if (sock != INVALID_SOCKET) {
		if(onclose != nullptr) onclose(*this);
		closesocket(sock);
	}
	sock = INVALID_SOCKET;
}
inline void Connection::bindSocket() throw(StreamException)
{
	if(bind(sock, (sockaddr*)&addr.address, sizeof(addr.address)) != 0) throw StreamException(WSAGetLastError(), "WSA error binding socket");
	if(listen(sock, SOMAXCONN) != 0) throw StreamException(WSAGetLastError(), "WSA error listening socket");
}
inline Connection Connection::acceptSocket() throw(StreamException)
{
	Connection con;
	int sz = sizeof(sockaddr_in);
	con.sock = accept(sock, (sockaddr*)&con.addr.address, &sz);
	if (!con.isValid()) throw StreamException(WSAGetLastError(), "WSA error accepting socket");
	return con;
}
inline bool Connection::isValid() const noexcept
{
	return sock != INVALID_SOCKET;
}
inline void Connection::connectSocket() throw(StreamException)
{
	if (connect(sock, (sockaddr*)&addr.address, sizeof(addr.address)) != 0) throw StreamException(WSAGetLastError(), "WSA error connecting socket");
}
inline void Connection::nonblock(bool enable)
{
	unsigned long enableNonBlock = enable;
	ioctlsocket(sock, FIONBIO, &enableNonBlock);
}
inline Address Connection::getAddr() const
{
	return addr;
}
inline int Connection::available() const throw(StreamException)
{
	u_long bytes;
	if(SOCKET_ERROR == ioctlsocket(sock, FIONREAD, &bytes)) throw StreamException(WSAGetLastError(), "ioctl failure");
	return bytes;
}
inline int Connection::getSockOpt(int param) const throw(StreamException)
{
	int sz = sizeof(param);
	int out;
	if ((out = getsockopt(sock, SOL_SOCKET, param, (char*)&param, &sz)) == SOCKET_ERROR) throw StreamException(WSAGetLastError(), "Error getting socket op");
	return out;
}
inline int Connection::errorCode() const throw(StreamException)
{
	return getSockOpt(SO_ERROR);
}
inline int Connection::setSockOpt(int param, int val) throw(StreamException)
{
	int err;
	if ((err = setsockopt(sock, SOL_SOCKET, param, (char*)&val, sizeof(val))) == SOCKET_ERROR) throw StreamException(WSAGetLastError(), "Error setting socket operator");
	return err;
}
inline int Connection::send(const char* buf, size_t len) const throw(StreamException)
{
	int sent;
	switch(method) {
	case FDMethod::UDP:
		sent = sendto(sock, buf, len, 0, (sockaddr*)&addr.address, sizeof(addr.address));
	default:
		sent =  ::send(sock, buf, len, 0);
	}
	if (sent < 0) throw StreamException(WSAGetLastError(), "Error sending data");
	return sent;
}
inline int Connection::recv(char* buf, size_t len) const throw(StreamException)
{
	int sz = sizeof(sockaddr_in);
	int rec;
	switch(method) {
	case FDMethod::UDP:
		rec = recvfrom(sock, buf, len, 0, (sockaddr*)&addr.address, &sz);
	default:
		rec = ::recv(sock, buf, len, 0);
	}
	if (rec < 0) throw StreamException(WSAGetLastError(), "Error fetching data");
	return rec;
}
class FD {
private:
	fd_set fd;
	std::list<const Connection*> socks;
	const FDType type;
public:
	/**
	 * Attaches a connection to the fd
	 * Requires C must outlive the FD or remove itself
	 * @param c the sock to attach to
	 */
	inline void attach(const Connection* c) {
		socks.push_back(c);
		FD_SET(c->sock, &fd);
	}

	/**
	 * Removes a connection from the FD
	 * @param c
	 */
	inline void detach(const Connection* c) {
		socks.remove(c);
	}
public:
	FD(FDType type);

	/**
	 * Determines if a connection is set
	 * @param c the connection
	 * @return true 
	 * @return false 
	 */
	inline bool inset(const Connection & c) const {
		return FD_ISSET(&fd, c.sock);
	}
	/**
	 * Waits until a socket in the FD is set
	 * @param t maximum time {sec, microsec} or {0, 0} to wait infinetely
	 */
	inline void wait(timeval t = { 0, 0 }) const {
		switch(type){
		case FDType::read:
			select(0, const_cast<fd_set*>(&fd), 0, 0, t.tv_sec == 0 && t.tv_usec == 0 ? 0 : &t);
			break;
		case FDType::write:
			select(0, 0, const_cast<fd_set*>(&fd), 0, t.tv_sec == 0 && t.tv_usec == 0 ? 0 : &t);
			break;
		case FDType::exception:
			select(0, 0, 0, const_cast<fd_set*>(&fd), t.tv_sec == 0 && t.tv_usec == 0 ? 0 : &t);
			break;
		}
	}
	/** 
	 * Clears the fd_set and readds all attached sockets
	 * Removes all past socket interactions from the FD
	 */
	inline void refresh() {
		FD_ZERO(&fd);
		for (auto& c : socks)
			FD_SET(c->sock, &fd);
	}
};
inline FD::FD(FDType type) : type(type)
{
	FD_ZERO(&fd);
}
inline const FD& Connection::getReadFD() const noexcept
{
	return readFd;
}

class Connectable {
public:
	/**
	 * Gets the underlying connection for this object
	 */
	virtual Connection& getConnection() = 0;
	/**
	 * Opens the underlying connection
	 * @throw StreamException on error
	 */
	virtual void open() throw(StreamException) = 0;
};
