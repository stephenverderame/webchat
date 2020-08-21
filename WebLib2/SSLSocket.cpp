#include "SSLSocket.h"
#include <openssl/bio.h>
#include <openssl/err.h>
#include <openssl/ssl3.h>
bool SSLSocket::init = false;
int SSLSocket::nvi_write(const char * data, size_t len)
{
	return SSL_write(ssl, data, len);
}

int SSLSocket::nvi_read(char * data, size_t amt) const
{
	return SSL_read(ssl, data, amt);
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

SSLSocket::SSLSocket(const SSL_METHOD * meth, socket_t sock) : sock(sock), method(meth), ssl(nullptr), ctx(nullptr)
{
	if (!init) {
		init = true;
		SSL_library_init();
		SSL_load_error_strings();
	}
}

SSLSocket::SSLSocket(Address addr, FDMethod meth, port_t p, const SSL_METHOD * smeth) : SSLSocket(smeth)
{
	open(addr, meth, p);
}

SSLSocket::~SSLSocket()
{
	if (ssl != nullptr) SSL_free(ssl);
	if (ctx != nullptr) SSL_CTX_free(ctx);
	if (sock != INVALID_SOCKET) closesocket(sock);
}

int SSLSocket::open(Address addr, FDMethod meth, port_t p)
{
	data.sin_addr = addr;
	data.sin_family = AF_INET;
	data.sin_port = htons(p);
	sock = socket(AF_INET, (int)meth, 0);
	if (sock == INVALID_SOCKET) return -1;
	int err = connect(sock, (sockaddr*)&data, sizeof(data));
	if (err != 0) return err;
	ctx = SSL_CTX_new(method);
	ssl = SSL_new(ctx);
	if (!ssl)
		return -50;
	SSL_set_fd(ssl, sock);
	if (err = SSL_connect(ssl) <= 0)
		return err;
	return 0;
}
