#pragma once

#include <glslang/Include/glslang_c_interface.h>
#include <vector>
#include <volk.h>

namespace mental {
struct ShaderModule final {
  std::vector<unsigned int> SPIRV;
  VkShaderModule shaderModule = VK_NULL_HANDLE;
  VkShaderStageFlagBits stage;
};

glslang_stage_t glslangShaderStageFromFileName(const char* fileName);

VkShaderStageFlagBits glslangShaderStageToVulkan(glslang_stage_t sh);

VkShaderStageFlagBits vulkanShaderStageFromFileName(const char* fileName);

size_t compileShaderFile(const char* file, ShaderModule& shaderModule);

VkResult createShaderModule(VkDevice device, ShaderModule* shader,
                            const char* fileName);
} // namespace mental
