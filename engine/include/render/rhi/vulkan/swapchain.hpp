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
  VkSurfaceFormatKHR chooseSurfaceFormat() const;
  VkPresentModeKHR choosePresentMode(const SwapchainDesc& desc) const;
  mental::rhi::TextureFormat surfaceFormatToTextureFormat(VkSurfaceFormatKHR surfaceFormat) const;

  VkSwapchainKHR mSwapchain;
  VkSurfaceFormatKHR mFormat;
  VkPresentModeKHR mPresentMode;
  VkExtent2D mExtent;
  std::vector<vk::Texture> mTextures;
  std::vector<vk::TextureView> mTextureViews;
  bool mIsInit = false;
};
} // namespace mental::rhi::vk
