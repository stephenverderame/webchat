#pragma once
#include "Streamable.h"
#include "Connection.h"
#include "SSLSocket.h"
#include "StreamException.h"
class Listener
{
public:
	/**
	 * Returns a streamable object from an incoming client connection
	 * @return a new streamable
	 * @throw StreamException on error
	 */
	virtual std::unique_ptr<Streamable> accept() throw (StreamException) = 0;
};

class SSLListener : public Listener {
private:
	Connection con;
	const SSL_METHOD* method;
	SSL_CTX* ctx;
public:
	/**
	 * Constructs an SSLListener with the given key, certificate and port
	 * @param key file path for the key
	 * @param cert file path for the certificate
	 * @param port the port to listen on
	 */
	SSLListener(const char* key, const char* cert, port_t port) throw (StreamException);
	std::unique_ptr<Streamable> accept() throw (StreamException) override;
	~SSLListener();
};

class SocketListener : public Listener {
private:
	Connection con;
public:
	/**
	 * Constructs a socket listener on the specified port
	 * @param port
	 * @param method type of listener TCP or UDP
	 */
	SocketListener(port_t port, FDMethod method);
	std::unique_ptr<Streamable> accept() throw(StreamException) override;
};

