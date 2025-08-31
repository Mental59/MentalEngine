#pragma once

#include <render/rhi/resource.hpp>

namespace mental::rhi
{
enum class GraphicsApi : uint8_t
{
    Vulkan
};

class IDevice : public IResource
{
    virtual void WaitIdle() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    // TODO: extend interface
};

typedef RefCountPtr<IDevice> DeviceHandle;

DeviceHandle createDevice(GraphicsApi api);
}  // namespace mental::rhi
