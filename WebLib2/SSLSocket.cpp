#include "SSLSocket.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl3.h>
#include "openssl.h"
int SSLSocket::nvi_write(const char * data, size_t len)
{
	int ret;
	if ((ret = SSL_write(ssl, data, len)) <= 0) throw StreamException(SSL_get_error(ssl, ret), "Error sending ssl data");
	return ret;
}

int SSLSocket::nvi_read(char * data, size_t amt) const
{
	int ret;
	if((ret = SSL_read(ssl, data, amt)) <= 0) throw StreamException(SSL_get_error(ssl, ret), "Error getting ssl data");
}

int SSLSocket::nvi_error(int errorCode) const
{
	return SSL_get_error(ssl, errorCode);
}

int SSLSocket::minAvailableBytes() const
{
	return SSL_pending(ssl);
}

bool SSLSocket::nvi_available() const
{
	return SSL_pending(ssl);
}

SSLSocket::SSLSocket(const SSL_METHOD * meth, socket_t sock) : method(meth), ssl(nullptr), ctx(nullptr)
{
}

SSLSocket::SSLSocket(Connection && c, const SSL_METHOD * smeth) : SSLSocket(smeth)
{
	InitSSL();
	con = std::move(c);
	open();
}

SSLSocket::~SSLSocket()
{
	if (ssl != nullptr) SSL_free(ssl);
	if (ctx != nullptr) SSL_CTX_free(ctx);
	con.close();
}

void SSLSocket::open()
{
	con.connectSocket();
	if((ctx = SSL_CTX_new(method)) == NULL) throw StreamException(ERR_get_error(), "SSL could not create ctx");
	if ((ssl = SSL_new(ctx)) == NULL) throw StreamException(ERR_get_error(), "SSL could not create ssl");
	int err;
	if((err = SSL_set_fd(ssl, con.sock)) <= 0) throw StreamException(SSL_get_error(ssl, err), "SSL could not set FD");
	if ((err = SSL_connect(ssl)) <= 0) throw StreamException(SSL_get_error(ssl, err), "SSL could not connect");
}
