#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{
Device::Device(const DeviceDesc& desc)
{
    mental::core::log::info("Vulkan device created");
}

Device::~Device()
{
    mental::core::log::info("Vulkan device destroyed");
}

void Device::WaitIdle() {}

GraphicsApi Device::getGraphicsApi()
{
    return GraphicsApi::Vulkan;
}

DeviceHandle createDevice(const DeviceDesc& desc)
{
    Device* device = new Device(desc);
    return DeviceHandle::Create(device);
}
}  // namespace mental::rhi::vk
