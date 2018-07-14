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

int HttpsClient::getMessage(HttpFrame & frame)
{
	char buf[500];
	int ret = 1;
	std::stringstream stream;
	do {
		ret = SSL_read(ssl, buf, 300);
		buf[ret] = 0;
		stream << buf;
	} while (ret > 0);
//	printf("%s \n", stream.str().c_str());
	if (ret <= 0) {
		int code = SSL_get_error(ssl, ret);
		if (code == SSL_ERROR_SYSCALL || code == SSL_ERROR_SSL)
			return code;
	}
	std::string msg = stream.str();
	frame.content = msg.substr(msg.find("\r\n\r\n") + 4);
	std::string header = msg.substr(0, msg.find("\r\n\r\n"));
	frame.load((char*)header.c_str());
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
	SSL_CTX_free(ctx);
	closesocket(sock);
}
