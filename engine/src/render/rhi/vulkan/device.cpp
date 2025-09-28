#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{
Context::Context(::vk::Instance instance, ::vk::SurfaceKHR surface, ::vk::PhysicalDevice physicalDevice, ::vk::Device device,
    ::vk::DebugReportCallbackEXT debugReportCallback, ::vk::DebugUtilsMessengerEXT debugUtilsMessenger)
    : instance(instance), surface(surface), physicalDevice(physicalDevice), device(device), debugReportCallback(debugReportCallback),
      debugUtilsMessenger(debugUtilsMessenger)
{
    if (!instance) mental::core::log::fatal("Vulkan instance is null");
    if (!surface) mental::core::log::fatal("Vulkan surface is null");
    if (!physicalDevice) mental::core::log::fatal("Vulkan physical device is null");
    if (!device) mental::core::log::fatal("Vulkan device is null");

    capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface).value;
    formats = physicalDevice.getSurfaceFormatsKHR(surface).value;
    presentModes = physicalDevice.getSurfacePresentModesKHR(surface).value;
}

void Context::destroy()
{
    device.destroy();
    instance.destroySurfaceKHR(surface);
    if (debugUtilsMessenger) instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
    if (debugReportCallback) instance.destroyDebugReportCallbackEXT(debugReportCallback);
    instance.destroy();
}

Device::Device(const DeviceDesc& desc)
    : mContext(desc.instance, desc.surface, desc.physicalDevice, desc.device, desc.debugReportCallback, desc.debugUtilsMessenger),
      mGraphicsQueue(desc.graphicsQueue), mGraphicsQueueIndex(desc.graphicsQueueIndex)
{
    if (!desc.graphicsQueue || desc.graphicsQueueIndex < 0)
    {
        mental::core::log::fatal("Vulkan graphics queue is invalid");
    }

    mental::core::log::info("Vulkan device initialized");
}

Device::~Device()
{
    mContext.destroy();
    mental::core::log::info("Vulkan device destroyed");
}

void Device::WaitIdle()
{
    mContext.device.waitIdle();
}

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
