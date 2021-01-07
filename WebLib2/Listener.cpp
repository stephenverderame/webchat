#include "Listener.h"
#include "SSLSocket.h"
#include "openssl.h"
#include <openssl/err.h>


SSLListener::SSLListener(const char* key, const char* cert, port_t port) : con(Connection(Address(port), FDMethod::TCP))
{
	InitSSL();
	method = TLSv1_2_server_method();
	if ((ctx = SSL_CTX_new(method)) == NULL) throw StreamException(ERR_get_error(), "Error creating SSL ctx");
	SSL_CTX_set_ecdh_auto(ctx, 1);
	if (SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0) throw StreamException(ERR_get_error(), "Error with certificate");
	if (SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0) throw StreamException(ERR_get_error(), "Error with private key");
	con.bindSocket();
}

std::unique_ptr<Streamable> SSLListener::accept() 
{ 
	Connection c = con.acceptSocket();
	SSL* ssl;
	if ((ssl = SSL_new(ctx)) == NULL) throw StreamException(ERR_get_error(), "Error creating ssl");
	if (SSL_set_fd(ssl, c.sock) == 0) throw StreamException(ERR_get_error(), "Error setting fd");
	int ret;
	if ((ret = SSL_accept(ssl)) <= 0) throw StreamException(SSL_get_error(ssl, ret), "Error accepting client");
	return std::make_unique<SSLSocket>(std::move(c), ssl);
}

SSLListener::~SSLListener()
{
	if (ctx != NULL) SSL_CTX_free(ctx);
}

SocketListener::SocketListener(port_t port, FDMethod method) : con(Connection(Address(port), method))
{
	con.bindSocket();
}

std::unique_ptr<Streamable> SocketListener::accept() 
{
	return std::make_unique<Socket>(con.acceptSocket());
}
