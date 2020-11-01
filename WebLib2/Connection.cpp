#include "Connection.h"
#include <ws2tcpip.h>
FD Connection::readFd(FDType::read);
RAIIContext<Address::winsockinit, Address::winsockdeinit> Address::ctx;
Address::Address(sockaddr_in data) : address(data)
{
}

Address::Address(port_t port) 
{
	this->address.sin_family = AF_INET;
	this->address.sin_port = htons(port);
	this->address.sin_addr.s_addr = INADDR_ANY;
}

Address::Address(in_addr address, port_t port)
{
	this->address.sin_family = AF_INET;
	this->address.sin_port = htons(port);
	this->address.sin_addr = address;
}

Address::Address(const char* str)
{
	if (isalpha(str[0])) {
		hostent* host = gethostbyname(str);
		address.sin_addr = *((in_addr*)*host->h_addr_list);
	}
	else {
		inet_pton(AF_INET, str, (void*)&address);
	}
}

Address::Address(const char* str, port_t port) : Address(str)
{
	address.sin_port = htons(port);
	address.sin_family = AF_INET;
}

std::wstring Address::toString() const
{
	std::wstring str;
	str.resize(20);
	InetNtop(AF_INET, &address, str.data(), str.size());
	return str;
}
Connection::Connection(Address& addr, FDMethod method) : addr(addr), method(method)
{
	sock = ::socket(AF_INET, (int)method, 0);
}
Connection::Connection(Address&& addr, FDMethod method) : addr(addr), method(method)
{
	sock = ::socket(AF_INET, (int)method, 0);
}
Connection::~Connection()
{
	readFd.detach(this);
	close();
}
Connection::Connection(Connection&& o) : sock(o.sock), addr(o.addr)
{
	o.sock = INVALID_SOCKET;
}
Connection& Connection::operator=(Connection&& o)
{
	close();
	sock = o.sock;
	addr = o.addr;
	o.sock = INVALID_SOCKET;
	return *this;
}
Address Connection::peername() const
{
	Address ad;
	int sz = sizeof(sockaddr_in);
	sockaddr_in data;
	getpeername(sock, (sockaddr*)&ad.address, &sz);
	return ad;
}
//RAIIContext<winsockinit, winsockdeinit> ctx;
Connection::Connection() : sock(INVALID_SOCKET), addr(sockaddr_in()), onclose(nullptr), method(FDMethod::TCP)
{
	readFd.attach(this);
}

void Address::winsockinit()
{
	WSAData d{};
	WSAStartup(MAKEWORD(2, 1), &d);
	printf("Wininit\n");
}
void Address::winsockdeinit()
{
	WSACleanup();
	printf("Win deinit\n");
}