#ifndef HTTPS_H
#define HTTPS_H
#include "Http.h"
#include "SSL.h"
#define IS_SSL_ERROR(code) ((code) <= -50)
#define IS_WINSOCK_ERROR(code) ((code) > -50)
class HttpsListener;
class HttpsClient : public Client {
	friend class HttpsListener;
private:
	SSL * ssl;
	SSL_CTX * ctx;
private:
	int recv_ss(char ** buffer, int size);
	int send_ss(char * buffer, int size = -1);
public:
	HttpsClient(const char * hostname, unsigned int port = STD_HTTPS_PORT);
	HttpsClient(SOCKET sock, SSL ** ssl);
	int connectClient();
	int sendMessage(HttpFrame frame);
	int sendMessage(char * msg);
	int sendHtmlFile(HtmlFile file);
	int getMessage(HttpFrame & frame);
	void changeHost(char * hostname, unsigned short port = STD_HTTPS_PORT);
	SSL * getSSL() { return ssl; }
	void closeConnection();
};
class HttpsListener : public Listener {
private:
	SSL_CTX * ctx;
public:
	HttpsListener(unsigned short port = STD_HTTPS_PORT) : Listener(port) {};
	void loadCert(const char * key, const char * cert);
	HttpsClient accept(int & error);
	~HttpsListener() { SSL_CTX_free(ctx); }
};
#endif
