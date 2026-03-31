#include <render/render.hpp>

#include <editor/app/frameContext.hpp>

#include <core/log.hpp>
#include <render/renderHostAdapter.hpp>
#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include <cstdint>
#include <cmath>

namespace
{
float getAnimatedClearChannel(double timeSeconds, double phaseOffset)
{
  const double wave = std::sin(timeSeconds + phaseOffset);
  return static_cast<float>(0.5 * (wave + 1.0));
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

  for (const resource::FrameDataHandle frameDataHandle : mFrameDataHandles)
  {
    if (frameDataHandle.isValid())
    {
      frameDataHandle.destroy();
    }
  }
  mFrameDataHandles.clear();
  resource::destroyResourceManager();
  rhi::destroyDevice();

  mCurrentFrame = 0;
  mMaxFramesInFlight = 0;
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

mental::core::Result mental::render::RenderSystem::render(const mental::editor::FrameContext& frameContext)
{
  FrameUpdater frameUpdadater(*this);

  resource::FrameData frameData = mFrameDataHandles[mCurrentFrame].get();
  if (!frameData.isValid())
  {
    MENTAL_ERROR("Invalid frame data");
    return core::Result::eOperationFailed;
  }

  core::Result res = frameData.fence->wait();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to wait for frame fence");
    return core::Result::eOperationFailed;
  }

  rhi::ISwapchain* swapchain = rhi::getDevice().getSwapchain();
  uint32_t swapchainTextureIndex = 0;
  res = swapchain->acquireNextTexture(UINT64_MAX, frameData.imageAvailableSemaphore, nullptr, swapchainTextureIndex);
  if (res == core::Result::eSuboptimal || res == core::Result::eOutOfDate)
  {
    const core::Result resizeResult = resizeSwapchain(swapchain);
    if (resizeResult != core::Result::eSuccess)
    {
      return resizeResult;
    }
    return core::Result::eSuccess;
  }
  else if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to acquire swapchain texture, error: {}", core::resultToString(res));
    return core::Result::eOperationFailed;
  }

  if (isSubmitEligibleAcquireResult(res))
  {
    res = frameData.fence->reset();
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to reset frame fence");
      return core::Result::eOperationFailed;
    }
  }

  rhi::ICommandList* cmdList = frameData.cmdList;
  res = cmdList->begin({.isOneTimeSubmit = false});
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to begin command list");
    return core::Result::eOperationFailed;
  }

  rhi::ITexture* swapchainTexture = swapchain->getTexture(swapchainTextureIndex);
  rhi::ITextureView* swapchainTextureView = swapchain->getTextureView(swapchainTextureIndex);

  res = cmdList->transitionTexture({
    .texture = swapchainTexture,
    .oldLayout = swapchainTexture->getDesc().layout,
    .newLayout = rhi::TextureLayout::eColorAttachment,
  });

  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to transition swapchain texture to color attachment layout");
    return core::Result::eOperationFailed;
  }

  const rhi::TextureDesc& swapchainTextureDesc = swapchainTexture->getDesc();
  rhi::CommandListBeginRenderingInfo renderingInfo {};
  renderingInfo.swapchainImageView = swapchainTextureView;
  renderingInfo.renderArea = {
    .width = swapchainTextureDesc.extent.width,
    .height = swapchainTextureDesc.extent.height,
  };
  const double timeSeconds = frameContext.absoluteTimeSeconds;
  renderingInfo.clearValue.color[0] = getAnimatedClearChannel(timeSeconds, 0.0);
  renderingInfo.clearValue.color[1] = getAnimatedClearChannel(timeSeconds, 2.0943951023931953);
  renderingInfo.clearValue.color[2] = getAnimatedClearChannel(timeSeconds, 4.1887902047863905);
  renderingInfo.clearValue.color[3] = 1.0f;
  renderingInfo.clearValue.depth = 0.0f;
  renderingInfo.clearValue.stencil = 0;
  cmdList->beginRendering(renderingInfo);

  cmdList->endRendering();

  res = cmdList->transitionTexture({
    .texture = swapchainTexture,
    .oldLayout = swapchainTexture->getDesc().layout,
    .newLayout = rhi::TextureLayout::ePresent,
  });
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to transition swapchain texture to present layout");
    return core::Result::eOperationFailed;
  }

  res = cmdList->end();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to end command list");
    return core::Result::eOperationFailed;
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
    return core::Result::eOperationFailed;
  }
  frameUpdadater.commit();

  res = swapchain->present(swapchainTextureIndex, frameData.renderFinishedSemaphore);
  if (res == core::Result::eSuboptimal || res == core::Result::eOutOfDate)
  {
    const core::Result resizeResult = resizeSwapchain(swapchain);
    if (resizeResult != core::Result::eSuccess)
    {
      return resizeResult;
    }
    return core::Result::eSuccess;
  }
  else if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to present swapchain texture, error: {}", core::resultToString(res));
    return core::Result::eOperationFailed;
  }

  return core::Result::eSuccess;
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
