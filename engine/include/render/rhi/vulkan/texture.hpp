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

  virtual core::Result init(const TextureDesc& desc) override;
  virtual void destroy() override;

  virtual bool isValid() const override;

  inline virtual const TextureDesc& getDesc() const override
  {
    return mDesc;
  }
  virtual void setLayout(TextureLayout layout) override;
  void initSwapchainTexture(const SwapchainTextureDesc& desc);

 private:
  bool mIsInit = false;
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
  virtual void destroy() override;

  virtual bool isValid() const override;

  virtual const TextureViewDesc& getDesc() const override;

 private:
  bool mIsInit = false;
  TextureViewDesc mDesc;
  VkImageView mImageView;
};
} // namespace mental::rhi::vk
