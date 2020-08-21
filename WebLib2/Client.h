#pragma once
#include "Streamable.h"
class Client
{
protected:
	std::unique_ptr<Streamable> stream;
public:
	void attach(std::unique_ptr<Streamable> && stream);
};
class Server
{
public:
	virtual std::unique_ptr<Client> accept() = 0;
};

