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

  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

  virtual bool isValid() const override;

  virtual core::Result begin(const CommandListBegindDesc& desc) override;
  virtual core::Result end() override;
  virtual core::Result copyBuffer(
    IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size) override;
  virtual core::Result copyBufferToImage(IBuffer* buffer,
    size_t bufferOffset,
    ITexture* texture,
    uint32_t mipLevel,
    const TextureOffset3D& textureOffset) override;
  virtual core::Result transitionTexture(const TextureTransitionInfo& info) override;

  virtual void beginRendering(CommandListBeginRenderingInfo& info) override;
  virtual void endRendering() override;
  virtual void bindGraphicsPipeline(IGraphicsPipeline* pipeline) override;
  virtual void bindResourceSets(IGraphicsPipeline* graphicsPipeline,
    uint32_t firstSet,
    IResourceSet* const* resourceSets,
    uint32_t resourceSetCount) override;
  virtual core::Result pushConstants(
    IGraphicsPipeline* graphicsPipeline, const PushConstantRangeDesc& range, const void* data) override;
  virtual void draw(
    uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;

 private:
  VkCommandBuffer mCmdBuffer;
  VkCommandPool mCmdPool;
  bool mIsInit = false;
};
} // namespace mental::rhi::vk
