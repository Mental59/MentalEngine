#include <render/rhi/vulkan/commandQueue.hpp>

void mental::rhi::vk::CommandQueue::init(VkQueue queue, uint32_t index)
{
    mQueue = queue;
    mIndex = index;
}
