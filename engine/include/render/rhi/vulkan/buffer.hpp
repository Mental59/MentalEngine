#pragma once

#include <render/rhi/rhi.hpp>
#include <core/memory.hpp>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace mental::rhi::vk
{
class Buffer : public core::memory::RefCounter<IBuffer>
{
public:
    explicit Buffer(const BufferDesc& desc);
    virtual ~Buffer() override;

    virtual const BufferDesc& getDesc() const override { return mDesc; };
    virtual Result upload(void* data, uint64_t size) override;

    Buffer& setBuffer(VkBuffer buffer)
    {
        mBuffer = buffer;
        return *this;
    }
    Buffer& setAllocator(VmaAllocator allocator)
    {
        mAllocator = allocator;
        return *this;
    }
    Buffer& setAllocation(VmaAllocation allocation)
    {
        mAllocation = allocation;
        return *this;
    }
    Buffer& setAllocationInfo(const VmaAllocationInfo& allocationInfo)
    {
        mAllocationInfo = allocationInfo;
        return *this;
    }

private:
    BufferDesc mDesc;
    VkBuffer mBuffer;
    VmaAllocator mAllocator;
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
};

}  // namespace mental::rhi::vk
