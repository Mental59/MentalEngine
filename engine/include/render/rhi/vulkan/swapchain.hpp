#pragma once
#include <Volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  class Swapchain : public ISwapchain
  {
   public:
    virtual core::Result init(const SwapchainDesc& desc) override;
    virtual void destroy() override;

   private:
    VkSwapchainKHR mSwapchain;
  };
}  // namespace mental::rhi::vk
