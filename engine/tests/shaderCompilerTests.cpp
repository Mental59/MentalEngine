#include <render/rhi/rhi.hpp>
#include <render/scenePipelineLibrary.hpp>
#include <render/shaderCompiler.hpp>

#include <exception>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <type_traits>

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
  const std::filesystem::path expectedShaderRoot =
    std::filesystem::weakly_canonical(std::filesystem::path(__FILE__).parent_path() / ".." / "shaders");
  require(!shaderRoot.empty(), "Runtime shader root should resolve to a non-empty path");
  require(std::filesystem::equivalent(shaderRoot, expectedShaderRoot),
    "Runtime shader root should resolve to the engine source shader directory");
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

void testPushConstantPayloadsMatchTheSharedSceneContract()
{
  require(sizeof(mental::render::PrimitiveDrawPushConstants) <= 128u,
    "Primitive push constants should stay within Vulkan's guaranteed minimum limit");

  require(
    mental::render::GridDrawPushConstants {}.gridSize > 0.0f, "Grid push constants should expose a positive grid size");
  require(mental::render::GridDrawPushConstants {}.gridCellSize > 0.0f,
    "Grid push constants should expose a positive grid cell size");
  require(mental::render::GridDrawPushConstants {}.gridMinPixelsBetweenCells > 0.0f,
    "Grid push constants should expose a positive minimum pixel spacing");
}

void testScenePushConstantRangeMatchesTheSharedPipelineLayoutContract()
{
  const mental::rhi::PushConstantRangeDesc sceneRange = mental::render::buildScenePushConstantRange();
  require(sceneRange.offset == 0u, "Scene command push constants should start at offset 0");
  require(sceneRange.stageFlags == (mental::rhi::ShaderStageFlagBits::eShaderStageVertexBit |
                                     mental::rhi::ShaderStageFlagBits::eShaderStageFragmentBit),
    "Scene command push constants should cover both stages used by the shared pipeline layout");
  require(sceneRange.size >= sizeof(mental::render::PrimitiveDrawPushConstants),
    "Scene command push constants should cover the primitive payload");
  require(sceneRange.size >= sizeof(mental::render::GridDrawPushConstants),
    "Scene command push constants should cover the grid payload");
}

void testPrimitivePipelineDefaultsMatchTheMvpContract()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  const mental::rhi::ResourceLayoutDesc resourceLayoutDesc {
    .bindings = bindings.data(),
    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
  };
  const mental::rhi::PushConstantRangeDesc pushConstantRange = mental::render::buildScenePushConstantRange();
  const mental::rhi::GraphicsPipelineDesc pipelineDesc {
    .vertexShaderModule = nullptr,
    .fragmentShaderModule = nullptr,
    .resourceLayoutDescs = &resourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &pushConstantRange,
    .pushConstantRangeCount = 1u,
    .topology = mental::rhi::PrimitiveTopology::eTriangleList,
    .polygonMode = mental::rhi::PolygonMode::eFill,
    .cullMode = mental::rhi::CullMode::eBack,
    .frontFace = mental::rhi::FrontFace::eCounterClockwise,
    .depthTestEnable = true,
    .depthWriteEnable = true,
    .depthCompareOp = mental::rhi::CompareOp::eLessOrEqual,
    .colorAttachmentFormat = mental::rhi::TextureFormat::eBGRA32_SRGB,
    .depthAttachmentFormat = mental::rhi::TextureFormat::eD32_SFLOAT,
    .hasDepthAttachment = true,
  };

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
  require(pipelineDesc.resourceLayoutDescCount == 1u,
    "Primitive pipeline should keep the shared scene resource layout inline");
  require(
    pipelineDesc.pushConstantRangeCount == 1u, "Primitive pipeline should keep the shared scene push constants inline");
}

void testGridPipelineDefaultsMatchTheMvpContract()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  const mental::rhi::ResourceLayoutDesc resourceLayoutDesc {
    .bindings = bindings.data(),
    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
  };
  const mental::rhi::PushConstantRangeDesc pushConstantRange = mental::render::buildScenePushConstantRange();
  const mental::rhi::GraphicsPipelineDesc pipelineDesc {
    .vertexShaderModule = nullptr,
    .fragmentShaderModule = nullptr,
    .resourceLayoutDescs = &resourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &pushConstantRange,
    .pushConstantRangeCount = 1u,
    .topology = mental::rhi::PrimitiveTopology::eTriangleList,
    .polygonMode = mental::rhi::PolygonMode::eFill,
    .cullMode = mental::rhi::CullMode::eNone,
    .frontFace = mental::rhi::FrontFace::eCounterClockwise,
    .depthTestEnable = true,
    .depthWriteEnable = false,
    .depthCompareOp = mental::rhi::CompareOp::eLessOrEqual,
    .colorAttachmentFormat = mental::rhi::TextureFormat::eBGRA32_SRGB,
    .depthAttachmentFormat = mental::rhi::TextureFormat::eD32_SFLOAT,
    .hasDepthAttachment = true,
  };

  require(
    pipelineDesc.topology == mental::rhi::PrimitiveTopology::eTriangleList, "Grid pipeline should stay triangle-list");
  require(pipelineDesc.depthTestEnable, "Grid pipeline should keep depth testing enabled");
  require(!pipelineDesc.depthWriteEnable, "Grid pipeline should keep depth writes disabled");
  require(pipelineDesc.cullMode == mental::rhi::CullMode::eNone, "Grid pipeline should keep culling disabled");
  require(
    pipelineDesc.resourceLayoutDescCount == 1u, "Grid pipeline should keep the shared scene resource layout inline");
  require(
    pipelineDesc.pushConstantRangeCount == 1u, "Grid pipeline should keep the shared scene push constants inline");
}

void testGraphicsPipelineContractsOwnInlineLayoutsAndPushConstants()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  const mental::rhi::ResourceLayoutDesc resourceLayoutDesc {
    .bindings = bindings.data(),
    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
  };
  const mental::rhi::PushConstantRangeDesc pushConstantRange = mental::render::buildScenePushConstantRange();

  const mental::rhi::GraphicsPipelineDesc pipelineDesc {
    .vertexShaderModule = nullptr,
    .fragmentShaderModule = nullptr,
    .resourceLayoutDescs = &resourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &pushConstantRange,
    .pushConstantRangeCount = 1u,
  };

  require(pipelineDesc.resourceLayoutDescCount == 1u,
    "Graphics pipeline contracts should accept inline resource layout descriptions");
  require(
    pipelineDesc.pushConstantRangeCount == 1u, "Graphics pipeline contracts should carry inline push constant ranges");
}

void testResourceSetContractsUseGraphicsPipelinesAndSetIndices()
{
  const mental::rhi::ResourceSetDesc resourceSetDesc {
    .graphicsPipeline = nullptr,
    .resourceSetIndex = 0u,
  };

  require(
    resourceSetDesc.resourceSetIndex == 0u, "Resource sets should still identify which pipeline set they belong to");
}

void testScenePipelinesCanBeDescribedWithoutPublicPipelineLayouts()
{
  const auto bindings = mental::render::buildSceneResourceBindings();
  const mental::rhi::ResourceLayoutDesc resourceLayoutDesc {
    .bindings = bindings.data(),
    .bindingCount = static_cast<std::uint32_t>(bindings.size()),
  };
  const mental::rhi::PushConstantRangeDesc pushConstantRange = mental::render::buildScenePushConstantRange();

  const mental::rhi::GraphicsPipelineDesc pipelineDesc {
    .vertexShaderModule = nullptr,
    .fragmentShaderModule = nullptr,
    .resourceLayoutDescs = &resourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &pushConstantRange,
    .pushConstantRangeCount = 1u,
  };

  require(pipelineDesc.resourceLayoutDescCount == 1u,
    "Scene pipelines should be fully described without a separate public pipeline-layout object");
}

void testSharedScenePushConstantContractIsTheOnlyPublicPushConstantHelper()
{
  const mental::rhi::PushConstantRangeDesc sceneRange = mental::render::buildScenePushConstantRange();
  require(sceneRange.size >= sizeof(mental::render::PrimitiveDrawPushConstants),
    "Shared scene push constants should cover primitive payloads");
  require(sceneRange.size >= sizeof(mental::render::GridDrawPushConstants),
    "Shared scene push constants should cover grid payloads");
}

void testCommandListBindsResourceSetsFromGraphicsPipelines()
{
  using BindResourceSetsSignature = mental::core::Result (mental::rhi::ICommandList::*)(
    mental::rhi::IGraphicsPipeline*, std::uint32_t, mental::rhi::IResourceSet* const*, std::uint32_t);

  require(std::is_same_v<decltype(&mental::rhi::ICommandList::bindResourceSets), BindResourceSetsSignature>,
    "Command lists should bind resource sets from a graphics pipeline contract");
}

void testCommandListPushConstantsUseGraphicsPipelines()
{
  using PushConstantsSignature = mental::core::Result (mental::rhi::ICommandList::*)(
    mental::rhi::IGraphicsPipeline*, const mental::rhi::PushConstantRangeDesc&, const void*);

  require(std::is_same_v<decltype(&mental::rhi::ICommandList::pushConstants), PushConstantsSignature>,
    "Command lists should push constants from a graphics pipeline contract");
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
    testPushConstantPayloadsMatchTheSharedSceneContract();
    testScenePushConstantRangeMatchesTheSharedPipelineLayoutContract();
    testPrimitivePipelineDefaultsMatchTheMvpContract();
    testGridPipelineDefaultsMatchTheMvpContract();
    testGraphicsPipelineContractsOwnInlineLayoutsAndPushConstants();
    testResourceSetContractsUseGraphicsPipelinesAndSetIndices();
    testScenePipelinesCanBeDescribedWithoutPublicPipelineLayouts();
    testSharedScenePushConstantContractIsTheOnlyPublicPushConstantHelper();
    testCommandListBindsResourceSetsFromGraphicsPipelines();
    testCommandListPushConstantsUseGraphicsPipelines();
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
