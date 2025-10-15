#include <render/render.hpp>
#include <render/rhi/rhi.hpp>
#include <platform/window.hpp>
#include <core/log.hpp>

namespace mental::render
{
RenderSystem::RenderSystem(const mental::platform::IWindow* const window) : mDevice(rhi::createDevice(rhi::GraphicsApi::Vulkan, window))
{
    mental::core::log::info("Render system initialized");
}

RenderSystem::~RenderSystem()
{
    mental::core::log::info("Render system destroyed");
}

}  // namespace mental::render
