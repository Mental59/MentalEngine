#include <render/rhi/vulkan/buffer.hpp>
#include <core/log.hpp>

mental::rhi::vk::Buffer::Buffer(const BufferDesc& desc) : mDesc(desc)
{
    mental::core::log::info("Vulkan buffer created");
}

mental::rhi::vk::Buffer::~Buffer()
{
    vmaDestroyBuffer(mAllocator, mBuffer, mAllocation);
    mental::core::log::info("Vulkan buffer destroyed");
}
