#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{
Context::Context(::vk::Instance instance, ::vk::SurfaceKHR surface, ::vk::PhysicalDevice physicalDevice, ::vk::Device device,
    ::vk::DebugReportCallbackEXT debugReportCallback, ::vk::DebugUtilsMessengerEXT debugUtilsMessenger,
    const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions)
    : mInstance(instance), mSurface(surface), mPhysicalDevice(physicalDevice), mDevice(device), mDebugReportCallback(debugReportCallback),
      mDebugUtilsMessenger(debugUtilsMessenger)
{
    if (!instance) mental::core::log::fatal("Vulkan instance is null");
    if (!surface) mental::core::log::fatal("Vulkan surface is null");
    if (!physicalDevice) mental::core::log::fatal("Vulkan physical device is null");
    if (!device) mental::core::log::fatal("Vulkan device is null");

    mCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface).value;
    mFormats = physicalDevice.getSurfaceFormatsKHR(surface).value;
    mPresentModes = physicalDevice.getSurfacePresentModesKHR(surface).value;

    for (const char* extensionName : instanceExtensions)
        mInstanceExtensions.insert(extensionName);
    for (const char* extensionName : deviceExtensions)
        mDeviceExtensions.insert(extensionName);
}

void Context::destroy()
{
    mDevice.destroy();
    mInstance.destroySurfaceKHR(mSurface);
    if (mDebugUtilsMessenger) mInstance.destroyDebugUtilsMessengerEXT(mDebugUtilsMessenger);
    if (mDebugReportCallback) mInstance.destroyDebugReportCallbackEXT(mDebugReportCallback);
    mInstance.destroy();
}

Device::Device(const DeviceDesc& desc)
    : mContext(desc.instance, desc.surface, desc.physicalDevice, desc.device, desc.debugReportCallback, desc.debugUtilsMessenger,
          desc.instanceExtensions, desc.deviceExtensions),
      mGraphicsQueue(desc.graphicsQueue), mGraphicsQueueIndex(desc.graphicsQueueIndex)
{
    if (!desc.graphicsQueue || desc.graphicsQueueIndex < 0)
    {
        mental::core::log::fatal("Vulkan graphics queue is invalid");
    }

    mental::core::log::info("Vulkan device initialized");
}

void Device::destroy()
{
    mContext.destroy();
    mental::core::log::info("Vulkan device destroyed");
}

void Device::waitIdle()
{
    mContext.mDevice.waitIdle();
}

GraphicsApi Device::getGraphicsApi()
{
    return GraphicsApi::Vulkan;
}

Device* createDevice(const DeviceDesc& desc)
{
    static Device device(desc);
    return &device;
}

}  // namespace mental::rhi::vk
