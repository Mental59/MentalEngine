#include <core/log.hpp>
#include <platform/window.hpp>
#include <render/render.hpp>
#include <render/rhi/rhi.hpp>

namespace mental::render
{
  RenderSystem::RenderSystem(const mental::platform::IWindow* const window)
  {
    rhi::initDevice(rhi::GraphicsApi::Vulkan, window);
    MENTAL_INFO("Render system initialized");
  }

    RenderSystem::~RenderSystem()
  {
    MENTAL_INFO("Render system destroyed");
  }

}  // namespace mental::render
