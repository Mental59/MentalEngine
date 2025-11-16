#pragma once
#include <volk/volk.h>

#include <render/rhi/vulkan/device.hpp>
#include <vector>

namespace mental::rhi::vk
{
  struct DebugMessenger
  {
    VkDebugUtilsMessengerEXT utilsMessenger;
    VkDebugReportCallbackEXT reportCallback;
  };

  struct SwapchainSupportDetails
  {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
  };

  class PhysicalDeviceInfo
  {
   public:
    PhysicalDeviceInfo() = default;
    PhysicalDeviceInfo(
        VkPhysicalDevice physicalDevice,
        VkSurfaceKHR surface,
        const std::vector<const char*>& requiredExtensions,
        const VkPhysicalDeviceFeatures& features);

    bool isDiscreteGPU() const;
    bool isIntegratedGPU() const;
    bool isGPU() const
    {
      return isDiscreteGPU() || isIntegratedGPU();
    }
    bool areExtensionsSupported() const
    {
      return mAreExtensionsSupported;
    }
    bool isSuitable() const
    {
      return mScore > 0;
    }

    int getScore() const
    {
      return mScore;
    }
    VkPhysicalDevice getPhysicalDevice() const
    {
      return mPhysicalDevice;
    };
    int getGraphicsQueueFamily() const
    {
      return mGraphicsQueueFamily;
    }
    const std::vector<const char*>& getRequiredExtensions() const
    {
      return mRequiredExtensions;
    }
    const VkPhysicalDeviceFeatures& getRequiredFeatures() const
    {
      return mRequiredFeatures;
    }
    const SwapchainSupportDetails& getSwapchainSupportDetails() const
    {
      return mSwapchainSupportDetails;
    }

   private:
    bool checkDeviceExtensionSupport(const std::vector<const char*>& extensions) const;
    int findGraphicsQueueFamily() const;
    SwapchainSupportDetails querySwapchainSupport() const;
    int calculateScore() const;

    VkPhysicalDevice mPhysicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR mSurface = VK_NULL_HANDLE;
    std::vector<const char*> mRequiredExtensions{};

    int mGraphicsQueueFamily = -1;
    SwapchainSupportDetails mSwapchainSupportDetails{};
    bool mAreExtensionsSupported = false;
    int mScore = 0;
    VkPhysicalDeviceFeatures mRequiredFeatures{};
  };

  class InstanceInfo
  {
   public:
    InstanceInfo() = default;
    InstanceInfo(VkInstance instance, const std::vector<const char*>& extensions)
        : mInstance(instance),
          mExtensions(extensions)
    {
    }

    VkInstance getInstance() const
    {
      return mInstance;
    }
    const std::vector<const char*>& getExtensions() const
    {
      return mExtensions;
    };

   private:
    VkInstance mInstance = VK_NULL_HANDLE;
    std::vector<const char*> mExtensions;
  };

  class DeviceFactory
  {
   public:
    core::Result initDevice(const InstanceInfo& instanceInfo, VkSurfaceKHR surface) const;
    core::Result createInstance(InstanceInfo& info) const;

   private:
    bool checkInstanceExtensionSupport(const std::vector<const char*>& extensions) const;
    bool checkInstanceLayerSupport(const std::vector<const char*>& layers) const;

    core::Result choosePhysicalDevice(VkInstance instance, VkSurfaceKHR surface, PhysicalDeviceInfo& physicalDeviceInfo)
        const;
    core::Result createLogicalDevice(const PhysicalDeviceInfo& physicalDeviceInfo, VkDevice& device) const;

#if defined(_DEBUG)
    DebugMessenger createDebugMessenger(VkInstance instance) const;
#endif
  };
}  // namespace mental::rhi::vk
