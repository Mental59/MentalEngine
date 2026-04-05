#include <render/shaderCompiler.hpp>

#include <core/log.hpp>

#include <array>
#include <cstring>
#include <format>
#include <span>
#include <string_view>

#include <slang/slang-com-ptr.h>
#include <slang/slang.h>

namespace
{
using mental::render::ShaderCompiler;
using mental::render::ShaderCompileRequest;
using mental::render::ShaderCompileResult;
using mental::render::ShaderStage;

[[nodiscard]] SlangStage convertShaderStage(const ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::eVertex:
      return SLANG_STAGE_VERTEX;
    case ShaderStage::eFragment:
      return SLANG_STAGE_FRAGMENT;
  }

  return SLANG_STAGE_NONE;
}

void appendDiagnostics(std::string& destination, slang::IBlob* diagnosticsBlob)
{
  if (diagnosticsBlob == nullptr || diagnosticsBlob->getBufferPointer() == nullptr ||
      diagnosticsBlob->getBufferSize() == 0)
  {
    return;
  }

  if (!destination.empty() && destination.back() != '\n')
  {
    destination.push_back('\n');
  }

  const char* diagnosticsData = static_cast<const char*>(diagnosticsBlob->getBufferPointer());
  destination.append(diagnosticsData, diagnosticsBlob->getBufferSize());
}

[[nodiscard]] std::filesystem::path resolveShaderFilePath(const ShaderCompileRequest& request)
{
  if (request.shaderFilePath.is_absolute())
  {
    return request.shaderFilePath;
  }

  return request.shaderRootPath / request.shaderFilePath;
}

[[nodiscard]] ShaderCompileResult failCompile(const mental::core::Result result, std::string diagnostics)
{
  ShaderCompileResult compileResult {};
  compileResult.result = result;
  compileResult.diagnostics = std::move(diagnostics);
  return compileResult;
}
} // namespace

const char* mental::render::shaderStageToString(const ShaderStage stage)
{
  switch (stage)
  {
    case ShaderStage::eVertex:
      return "vertex";
    case ShaderStage::eFragment:
      return "fragment";
  }

  return "unknown";
}

bool mental::render::ShaderCompileResult::succeeded() const noexcept
{
  return result == core::Result::eSuccess && !spirvWords.empty();
}

std::filesystem::path mental::render::ShaderCompiler::getRuntimeShaderRoot()
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

ShaderCompileResult mental::render::ShaderCompiler::compileToSpirv(const ShaderCompileRequest& request) const
{
  const std::filesystem::path shaderRootPath = std::filesystem::weakly_canonical(request.shaderRootPath);
  const std::filesystem::path shaderFilePath = resolveShaderFilePath(request);
  const std::filesystem::path canonicalShaderFilePath = std::filesystem::weakly_canonical(shaderFilePath);
  if (shaderRootPath.empty() || canonicalShaderFilePath.empty())
  {
    return failCompile(core::Result::eOperationFailed, "Shader root and shader file path must both be valid");
  }

  if (!std::filesystem::exists(canonicalShaderFilePath))
  {
    return failCompile(core::Result::eOperationFailed,
      std::format("Shader source file does not exist: {}", canonicalShaderFilePath.string()));
  }

  Slang::ComPtr<slang::IGlobalSession> globalSession;
  if (SLANG_FAILED(slang::createGlobalSession(globalSession.writeRef())))
  {
    return failCompile(core::Result::eInitializationFailed, "Failed to create Slang global session");
  }

  const std::string shaderSearchPath = shaderRootPath.string();
  const char* searchPaths[] = {shaderSearchPath.c_str()};

  slang::TargetDesc targetDesc {};
  targetDesc.format = SLANG_SPIRV;
  targetDesc.profile = globalSession->findProfile("sm_6_5");
  targetDesc.forceGLSLScalarBufferLayout = true;

  slang::SessionDesc sessionDesc {};
  sessionDesc.targets = &targetDesc;
  sessionDesc.targetCount = 1;
  sessionDesc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;
  sessionDesc.searchPaths = searchPaths;
  sessionDesc.searchPathCount = 1;
  sessionDesc.skipSPIRVValidation = false;

  Slang::ComPtr<slang::ISession> session;
  if (SLANG_FAILED(globalSession->createSession(sessionDesc, session.writeRef())))
  {
    return failCompile(core::Result::eInitializationFailed,
      std::format("Failed to create Slang session for shader root {}", shaderRootPath.string()));
  }

  const std::string moduleName = canonicalShaderFilePath.stem().string();
  std::string diagnostics {};

  Slang::ComPtr<slang::IBlob> moduleDiagnostics;
  Slang::ComPtr<slang::IModule> module(session->loadModule(moduleName.c_str(), moduleDiagnostics.writeRef()));
  appendDiagnostics(diagnostics, moduleDiagnostics.get());
  if (!module)
  {
    return failCompile(core::Result::eOperationFailed,
      std::format(
        "Failed to load shader module '{}' from {}\n{}", moduleName, canonicalShaderFilePath.string(), diagnostics));
  }

  Slang::ComPtr<slang::IBlob> entryPointDiagnostics;
  Slang::ComPtr<slang::IEntryPoint> entryPoint;
  const SlangResult entryPointResult = module->findAndCheckEntryPoint(request.entryPointName.c_str(),
    convertShaderStage(request.stage),
    entryPoint.writeRef(),
    entryPointDiagnostics.writeRef());
  appendDiagnostics(diagnostics, entryPointDiagnostics.get());
  if (SLANG_FAILED(entryPointResult))
  {
    return failCompile(core::Result::eOperationFailed,
      std::format("Failed to load {} entry point '{}' from {}\n{}",
        shaderStageToString(request.stage),
        request.entryPointName,
        canonicalShaderFilePath.string(),
        diagnostics));
  }

  std::array<slang::IComponentType*, 2> componentTypes {
    module.get(),
    entryPoint.get(),
  };

  Slang::ComPtr<slang::IBlob> compositeDiagnostics;
  Slang::ComPtr<slang::IComponentType> compositeProgram;
  const SlangResult compositeResult = session->createCompositeComponentType(componentTypes.data(),
    static_cast<SlangInt>(componentTypes.size()),
    compositeProgram.writeRef(),
    compositeDiagnostics.writeRef());
  appendDiagnostics(diagnostics, compositeDiagnostics.get());
  if (SLANG_FAILED(compositeResult))
  {
    return failCompile(core::Result::eOperationFailed,
      std::format(
        "Failed to create composite shader program for {}\n{}", canonicalShaderFilePath.string(), diagnostics));
  }

  Slang::ComPtr<slang::IBlob> linkDiagnostics;
  Slang::ComPtr<slang::IComponentType> linkedProgram;
  const SlangResult linkResult = compositeProgram->link(linkedProgram.writeRef(), linkDiagnostics.writeRef());
  appendDiagnostics(diagnostics, linkDiagnostics.get());
  if (SLANG_FAILED(linkResult))
  {
    return failCompile(core::Result::eOperationFailed,
      std::format("Failed to link shader program for {}\n{}", canonicalShaderFilePath.string(), diagnostics));
  }

  Slang::ComPtr<slang::IBlob> codeDiagnostics;
  Slang::ComPtr<slang::IBlob> spirvCode;
  const SlangResult codeResult =
    linkedProgram->getEntryPointCode(0, 0, spirvCode.writeRef(), codeDiagnostics.writeRef());
  appendDiagnostics(diagnostics, codeDiagnostics.get());
  if (SLANG_FAILED(codeResult) || !spirvCode || spirvCode->getBufferSize() == 0u)
  {
    return failCompile(core::Result::eOperationFailed,
      std::format("Failed to generate SPIR-V for {} entry point '{}' from {}\n{}",
        shaderStageToString(request.stage),
        request.entryPointName,
        canonicalShaderFilePath.string(),
        diagnostics));
  }

  if ((spirvCode->getBufferSize() % sizeof(std::uint32_t)) != 0u)
  {
    return failCompile(core::Result::eOperationFailed,
      std::format("Generated SPIR-V blob for {} has an invalid byte size: {}",
        canonicalShaderFilePath.string(),
        spirvCode->getBufferSize()));
  }

  ShaderCompileResult compileResult {};
  compileResult.result = core::Result::eSuccess;
  compileResult.diagnostics = std::move(diagnostics);
  compileResult.spirvWords.resize(spirvCode->getBufferSize() / sizeof(std::uint32_t));
  std::memcpy(compileResult.spirvWords.data(), spirvCode->getBufferPointer(), spirvCode->getBufferSize());
  return compileResult;
}
