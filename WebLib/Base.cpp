#include "Base.h"

int Socket::send_s(char * msg, num_t size)
{
	if (size == -1) size = strlen(msg);
	num_t dataSent = 0;
	while (dataSent < size) {
		num_t ret = send(sock, msg + dataSent, size - dataSent, NULL);
		if (ret == SOCKET_ERROR)
			return SOCKET_ERROR;
		dataSent += ret;
	}
	return 0;
}

int Socket::recv_s(char ** msg, num_t size)
{
	num_t dataGot = 0;
	while (dataGot < size) {
		num_t ret = recv(sock, *msg + dataGot, size - dataGot, NULL);
		if (ret == SOCKET_ERROR)
			return SOCKET_ERROR;
		dataGot += ret;
	}
	return 0;
}

void Socket::addToFD(FD_SET * fd)
{
	FD_SET(sock, fd);
}

bool Socket::isInFD(FD_SET * fd)
{
	return FD_ISSET(sock, fd);
}

void Socket::nagle(int enable)
{
#ifdef _WIN32
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char*)&enable, sizeof(enable));
#endif
}

void Socket::nonblocking(int enable)
{
#ifdef _WIN32
	ioctlsocket(sock, FIONBIO, (u_long*)&enable);
#else
	int flags = fcntl(sock, F_GETFL, 0);
	fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

int Socket::getErrorCode()
{
	int errCode;
#ifdef _WIN32
	int size = 4;
#else
	unsigned int size = 4;
#endif
	getsockopt(sock, SOL_SOCKET, SO_ERROR, (char*)&errCode, &size);
	return errCode;
}

std::string Socket::getIpAddress()
{
	SOCKADDR_IN address;
	address = addrData;
#ifdef _WIN32
	int length = sizeof(address);
#else
	unsigned int length = sizeof(address);
#endif
	getpeername(sock, (SOCKADDR*)&address, &length);
	return std::string(inet_ntoa(address.sin_addr));
}

void Socket::closeConnection()
{
	closesocket(sock);
}
#ifdef _WIN32
Winsock::~Winsock()
{
	WSACleanup();
}
Winsock::Winsock() {
	WSAData data;
	WSAStartup(MAKEWORD(2, 1), &data);
}
#endif //_WIN32
Client::Client(SOCKET s)
{
	sock = s;
}

void Client::closeConnection()
{
	closed = true;
	closesocket(sock);
}

void FD::wait(FD * read, FD * write, FD * except)
{
	FD_SET * readfd = read == nullptr ? NULL : &read->set;
	FD_SET * writefd = write == nullptr ? NULL : &write->set;
	FD_SET * exceptfd = except == nullptr ? NULL : &except->set;
	select(0, readfd, writefd, exceptfd, NULL);
}

void FD::waitUntil(timeval * t, FD * read, FD * write, FD * except)
{
	FD_SET * readfd = read == nullptr ? NULL : &read->set;
	FD_SET * writefd = write == nullptr ? NULL : &write->set;
	FD_SET * exceptfd = except == nullptr ? NULL : &except->set;
	select(0, readfd, writefd, exceptfd, t);
}

Listener::Listener(unsigned short port)
{
	sock = socket(AF_INET, SOCK_STREAM, NULL);
	addrData.sin_family = AF_INET;
	addrData.sin_port = htons(port);
	addrData.sin_addr.s_addr = INADDR_ANY;
	bind(sock, (SOCKADDR*)&addrData, sizeof(addrData));
	listen(sock, SOMAXCONN);
}
#ifndef _WIN32
long long ntohll(long long a){
	long long ret;
	unsigned char * out = (unsigned char *)&ret;
	out[0] = a >> 56;
	out[1] = a >> 48;
	out[2] = a >> 40;
	out[3] = a >> 32;
	out[4] = a >> 24;
	out[5] = a >> 16;
	out[6] = a >> 8;
	out[7] = a;
	return ret;
	
}
int fopen_s(FILE ** stream, const char * filename, const char * mode){
	*stream = fopen(filename, mode);
	if(*stream != NULL) return 0;
	return 1;
}
#endif
