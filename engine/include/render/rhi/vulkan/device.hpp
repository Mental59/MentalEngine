#pragma once

#include <render/rhi/rhi.hpp>
#include <vulkan/vulkan.hpp>
#include <core/memory.hpp>

namespace mental::rhi::vk
{
struct DeviceDesc
{
    ::vk::Instance instance;
    ::vk::DebugUtilsMessengerEXT debugUtilsMessenger;
    ::vk::DebugReportCallbackEXT debugReportCallback;
    ::vk::SurfaceKHR surface;

    ::vk::PhysicalDevice physicalDevice;
    ::vk::Device device;
};

class Device : public core::memory::RefCounter<IDevice>
{
public:
    Device(const DeviceDesc& desc);
    virtual ~Device() override;

    virtual void WaitIdle() override;
    virtual GraphicsApi getGraphicsApi() override;

private:
    ::vk::Instance mInstance;
    ::vk::DebugUtilsMessengerEXT mDebugUtilsMessenger;
    ::vk::DebugReportCallbackEXT mDebugReportCallback;
    ::vk::SurfaceKHR mSurface;

    ::vk::PhysicalDevice mPhysicalDevice;
    ::vk::Device mDevice;
};

DeviceHandle createDevice(const DeviceDesc& desc);
}  // namespace mental::rhi::vk
