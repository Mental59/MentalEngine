#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#include <core/types.hpp>
#include <format>
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

core::Result validateDeviceInitInput(GraphicsApi api, const DeviceInitInput& initInput)
{
  switch (api)
  {
#if defined MENTAL_WITH_VULKAN
    case GraphicsApi::Vulkan:
    {
      if (initInput.platformExtensions.empty())
      {
        MENTAL_ERROR("Missing platform specific required extensions");
        return core::Result::eInitializationFailed;
      }

      if (initInput.vulkanSurface.createSurface == nullptr || initInput.vulkanSurface.userData == nullptr)
      {
        MENTAL_ERROR("Missing Vulkan surface creation input");
        return core::Result::eInitializationFailed;
      }

      return core::Result::eSuccess;
    }
#endif
    default:
    {
      MENTAL_ERROR("Unsupported graphics api {}", graphicsApiToString(api));
      return core::Result::eInitializationFailed;
    }
  }
}

core::Result initDevice(GraphicsApi api, const DeviceInitInput& initInput)
{
  const core::Result validationResult = validateDeviceInitInput(api, initInput);
  if (validationResult != core::Result::eSuccess)
  {
    return validationResult;
  }

  switch (api)
  {
#if defined MENTAL_WITH_VULKAN
    case GraphicsApi::Vulkan:
    {
      core::Result res;
      rhi::vk::DeviceFactory factory;

      rhi::vk::InstanceInfo instanceInfo;
      res = factory.createInstance(initInput.platformExtensions, instanceInfo);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create vulkan instance. Error: {}", core::resultToString(res));
        return core::Result::eInitializationFailed;
      }

      VkSurfaceKHR surface;
      res =
        initInput.vulkanSurface.createSurface(instanceInfo.getInstance(), initInput.vulkanSurface.userData, &surface);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create vulkan surface. Error: {}", core::resultToString(res));
        return core::Result::eInitializationFailed;
      }

      rhi::vk::DeviceFactory::SwapchainSettings swapchainSettings {};
      swapchainSettings.enableTripleBuffering = false;
      swapchainSettings.enableVerticalSync = true;

      res = factory.initDevice(instanceInfo, surface, swapchainSettings);
      if (res != core::Result::eSuccess)
      {
        MENTAL_ERROR("Failed to create vulkan device. Error: {}", core::resultToString(res));
        return core::Result::eInitializationFailed;
      }

      gDevice = &rhi::vk::getDevice();
      return core::Result::eSuccess;
    }
#endif

    default:
    {
      MENTAL_ERROR("Unsupported graphics api {}", graphicsApiToString(api));
      return core::Result::eInitializationFailed;
    }
  }
}

IDevice& getDevice()
{
  return *gDevice;
}

void destroyDevice()
{
  if (gDevice != nullptr)
  {
    gDevice->destroy();
    gDevice = nullptr;
  }
}

} // namespace mental::rhi
