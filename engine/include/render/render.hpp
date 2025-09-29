#pragma once

#include <render/rhi/rhi.hpp>

namespace mental::platform
{
class IWindow;
}

namespace mental::render
{
class RenderSystem
{
public:
    RenderSystem(const mental::platform::IWindow* const window);
    ~RenderSystem();

private:
    mental::rhi::IDevice* mDevice;
};
}  // namespace mental::render
