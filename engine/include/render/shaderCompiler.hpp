#pragma once

#include <core/types.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace mental::render
{
enum class ShaderStage : std::uint8_t
{
  eVertex = 0,
  eFragment,
};

[[nodiscard]] const char* shaderStageToString(ShaderStage stage);

struct ShaderCompileRequest
{
  std::filesystem::path shaderRootPath {};
  std::filesystem::path shaderFilePath {};
  std::string entryPointName {};
  ShaderStage stage = ShaderStage::eVertex;
};

struct ShaderCompileResult
{
  core::Result result = core::Result::eSuccess;
  std::vector<std::uint32_t> spirvWords {};
  std::string diagnostics {};

  [[nodiscard]] bool succeeded() const noexcept;
};

class ShaderCompiler
{
 public:
  [[nodiscard]] ShaderCompileResult compileToSpirv(const ShaderCompileRequest& request) const;

  [[nodiscard]] static std::filesystem::path getRuntimeShaderRoot();
};
} // namespace mental::render
