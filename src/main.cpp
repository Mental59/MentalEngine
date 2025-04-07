#define VK_NO_PROTOTYPES
#define GLFW_INCLUDE_VULKAN

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "utils/utils.hpp"
#include "vulkanFramework/vulkanUtils.hpp"

void saveSPIRVBinaryFile(const char* filename, unsigned int* code, size_t size)
{
	FILE* f = fopen(filename, "w");

	if (!f) return;

	fwrite(code, sizeof(uint32_t), size, f);

	fclose(f);
}

void testShaderCompilation(const char* sourceFilename, const char* destFilename)
{
	mental::ShaderModule shaderModule;

	if (mental::compileShaderFile(sourceFilename, shaderModule) < 1) return;

	saveSPIRVBinaryFile(destFilename, shaderModule.SPIRV.data(), shaderModule.SPIRV.size());
}

int main()
{
	glslang_initialize_process();

	testShaderCompilation("data/shaders/chapter03/VK01.vert", "VK01.vert.bin");
	testShaderCompilation("data/shaders/chapter03/VK01.frag", "VK01.frag.bin");

	glslang_finalize_process();

	return 0;
}
