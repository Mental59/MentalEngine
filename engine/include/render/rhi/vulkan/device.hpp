#pragma once

#include <render/rhi/rhi.hpp>
#include <vulkan/vulkan.hpp>
#include <unordered_set>
#include <string>
#include <vector>

namespace mental::rhi::vk
{
struct DeviceDesc
{
    ::vk::Instance instance;
    ::vk::SurfaceKHR surface;
    ::vk::PhysicalDevice physicalDevice;
    ::vk::Device device;

    ::vk::Queue graphicsQueue;
    int graphicsQueueIndex = -1;

    ::vk::DebugUtilsMessengerEXT debugUtilsMessenger;
    ::vk::DebugReportCallbackEXT debugReportCallback;

    std::vector<const char*> instanceExtensions;
    std::vector<const char*> deviceExtensions;
};

struct Context
{
    ::vk::Instance mInstance;
    ::vk::SurfaceKHR mSurface;
    ::vk::PhysicalDevice mPhysicalDevice;
    ::vk::Device mDevice;

    ::vk::DebugReportCallbackEXT mDebugReportCallback;
    ::vk::DebugUtilsMessengerEXT mDebugUtilsMessenger;

    ::vk::SurfaceCapabilitiesKHR mCapabilities;
    std::vector<::vk::SurfaceFormatKHR> mFormats;
    std::vector<::vk::PresentModeKHR> mPresentModes;

    std::unordered_set<std::string> mInstanceExtensions;
    std::unordered_set<std::string> mDeviceExtensions;

    Context(::vk::Instance instance, ::vk::SurfaceKHR surface, ::vk::PhysicalDevice physicalDevice, ::vk::Device device,
        ::vk::DebugReportCallbackEXT debugReportCallback, ::vk::DebugUtilsMessengerEXT debugUtilsMessenger,
        const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions);
    void destroy();
};

class Device : public IDevice
{
public:
    static Device* create(const DeviceDesc& desc);

    virtual void destroy() override;

    virtual void waitIdle() override;
    virtual GraphicsApi getGraphicsApi() override;

private:
    Device(const DeviceDesc& desc);
    Context mContext;

    ::vk::Queue mGraphicsQueue;
    int mGraphicsQueueIndex = -1;
};

}  // namespace mental::rhi::vk
