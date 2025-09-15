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
    RenderSystem() = default;
    ~RenderSystem() = default;

    bool init(const mental::platform::IWindow* const window);

private:
    mental::rhi::DeviceHandle mDevice;
};
}  // namespace mental::render
