#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/core.hpp>
#include <core/log.hpp>

namespace mental::rhi::vk
{

Context::Context(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t apiVersion,
    VkDebugReportCallbackEXT debugReportCallback, VkDebugUtilsMessengerEXT debugUtilsMessenger, VkSurfaceCapabilitiesKHR capabilities,
    const std::vector<VkSurfaceFormatKHR>& formats, const std::vector<VkPresentModeKHR>& presentModes,
    const std::vector<const char*>& instanceExtensions, const std::vector<const char*>& deviceExtensions)
    : mInstance(instance), mSurface(surface), mPhysicalDevice(physicalDevice), mDevice(device), mCapabilities(capabilities),
      mFormats(formats), mPresentModes(presentModes), mDebugReportCallback(debugReportCallback), mDebugUtilsMessenger(debugUtilsMessenger)
{
    if (!instance) mental::core::log::fatal("Vulkan instance is null");
    if (!surface) mental::core::log::fatal("Vulkan surface is null");
    if (!physicalDevice) mental::core::log::fatal("Vulkan physical device is null");
    if (!device) mental::core::log::fatal("Vulkan device is null");

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
    if (importRes != VK_SUCCESS) mental::core::log::fatal("Failed to import vulkan functions");

    allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

    VkResult createAllocatorRes = vmaCreateAllocator(&allocatorCreateInfo, &mAllocator);
    if (createAllocatorRes != VK_SUCCESS) mental::core::log::fatal("Failed to create vulkan allocator");
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

Device::Device(const DeviceDesc& desc)
    : mContext(desc.instance, desc.surface, desc.physicalDevice, desc.device, desc.apiVersion, desc.debugReportCallback,
          desc.debugUtilsMessenger, desc.capabilities, desc.formats, desc.presentModes, desc.instanceExtensions, desc.deviceExtensions),
      mGraphicsQueue(desc.graphicsQueue), mGraphicsQueueIndex(desc.graphicsQueueIndex)
{
    if (!desc.graphicsQueue || desc.graphicsQueueIndex < 0) mental::core::log::fatal("Vulkan graphics queue is invalid");

    mental::core::log::info("Vulkan device initialized");
}

void Device::waitIdle()
{
    vkDeviceWaitIdle(mContext.mDevice);
}

GraphicsApi Device::getGraphicsApi()
{
    return GraphicsApi::Vulkan;
}

rhi::Result Device::createBuffer(BufferDesc desc, BufferHandle& buffer)
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

    VmaAllocationCreateInfo allocInfo{.usage = VMA_MEMORY_USAGE_AUTO};
    VkBuffer vkBuffer;
    VmaAllocation allocation;
    VmaAllocationInfo allocationInfo;
    VkResult createBufferRes = vmaCreateBuffer(mContext.mAllocator, &bufferCreateInfo, &allocInfo, &vkBuffer, &allocation, &allocationInfo);

    if (createBufferRes != VK_SUCCESS)
    {
        mental::core::log::warning("Failed to create buffer %s", vkResultToString(createBufferRes));
        return Result::eBufferInitializationFailed;
    }

    Buffer* pBuffer = new Buffer(desc);
    pBuffer->setBuffer(vkBuffer).setAllocator(mContext.mAllocator).setAllocation(allocation).setAllocationInfo(allocationInfo);
    buffer = BufferHandle::create(pBuffer);
    return Result::eSuccess;
}

DeviceHandle Device::create(const DeviceDesc& desc)
{
    Device* pDevice = new Device(desc);
    return DeviceHandle::create(pDevice);
}

Device::~Device()
{
    mContext.destroy();
    mental::core::log::info("Vulkan device destroyed");
}

}  // namespace mental::rhi::vk
