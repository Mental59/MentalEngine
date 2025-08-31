#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{
Device::Device(const DeviceDesc& desc)
{
    mental::core::log::info("Vulkan device created");
}

void Device::WaitIdle() {}

GraphicsApi Device::getGraphicsApi()
{
    return GraphicsApi::Vulkan;
}

Object Device::getNativeObject(ObjectType type)
{
    return nullptr;
}

DeviceHandle createDevice(const DeviceDesc& desc)
{
    Device* device = new Device(desc);
    return DeviceHandle::Create(device);
}
}  // namespace mental::rhi::vk
