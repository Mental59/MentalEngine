#include <render/renderHostAdapter.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>

namespace
{
#if defined MENTAL_WITH_VULKAN
mental::core::Result createWindowSurface(VkInstance instance, void* userData, VkSurfaceKHR* surface)
{
  auto* window = static_cast<mental::platform::IWindow*>(userData);
  MENTAL_ASSERT_DEBUG(window != nullptr);
  return mental::platform::createVulkanSurface(window, instance, surface);
}
#endif
} // namespace

mental::rhi::DeviceInitInput mental::render::WindowRenderHostAdapter::createDeviceInitInput(
  mental::rhi::GraphicsApi api) const
{
  rhi::DeviceInitInput initInput {};
  if (mWindow == nullptr)
  {
    MENTAL_ERROR("Cannot create render host adapter device input without a window");
    return initInput;
  }

  switch (api)
  {
#if defined MENTAL_WITH_VULKAN
    case rhi::GraphicsApi::Vulkan:
    {
      initInput.vulkanSurface = {
        .createSurface = &createWindowSurface,
        .userData = mWindow,
      };
      break;
    }
#endif
    default:
    {
      break;
    }
  }

  return initInput;
}

mental::render::FramebufferExtentRecoveryResult mental::render::WindowRenderHostAdapter::recoverNextFramebufferExtent()
  const
{
  MENTAL_ASSERT_DEBUG(mWindow != nullptr);

  FramebufferExtent extent {};
  while (extent.width == 0 || extent.height == 0)
  {
    if (mWindow->shouldClose())
    {
      return {
        .status = FramebufferExtentRecoveryStatus::eClosing,
      };
    }

    const platform::WindowSize windowSize = mWindow->getWindowSize();
    extent = {
      .width = windowSize.width,
      .height = windowSize.height,
    };
    if (extent.width == 0 || extent.height == 0)
    {
      mWindow->waitEvents();
    }
  }

  return {
    .status = FramebufferExtentRecoveryStatus::eUsableExtent,
    .extent = extent,
  };
}
