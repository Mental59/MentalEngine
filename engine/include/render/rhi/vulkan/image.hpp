#pragma once
#include <Volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  struct SwapchainImageDesc
  {
    VkImage image;
    rhi::ImageExtent extent;
  };

  class Image : public IImage
  {
   public:
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
    virtual void destroy() override;

    void initSwapchainImage(const SwapchainImageDesc& desc);

   private:
    bool mShouldDestroyImage;
    VkImage mImage;
  };
}  // namespace mental::rhi::vk
