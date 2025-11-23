#include <render/rhi/vulkan/device.hpp>
#include <Volk/volk.h>
#include <core/log.hpp>
#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/allocator.hpp>
#include "core/resource.hpp"
#include "core/types.hpp"
#include "render/rhi/rhi.hpp"

namespace mental::rhi::vk
{
  core::Result Context::init(
      VkInstance instance,
      VkSurfaceKHR surface,
      VkPhysicalDevice physicalDevice,
      VkDevice device,
      VkDebugReportCallbackEXT debugReportCallback,
      VkDebugUtilsMessengerEXT debugUtilsMessenger,
      VkSurfaceCapabilitiesKHR capabilities,
      const std::vector<VkSurfaceFormatKHR>& formats,
      const std::vector<VkPresentModeKHR>& presentModes,
      const std::vector<const char*>& instanceExtensions,
      const std::vector<const char*>& deviceExtensions)

  {
    mInstance = instance;
    mSurface = surface;
    mPhysicalDevice = physicalDevice;
    mDevice = device;
    mCapabilities = capabilities;
    mFormats = formats;
    mPresentModes = presentModes;
    mDebugReportCallback = debugReportCallback;
    mDebugUtilsMessenger = debugUtilsMessenger;

    MENTAL_ASSERT_DEBUG(instance != VK_NULL_HANDLE);
    MENTAL_ASSERT_DEBUG(surface != VK_NULL_HANDLE);
    MENTAL_ASSERT_DEBUG(physicalDevice != VK_NULL_HANDLE);
    MENTAL_ASSERT_DEBUG(device != VK_NULL_HANDLE);
    MENTAL_ASSERT_DEBUG(instance != VK_NULL_HANDLE);

    for (const char* extensionName : instanceExtensions)
      mInstanceExtensions.insert(extensionName);
    for (const char* extensionName : deviceExtensions)
      mDeviceExtensions.insert(extensionName);

    return core::Result::eSuccess;
  }

  void Context::destroy()
  {
    vkDestroyDevice(mDevice, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(mInstance, mSurface, VK_NULL_HANDLE);
    if (mDebugUtilsMessenger)
      vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, VK_NULL_HANDLE);
    if (mDebugReportCallback)
      vkDestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, VK_NULL_HANDLE);
    vkDestroyInstance(mInstance, VK_NULL_HANDLE);
  }

  void Device::waitIdle()
  {
    vkDeviceWaitIdle(mContext.mDevice);
  }

  GraphicsApi Device::getGraphicsApi()
  {
    return GraphicsApi::Vulkan;
  }

  ICommandQueue* Device::getGraphicsQueue()
  {
    return &mGraphicsQueue;
  }

  ISwapchain* Device::getSwapchain()
  {
    return &mSwapchain;
  }

  Device& Device::instance()
  {
    static Device device;
    return device;
  }

  core::Result Device::init(const DeviceDesc& desc)
  {
    core::Result res = mContext.init(
        desc.instance,
        desc.surface,
        desc.physicalDevice,
        desc.device,
        desc.debugReportCallback,
        desc.debugUtilsMessenger,
        desc.capabilities,
        desc.formats,
        desc.presentModes,
        desc.instanceExtensions,
        desc.deviceExtensions);
    if (res != core::Result::eSuccess)
    {
      return core::Result::eInitializationFailed;
    }

    if (!desc.graphicsQueue || desc.graphicsQueueIndex < 0)
    {
      MENTAL_ERROR("Vulkan graphics queue is invalid");
      return core::Result::eInitializationFailed;
    }
    res = mGraphicsQueue.init(desc.graphicsQueue, desc.graphicsQueueIndex);
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to initialize graphics queue, error: {}", core::resultToString(res));
      return core::Result::eInitializationFailed;
    }

    AllocatorDesc allocatorDesc{};
    allocatorDesc.device = desc.device;
    allocatorDesc.physicalDevice = desc.physicalDevice;
    allocatorDesc.instance = desc.instance;
    allocatorDesc.vulkanApiVersion = desc.apiVersion;
    res = initAllocator(allocatorDesc);
    if (res != core::Result::eSuccess)
    {
      MENTAL_ERROR("Failed to initialize allocator, error: {}", core::resultToString(res));
      return core::Result::eInitializationFailed;
    }

    MENTAL_INFO("Vulkan device initialized");
    return core::Result::eSuccess;
  }

  void Device::destroy()
  {
    destroyAllocator();
    mSwapchain.destroy();
    mGraphicsQueue.destroy();
    mContext.destroy();
    MENTAL_INFO("Vulkan device destroyed");
  }

  core::resource::Object Device::getNativeObject(core::resource::ObjectType objectType)
  {
    return nullptr;
  }

}  // namespace mental::rhi::vk
