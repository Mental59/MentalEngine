#pragma once
#include <glslang/Include/glslang_c_interface.h>
#include <vector>
#include <volk.h>

#define MENTAL_VK_CHECK(value)                                                 \
  mental::check(value == VK_SUCCESS, __FILE__, __LINE__);
#define MENTAL_VK_CHECK_BOOL(value) mental::check(value, __FILE__, __LINE__);

namespace mental {
struct ShaderModule final {
  std::vector<unsigned int> SPIRV;
  VkShaderModule shaderModule = nullptr;
};

glslang_stage_t glslangShaderStageFromFileName(const char* fileName);

size_t compileShaderFile(const char* file, ShaderModule& shaderModule);

void check(bool check, const char* fileName, int lineNumber);

VkResult createSemaphore(VkDevice device, VkSemaphore* outSemaphore);

bool hasStencilComponent(VkFormat format);

uint32_t bytesPerTexFormat(VkFormat fmt);
} // namespace mental
