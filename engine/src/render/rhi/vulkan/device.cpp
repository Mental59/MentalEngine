#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{
Context::Context(::vk::Instance instance, ::vk::SurfaceKHR surface, ::vk::PhysicalDevice physicalDevice, ::vk::Device device,
    uint32_t apiVersion, ::vk::DebugReportCallbackEXT debugReportCallback, ::vk::DebugUtilsMessengerEXT debugUtilsMessenger,
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

    VmaAllocatorCreateInfo allocatorCreateInfo{};
    allocatorCreateInfo.vulkanApiVersion = apiVersion;
    allocatorCreateInfo.physicalDevice = physicalDevice;
    allocatorCreateInfo.device = device;
    allocatorCreateInfo.instance = instance;

    VmaVulkanFunctions vulkanFunctions;
    VkResult importRes = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
    if (importRes != VK_SUCCESS) mental::core::log::fatal("Failed import vulkan functions for vma");

    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VkResult createAllocatorRes = vmaCreateAllocator(&allocatorCreateInfo, &mAllocator);
    if (createAllocatorRes != VK_SUCCESS) mental::core::log::fatal("Failed to create vulkan allocator");
}

void Context::destroy()
{
    vmaDestroyAllocator(mAllocator);
    mDevice.destroy();
    mInstance.destroySurfaceKHR(mSurface);
    if (mDebugUtilsMessenger) mInstance.destroyDebugUtilsMessengerEXT(mDebugUtilsMessenger);
    if (mDebugReportCallback) mInstance.destroyDebugReportCallbackEXT(mDebugReportCallback);
    mInstance.destroy();
}

Device::Device(const DeviceDesc& desc)
    : mContext(desc.instance, desc.surface, desc.physicalDevice, desc.device, desc.apiVersion, desc.debugReportCallback,
          desc.debugUtilsMessenger, desc.instanceExtensions, desc.deviceExtensions),
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

BufferHandle Device::createBuffer(BufferDesc desc)
{
    ::vk::BufferUsageFlags usageFlags;
    switch (desc.type)
    {
        case BufferType::eStorage: usageFlags |= ::vk::BufferUsageFlagBits::eStorageBuffer; break;
        case BufferType::eUniform: usageFlags |= ::vk::BufferUsageFlagBits::eUniformBuffer; break;
    }
    if (desc.isTransferDst) usageFlags |= ::vk::BufferUsageFlagBits::eTransferDst;
    if (desc.isTransferSrc) usageFlags |= ::vk::BufferUsageFlagBits::eTransferSrc;

    ::vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.setSize(desc.byteSize).setUsage(usageFlags).setSharingMode(::vk::SharingMode::eExclusive);

    VmaAllocationCreateInfo allocInfo{.usage = VMA_MEMORY_USAGE_AUTO};
    VkBuffer vkBuffer;
    VmaAllocation allocation;
    VkResult createBufferRes = vmaCreateBuffer(mContext.mAllocator, bufferCreateInfo, &allocInfo, &vkBuffer, &allocation, nullptr);

    if (createBufferRes != VK_SUCCESS)
    {
        mental::core::log::warning("Failed to create buffer %s", resultToString(static_cast<::vk::Result>(createBufferRes)));
        return nullptr;
    }

    Buffer* buffer = new Buffer(desc);
    buffer->setBuffer(vkBuffer).setAllocator(mContext.mAllocator).setAllocation(allocation);
    return BufferHandle::create(buffer);
}

Device* Device::create(const DeviceDesc& desc)
{
    static Device device(desc);
    return &device;
}

}  // namespace mental::rhi::vk
