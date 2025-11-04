#include <render/render.hpp>
#include <render/rhi/rhi.hpp>
#include <platform/window.hpp>
#include <core/log.hpp>

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
