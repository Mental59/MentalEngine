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
    ::vk::PhysicalDevice mPhysicalDevice;
    ::vk::Device mDevice;
    ::vk::SurfaceKHR mSurface;
};

DeviceHandle createDevice(const DeviceDesc& desc);
}  // namespace mental::rhi::vk
