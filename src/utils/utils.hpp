#pragma once
#include <string>

namespace mental
{
	bool endsWith(const char* s, const char* part);

	std::string readShaderFile(const char* fileName);

	void printShaderSource(const char* text);
}
