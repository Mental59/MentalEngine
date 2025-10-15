#pragma once

#include <core/memory.hpp>

#if defined(MENTAL_WITH_VULKAN)
#include <volk/volk.h>
#endif

namespace mental::platform
{

struct WindowDesc
{
    const char* title = nullptr;
    int width = 0;
    int height = 0;
};

class IWindow : public core::memory::IObject
{
public:
    IWindow() = default;
    IWindow(const IWindow&) = delete;
    IWindow(const IWindow&&) = delete;
    IWindow& operator=(const IWindow&) = delete;
    IWindow& operator=(const IWindow&&) = delete;

    virtual void pollEvents() const = 0;
    virtual double getTime() const = 0;
    virtual bool shouldClose() const = 0;

#if defined(MENTAL_WITH_VULKAN)
    virtual VkSurfaceKHR createSurface(VkInstance instance) const = 0;
#endif
};

}  // namespace mental::platform
