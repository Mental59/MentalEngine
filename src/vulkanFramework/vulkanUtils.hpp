#pragma once
#include <vector>
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <glslang/Include/glslang_c_interface.h>

namespace mental
{
#define VK_CHECK(value) check(value == VK_SUCCESS, __FILE__, __LINE__);
#define VK_CHECK_RET(value) if ( value != VK_SUCCESS ) { check(false, __FILE__, __LINE__); return value; }
#define BL_CHECK(value) check(value, __FILE__, __LINE__);

	struct ShaderModule final
	{
		std::vector<unsigned int> SPIRV;
		VkShaderModule shaderModule = nullptr;
	};

	glslang_stage_t glslangShaderStageFromFileName(const char* fileName);

	size_t compileShaderFile(const char* file, ShaderModule& shaderModule);

	void check(bool check, const char* fileName, int lineNumber);
}
