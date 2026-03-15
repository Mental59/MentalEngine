#pragma once
#include <vma/vk_mem_alloc.h>
#include <Volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  struct SwapchainTextureDesc
  {
    VkImage image;
    rhi::TextureFormat format;
    rhi::TextureExtent extent;
    rhi::TextureUsageFlags usage;
  };

  class Texture : public ITexture
  {
   public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
    virtual void destroy() override;

    virtual core::Result init(const TextureDesc& desc) override;
    inline virtual const TextureDesc& getDesc() const override
    {
      return mDesc;
    }
    void initSwapchainTexture(const SwapchainTextureDesc& desc);

   private:
    bool mShouldDestroyImage;
    VkImage mImage;
    VmaAllocation mAllocation;
    TextureDesc mDesc;
  };

  class TextureView : public ITextureView
  {
   public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
    virtual core::Result init(const TextureViewDesc& desc) override;
    virtual const TextureViewDesc& getDesc() const override;
    virtual void destroy() override;

   private:
    TextureViewDesc mDesc;
    VkImageView mImageView;
  };
}  // namespace mental::rhi::vk
