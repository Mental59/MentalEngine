#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <core/log.hpp>

mental::rhi::vk::Buffer::Buffer(const BufferDesc& desc)
    : mDesc(desc), mBuffer(VK_NULL_HANDLE), mAllocator(VK_NULL_HANDLE), mAllocation(VK_NULL_HANDLE), mIsMapped(false)
{
    MENTAL_INFO("Vulkan buffer created");
}

mental::rhi::vk::Buffer::~Buffer()
{
    vmaDestroyBuffer(mAllocator, mBuffer, mAllocation);
    MENTAL_INFO("Vulkan buffer destroyed");
}

mental::core::resource::Object mental::rhi::vk::Buffer::getNativeObject(core::resource::ObjectType objectType)
{
    if (objectType == core::resource::ObjectType::eVkBuffer) return mBuffer;
    return mBuffer;
}

mental::rhi::Result mental::rhi::vk::Buffer::map(void** mappedData)
{
    VkResult res = vmaMapMemory(mAllocator, mAllocation, mappedData);
    if (res != VK_SUCCESS)
    {
        MENTAL_ERROR("Vulkan buffer map failed, error: {}", vkResultToString(res));
        return rhi::Result::eBufferMapFailed;
    }
    mIsMapped = true;
    return rhi::Result::eSuccess;
}

mental::rhi::Result mental::rhi::vk::Buffer::unmap()
{
    vmaUnmapMemory(mAllocator, mAllocation);
    mIsMapped = false;
    return rhi::Result::eSuccess;
}

mental::rhi::Result mental::rhi::vk::Buffer::copy(void* data, uint64_t size, uint64_t offset)
{
    if (!mIsMapped)
    {
        MENTAL_ERROR("Buffer is not mapped");
        return rhi::Result::eBufferCopyFailed;
    }

    VmaAllocationInfo allocationInfo;
    vmaGetAllocationInfo(mAllocator, mAllocation, &allocationInfo);
    memcpy((uint8_t*)allocationInfo.pMappedData + offset, data, size);

    return Result::eSuccess;
}
