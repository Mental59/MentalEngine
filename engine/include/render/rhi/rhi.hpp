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

const char* graphicsApiToString(GraphicsApi api);

class IDevice : public core::memory::IObject
{
public:
    virtual void waitIdle() = 0;
    virtual void destroy() = 0;
    virtual GraphicsApi getGraphicsApi() = 0;

    // TODO: extend interface
};

IDevice* createDevice(GraphicsApi api, const mental::platform::IWindow* const window);
}  // namespace mental::rhi
