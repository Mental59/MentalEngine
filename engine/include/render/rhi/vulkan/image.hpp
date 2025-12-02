#pragma once
#include <vma/vk_mem_alloc.h>
#include <Volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  struct SwapchainImageDesc
  {
    VkImage image;
    rhi::ImageFormat format;
    rhi::ImageExtent extent;
    rhi::ImageUsageFlags usage;
  };

  class Image : public IImage
  {
   public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
    virtual void destroy() override;

    virtual core::Result init(const ImageDesc& desc) override;
    inline virtual const ImageDesc& getDesc() const override
    {
      return mDesc;
    }
    void initSwapchainImage(const SwapchainImageDesc& desc);

   private:
    VkFormat convertFormat(ImageFormat format);
    VkImageLayout convertLayout(ImageLayout layout);
    VkImageTiling convertTiling(ImageTiling tiling);
    VkImageUsageFlags convertUsageFlags(ImageUsageFlags usage);

    bool mShouldDestroyImage;
    VkImage mImage;
    VmaAllocation mAllocation;
    ImageDesc mDesc;
  };

  class ImageView : public IImageView
  {
   public:
    virtual void destroy() override;

   private:
    VkImageView mImageView;
  };
}  // namespace mental::rhi::vk
