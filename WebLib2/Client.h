#pragma once
#include "Streamable.h"
class Client
{
protected:
	std::unique_ptr<Streamable> stream;
public:
	/**
	 * Attaches the client to the given Stream
	 * The stream becomes owned by the client
	 * @param stream stream to attach to
	 */
	void attach(std::unique_ptr<Streamable> && stream);
};

