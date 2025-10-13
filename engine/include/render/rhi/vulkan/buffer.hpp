#pragma once

#include <render/rhi/rhi.hpp>
#include <core/memory.hpp>
#include <vulkan/vulkan.hpp>
#include <vma/vk_mem_alloc.h>

namespace mental::rhi::vk
{
class Buffer : public core::memory::RefCounter<IBuffer>
{
public:
    explicit Buffer(const BufferDesc& desc);
    virtual ~Buffer() override;

    virtual const BufferDesc& getDesc() const override { return mDesc; };

    Buffer& setBuffer(::vk::Buffer buffer)
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
    ::vk::Buffer mBuffer;
    VmaAllocator mAllocator;
    VmaAllocation mAllocation;
};

}  // namespace mental::rhi::vk
