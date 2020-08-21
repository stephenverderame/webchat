#include "Client.h"

void Client::attach(std::unique_ptr<Streamable> && stream)
{
	this->stream = std::move(stream);
}
