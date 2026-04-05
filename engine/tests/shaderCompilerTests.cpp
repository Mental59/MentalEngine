#include <render/rhi/rhi.hpp>
#include <render/scenePipelineLibrary.hpp>
#include <render/shaderCompiler.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

void requireCompiledShader(const std::filesystem::path& shaderRoot,
  const std::filesystem::path& shaderFilePath,
  const char* entryPointName,
  mental::render::ShaderStage stage)
{
  mental::render::ShaderCompiler compiler {};
  const mental::render::ShaderCompileResult compileResult = compiler.compileToSpirv({
    .shaderRootPath = shaderRoot,
    .shaderFilePath = shaderFilePath,
    .entryPointName = entryPointName,
    .stage = stage,
  });

  require(compileResult.result == mental::core::Result::eSuccess, compileResult.diagnostics.c_str());
  require(!compileResult.spirvWords.empty(), "Compiled shader should produce a non-empty SPIR-V blob");
}

void testRuntimeShaderRootContainsExpectedShaders()
{
  const std::filesystem::path shaderRoot = mental::render::ShaderCompiler::getRuntimeShaderRoot();
  require(!shaderRoot.empty(), "Runtime shader root should resolve to a non-empty path");
  require(std::filesystem::exists(shaderRoot / "primitiveScene.slang"),
    "Runtime shader root should contain primitiveScene.slang");
  require(
    std::filesystem::exists(shaderRoot / "editorGrid.slang"), "Runtime shader root should contain editorGrid.slang");
}

void testPrimitiveSceneShaderEntryPointsCompile()
{
  const std::filesystem::path shaderRoot = mental::render::ShaderCompiler::getRuntimeShaderRoot();
  requireCompiledShader(shaderRoot, "primitiveScene.slang", "vertexMain", mental::render::ShaderStage::eVertex);
  requireCompiledShader(shaderRoot, "primitiveScene.slang", "fragmentMain", mental::render::ShaderStage::eFragment);
}

void testEditorGridShaderEntryPointsCompile()
{
  const std::filesystem::path shaderRoot = mental::render::ShaderCompiler::getRuntimeShaderRoot();
  requireCompiledShader(shaderRoot, "editorGrid.slang", "vertexMain", mental::render::ShaderStage::eVertex);
  requireCompiledShader(shaderRoot, "editorGrid.slang", "fragmentMain", mental::render::ShaderStage::eFragment);
}

void testSceneResourceLayoutBindingsMatchTheSceneContract()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  require(bindings.size() == 2u, "Scene resource layout should expose exactly two bindings");

  require(bindings[0].binding == 0u, "Camera binding should stay at binding 0");
  require(bindings[0].type == mental::rhi::ResourceBindingType::eUniformBuffer,
    "Camera binding should stay a uniform buffer");
  require(bindings[0].stageFlags == (mental::rhi::ShaderStageFlagBits::eShaderStageVertexBit |
                                      mental::rhi::ShaderStageFlagBits::eShaderStageFragmentBit),
    "Camera binding should be visible to both shader stages");

  require(bindings[1].binding == 1u, "Primitive geometry binding should stay at binding 1");
  require(bindings[1].type == mental::rhi::ResourceBindingType::eStorageBuffer,
    "Primitive geometry binding should stay a storage buffer");
  require(bindings[1].stageFlags == mental::rhi::ShaderStageFlagBits::eShaderStageVertexBit,
    "Primitive geometry binding should stay vertex-only for programmable vertex pulling");
}

void testPushConstantRangesMatchThePrimitiveAndGridContracts()
{
  const mental::rhi::PushConstantRangeDesc primitiveRange = mental::render::buildPrimitivePushConstantRange();
  require(primitiveRange.offset == 0u, "Primitive push constants should start at offset 0");
  require(primitiveRange.stageFlags == mental::rhi::ShaderStageFlagBits::eShaderStageVertexBit,
    "Primitive push constants should stay vertex-only");
  require(primitiveRange.size == sizeof(mental::render::PrimitiveDrawPushConstants),
    "Primitive push constant range should match PrimitiveDrawPushConstants");
  require(sizeof(mental::render::PrimitiveDrawPushConstants) <= 128u,
    "Primitive push constants should stay within Vulkan's guaranteed minimum limit");

  const mental::rhi::PushConstantRangeDesc gridRange = mental::render::buildGridPushConstantRange();
  require(gridRange.offset == 0u, "Grid push constants should start at offset 0");
  require(gridRange.stageFlags == (mental::rhi::ShaderStageFlagBits::eShaderStageVertexBit |
                                    mental::rhi::ShaderStageFlagBits::eShaderStageFragmentBit),
    "Grid push constants should stay visible to both stages");
  require(gridRange.size == sizeof(mental::render::GridDrawPushConstants),
    "Grid push constant range should match GridDrawPushConstants");
  require(
    mental::render::GridDrawPushConstants {}.gridSize > 0.0f, "Grid push constants should expose a positive grid size");
  require(mental::render::GridDrawPushConstants {}.gridCellSize > 0.0f,
    "Grid push constants should expose a positive grid cell size");
  require(mental::render::GridDrawPushConstants {}.gridMinPixelsBetweenCells > 0.0f,
    "Grid push constants should expose a positive minimum pixel spacing");
}

void testPrimitivePipelineDefaultsMatchTheMvpContract()
{
  const mental::rhi::GraphicsPipelineDesc pipelineDesc = mental::render::buildPrimitiveGraphicsPipelineDesc(
    nullptr, nullptr, nullptr, mental::rhi::TextureFormat::eBGRA32_SRGB, mental::rhi::TextureFormat::eD32_SFLOAT);

  require(pipelineDesc.topology == mental::rhi::PrimitiveTopology::eTriangleList,
    "Primitive pipeline should stay triangle-list");
  require(pipelineDesc.polygonMode == mental::rhi::PolygonMode::eFill, "Primitive pipeline should stay solid fill");
  require(pipelineDesc.cullMode == mental::rhi::CullMode::eBack, "Primitive pipeline should stay back-face culled");
  require(pipelineDesc.depthTestEnable, "Primitive pipeline should keep depth testing enabled");
  require(pipelineDesc.depthWriteEnable, "Primitive pipeline should keep depth writes enabled");
  require(pipelineDesc.depthCompareOp == mental::rhi::CompareOp::eLessOrEqual,
    "Primitive pipeline should use a less-or-equal depth compare");
  require(pipelineDesc.colorAttachmentFormat == mental::rhi::TextureFormat::eBGRA32_SRGB,
    "Primitive pipeline should preserve the requested color format");
  require(pipelineDesc.depthAttachmentFormat == mental::rhi::TextureFormat::eD32_SFLOAT,
    "Primitive pipeline should preserve the requested depth format");
}

void testGridPipelineDefaultsMatchTheMvpContract()
{
  const mental::rhi::GraphicsPipelineDesc pipelineDesc = mental::render::buildGridGraphicsPipelineDesc(
    nullptr, nullptr, nullptr, mental::rhi::TextureFormat::eBGRA32_SRGB, mental::rhi::TextureFormat::eD32_SFLOAT);

  require(
    pipelineDesc.topology == mental::rhi::PrimitiveTopology::eTriangleList, "Grid pipeline should stay triangle-list");
  require(pipelineDesc.depthTestEnable, "Grid pipeline should keep depth testing enabled");
  require(!pipelineDesc.depthWriteEnable, "Grid pipeline should keep depth writes disabled");
  require(pipelineDesc.cullMode == mental::rhi::CullMode::eNone, "Grid pipeline should keep culling disabled");
}
} // namespace

int main()
{
  try
  {
    testRuntimeShaderRootContainsExpectedShaders();
    testPrimitiveSceneShaderEntryPointsCompile();
    testEditorGridShaderEntryPointsCompile();
    testSceneResourceLayoutBindingsMatchTheSceneContract();
    testPushConstantRangesMatchThePrimitiveAndGridContracts();
    testPrimitivePipelineDefaultsMatchTheMvpContract();
    testGridPipelineDefaultsMatchTheMvpContract();
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unknown exception\n";
  }

  return 1;
}
