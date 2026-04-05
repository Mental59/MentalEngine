#include <render/scenePipelineLibrary.hpp>

#include <core/log.hpp>
#include <render/shaderCompiler.hpp>

#include <algorithm>
#include <format>
#include <limits>

namespace
{
constexpr std::uint32_t kSceneResourceSetIndex = 0u;

[[nodiscard]] std::filesystem::path resolveShaderRootPath(const mental::render::ScenePipelineLibraryConfig& config)
{
  if (!config.shaderRootPath.empty())
  {
    return config.shaderRootPath;
  }

  return mental::render::ShaderCompiler::getRuntimeShaderRoot();
}

[[nodiscard]] mental::core::Result compileShaderModule(mental::rhi::IShaderModule& shaderModule,
  mental::render::ShaderCompiler& compiler,
  const std::filesystem::path& shaderRootPath,
  const std::filesystem::path& shaderFilePath,
  const char* entryPointName,
  mental::render::ShaderStage stage)
{
  const mental::render::ShaderCompileResult compileResult = compiler.compileToSpirv({
    .shaderRootPath = shaderRootPath,
    .shaderFilePath = shaderFilePath,
    .entryPointName = entryPointName,
    .stage = stage,
  });
  if (!compileResult.succeeded())
  {
    MENTAL_ERROR(
      "Failed to compile shader {}:{}\n{}", shaderFilePath.string(), entryPointName, compileResult.diagnostics);
    return mental::core::Result::eInitializationFailed;
  }

  return shaderModule.init({
    .stage = stage == mental::render::ShaderStage::eVertex ? mental::rhi::ShaderStage::eVertex
                                                           : mental::rhi::ShaderStage::eFragment,
    .spirvCode = compileResult.spirvWords.data(),
    .wordCount = static_cast<std::uint64_t>(compileResult.spirvWords.size()),
    .entryPointName = entryPointName,
    .debugName = shaderFilePath.string(),
  });
}
} // namespace

std::array<mental::rhi::ResourceBindingDesc, 2> mental::render::buildSceneResourceBindings()
{
  return {
    rhi::ResourceBindingDesc {
                              .binding = 0u,
                              .type = rhi::ResourceBindingType::eUniformBuffer,
                              .descriptorCount = 1u,
                              .stageFlags = rhi::ShaderStageFlagBits::eShaderStageVertexBit | rhi::ShaderStageFlagBits::eShaderStageFragmentBit,
                              },
    rhi::ResourceBindingDesc {
                              .binding = 1u,
                              .type = rhi::ResourceBindingType::eStorageBuffer,
                              .descriptorCount = 1u,
                              .stageFlags = rhi::ShaderStageFlagBits::eShaderStageVertexBit,
                              },
  };
}

mental::rhi::PushConstantRangeDesc mental::render::buildScenePushConstantRange()
{
  return {
    .stageFlags = rhi::ShaderStageFlagBits::eShaderStageVertexBit | rhi::ShaderStageFlagBits::eShaderStageFragmentBit,
    .offset = 0u,
    .size = static_cast<std::uint32_t>(std::max(sizeof(PrimitiveDrawPushConstants), sizeof(GridDrawPushConstants))),
  };
}

mental::rhi::GraphicsPipelineDesc mental::render::buildPrimitiveGraphicsPipelineDesc(
  rhi::IPipelineLayout* pipelineLayout,
  rhi::IShaderModule* vertexShaderModule,
  rhi::IShaderModule* fragmentShaderModule,
  rhi::TextureFormat colorAttachmentFormat,
  rhi::TextureFormat depthAttachmentFormat)
{
  return {
    .vertexShaderModule = vertexShaderModule,
    .fragmentShaderModule = fragmentShaderModule,
    .pipelineLayout = pipelineLayout,
    .topology = rhi::PrimitiveTopology::eTriangleList,
    .polygonMode = rhi::PolygonMode::eFill,
    .cullMode = rhi::CullMode::eBack,
    .frontFace = rhi::FrontFace::eCounterClockwise,
    .depthTestEnable = true,
    .depthWriteEnable = true,
    .depthCompareOp = rhi::CompareOp::eLessOrEqual,
    .colorAttachmentFormat = colorAttachmentFormat,
    .depthAttachmentFormat = depthAttachmentFormat,
    .hasDepthAttachment = true,
  };
}

mental::rhi::GraphicsPipelineDesc mental::render::buildGridGraphicsPipelineDesc(rhi::IPipelineLayout* pipelineLayout,
  rhi::IShaderModule* vertexShaderModule,
  rhi::IShaderModule* fragmentShaderModule,
  rhi::TextureFormat colorAttachmentFormat,
  rhi::TextureFormat depthAttachmentFormat)
{
  return {
    .vertexShaderModule = vertexShaderModule,
    .fragmentShaderModule = fragmentShaderModule,
    .pipelineLayout = pipelineLayout,
    .topology = rhi::PrimitiveTopology::eTriangleList,
    .polygonMode = rhi::PolygonMode::eFill,
    .cullMode = rhi::CullMode::eNone,
    .frontFace = rhi::FrontFace::eCounterClockwise,
    .depthTestEnable = true,
    .depthWriteEnable = false,
    .depthCompareOp = rhi::CompareOp::eLessOrEqual,
    .colorAttachmentFormat = colorAttachmentFormat,
    .depthAttachmentFormat = depthAttachmentFormat,
    .hasDepthAttachment = true,
  };
}

mental::core::Result mental::render::ScenePipelineLibrary::init(const ScenePipelineLibraryConfig& config)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized ScenePipelineLibrary");
    return core::Result::eInitializationFailed;
  }

  if (config.framesInFlight == 0u || config.geometryStorageBuffer == nullptr)
  {
    MENTAL_ERROR("ScenePipelineLibrary init requires frames in flight and a geometry storage buffer");
    return core::Result::eInitializationFailed;
  }

  mConfig = config;
  const std::filesystem::path shaderRootPath = resolveShaderRootPath(config);
  if (shaderRootPath.empty())
  {
    MENTAL_ERROR("ScenePipelineLibrary failed to resolve a runtime shader root");
    return core::Result::eInitializationFailed;
  }

  core::Result result = createShaders(shaderRootPath);
  if (result != core::Result::eSuccess)
  {
    destroy();
    return result;
  }

  result = createPipelines();
  if (result != core::Result::eSuccess)
  {
    destroy();
    return result;
  }

  result = createSceneResourceSets();
  if (result != core::Result::eSuccess)
  {
    destroy();
    return result;
  }

  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::render::ScenePipelineLibrary::destroy()
{
  if (mPrimitivePipeline)
  {
    mPrimitivePipeline->destroy();
    mPrimitivePipeline.reset();
  }
  if (mGridPipeline)
  {
    mGridPipeline->destroy();
    mGridPipeline.reset();
  }
  for (std::unique_ptr<rhi::IResourceSet>& resourceSet : mSceneResourceSets)
  {
    if (resourceSet)
    {
      resourceSet->destroy();
      resourceSet.reset();
    }
  }
  mSceneResourceSets.clear();

  if (mScenePipelineLayout)
  {
    mScenePipelineLayout->destroy();
    mScenePipelineLayout.reset();
  }

  if (mPrimitiveVertexShader)
  {
    mPrimitiveVertexShader->destroy();
    mPrimitiveVertexShader.reset();
  }
  if (mPrimitiveFragmentShader)
  {
    mPrimitiveFragmentShader->destroy();
    mPrimitiveFragmentShader.reset();
  }
  if (mGridVertexShader)
  {
    mGridVertexShader->destroy();
    mGridVertexShader.reset();
  }
  if (mGridFragmentShader)
  {
    mGridFragmentShader->destroy();
    mGridFragmentShader.reset();
  }

  mConfig = {};
  mIsInitialized = false;
}

bool mental::render::ScenePipelineLibrary::isValid() const noexcept
{
  return mIsInitialized;
}

mental::core::Result mental::render::ScenePipelineLibrary::updateFrameResourceSet(
  const std::uint32_t frameIndex, rhi::IBuffer* cameraBuffer)
{
  if (!isFrameIndexValid(frameIndex) || cameraBuffer == nullptr)
  {
    return core::Result::eOperationFailed;
  }

  std::array<rhi::ResourceWriteDesc, 2> writes {
    rhi::ResourceWriteDesc {
                            .resourceSet = mSceneResourceSets[frameIndex].get(),
                            .binding = 0u,
                            .type = rhi::ResourceBindingType::eUniformBuffer,
                            .buffer =
        {
          .buffer = cameraBuffer,
          .offset = 0u,
          .range = cameraBuffer->getDesc().byteSize,
        }, },
    rhi::ResourceWriteDesc {
                            .resourceSet = mSceneResourceSets[frameIndex].get(),
                            .binding = 1u,
                            .type = rhi::ResourceBindingType::eStorageBuffer,
                            .buffer =
        {
          .buffer = mConfig.geometryStorageBuffer,
          .offset = 0u,
          .range = mConfig.geometryStorageBuffer->getDesc().byteSize,
        }, },
  };

  return rhi::getDevice().updateResourceSets(writes.data(), static_cast<std::uint32_t>(writes.size()));
}

mental::core::Result mental::render::ScenePipelineLibrary::recordGridDraw(
  rhi::ICommandList* cmdList, const std::uint32_t frameIndex) const
{
  if (cmdList == nullptr || !isFrameIndexValid(frameIndex))
  {
    return core::Result::eOperationFailed;
  }

  rhi::IResourceSet* resourceSets[] = {mSceneResourceSets[frameIndex].get()};
  const GridDrawPushConstants pushConstants {};
  const rhi::PushConstantRangeDesc pushConstantRange = buildScenePushConstantRange();

  cmdList->bindGraphicsPipeline(mGridPipeline.get());
  cmdList->bindResourceSets(mScenePipelineLayout.get(), kSceneResourceSetIndex, resourceSets, 1u);
  const core::Result pushResult = cmdList->pushConstants(mScenePipelineLayout.get(), pushConstantRange, &pushConstants);
  if (pushResult != core::Result::eSuccess)
  {
    return pushResult;
  }

  cmdList->draw(6u, 1u, 0u, 0u);
  return core::Result::eSuccess;
}

mental::core::Result mental::render::ScenePipelineLibrary::recordPrimitiveDraw(rhi::ICommandList* cmdList,
  const std::uint32_t frameIndex,
  const PrimitiveMeshView& primitiveMeshView,
  const glm::mat4& worldTransform) const
{
  if (cmdList == nullptr || !isFrameIndexValid(frameIndex) || !primitiveMeshView.isValid())
  {
    return core::Result::eOperationFailed;
  }

  if (primitiveMeshView.vertexOffsetBytes > std::numeric_limits<std::uint32_t>::max() ||
      primitiveMeshView.indexOffsetBytes > std::numeric_limits<std::uint32_t>::max() ||
      primitiveMeshView.indexCount == 0u)
  {
    MENTAL_ERROR("Primitive mesh view exceeds the MVP push-constant contract");
    return core::Result::eOperationFailed;
  }

  rhi::IResourceSet* resourceSets[] = {mSceneResourceSets[frameIndex].get()};
  const PrimitiveDrawPushConstants pushConstants {
    .worldTransform = worldTransform,
    .vertexOffsetBytes = static_cast<std::uint32_t>(primitiveMeshView.vertexOffsetBytes),
    .indexOffsetBytes = static_cast<std::uint32_t>(primitiveMeshView.indexOffsetBytes),
  };
  const rhi::PushConstantRangeDesc pushConstantRange = buildScenePushConstantRange();

  cmdList->bindGraphicsPipeline(mPrimitivePipeline.get());
  cmdList->bindResourceSets(mScenePipelineLayout.get(), kSceneResourceSetIndex, resourceSets, 1u);
  const core::Result pushResult = cmdList->pushConstants(mScenePipelineLayout.get(), pushConstantRange, &pushConstants);
  if (pushResult != core::Result::eSuccess)
  {
    return pushResult;
  }

  cmdList->draw(primitiveMeshView.indexCount, 1u, 0u, 0u);
  return core::Result::eSuccess;
}

mental::core::Result mental::render::ScenePipelineLibrary::createShaders(const std::filesystem::path& shaderRootPath)
{
  ShaderCompiler compiler {};
  rhi::IDevice& device = rhi::getDevice();

  mPrimitiveVertexShader = device.createShaderModule();
  mPrimitiveFragmentShader = device.createShaderModule();
  mGridVertexShader = device.createShaderModule();
  mGridFragmentShader = device.createShaderModule();
  if (!mPrimitiveVertexShader || !mPrimitiveFragmentShader || !mGridVertexShader || !mGridFragmentShader)
  {
    return core::Result::eInitializationFailed;
  }

  core::Result result = compileShaderModule(
    *mPrimitiveVertexShader, compiler, shaderRootPath, "primitiveScene.slang", "vertexMain", ShaderStage::eVertex);
  if (result != core::Result::eSuccess)
  {
    return result;
  }

  result = compileShaderModule(*mPrimitiveFragmentShader,
    compiler,
    shaderRootPath,
    "primitiveScene.slang",
    "fragmentMain",
    ShaderStage::eFragment);
  if (result != core::Result::eSuccess)
  {
    return result;
  }

  result = compileShaderModule(
    *mGridVertexShader, compiler, shaderRootPath, "editorGrid.slang", "vertexMain", ShaderStage::eVertex);
  if (result != core::Result::eSuccess)
  {
    return result;
  }

  return compileShaderModule(
    *mGridFragmentShader, compiler, shaderRootPath, "editorGrid.slang", "fragmentMain", ShaderStage::eFragment);
}

mental::core::Result mental::render::ScenePipelineLibrary::createSceneResourceSets()
{
  rhi::IDevice& device = rhi::getDevice();
  if (!mScenePipelineLayout)
  {
    return core::Result::eInitializationFailed;
  }

  mSceneResourceSets.reserve(mConfig.framesInFlight);
  for (std::uint32_t frameIndex = 0u; frameIndex < mConfig.framesInFlight; ++frameIndex)
  {
    std::unique_ptr<rhi::IResourceSet> resourceSet = device.createResourceSet();
    if (!resourceSet)
    {
      return core::Result::eInitializationFailed;
    }

    const core::Result result = resourceSet->init({
      .pipelineLayout = mScenePipelineLayout.get(),
      .resourceSetIndex = kSceneResourceSetIndex,
    });
    if (result != core::Result::eSuccess)
    {
      return result;
    }

    mSceneResourceSets.push_back(std::move(resourceSet));
  }

  return core::Result::eSuccess;
}

mental::core::Result mental::render::ScenePipelineLibrary::createPipelines()
{
  rhi::IDevice& device = rhi::getDevice();
  const auto sceneBindings = buildSceneResourceBindings();
  const rhi::ResourceLayoutDesc sceneResourceLayoutDesc {
    .bindings = sceneBindings.data(),
    .bindingCount = static_cast<std::uint32_t>(sceneBindings.size()),
  };

  mScenePipelineLayout = device.createPipelineLayout();
  mPrimitivePipeline = device.createGraphicsPipeline();
  mGridPipeline = device.createGraphicsPipeline();
  if (!mScenePipelineLayout || !mPrimitivePipeline || !mGridPipeline)
  {
    return core::Result::eInitializationFailed;
  }

  const rhi::PushConstantRangeDesc scenePushConstantRange = buildScenePushConstantRange();

  core::Result result = mScenePipelineLayout->init({
    .resourceLayoutDescs = &sceneResourceLayoutDesc,
    .resourceLayoutDescCount = 1u,
    .pushConstantRanges = &scenePushConstantRange,
    .pushConstantRangeCount = 1u,
  });
  if (result != core::Result::eSuccess)
  {
    return result;
  }

  result = mPrimitivePipeline->init(buildPrimitiveGraphicsPipelineDesc(mScenePipelineLayout.get(),
    mPrimitiveVertexShader.get(),
    mPrimitiveFragmentShader.get(),
    mConfig.colorAttachmentFormat,
    mConfig.depthAttachmentFormat));
  if (result != core::Result::eSuccess)
  {
    return result;
  }

  return mGridPipeline->init(buildGridGraphicsPipelineDesc(mScenePipelineLayout.get(),
    mGridVertexShader.get(),
    mGridFragmentShader.get(),
    mConfig.colorAttachmentFormat,
    mConfig.depthAttachmentFormat));
}

bool mental::render::ScenePipelineLibrary::isFrameIndexValid(const std::uint32_t frameIndex) const noexcept
{
  return frameIndex < mSceneResourceSets.size() && mSceneResourceSets[frameIndex] != nullptr;
}
