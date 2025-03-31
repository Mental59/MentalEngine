#pragma once
#include <vector>
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <glslang/Include/glslang_c_interface.h>

namespace mental
{
	struct ShaderModule final
	{
		std::vector<unsigned int> SPIRV;
		VkShaderModule shaderModule = nullptr;
	};

	glslang_stage_t glslangShaderStageFromFileName(const char* fileName);

	size_t compileShaderFile(const char* file, ShaderModule& shaderModule);
}
