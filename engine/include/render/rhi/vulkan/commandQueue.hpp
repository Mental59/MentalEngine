#pragma once

#include <volk/volk.h>

#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  class CommandQueue : public rhi::ICommandQueue
  {
   public:
    core::Result init(VkQueue queue, uint32_t index);
    virtual void destroy() override;

    virtual core::Result submit(const SubmitInfo& info) override;
    virtual void waitIdle() override;

    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

   private:
    core::Result createCommandPool(uint32_t queueFamilyIndex);

    VkQueue mQueue;
    VkCommandPool mCommandPool;
    uint32_t mIndex;
  };
}  // namespace mental::rhi::vk
