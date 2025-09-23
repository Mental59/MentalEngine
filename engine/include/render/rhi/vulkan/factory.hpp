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
    PhysicalDeviceInfo(
        const ::vk::PhysicalDevice& physicalDevice, const ::vk::SurfaceKHR& surface, const std::vector<const char*>& requiredExtensions);

    bool isDiscreteGPU() const { return mProperties.deviceType == ::vk::PhysicalDeviceType::eDiscreteGpu; }
    bool isIntegratedGPU() const { return mProperties.deviceType == ::vk::PhysicalDeviceType::eIntegratedGpu; }
    bool isGPU() const { return isDiscreteGPU() || isIntegratedGPU(); }
    bool areExtensionsSupported() const { return mAreExtensionsSupported; }
    int getScore() const;
    ::vk::PhysicalDevice getPhysicalDevice() const { return mPhysicalDevice; };

private:
    bool checkDeviceExtensionSupport(const std::vector<const char*>& extensions) const;
    int findQueueFamilyWithPresentSupport(::vk::QueueFlags desiredFlags) const;
    SwapchainSupportDetails querySwapchainSupport() const;

    ::vk::PhysicalDevice mPhysicalDevice = nullptr;
    ::vk::SurfaceKHR mSurface = nullptr;
    ::vk::PhysicalDeviceProperties mProperties{};
    ::vk::PhysicalDeviceFeatures mFeatures{};

    int mQueueFamilyWithPresentSupport = -1;
    SwapchainSupportDetails mSwapchainSupportDetails{};
    bool mAreExtensionsSupported = false;
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

#if defined(_DEBUG)
    DebugMessenger createDebugMessenger(::vk::Instance instance) const;
#endif
};
}  // namespace mental::rhi::vk
