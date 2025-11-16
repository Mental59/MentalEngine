#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <render/rhi/rhi.hpp>

mental::core::Result mental::render::RenderSystem::init(const mental::render::RenderSystemConfig& conf)
{
  if (mIsInitialized)
  {
    MENTAL_ERROR("Trying to initialize an already initialized RenderSystem");
    return core::Result::eInitializationFailed;
  }

  MENTAL_ASSERT_DEBUG(conf.window != nullptr);

  rhi::initDevice(conf.graphicsApi, conf.window);
  mIsInitialized = true;
  MENTAL_INFO("Render system initialized");

  return core::Result::eSuccess;
}

void mental::render::RenderSystem::destroy()
{
  if (!mIsInitialized)
  {
    return;
  }

  rhi::destroyDevice();
  MENTAL_INFO("Render system destroyed");
}
