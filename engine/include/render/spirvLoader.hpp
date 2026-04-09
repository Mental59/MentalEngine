#pragma once

#include <core/types.hpp>

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace mental::render
{
struct SpirvLoadResult
{
  core::Result result = core::Result::eSuccess;
  std::vector<std::uint32_t> spirvWords {};
  std::string diagnostics {};

  [[nodiscard]] bool succeeded() const noexcept;
};

[[nodiscard]] SpirvLoadResult loadSpirvFile(const std::filesystem::path& filePath);
[[nodiscard]] std::filesystem::path getRuntimeShaderRoot();
} // namespace mental::render
