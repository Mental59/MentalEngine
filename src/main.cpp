#include <cstdio>
#include <vector>
#include "vulkanFramework/vulkanFramework.hpp"

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
	volkInitialize();

	VkInstance vulkanInstance = mental::createVulkanInstance();

#if defined(_DEBUG)
	VkDebugUtilsMessengerEXT messenger;
	VkDebugReportCallbackEXT reportCallback;
	mental::setupDebugCallbacks(vulkanInstance, &messenger, &reportCallback);
#endif	// (_DEBUG)

	glslang_initialize_process();

	testShaderCompilation("data/shaders/chapter03/VK01.vert", "VK01.vert.bin");
	testShaderCompilation("data/shaders/chapter03/VK01.frag", "VK01.frag.bin");

	glslang_finalize_process();

#if defined(_DEBUG)
	mental::destroyDebugCallbacks(vulkanInstance, messenger, reportCallback);
#endif
	mental::destroyVulkanInstance(vulkanInstance);

	return 0;
}
