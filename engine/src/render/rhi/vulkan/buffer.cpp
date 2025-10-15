#include <render/rhi/vulkan/buffer.hpp>
#include <core/log.hpp>

mental::rhi::vk::Buffer::Buffer(const BufferDesc& desc)
    : mDesc(desc), mBuffer(VK_NULL_HANDLE), mAllocator(VK_NULL_HANDLE), mAllocation(VK_NULL_HANDLE), mAllocationInfo()
{
    mental::core::log::info("Vulkan buffer created");
}

mental::rhi::vk::Buffer::~Buffer()
{
    vmaDestroyBuffer(mAllocator, mBuffer, mAllocation);
    mental::core::log::info("Vulkan buffer destroyed");
}

mental::rhi::Result mental::rhi::vk::Buffer::upload(void* data, uint64_t size)
{
    VkBufferCreateInfo bufCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bufCreateInfo.size = size;
    bufCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

    VmaAllocationCreateInfo allocCreateInfo{};
    allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
    allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VkResult res;
    VkBuffer stagingBuf;
    VmaAllocation stagingAlloc;
    VmaAllocationInfo stagingAllocInfo;
    res = vmaCreateBuffer(mAllocator, &bufCreateInfo, &allocCreateInfo, &stagingBuf, &stagingAlloc, &stagingAllocInfo);
    if (res != VK_SUCCESS) return Result::eBufferUploadFailed;

    memcpy(stagingAllocInfo.pMappedData, data, size);

    // TODO: begin single time command to copy from staging buffer to the current buffer

    vmaDestroyBuffer(mAllocator, stagingBuf, stagingAlloc);
    return Result::eSuccess;
}
