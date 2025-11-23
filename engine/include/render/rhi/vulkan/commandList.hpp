#pragma once
#include <volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  class CommandList : public ICommandList
  {
   public:
    CommandList() = default;
    virtual core::Result init(const CommandListDesc& desc) override;
    virtual void destroy() override;

    virtual core::Result begin(const CommandListBegindDesc& desc) override;
    virtual core::Result end() override;
    virtual core::Result copyBuffer(IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size)
        override;

   private:
    VkCommandBuffer mCmdBuffer;
    VkCommandPool mCmdPool;
  };
}  // namespace mental::rhi::vk
