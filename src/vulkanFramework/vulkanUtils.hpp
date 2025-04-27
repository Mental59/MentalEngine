#pragma once
#include <glslang/Include/glslang_c_interface.h>
#include <vector>
#include <volk.h>

#define MENTAL_VK_CHECK(value) check(value == VK_SUCCESS, __FILE__, __LINE__);
#define MENTAL_VK_CHECK_RET(value)                                             \
  if (value != VK_SUCCESS) {                                                   \
    check(false, __FILE__, __LINE__);                                          \
    return value;                                                              \
  }
#define MENTAL_BL_CHECK(value) check(value, __FILE__, __LINE__);

namespace mental {
struct ShaderModule final {
  std::vector<unsigned int> SPIRV;
  VkShaderModule shaderModule = nullptr;
};

glslang_stage_t glslangShaderStageFromFileName(const char* fileName);

size_t compileShaderFile(const char* file, ShaderModule& shaderModule);

void check(bool check, const char* fileName, int lineNumber);
} // namespace mental
