#pragma once

#include <core/memory.hpp>

#if defined(MENTAL_WITH_VULKAN)
#include <vulkan/vulkan.hpp>
#endif

namespace mental::platform
{

struct WindowDesc
{
    const char* title = nullptr;
    int width = 0;
    int height = 0;
};

class IWindow : public core::memory::IResource
{
public:
    virtual void pollEvents() const = 0;
    virtual double getTime() const = 0;
    virtual bool shouldClose() const = 0;

#if defined(MENTAL_WITH_VULKAN)
    virtual ::vk::SurfaceKHR createSurface(const ::vk::Instance& instance) const = 0;
#endif
};

typedef core::memory::RefCountPtr<IWindow> WindowHandle;

}  // namespace mental::platform
