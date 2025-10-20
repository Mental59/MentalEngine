#include <render/rhi/vulkan/buffer.hpp>
#include <render/rhi/vulkan/core.hpp>
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

mental::rhi::Result mental::rhi::vk::Buffer::map(void** mappedData)
{
    VkResult res = vmaMapMemory(mAllocator, mAllocation, mappedData);
    VK_RHI_RETURN_IF_NOT_SUCCESS(res, rhi::Result::eBufferMapFailed);
    return rhi::Result::eSuccess;
}

mental::rhi::Result mental::rhi::vk::Buffer::unmap()
{
    vmaUnmapMemory(mAllocator, mAllocation);
    return rhi::Result::eSuccess;
}

mental::rhi::Result mental::rhi::vk::Buffer::copy(void* data, uint64_t size, uint64_t offset)
{
    switch (mDesc.cpuAccess)
    {
        case BufferCpuAccess::None:
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

            memcpy((byte*)stagingAllocInfo.pMappedData + offset, data, size);

            // TODO: begin single time command to copy from staging buffer to the current buffer

            vmaDestroyBuffer(mAllocator, stagingBuf, stagingAlloc);
            break;
        }

        case BufferCpuAccess::Write:
        case BufferCpuAccess::ReadWrite:
        {
            vmaGetAllocationInfo(mAllocator, mAllocation, &mAllocationInfo);
            memcpy((byte*)mAllocationInfo.pMappedData + offset, data, size);
            break;
        }
    }
    return Result::eSuccess;
}
