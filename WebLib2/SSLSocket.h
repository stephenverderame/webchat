#pragma once
#include "Socket.h"
#include <openssl/ssl.h>
class SSLSocket : public Streamable
{
private:
	socket_t sock;
	sockaddr_in data;
	SSL * ssl;
	SSL_CTX * ctx;
	const SSL_METHOD * method;
	static bool init;
protected:
	int nvi_write(const char * data, size_t len) override;
	int nvi_read(char * data, size_t amt) const override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;
public:
	SSLSocket(const SSL_METHOD * meth, socket_t sock = INVALID_SOCKET);
	SSLSocket(Address addr, FDMethod meth, port_t p, const SSL_METHOD * smeth = TLSv1_2_client_method());
	~SSLSocket();
	int open(Address addr, FDMethod meth, port_t p);
};

