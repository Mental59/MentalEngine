#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{
Device::Device(const DeviceDesc& desc)
    : mInstance(desc.instance), mDebugUtilsMessenger(desc.debugUtilsMessenger), mDebugReportCallback(desc.debugReportCallback),
      mSurface(desc.surface), mPhysicalDevice(desc.physicalDevice), mDevice(desc.device)
{
    // TODO: validate desc

    mental::core::log::info("Vulkan device initialized");
}

Device::~Device()
{
    mDevice.destroy();

    mInstance.destroySurfaceKHR(mSurface);

    if (mDebugUtilsMessenger) mInstance.destroyDebugUtilsMessengerEXT(mDebugUtilsMessenger);
    if (mDebugReportCallback) mInstance.destroyDebugReportCallbackEXT(mDebugReportCallback);

    mInstance.destroy();

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
