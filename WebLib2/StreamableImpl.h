#pragma once
#include <unordered_map>
#include <vector>
#include "Streamable.h"
#include "StreamView.h"
struct Streamable::impl {
	std::vector<char> obuffer; ///< buffer to write to
	std::shared_ptr<std::vector<char>> ibuffer; ///< buffer to read from
	std::streamsize iend = 0; ///< current input pos pointer, end pointer (off the back of the buffer)
	std::streamsize o = 0, ///< o is off the back of the buffer as well
		maxOutputSize = 2048; 
	bool filled = false;
	impl()
	{
		ibuffer = std::make_shared<std::vector<char>>(4096);
	}

};