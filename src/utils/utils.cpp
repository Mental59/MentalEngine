#include <malloc.h>
#include <string.h>
#include <string>

#include "utils.hpp"

void mental::printShaderSource(const char* text)
{
	int line = 1;

	printf("\n(%3i) ", line);

	while (text && *text++)
	{
		if (*text == '\n')
		{
			printf("\n(%3i) ", ++line);
		}
		else if (*text == '\r')
		{
		}
		else
		{
			printf("%c", *text);
		}
	}

	printf("\n");
}

bool mental::endsWith(const char* s, const char* part)
{
	const size_t sLength = strlen(s);
	const size_t partLength = strlen(part);
	if (sLength < partLength) return false;
	return strcmp(s + sLength - partLength, part) == 0;
}

std::string mental::readShaderFile(const char* fileName)
{
	FILE* file = fopen(fileName, "r");

	if (!file)
	{
		printf("I/O error. Cannot open shader file '%s'\n", fileName);
		return std::string();
	}

	fseek(file, 0L, SEEK_END);
	const auto bytesinfile = ftell(file);
	fseek(file, 0L, SEEK_SET);

	char* buffer = (char*)_malloca(bytesinfile + 1);
	if (!buffer)
	{
		return std::string();
	}
	const size_t bytesread = fread(buffer, 1, bytesinfile, file);
	fclose(file);

	buffer[bytesread] = 0;

	static constexpr unsigned char BOM[] = {0xEF, 0xBB, 0xBF};

	if (bytesread > 3)
	{
		if (!memcmp(buffer, BOM, 3)) memset(buffer, ' ', 3);
	}

	std::string code(buffer);

	while (code.find("#include ") != code.npos)
	{
		const auto pos = code.find("#include ");
		const auto p1 = code.find('<', pos);
		const auto p2 = code.find('>', pos);
		if (p1 == code.npos || p2 == code.npos || p2 <= p1)
		{
			printf("Error while loading shader program: %s\n", code.c_str());
			return std::string();
		}
		const std::string name = code.substr(p1 + 1, p2 - p1 - 1);
		const std::string include = readShaderFile(name.c_str());
		code.replace(pos, p2 - pos + 1, include.c_str());
	}

	return code;
}
