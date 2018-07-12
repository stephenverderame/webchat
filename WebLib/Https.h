#pragma once
#include "Http.h"
#include "SSL.h"
#define IS_SSL_ERROR(code) ((code) <= -50)
#define IS_WINSOCK_ERROR(code) ((code) > -50)
class HttpsClient : public Client {
private:
	SSL * ssl;
	SSL_CTX * ctx;
private:
	int recv_ss(char ** buffer, int size);
	int send_ss(char * buffer, int size = -1);
public:
	HttpsClient(char * hostname, unsigned int port = STD_HTTPS_PORT);
	int connectClient();
	int sendMessage(HttpFrame frame);
	int sendMessage(char * msg);
	int getMessage(HttpFrame & frame);
	void changeHost(char * hostname, unsigned short port = STD_HTTPS_PORT);
	void close();
};