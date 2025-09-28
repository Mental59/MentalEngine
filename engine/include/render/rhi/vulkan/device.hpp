#pragma once

#include <render/rhi/rhi.hpp>
#include <vulkan/vulkan.hpp>
#include <core/memory.hpp>

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
};

struct Context
{
    ::vk::Instance instance;
    ::vk::SurfaceKHR surface;
    ::vk::PhysicalDevice physicalDevice;
    ::vk::Device device;

    ::vk::DebugReportCallbackEXT debugReportCallback;
    ::vk::DebugUtilsMessengerEXT debugUtilsMessenger;

    ::vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<::vk::SurfaceFormatKHR> formats;
    std::vector<::vk::PresentModeKHR> presentModes;

    Context(::vk::Instance instance, ::vk::SurfaceKHR surface, ::vk::PhysicalDevice physicalDevice, ::vk::Device device,
        ::vk::DebugReportCallbackEXT debugReportCallback, ::vk::DebugUtilsMessengerEXT debugUtilsMessenger);
    void destroy();
};

class Device : public core::memory::RefCounter<IDevice>
{
public:
    Device(const DeviceDesc& desc);
    virtual ~Device() override;

    virtual void WaitIdle() override;
    virtual GraphicsApi getGraphicsApi() override;

private:
    Context mContext;

    ::vk::Queue mGraphicsQueue;
    int mGraphicsQueueIndex = -1;
};

DeviceHandle createDevice(const DeviceDesc& desc);
}  // namespace mental::rhi::vk
