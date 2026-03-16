#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>

mental::core::Result mental::render::RenderSystem::init(const mental::render::RenderSystemConfig& conf)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized RenderSystem");
    return core::Result::eInitializationFailed;
  }

  MENTAL_ASSERT_DEBUG(conf.window != nullptr);

  rhi::initDevice(conf.graphicsApi, conf.window);
  resource::initResourceManager();

  rhi::CommandListDesc cmdListDesc{};
  cmdListDesc.commandQueue = rhi::getDevice().getGraphicsQueue();
  mCmdListHandle = resource::getResourceManager().createCommandList(cmdListDesc);
  if (!mCmdListHandle.isValid())
  {
    MENTAL_ERROR("Failed to create a command list");
    return core::Result::eInitializationFailed;
  }

  uint32_t swapchainTextureCount = rhi::getDevice().getSwapchain()->getTextureCount();
  mMaxFramesInFlight = swapchainTextureCount;
  MENTAL_INFO("swapchainTextureCount={}, maxFramesInFlight={}", swapchainTextureCount, mMaxFramesInFlight);

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

  rhi::destroyDevice();
  MENTAL_INFO("Render system destroyed");
}

mental::core::Result mental::render::RenderSystem::render()
{
  rhi::ICommandList* cmdList = mCmdListHandle.get();
  if (!cmdList || !cmdList->isValid())
  {
    return core::Result::eOperationFailed;
  }

  core::Result res = cmdList->begin({ .isOneTimeSubmit = false });
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to begin command list");
    return core::Result::eOperationFailed;
  }

  rhi::CommandListBeginRenderingInfo renderingInfo{};  // TODO: fill rendering info
  cmdList->beginRendering(renderingInfo);

  cmdList->endRendering();

  res = cmdList->end();
  if (res != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to end command list");
    return core::Result::eOperationFailed;
  }

  return core::Result::eSuccess;
}
