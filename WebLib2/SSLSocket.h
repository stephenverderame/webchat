#pragma once
#include "Socket.h"
#include <openssl/ssl.h>
class SSLSocket : public Streamable, public Connectable
{
private:
	Connection con;
	SSL * ssl;
	SSL_CTX * ctx;
	const SSL_METHOD * method;
protected:
	int nvi_write(const char * data, size_t len) throw(StreamException) override;
	int nvi_read(char * data, size_t amt) const throw(StreamException) override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;
public:
	SSLSocket(const SSL_METHOD * meth, socket_t sock = INVALID_SOCKET);
	SSLSocket(Connection && c, const SSL_METHOD * smeth = TLSv1_2_client_method());
	SSLSocket(Connection&& c, SSL* ssl) : con(std::move(c)), ssl(ssl), ctx(nullptr), method(nullptr) {};
	Connection& getConnection() { return con; }
	~SSLSocket();
	void open() throw(StreamException) override;
};

