#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#include <core/types.hpp>
#include <format>
#include <platform/window.hpp>
#if defined MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/factory.hpp>
#endif

namespace mental::rhi
{
static IDevice* gDevice = nullptr;

const char* graphicsApiToString(GraphicsApi api)
{
  switch (api)
  {
    case GraphicsApi::Vulkan:
    {
      return "Vulkan";
    }
    default:
    {
      return "";
    }
  }
}

void initDevice(GraphicsApi api, mental::platform::IWindow* window)
{
  MENTAL_ASSERT_DEBUG(window != nullptr);

  switch (api)
  {
#if defined MENTAL_WITH_VULKAN
    case GraphicsApi::Vulkan:
    {
      core::Result res;
      rhi::vk::DeviceFactory factory;

      rhi::vk::InstanceInfo instanceInfo;
      res = factory.createInstance(instanceInfo);
      MENTAL_ASSERT_MESSAGE(res == core::Result::eSuccess,
        std::format("Failed to create vulkan instance. Error: {}", core::resultToString(res)));

      VkSurfaceKHR surface;
      res = mental::platform::createVulkanSurface(window, instanceInfo.getInstance(), &surface);
      MENTAL_ASSERT_MESSAGE(res == core::Result::eSuccess,
        std::format("Failed to create vulkan surface. Error: {}", core::resultToString(res)));

      rhi::vk::DeviceFactory::SwapchainSettings swapchainSettings {};
      swapchainSettings.enableTripleBuffering = false;
      swapchainSettings.enableVerticalSync = true;

      res = factory.initDevice(instanceInfo, surface, swapchainSettings);
      MENTAL_ASSERT_MESSAGE(res == core::Result::eSuccess,
        std::format("Failed to create vulkan device. Error: {}", core::resultToString(res)));

      gDevice = &rhi::vk::getDevice();
      break;
    }
#endif

    default:
    {
      MENTAL_ASSERT_MESSAGE(false, std::format("Unsupported graphics api {}", graphicsApiToString(api)));
    }
  }
}

IDevice& getDevice()
{
  return *gDevice;
}

void destroyDevice()
{
  gDevice->destroy();
}

} // namespace mental::rhi
