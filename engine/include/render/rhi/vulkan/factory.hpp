#pragma once
#include <render/rhi/vulkan/device.hpp>
#include <vulkan/vulkan.hpp>
#include <vector>

namespace mental::rhi::vk
{
struct DebugMessenger
{
    ::vk::DebugUtilsMessengerEXT utilsMessenger = nullptr;
    ::vk::DebugReportCallbackEXT reportCallback = nullptr;
};

struct SwapchainSupportDetails
{
    ::vk::SurfaceCapabilitiesKHR capabilities{};
    std::vector<::vk::SurfaceFormatKHR> formats{};
    std::vector<::vk::PresentModeKHR> presentModes{};
};

class PhysicalDeviceInfo
{
public:
    PhysicalDeviceInfo() = default;
    PhysicalDeviceInfo(const ::vk::PhysicalDevice& physicalDevice, const ::vk::SurfaceKHR& surface,
        const std::vector<const char*>& requiredExtensions, const ::vk::PhysicalDeviceFeatures& features);

    bool isDiscreteGPU() const { return mPhysicalDevice.getProperties().deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu; }
    bool isIntegratedGPU() const { return mPhysicalDevice.getProperties().deviceType == ::vk::PhysicalDeviceType::eIntegratedGpu; }
    bool isGPU() const { return isDiscreteGPU() || isIntegratedGPU(); }
    bool areExtensionsSupported() const { return mAreExtensionsSupported; }
    bool isSuitable() const { return mScore > 0; }

    int getScore() const { return mScore; }
    ::vk::PhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; };
    int getGraphicsQueueFamily() const { return mGraphicsQueueFamily; }
    const std::vector<const char*>& getRequiredExtensions() const { return mRequiredExtensions; }
    const ::vk::PhysicalDeviceFeatures& getRequiredFeatures() const { return mRequiredFeatures; }

private:
    bool checkDeviceExtensionSupport(const std::vector<const char*>& extensions) const;
    int findGraphicsQueueFamily() const;
    SwapchainSupportDetails querySwapchainSupport() const;
    int calculateScore() const;

    ::vk::PhysicalDevice mPhysicalDevice = nullptr;
    ::vk::SurfaceKHR mSurface = nullptr;
    std::vector<const char*> mRequiredExtensions{};

    int mGraphicsQueueFamily = -1;
    SwapchainSupportDetails mSwapchainSupportDetails{};
    bool mAreExtensionsSupported = false;
    int mScore = 0;
    ::vk::PhysicalDeviceFeatures mRequiredFeatures{};
};

class DeviceFactory
{
public:
    DeviceHandle create(const ::vk::Instance& instance, const ::vk::SurfaceKHR& surface) const;
    ::vk::Instance createInstance() const;

private:
    bool checkInstanceExtensionSupport(const std::vector<const char*>& extensions) const;
    bool checkInstanceLayerSupport(const std::vector<const char*>& layers) const;

    PhysicalDeviceInfo choosePhysicalDevice(const ::vk::Instance& instance, const ::vk::SurfaceKHR& surface) const;
    ::vk::Device createLogicalDevice(const PhysicalDeviceInfo& physicalDeviceInfo) const;

#if defined(_DEBUG)
    DebugMessenger createDebugMessenger(::vk::Instance instance) const;
#endif
};
}  // namespace mental::rhi::vk
