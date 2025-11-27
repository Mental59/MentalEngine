#pragma once
#include <Volk/volk.h>
#include <render/rhi/rhi.hpp>
#include <vector>

namespace mental::rhi::vk
{
  class Swapchain : public ISwapchain
  {
   public:
    virtual core::Result init(const SwapchainDesc& desc) override;
    virtual void destroy() override;

    virtual core::Result
    acquireNextImage(uint64_t timeout, ISemaphore* signalSemaphore, IFence* signalFence, uint32_t& imageIndex) override;
    virtual uint32_t getImageCount() const override;
    virtual ITexture* getImage(uint32_t index) const override;
    virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

   private:
    VkSurfaceFormatKHR chooseSurfaceFormat() const;
    VkPresentModeKHR choosePresentMode(const SwapchainDesc& desc) const;

    VkSwapchainKHR mSwapchain;
    VkSurfaceFormatKHR mFormat;
    VkPresentModeKHR mPresentMode;
    VkExtent2D mExtent;
    std::vector<VkImage> mImages;
  };
}  // namespace mental::rhi::vk
