#pragma once

#include <render/rhi/rhi.hpp>
#include <volk/volk.h>

namespace mental::rhi::vk
{
class CommandQueue : public rhi::ICommandQueue
{
public:
    CommandQueue() = default;
    void init(VkQueue queue, uint32_t index);

private:
    VkQueue mQueue;
    uint32_t mIndex;
};
}  // namespace mental::rhi::vk
