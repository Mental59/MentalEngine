#pragma once
#ifdef MENTAL_WITH_VULKAN
#include <Volk/volk.h>
#endif

#include "core/resource.hpp"
#include "core/types.hpp"
#include <input/inputSnapshot.hpp>

namespace mental::platform
{
struct WindowDesc
{
  const char* title = nullptr;
  int width = 0;
  int height = 0;
};

struct WindowSize
{
  uint32_t width = 0;
  uint32_t height = 0;
};

class IWindow : public core::resource::IResource
{
 public:
  virtual core::Result init(const WindowDesc& desc) = 0;
  virtual void pollEvents() const = 0;
  virtual void waitEvents() const = 0;
  virtual double getTime() const = 0;
  [[nodiscard]] virtual input::InputSnapshot sampleInput() const = 0;
  virtual bool shouldClose() const = 0;
  virtual WindowSize getWindowSize() const = 0;
};

#ifdef MENTAL_WITH_VULKAN
core::Result createVulkanSurface(IWindow* window, VkInstance instance, VkSurfaceKHR* surface);
#endif
} // namespace mental::platform
