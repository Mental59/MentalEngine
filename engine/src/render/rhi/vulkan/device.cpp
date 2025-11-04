#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{

rhi::Result Context::init(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t apiVersion,
    VkDebugReportCallbackEXT debugReportCallback, VkDebugUtilsMessengerEXT debugUtilsMessenger, VkSurfaceCapabilitiesKHR capabilities,
    const std::vector<VkSurfaceFormatKHR>& formats, const std::vector<VkPresentModeKHR>& presentModes,
    const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions)

{
    mInstance = instance;
    mSurface = surface;
    mPhysicalDevice = physicalDevice;
    mDevice = device;
    mCapabilities = capabilities;
    mFormats = formats;
    mPresentModes = presentModes;
    mDebugReportCallback = debugReportCallback;
    mDebugUtilsMessenger = debugUtilsMessenger;

    if (!instance) mental::core::log::error("Vulkan instance is null");
    if (!surface) mental::core::log::error("Vulkan surface is null");
    if (!physicalDevice) mental::core::log::error("Vulkan physical device is null");
    if (!device) mental::core::log::error("Vulkan device is null");
    if (!instance || !surface || !physicalDevice || !device) return rhi::Result::eDeviceInitializationFailed;

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
    if (importRes != VK_SUCCESS)
    {
        mental::core::log::error("Failed to import vulkan functions");
        return rhi::Result::eDeviceInitializationFailed;
    }

    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VkResult createAllocatorRes = vmaCreateAllocator(&allocatorCreateInfo, &mAllocator);
    if (createAllocatorRes != VK_SUCCESS)
    {
        mental::core::log::error("Failed to create vulkan allocator");
        return rhi::Result::eDeviceInitializationFailed;
    }

    return rhi::Result::eSuccess;
}

void Context::destroy()
{
    vmaDestroyAllocator(mAllocator);
    vkDestroyDevice(mDevice, VK_NULL_HANDLE);
    vkDestroySurfaceKHR(mInstance, mSurface, VK_NULL_HANDLE);
    if (mDebugUtilsMessenger) vkDestroyDebugUtilsMessengerEXT(mInstance, mDebugUtilsMessenger, VK_NULL_HANDLE);
    if (mDebugReportCallback) vkDestroyDebugReportCallbackEXT(mInstance, mDebugReportCallback, VK_NULL_HANDLE);
    vkDestroyInstance(mInstance, VK_NULL_HANDLE);
}

void Device::waitIdle()
{
    vkDeviceWaitIdle(mContext.mDevice);
}

GraphicsApi Device::getGraphicsApi()
{
    return GraphicsApi::Vulkan;
}

rhi::Result Device::createBuffer(BufferDesc desc, core::memory::SharedHandle<IBuffer>& outBuffer)
{
    if (desc.byteSize == 0) return Result::eBufferInitializationFailed;

    VkBufferUsageFlags usage = 0;
    if (desc.usage & BufferUsageFlagBits::eStorageBuffer) usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (desc.usage & BufferUsageFlagBits::eUniformBuffer) usage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (desc.usage & BufferUsageFlagBits::eTransferSrc) usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    if (desc.usage & BufferUsageFlagBits::eTransferDst) usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufferCreateInfo.size = desc.byteSize;
    bufferCreateInfo.usage = usage;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
    switch (desc.cpuAccess)
    {
        case BufferCpuAccess::Write: allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT; break;
        case BufferCpuAccess::ReadWrite: allocInfo.flags |= VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT; break;
    }

    VkBuffer vkBuffer;
    VmaAllocation allocation;
    VkResult createBufferRes = vmaCreateBuffer(mContext.mAllocator, &bufferCreateInfo, &allocInfo, &vkBuffer, &allocation, nullptr);

    if (createBufferRes != VK_SUCCESS)
    {
        mental::core::log::warning("Failed to create buffer %s", vkResultToString(createBufferRes));
        return Result::eBufferInitializationFailed;
    }

    core::memory::SharedHandle<Buffer> buffer = core::memory::makeShared<Buffer>(desc);
    buffer->setBuffer(vkBuffer).setAllocator(mContext.mAllocator).setAllocation(allocation);
    outBuffer = std::move(buffer);
    return Result::eSuccess;
}

ICommandQueue* Device::getGraphicsQueue()
{
    return &mGraphicsQueue;
}

vk::CommandQueue& Device::getVulkanGraphicsQueue()
{
    return mGraphicsQueue;
}

Device& Device::instance()
{
    static Device device;
    return device;
}

rhi::Result Device::init(const DeviceDesc& desc)
{
    rhi::Result res =
        mContext.init(desc.instance, desc.surface, desc.physicalDevice, desc.device, desc.apiVersion, desc.debugReportCallback,
            desc.debugUtilsMessenger, desc.capabilities, desc.formats, desc.presentModes, desc.instanceExtensions, desc.deviceExtensions);
    if (res != rhi::Result::eSuccess) return res;

    if (!desc.graphicsQueue || desc.graphicsQueueIndex < 0)
    {
        mental::core::log::error("Vulkan graphics queue is invalid");
        return rhi::Result::eDeviceInitializationFailed;
    }
    mGraphicsQueue.init(desc.graphicsQueue, desc.graphicsQueueIndex);

    mental::core::log::info("Vulkan device initialized");
    return rhi::Result::eSuccess;
}

Device::~Device()
{
    mContext.destroy();
    mental::core::log::info("Vulkan device destroyed");
}

}  // namespace mental::rhi::vk
