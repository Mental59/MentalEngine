#pragma once

#include <core/memory.hpp>
#include <render/rhi/rhi.hpp>

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

  class IWindow : public core::memory::NonCopyable
  {
   public:
    virtual void pollEvents() const = 0;
    virtual double getTime() const = 0;
    virtual bool shouldClose() const = 0;

#if defined(MENTAL_WITH_VULKAN)
    virtual rhi::Result createSurface(VkInstance instance, VkSurfaceKHR& surface) const = 0;
#endif
  };

}  // namespace mental::platform
