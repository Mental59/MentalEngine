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

    virtual bool isValid() const override;

    virtual core::Result begin(const CommandListBegindDesc& desc) override;
    virtual core::Result end() override;
    virtual core::Result copyBuffer(IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size)
        override;
    virtual core::Result copyBufferToImage(
        IBuffer* buffer,
        size_t bufferOffset,
        ITexture* texture,
        uint32_t mipLevel,
        const TextureOffset3D& textureOffset) override;

    virtual core::Result beginRendering(CommandListBeginRenderingInfo& info) override;
    virtual core::Result endRendering() override;

   private:
    VkCommandBuffer mCmdBuffer;
    VkCommandPool mCmdPool;
    bool mIsInit = false;
  };
}  // namespace mental::rhi::vk
