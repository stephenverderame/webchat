#pragma once
#include <unordered_map>
#include <vector>
#include "Streamable.h"
#include "StreamView.h"
struct Streamable::impl {
	std::vector<char> obuffer;
	std::shared_ptr<std::vector<char>> ibuffer;
	std::streamsize iend = 0; //current input pos pointer, end pointer (off the back of the buffer)
	std::streamsize o = 0, maxOutputSize = 2048; //o is off the back of the buffer as well
	bool filled = false;
	impl()
	{
		ibuffer = std::make_shared<std::vector<char>>(4096);
	}

};