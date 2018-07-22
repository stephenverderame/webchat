#include "Https.h"

int HttpsClient::recv_ss(char ** buffer, int size)
{
	int dataGot = 0;
	while (dataGot < size) {
		int ret = SSL_read(ssl, *buffer + dataGot, size - dataGot);
		if (ret <= 0)
			return SSL_get_error(ssl, ret);
		dataGot += ret;
	}
	return 0;
}

int HttpsClient::send_ss(char * buffer, int size)
{
	int dataSent = 0;
	int _size = size == -1 ? strlen(buffer) : size;
	while (dataSent < _size) {
		int ret = SSL_write(ssl, buffer + dataSent, size - dataSent);
		if (ret <= 0)
			return SSL_get_error(ssl, ret);
		dataSent += ret;
	}
	return 0;
}
HttpsClient::HttpsClient(char * hostname, unsigned int port)
{
	hostent * server = gethostbyname(hostname);
	sock = socket(AF_INET, SOCK_STREAM, NULL);
	addrData.sin_family = AF_INET;
	addrData.sin_port = htons(port);
	addrData.sin_addr = *((LPIN_ADDR)*server->h_addr_list);
}

HttpsClient::HttpsClient(SOCKET sock, SSL ** ssl)
{
	this->sock = sock;
	this->ssl = *ssl;
}

int HttpsClient::connectClient()
{
	if (connect(sock, (SOCKADDR*)&addrData, sizeof(addrData)))
		return -1;
	const SSL_METHOD * method = TLSv1_2_client_method();
	ctx = SSL_CTX_new(method);
	ssl = SSL_new(ctx);
	if (!ssl)
		return -50;
	SSL_set_fd(ssl, sock);
	if (SSL_connect(ssl) <= 0)
		return -51;
	return 0;
}

int HttpsClient::sendMessage(HttpFrame frame)
{
	int ret = SSL_write(ssl, frame.data.c_str(), frame.data.size());
	if (ret <= 0)
		return SSL_get_error(ssl, ret);
	return 0;
}

int HttpsClient::sendMessage(char * msg)
{
	return send_ss(msg);
}

int HttpsClient::sendHtmlFile(HtmlFile file)
{
	int ret = SSL_write(ssl, file.data, file.size);
	if (ret <= 0)
		return SSL_get_error(ssl, ret);
	return 0;
}

int HttpsClient::getMessage(HttpFrame & frame)
{
	char buf[500];
	int ret = 1;
	std::stringstream stream;
	do {
		ret = SSL_read(ssl, buf, 500);
		if (ret > 0) {
			buf[ret] = 0;
//			printf("Read %s \n", buf);
			stream << buf;
		}
//		else
//			printf("Should exit loop bc %d \n", ret);
	} while (SSL_get_error(ssl, ret) == SSL_ERROR_WANT_READ);
	if (ret <= 0) {
		int code = SSL_get_error(ssl, ret);
//		if (code == SSL_ERROR_SYSCALL || code == SSL_ERROR_SSL)
			return code;
	}
	std::string msg = stream.str();
	frame.content = msg.substr(msg.find("\r\n\r\n") + 4);
	std::string header = msg.substr(0, msg.find("\r\n\r\n"));
	frame.load((char*)header.c_str());
//	printf("Got message \n");
	return 0;
}

void HttpsClient::changeHost(char * hostname, unsigned short port)
{
	hostent * server = gethostbyname(hostname);
	addrData.sin_port = htons(port);
	addrData.sin_addr = *((LPIN_ADDR)*server->h_addr_list);
	sock = socket(AF_INET, SOCK_STREAM, NULL);
}

void HttpsClient::closeConnection()
{
	SSL_free(ssl);
	if(ctx != NULL)
		SSL_CTX_free(ctx);
	closesocket(sock);
}

void HttpsListener::loadCert(const char * key, const char * cert)
{
	const SSL_METHOD * meth = TLSv1_2_server_method();
	ctx = SSL_CTX_new(meth);
	SSL_CTX_set_ecdh_auto(ctx, 1);
	if(SSL_CTX_use_certificate_file(ctx, cert, SSL_FILETYPE_PEM) <= 0)
		ERR_print_errors_fp(stderr);
	if(SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM) <= 0)
		ERR_print_errors_fp(stderr);
}

HttpsClient HttpsListener::accept(int & error)
{
#ifdef _WIN32
	int len = sizeof(addrData);
#else
	unsigned int len = sizeof(addrData);
#endif
	error = 0;
	SOCKET connection = ::accept(sock, (SOCKADDR*)&addrData, &len);
	if (connection == INVALID_SOCKET) {
		printf("Could not accept\n");
		error = -1;
	}
	else
		printf("Accepted tcp socket\n");
	SSL * ssl = SSL_new(ctx);
	if (ssl == NULL) error = -55;
	int ret = SSL_set_fd(ssl, connection);
	if (ret == 0)
		error = SSL_get_error(ssl, ret);
	else
		printf("Set ssl fd \n");
	ret = SSL_accept(ssl);
	if (ret <= 0) {
		error = SSL_get_error(ssl, ret);
		printf("Failed to accept \n");
	}
	else
		printf("Accepted ! \n");
	HttpsClient client(connection, &ssl);
	client.ctx = NULL;
	return client;
}
