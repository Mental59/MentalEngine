#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include <cstdint>

mental::core::Result mental::render::RenderSystem::init(const mental::render::RenderSystemConfig& conf)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized RenderSystem");
    return core::Result::eInitializationFailed;
  }

  MENTAL_ASSERT_DEBUG(conf.window != nullptr);

  rhi::initDevice(conf.graphicsApi, conf.window);

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

  MENTAL_INFO("Render system destroyed");
}

bool mental::render::RenderSystem::isValid() const
{
  return mIsInitialized;
}

mental::core::Result mental::render::RenderSystem::render()
{
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

  res = frameData.fence->reset();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to reset frame fence");
    return core::Result::eOperationFailed;
  }

  rhi::ISwapchain* swapchain = rhi::getDevice().getSwapchain();
  uint32_t swapchainTextureIndex = 0;
  res = swapchain->acquireNextTexture(UINT64_MAX, frameData.imageAvailableSemaphore, nullptr, swapchainTextureIndex);
  if (res == core::Result::eSuboptimal)
  {
    // TODO: resize swapchain, update current frame and return with success status
    MENTAL_ERROR("Swapchain needs to resized");
    return core::Result::eOperationFailed;
  }
  else if (res != core::Result::eSuccess && res != core::Result::eSuboptimal)
  {
    MENTAL_ERROR("Failed to acquire swapchain texture, error: {}", core::resultToString(res));
    return core::Result::eOperationFailed;
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
  renderingInfo.clearValue.color[0] = 0.2f;
  renderingInfo.clearValue.color[1] = 0.2f;
  renderingInfo.clearValue.color[2] = 0.2f;
  renderingInfo.clearValue.color[3] = 0.0f;
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

  res = swapchain->present(swapchainTextureIndex, frameData.renderFinishedSemaphore);
  if (res == core::Result::eSuboptimal || res == core::Result::eOutOfDate)
  {
    // TODO: resize swapchain, update current frame and return with success status
    MENTAL_ERROR("Swapchain needs to resized");
    return core::Result::eOperationFailed;
  }
  else if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to present swapchain texture, error: {}", core::resultToString(res));
    return core::Result::eOperationFailed;
  }

  mCurrentFrame = (mCurrentFrame + 1) % mMaxFramesInFlight;
  return core::Result::eSuccess;
}
