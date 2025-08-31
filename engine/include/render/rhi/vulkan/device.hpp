#pragma once

#include <render/rhi/rhi.hpp>
#include <vulkan/vulkan.hpp>

namespace mental::rhi::vk
{
struct DeviceDesc
{
    ::vk::Instance instance;
    ::vk::PhysicalDevice physicalDevice;
    ::vk::Device device;
};

class Device : public RefCounter<IDevice>
{
public:
    Device(const DeviceDesc& desc);

    virtual void WaitIdle() override;
    virtual GraphicsApi getGraphicsApi() override;

    virtual Object getNativeObject(ObjectType type) override;
};

DeviceHandle createDevice(const DeviceDesc& desc);
}  // namespace mental::rhi::vk
