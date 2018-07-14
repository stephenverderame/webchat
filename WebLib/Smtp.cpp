#include "Smtp.h"

SmtpClient::SmtpClient(char * hostName, unsigned short port)
{
	sock = socket(AF_INET, SOCK_STREAM, NULL);
	hostent * server = gethostbyname(hostName);
	if (!server) printf("Cannot find host! \n");
	addrData.sin_port = htons(port);
	addrData.sin_family = AF_INET;
	addrData.sin_addr = *((LPIN_ADDR)*server->h_addr_list);
	host = hostName;
}

void SmtpClient::login(char * user, char * password)
{
	this->user = base64_encode(user);
	this->password = base64_encode(password);
}

int SmtpClient::sendMessage(char * recp, char * subject, char * message)
{
	char buffer[4096];
	char msg[10000];
	SSL * ssl;
	SSL_CTX * ctx;
	const SSL_METHOD * method = SSLv23_client_method();
	ctx = SSL_CTX_new(method);
	int ret = connect(sock, (PSOCKADDR)&addrData, sizeof(addrData));
	if (ret != 0) return -1;
	if (recv(sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) return -2;
	sprintf_s(msg, 10000, "EHLO %s\r\n", host.c_str());
	ret = send(sock, msg, strlen(msg), 0);
	if (ret == SOCKET_ERROR) return -3;
	if(recv(sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) return -5;
	sprintf_s(msg, 10000, "STARTTLS \r\n", "");
	if (send(sock, msg, strlen(msg), 0) == SOCKET_ERROR) return -4;
	if (recv(sock, buffer, sizeof(buffer), 0) == SOCKET_ERROR) return -6;

	ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sock);
	if(SSL_connect(ssl) <= 0) return -10;

	sprintf_s(msg, 10000, "EHLO %s\r\n", host.c_str());
	if(SSL_write(ssl, msg, strlen(msg)) <= 0) return -11;
	if(SSL_read(ssl, buffer, sizeof(buffer)) <= 0) return -12;

	sprintf_s(msg, 10000, "AUTH LOGIN\r\n", "");
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "%s \r\n", user.c_str());
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "%s \r\n", password.c_str());
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "MAIL FROM:<%s> \r\n", base64_decode(user).c_str());
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "RCPT TO:<%s> \r\n", recp);
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "DATA \r\n", "");
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "Subject:%s \r\n\r\n", subject);
	SSL_write(ssl, msg, strlen(msg));
	sprintf_s(msg, 10000, "%s \r\n", message);
	long long dataSent = 0;
	while (dataSent < strlen(message)) {
		long long ret = SSL_write(ssl, msg + dataSent, strlen(msg) - dataSent);
		if (ret <= 0) return -60;
		dataSent += ret;
	}
	sprintf_s(msg, 10000, "\r\n.\r\n", "");
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	sprintf_s(msg, 10000, "QUIT\r\n", "");
	SSL_write(ssl, msg, strlen(msg));
	SSL_read(ssl, buffer, sizeof(buffer));

	SSL_free(ssl);
	SSL_CTX_free(ctx);
	closesocket(sock);

	return 0;


}
