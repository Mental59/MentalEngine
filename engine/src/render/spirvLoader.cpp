#include <render/spirvLoader.hpp>

#include <core/log.hpp>

#include <filesystem>
#include <format>
#include <fstream>

bool mental::render::SpirvLoadResult::succeeded() const noexcept
{
  return result == core::Result::eSuccess && !spirvWords.empty();
}

std::filesystem::path mental::render::getRuntimeShaderRoot()
{
#if !defined(MENTAL_RUNTIME_SHADER_ROOT)
  MENTAL_ERROR("Missing configured runtime shader root path");
  return {};
#else
  const std::filesystem::path shaderRoot = std::filesystem::path(MENTAL_RUNTIME_SHADER_ROOT);
  if (shaderRoot.empty())
  {
    MENTAL_ERROR("Configured runtime shader root path is empty");
    return {};
  }

  return shaderRoot;
#endif
}

mental::render::SpirvLoadResult mental::render::loadSpirvFile(const std::filesystem::path& filePath)
{
  SpirvLoadResult result {};
  const std::filesystem::path canonicalPath = std::filesystem::weakly_canonical(filePath);
  if (canonicalPath.empty() || !std::filesystem::exists(canonicalPath))
  {
    result.result = core::Result::eOperationFailed;
    result.diagnostics = std::format("SPIR-V file does not exist: {}", filePath.string());
    return result;
  }

  std::ifstream stream(canonicalPath, std::ios::binary | std::ios::ate);
  if (!stream.is_open())
  {
    result.result = core::Result::eOperationFailed;
    result.diagnostics = std::format("Failed to open SPIR-V file: {}", canonicalPath.string());
    return result;
  }

  const std::streamsize byteSize = stream.tellg();
  if (byteSize <= 0 || (byteSize % static_cast<std::streamsize>(sizeof(std::uint32_t))) != 0)
  {
    result.result = core::Result::eOperationFailed;
    result.diagnostics = std::format("Invalid SPIR-V byte size for {}", canonicalPath.string());
    return result;
  }

  result.spirvWords.resize(static_cast<std::size_t>(byteSize) / sizeof(std::uint32_t));
  stream.seekg(0, std::ios::beg);
  stream.read(reinterpret_cast<char*>(result.spirvWords.data()), byteSize);
  if (!stream)
  {
    result.result = core::Result::eOperationFailed;
    result.diagnostics = std::format("Failed to read SPIR-V file: {}", canonicalPath.string());
    result.spirvWords.clear();
    return result;
  }

  MENTAL_INFO("Loaded SPIR-V shader '{}' ({} words)", canonicalPath.string(), result.spirvWords.size());
  return result;
}
