#pragma once
#include <Volk/volk.h>
#include <render/rhi/rhi.hpp>
#include <render/rhi/vulkan/texture.hpp>
#include <vector>

namespace mental::rhi::vk
{
class Swapchain : public ISwapchain
{
 public:
  virtual core::Result init(const SwapchainDesc& desc) override;
  virtual core::Result resize(uint32_t width, uint32_t height) override;
  virtual void destroy() override;

  virtual bool isValid() const override;

  virtual core::Result acquireNextTexture(
    uint64_t timeout, ISemaphore* signalSemaphore, IFence* signalFence, uint32_t& textureIndex) override;

  virtual core::Result present(uint32_t textureIndex, ISemaphore* waitSemaphore) override;
  virtual uint32_t getTextureCount() const override;
  virtual ITexture* getTexture(uint32_t index) override;
  virtual ITextureView* getTextureView(uint32_t index) override;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;

 private:
  core::Result createSwapchain(VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height);
  void destroyImageViewsAndWrappers();

  VkSurfaceFormatKHR chooseSurfaceFormat() const;
  VkPresentModeKHR choosePresentMode(const SwapchainDesc& desc) const;
  mental::rhi::TextureFormat surfaceFormatToTextureFormat(VkSurfaceFormatKHR surfaceFormat) const;

  SwapchainDesc mDesc {};
  VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
  VkSurfaceFormatKHR mFormat {};
  VkPresentModeKHR mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  VkExtent2D mExtent {};
  std::vector<vk::Texture> mTextures;
  std::vector<vk::TextureView> mTextureViews;
  bool mIsInit = false;
};
} // namespace mental::rhi::vk
