#pragma once

#include <volk/volk.h>
#include <render/rhi/rhi.hpp>
#include <render/rhi/vulkan/commandQueue.hpp>
#include <render/rhi/vulkan/swapchain.hpp>
#include <string>
#include <unordered_set>
#include <vector>
#include "core/resource.hpp"

namespace mental::rhi::vk
{
  struct DeviceDesc
  {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t apiVersion;

    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;

    VkQueue graphicsQueue;
    int graphicsQueueIndex = -1;

    VkDebugUtilsMessengerEXT debugUtilsMessenger;
    VkDebugReportCallbackEXT debugReportCallback;

    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
  };

  struct Context
  {
    Context() = default;

    core::Result init(
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
        const std::vector<const char*>& deviceExtensions);

    void destroy();

    VkInstance mInstance;
    VkSurfaceKHR mSurface;
    VkPhysicalDevice mPhysicalDevice;
    VkDevice mDevice;

    VkDebugReportCallbackEXT mDebugReportCallback;
    VkDebugUtilsMessengerEXT mDebugUtilsMessenger;

    VkSurfaceCapabilitiesKHR mCapabilities;
    std::vector<VkSurfaceFormatKHR> mFormats;
    std::vector<VkPresentModeKHR> mPresentModes;

    std::unordered_set<std::string> mInstanceExtensions;
    std::unordered_set<std::string> mDeviceExtensions;
  };

  class Device : public IDevice
  {
   public:
    static Device& instance();
    core::Result init(const DeviceDesc& desc);
    virtual void destroy() override;

    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

    virtual void waitIdle() override;
    virtual GraphicsApi getGraphicsApi() override;
    virtual ICommandQueue* getGraphicsQueue() override;
    virtual ISwapchain* getSwapchain() override;

    inline VkDevice getVirtualDevice() const
    {
      return mContext.mDevice;
    }
    inline VkSurfaceKHR getSurface() const
    {
      return mContext.mSurface;
    }
    inline VkPhysicalDevice getPhysicalDevice() const
    {
      return mContext.mPhysicalDevice;
    }

   private:
    Context mContext;
    CommandQueue mGraphicsQueue;
    Swapchain mSwapchain;
  };

  inline Device& getDevice()
  {
    return Device::instance();
  }
}  // namespace mental::rhi::vk
