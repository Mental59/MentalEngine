#pragma once
#ifdef MENTAL_WITH_VULKAN
#include <volk.h>
#endif

#include "core/resource.hpp"
#include "core/types.hpp"
#include <input/inputSnapshot.hpp>
#include <vector>

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

enum class CursorMode
{
  eNormal = 0,
  eDisabled,
};

class IWindow : public core::resource::IResource
{
 public:
  virtual core::Result init(const WindowDesc& desc) = 0;
  virtual void pollEvents() const = 0;
  virtual void waitEvents() const = 0;
  virtual double getTime() const = 0;
  [[nodiscard]] virtual input::InputSnapshot sampleInput() const = 0;
  virtual void setCursorMode(CursorMode mode) = 0;
  virtual void setRawMouseMotionEnabled(bool enabled) = 0;
#ifdef MENTAL_WITH_VULKAN
  [[nodiscard]] virtual std::vector<const char*> getPlatformVulkanExtensions() const = 0;
#endif

  virtual bool shouldClose() const = 0;
  virtual WindowSize getWindowSize() const = 0;
};

#ifdef MENTAL_WITH_VULKAN
core::Result createVulkanSurface(IWindow* window, VkInstance instance, VkSurfaceKHR* surface);
#endif
} // namespace mental::platform
