#pragma once

#include <core/memory.hpp>

namespace mental::platform
{
class IWindow;
}

namespace mental::rhi
{
enum class GraphicsApi : uint8_t
{
    Vulkan
};

class IDevice : public core::memory::IResource
{
    virtual void WaitIdle() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    // TODO: extend interface
};

typedef core::memory::RefCountPtr<IDevice> DeviceHandle;

DeviceHandle createDevice(GraphicsApi api, const mental::platform::IWindow* const window);
}  // namespace mental::rhi
