#pragma once

#include <render/rhi/rhi.hpp>
#include <core/resource.hpp>
#include <volk/volk.h>
#include <vma/vk_mem_alloc.h>

namespace mental::rhi::vk
{
class Buffer : public IBuffer
{
public:
    explicit Buffer(const BufferDesc& desc);
    virtual ~Buffer();

    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

    virtual const BufferDesc& getDesc() const override { return mDesc; };
    virtual rhi::Result map(void** mappedData) override;
    virtual rhi::Result unmap() override;
    virtual rhi::Result copy(void* data, uint64_t size, uint64_t offset = 0) override;

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

private:
    BufferDesc mDesc;
    VkBuffer mBuffer;
    VmaAllocator mAllocator;
    VmaAllocation mAllocation;
    bool mIsMapped;
};

}  // namespace mental::rhi::vk
