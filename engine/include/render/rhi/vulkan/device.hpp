#pragma once

#include <render/rhi/rhi.hpp>
#include <vulkan/vulkan.hpp>
#include <core/memory.hpp>

namespace mental::rhi::vk
{
struct DeviceDesc
{
    ::vk::Instance instance;
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
};

DeviceHandle createDevice(const DeviceDesc& desc);
}  // namespace mental::rhi::vk
