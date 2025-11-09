#pragma once

#include <render/rhi/rhi.hpp>
#include <volk/volk.h>

namespace mental::rhi::vk
{
class CommandQueue : public rhi::ICommandQueue
{
public:
    rhi::Result init(VkQueue queue, uint32_t index);
    void destroy();

    virtual rhi::Result submit(const SubmitInfo& info) override;
    virtual void waitIdle() override;

    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

private:
    rhi::Result createCommandPool(uint32_t queueFamilyIndex);

    VkQueue mQueue;
    VkCommandPool mCommandPool;
    uint32_t mIndex;
};
}  // namespace mental::rhi::vk
