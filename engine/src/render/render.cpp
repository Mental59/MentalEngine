#include <render/render.hpp>

#include <core/log.hpp>
#include <render/renderHostAdapter.hpp>
#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include <cmath>
#include <cstdint>

namespace
{
constexpr mental::rhi::TextureFormat kDepthTextureFormat = mental::rhi::TextureFormat::eD32_SFLOAT;

struct CameraUploadData
{
  glm::vec4 worldPosition {0.0f, 0.0f, 0.0f, 1.0f};
  glm::mat4 view {1.0f};
  glm::mat4 projection {1.0f};
  glm::mat4 viewProjection {1.0f};
  glm::vec4 aspectRatio {1.0f, 0.0f, 0.0f, 0.0f};
};

[[nodiscard]] CameraUploadData buildCameraUploadData(const mental::render::SceneCameraData& cameraData)
{
  CameraUploadData uploadData {};
  uploadData.worldPosition = glm::vec4 {cameraData.worldPosition, 1.0f};
  uploadData.view = cameraData.view;
  uploadData.projection = cameraData.projection;
  uploadData.viewProjection = cameraData.viewProjection;
  uploadData.aspectRatio.x = cameraData.aspectRatio;
  return uploadData;
}

[[nodiscard]] bool hasUsableFramebufferSize(const mental::platform::WindowSize& framebufferSize)
{
  return framebufferSize.width > 0 && framebufferSize.height > 0;
}

void recordSceneObjectPlaceholder(const mental::render::SceneRenderObject& object)
{
  switch (object.geometryKind)
  {
    case mental::render::SceneGeometryKind::eCube:
    case mental::render::SceneGeometryKind::ePlane:
    case mental::render::SceneGeometryKind::eSphere:
      break;
  }
}

struct FrameUpdater
{
  FrameUpdater(mental::render::RenderSystem& renderSystem)
    : mRenderSystem(renderSystem)
  {
  }

  void commit()
  {
    mShouldAdvance = true;
  }

  ~FrameUpdater()
  {
    if (mShouldAdvance)
    {
      mRenderSystem.nextFrame();
    }
  }

 private:
  mental::render::RenderSystem& mRenderSystem;
  bool mShouldAdvance = false;
};
} // namespace

bool mental::render::isSubmitEligibleAcquireResult(mental::core::Result acquireResult)
{
  return acquireResult == core::Result::eSuccess;
}

mental::core::Result mental::render::RenderSystem::init(const mental::render::RenderSystemConfig& conf)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized RenderSystem");
    return core::Result::eInitializationFailed;
  }

  if (conf.hostAdapter == nullptr)
  {
    MENTAL_ERROR("RenderSystem init requires a render host adapter");
    return core::Result::eInitializationFailed;
  }

  const core::Result initDeviceResult =
    rhi::initDevice(conf.graphicsApi, conf.hostAdapter->createDeviceInitInput(conf.graphicsApi));
  if (initDeviceResult != core::Result::eSuccess)
  {
    return initDeviceResult;
  }

  uint32_t swapchainTextureCount = rhi::getDevice().getSwapchain()->getTextureCount();
  mMaxFramesInFlight = swapchainTextureCount;
  MENTAL_INFO("swapchainTextureCount={}, maxFramesInFlight={}", swapchainTextureCount, mMaxFramesInFlight);

  resource::initResourceManager(mMaxFramesInFlight);

  mFrameDataHandles.resize(mMaxFramesInFlight, resource::FrameDataHandle::invalid());
  mCameraBufferHandles.resize(mMaxFramesInFlight, resource::BufferHandle::invalid());

  resource::CreateFrameDataDesc frameDataDesc {};
  frameDataDesc.cmdListDesc.commandQueue = rhi::getDevice().getGraphicsQueue();
  frameDataDesc.fenceDesc.createSignaled = true;

  for (resource::FrameDataHandle& frameDataHandle : mFrameDataHandles)
  {
    frameDataHandle = resource::getResourceManager().createFrameData(frameDataDesc);
    if (!frameDataHandle.isValid())
    {
      MENTAL_ERROR("Failed to create frame data");
      for (const resource::FrameDataHandle createdFrameDataHandle : mFrameDataHandles)
      {
        if (createdFrameDataHandle.isValid())
        {
          createdFrameDataHandle.destroy();
        }
      }
      mFrameDataHandles.clear();
      resource::destroyResourceManager();
      rhi::destroyDevice();
      return core::Result::eInitializationFailed;
    }
  }

  const core::Result cameraBufferResult = createCameraUploadBuffers();
  if (cameraBufferResult != core::Result::eSuccess)
  {
    for (const resource::FrameDataHandle createdFrameDataHandle : mFrameDataHandles)
    {
      if (createdFrameDataHandle.isValid())
      {
        createdFrameDataHandle.destroy();
      }
    }
    mFrameDataHandles.clear();
    mCameraBufferHandles.clear();
    resource::destroyResourceManager();
    rhi::destroyDevice();
    return cameraBufferResult;
  }

  mCurrentFrame = 0;
  mIsInitialized = true;
  mHostAdapter = conf.hostAdapter;
  MENTAL_INFO("Render system initialized");

  return core::Result::eSuccess;
}

void mental::render::RenderSystem::destroy()
{
  if (!mIsInitialized)
  {
    MENTAL_WARN("Trying to destroy an uninitialized RenderSystem");
    return;
  }

  rhi::getDevice().waitIdle();

  destroyDepthTarget();
  destroyCameraUploadBuffers();
  for (const resource::FrameDataHandle frameDataHandle : mFrameDataHandles)
  {
    if (frameDataHandle.isValid())
    {
      frameDataHandle.destroy();
    }
  }
  mFrameDataHandles.clear();
  mCameraBufferHandles.clear();
  resource::destroyResourceManager();
  rhi::destroyDevice();

  mCurrentFrame = 0;
  mMaxFramesInFlight = 0;
  mDepthExtent = {};
  mIsInitialized = false;
  mHostAdapter = nullptr;

  MENTAL_INFO("Render system destroyed");
}

bool mental::render::RenderSystem::isValid() const
{
  return mIsInitialized;
}

void mental::render::RenderSystem::nextFrame()
{
  mCurrentFrame = (mCurrentFrame + 1) % mMaxFramesInFlight;
}

mental::render::RenderFrameOutcome mental::render::RenderSystem::render(
  const mental::render::FrameContext& frameContext)
{
  FrameUpdater frameUpdater(*this);

  resource::FrameData frameData = mFrameDataHandles[mCurrentFrame].get();
  if (!frameData.isValid())
  {
    MENTAL_ERROR("Invalid frame data");
    return {.result = core::Result::eOperationFailed};
  }

  core::Result res = frameData.fence->wait();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to wait for frame fence");
    return {.result = core::Result::eOperationFailed};
  }

  rhi::ISwapchain* swapchain = rhi::getDevice().getSwapchain();
  uint32_t swapchainTextureIndex = 0;
  res = swapchain->acquireNextTexture(UINT64_MAX, frameData.imageAvailableSemaphore, nullptr, swapchainTextureIndex);
  if (res == core::Result::eSuboptimal || res == core::Result::eOutOfDate)
  {
    const core::Result resizeResult = resizeSwapchain(swapchain);
    if (resizeResult != core::Result::eSuccess)
    {
      return {.result = resizeResult};
    }
    return {.result = core::Result::eSuccess, .submitted = false};
  }
  else if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to acquire swapchain texture, error: {}", core::resultToString(res));
    return {.result = core::Result::eOperationFailed};
  }

  if (isSubmitEligibleAcquireResult(res))
  {
    res = frameData.fence->reset();
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to reset frame fence");
      return {.result = core::Result::eOperationFailed};
    }
  }

  rhi::ICommandList* cmdList = frameData.cmdList;
  res = cmdList->begin({.isOneTimeSubmit = false});
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to begin command list");
    return {.result = core::Result::eOperationFailed};
  }

  rhi::ITexture* swapchainTexture = swapchain->getTexture(swapchainTextureIndex);
  rhi::ITextureView* swapchainTextureView = swapchain->getTextureView(swapchainTextureIndex);
  if (swapchainTexture == nullptr || swapchainTextureView == nullptr)
  {
    MENTAL_ERROR("Swapchain returned an invalid texture or texture view");
    return {.result = core::Result::eOperationFailed};
  }

  const core::Result depthTargetResult = ensureDepthTarget(frameContext.framebufferSize);
  if (depthTargetResult != core::Result::eSuccess)
  {
    return {.result = depthTargetResult};
  }

  rhi::ITexture* depthTexture = mDepthTextureHandle.get();
  rhi::ITextureView* depthTextureView = mDepthTextureViewHandle.get();
  if (depthTexture == nullptr || depthTextureView == nullptr)
  {
    MENTAL_ERROR("Depth target is invalid");
    return {.result = core::Result::eOperationFailed};
  }

  rhi::IBuffer* cameraBuffer = mCameraBufferHandles[mCurrentFrame].get();
  if (cameraBuffer == nullptr)
  {
    MENTAL_ERROR("Camera upload buffer is invalid");
    return {.result = core::Result::eOperationFailed};
  }

  res = cmdList->transitionTexture({
    .texture = swapchainTexture,
    .oldLayout = swapchainTexture->getDesc().layout,
    .newLayout = rhi::TextureLayout::eColorAttachment,
  });

  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to transition swapchain texture to color attachment layout");
    return {.result = core::Result::eOperationFailed};
  }

  res = cmdList->transitionTexture({
    .texture = depthTexture,
    .oldLayout = depthTexture->getDesc().layout,
    .newLayout = rhi::TextureLayout::eDepthStencilAttachment,
  });
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to transition depth texture to depth-stencil attachment layout");
    return {.result = core::Result::eOperationFailed};
  }

  const CameraUploadData cameraUploadData = buildCameraUploadData(frameContext.sceneRenderFrame.camera);
  void* mappedCameraData = nullptr;
  res = cameraBuffer->map(&mappedCameraData);
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to map camera upload buffer");
    return {.result = core::Result::eOperationFailed};
  }

  res = cameraBuffer->copy(const_cast<CameraUploadData*>(&cameraUploadData), sizeof(cameraUploadData));
  cameraBuffer->unmap();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to upload camera data");
    return {.result = core::Result::eOperationFailed};
  }

  const rhi::TextureDesc& swapchainTextureDesc = swapchainTexture->getDesc();
  rhi::CommandListBeginRenderingInfo renderingInfo {};
  renderingInfo.swapchainImageView = swapchainTextureView;
  renderingInfo.depthAttachmentView = depthTextureView;
  renderingInfo.renderArea = {
    .width = swapchainTextureDesc.extent.width,
    .height = swapchainTextureDesc.extent.height,
  };
  renderingInfo.clearValue.color[0] = 0.08f;
  renderingInfo.clearValue.color[1] = 0.09f;
  renderingInfo.clearValue.color[2] = 0.12f;
  renderingInfo.clearValue.color[3] = 1.0f;
  renderingInfo.clearValue.depth = 1.0f;
  renderingInfo.clearValue.stencil = 0;
  cmdList->beginRendering(renderingInfo);

  for (const SceneRenderObject& object : frameContext.sceneRenderFrame.objects)
  {
    recordSceneObjectPlaceholder(object);
  }

  cmdList->endRendering();

  res = cmdList->transitionTexture({
    .texture = swapchainTexture,
    .oldLayout = swapchainTexture->getDesc().layout,
    .newLayout = rhi::TextureLayout::ePresent,
  });
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to transition swapchain texture to present layout");
    return {.result = core::Result::eOperationFailed};
  }

  res = cmdList->end();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to end command list");
    return {.result = core::Result::eOperationFailed};
  }

  rhi::ICommandList* submitCmdLists[] = {cmdList};
  res = rhi::getDevice().getGraphicsQueue()->submit({
    .cmdLists = submitCmdLists,
    .cmdListCount = 1,
    .waitSemaphore = frameData.imageAvailableSemaphore,
    .waitStage = rhi::PipelineStage::eColorAttachmentOutput,
    .signalSemaphore = frameData.renderFinishedSemaphore,
    .signalFence = frameData.fence,
  });
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to submit command list");
    return {.result = core::Result::eOperationFailed};
  }
  frameUpdater.commit();

  res = swapchain->present(swapchainTextureIndex, frameData.renderFinishedSemaphore);
  if (res == core::Result::eSuboptimal || res == core::Result::eOutOfDate)
  {
    const core::Result resizeResult = resizeSwapchain(swapchain);
    if (resizeResult != core::Result::eSuccess)
    {
      return {.result = resizeResult};
    }
    return {.result = core::Result::eSuccess, .submitted = false};
  }
  else if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to present swapchain texture, error: {}", core::resultToString(res));
    return {.result = core::Result::eOperationFailed};
  }

  return {.result = core::Result::eSuccess, .submitted = true};
}

mental::core::Result mental::render::RenderSystem::resizeSwapchain(rhi::ISwapchain* swapchain)
{
  MENTAL_ASSERT(mHostAdapter != nullptr);

  const FramebufferExtentRecoveryResult recovery = mHostAdapter->recoverNextFramebufferExtent();
  if (recovery.status == FramebufferExtentRecoveryStatus::eClosing)
  {
    return core::Result::eSuccess;
  }

  core::Result res = swapchain->resize(recovery.extent.width, recovery.extent.height);
  MENTAL_ASSERT_MESSAGE(res == core::Result::eSuccess, "Failed to resize swapchain");
  return res;
}

mental::core::Result mental::render::RenderSystem::createCameraUploadBuffers()
{
  rhi::BufferDesc cameraBufferDesc {};
  cameraBufferDesc.usage = rhi::BufferUsageFlagBits::eBufferUsageUniformBit;
  cameraBufferDesc.cpuAccess = rhi::BufferCpuAccess::Write;
  cameraBufferDesc.byteSize = sizeof(CameraUploadData);

  for (resource::BufferHandle& cameraBufferHandle : mCameraBufferHandles)
  {
    cameraBufferHandle = resource::getResourceManager().createBuffer(cameraBufferDesc);
    if (!cameraBufferHandle.isValid())
    {
      MENTAL_ERROR("Failed to create camera upload buffer");
      destroyCameraUploadBuffers();
      return core::Result::eInitializationFailed;
    }
  }

  return core::Result::eSuccess;
}

void mental::render::RenderSystem::destroyCameraUploadBuffers()
{
  for (const resource::BufferHandle cameraBufferHandle : mCameraBufferHandles)
  {
    if (cameraBufferHandle.isValid())
    {
      cameraBufferHandle.destroy();
    }
  }
}

mental::core::Result mental::render::RenderSystem::ensureDepthTarget(const platform::WindowSize& framebufferSize)
{
  if (!hasUsableFramebufferSize(framebufferSize))
  {
    return core::Result::eSuccess;
  }

  if (mDepthTextureHandle.isValid() && mDepthTextureViewHandle.isValid() &&
      mDepthExtent.width == framebufferSize.width && mDepthExtent.height == framebufferSize.height)
  {
    return core::Result::eSuccess;
  }

  destroyDepthTarget();

  rhi::TextureDesc depthTextureDesc {};
  depthTextureDesc.format = kDepthTextureFormat;
  depthTextureDesc.layout = rhi::TextureLayout::eUndefined;
  depthTextureDesc.tiling = rhi::TextureTiling::eOptimal;
  depthTextureDesc.usage = rhi::TextureUsageFlagBits::eTextureUsageDepthStencilAttachmentBit;
  depthTextureDesc.extent = {
    .width = framebufferSize.width,
    .height = framebufferSize.height,
    .depth = 1,
  };
  depthTextureDesc.mipLevels = 1;
  depthTextureDesc.arrayLayers = 1;
  depthTextureDesc.cubeCompatible = false;

  mDepthTextureHandle = resource::getResourceManager().createTexture(depthTextureDesc);
  if (!mDepthTextureHandle.isValid())
  {
    MENTAL_ERROR("Failed to create depth texture");
    return core::Result::eInitializationFailed;
  }

  rhi::TextureViewDesc depthTextureViewDesc {};
  depthTextureViewDesc.texture = mDepthTextureHandle.get();
  depthTextureViewDesc.type = rhi::TextureType::eDepthMap;
  mDepthTextureViewHandle = resource::getResourceManager().createTextureView(depthTextureViewDesc);
  if (!mDepthTextureViewHandle.isValid())
  {
    MENTAL_ERROR("Failed to create depth texture view");
    destroyDepthTarget();
    return core::Result::eInitializationFailed;
  }

  mDepthExtent = framebufferSize;
  return core::Result::eSuccess;
}

void mental::render::RenderSystem::destroyDepthTarget()
{
  if (mDepthTextureViewHandle.isValid())
  {
    mDepthTextureViewHandle.destroy();
    mDepthTextureViewHandle = resource::TextureViewHandle::invalid();
  }

  if (mDepthTextureHandle.isValid())
  {
    mDepthTextureHandle.destroy();
    mDepthTextureHandle = resource::TextureHandle::invalid();
  }

  mDepthExtent = {};
}
