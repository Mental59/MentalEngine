#include <render/rhi/vulkan/device.hpp>
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <core/log.hpp>
#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include "render/rhi/rhi.hpp"

namespace mental::rhi::vk
{

  core::Result Context::init(
      VkInstance instance,
      VkSurfaceKHR surface,
      VkPhysicalDevice physicalDevice,
      VkDevice device,
      uint32_t apiVersion,
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

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = apiVersion;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;

    VmaVulkanFunctions vulkanFunctions;
    VkResult importRes = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
    if (importRes != VK_SUCCESS)
    {
      MENTAL_ERROR("Failed to import vulkan functions");
      return core::Result::eInitializationFailed;
    }

    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VkResult createAllocatorRes = vmaCreateAllocator(&allocatorCreateInfo, &mAllocator);
    if (createAllocatorRes != VK_SUCCESS)
    {
      MENTAL_ERROR("Failed to create vulkan allocator");
      return core::Result::eInitializationFailed;
    }

    return core::Result::eSuccess;
  }

  void Context::destroy()
  {
    vmaDestroyAllocator(mAllocator);
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

  core::Result Device::createBuffer(BufferDesc desc, core::memory::SharedHandle<IBuffer>& outBuffer)
  {
    core::memory::SharedHandle<Buffer> buffer = core::memory::makeShared<Buffer>();
    buffer->init(desc);
    outBuffer = std::move(buffer);
    return core::Result::eSuccess;
  }

  ICommandQueue* Device::getGraphicsQueue()
  {
    return &mGraphicsQueue;
  }

  vk::CommandQueue* Device::getVulkanGraphicsQueue()
  {
    return &mGraphicsQueue;
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
        desc.apiVersion,
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
      MENTAL_ERROR("Failed to initialize graphics queue");
      return core::Result::eInitializationFailed;
    }

    MENTAL_INFO("Vulkan device initialized");
    return core::Result::eSuccess;
  }

  void Device::destroy()
  {
    mGraphicsQueue.destroy();
    mContext.destroy();
    MENTAL_INFO("Vulkan device destroyed");
  }

  core::resource::Object Device::getNativeObject(core::resource::ObjectType objectType)
  {
    return nullptr;
  }

}  // namespace mental::rhi::vk
