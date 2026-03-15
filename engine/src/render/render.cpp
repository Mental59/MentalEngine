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
  // TODO: clear screen color
  return core::Result::eSuccess;
}
