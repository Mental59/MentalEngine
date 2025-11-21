#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <render/rhi/rhi.hpp>

mental::core::Result mental::render::RenderSystem::init(const mental::render::RenderSystemConfig& conf)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized RenderSystem");
    return core::Result::eInitializationFailed;
  }

  MENTAL_ASSERT_DEBUG(conf.window != nullptr);

  rhi::initDevice(conf.graphicsApi, conf.window);
  render::initResourceManager();

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
