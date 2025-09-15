#include <render/render.hpp>
#include <render/rhi/rhi.hpp>
#include <platform/window.hpp>

namespace mental::render
{
bool RenderSystem::init(const platform::IWindow* const window)
{
    mDevice = rhi::createDevice(rhi::GraphicsApi::Vulkan, window);
    return true;
}
}  // namespace mental::render
