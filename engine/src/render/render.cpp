#include <render/render.hpp>
#include <render/rhi/rhi.hpp>

namespace mental::render
{
bool RenderSystem::init()
{
    rhi::DeviceHandle device = rhi::createDevice(rhi::GraphicsApi::Vulkan);
    return true;
}
}  // namespace mental::render
