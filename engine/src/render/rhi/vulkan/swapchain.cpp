#include <algorithm>
#include <cstdint>
#include <limits>
#include <render/rhi/vulkan/swapchain.hpp>
#include "core/log.hpp"
#include "core/types.hpp"
#include "render/rhi/vulkan/constants.hpp"
#include "render/rhi/vulkan/device.hpp"
#include "render/rhi/rhi.hpp"

namespace
{
class SwapchainResourceSet
{
 public:
  mental::core::Result init(
    VkDevice device, VkSwapchainKHR swapchain, VkExtent2D extent, mental::rhi::TextureFormat format)
  {
    uint32_t imageCount = 0;
    VkResult res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    if (res != VK_SUCCESS)
    {
      MENTAL_ERROR("Failed to query swapchain image count, error: {}", mental::rhi::vk::vkResultToString(res));
      return mental::core::Result::eInitializationFailed;
    }

    mImages.resize(imageCount);
    res = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, mImages.data());
    if (res != VK_SUCCESS)
    {
      MENTAL_ERROR("Failed to query swapchain images, error: {}", mental::rhi::vk::vkResultToString(res));
      return mental::core::Result::eInitializationFailed;
    }

    mTextures.resize(imageCount);
    mTextureViews.resize(imageCount);
    for (uint32_t i = 0; i < imageCount; ++i)
    {
      mental::rhi::vk::SwapchainTextureDesc swapchainTextureDesc {};
      swapchainTextureDesc.image = mImages[i];
      swapchainTextureDesc.extent = {.width = extent.width, .height = extent.height, .depth = 1};
      swapchainTextureDesc.format = format;
      swapchainTextureDesc.usage = mental::rhi::TextureUsageFlagBits::eTextureUsageColorAttachmentBit;
      mTextures[i].initSwapchainTexture(swapchainTextureDesc);

      mental::rhi::TextureViewDesc viewDesc {};
      viewDesc.texture = &mTextures[i];
      viewDesc.type = mental::rhi::TextureType::eTexture2D;
      mental::core::Result viewRes = mTextureViews[i].init(viewDesc);
      if (viewRes != mental::core::Result::eSuccess)
      {
        destroy();
        return viewRes;
      }
    }

    return mental::core::Result::eSuccess;
  }

  void destroy()
  {
    for (mental::rhi::vk::TextureView& view : mTextureViews)
    {
      if (view.isValid())
      {
        view.destroy();
      }
    }
    for (mental::rhi::vk::Texture& texture : mTextures)
    {
      if (texture.isValid())
      {
        texture.destroy();
      }
    }
    mTextureViews.clear();
    mTextures.clear();
    mImages.clear();
  }

  std::vector<mental::rhi::vk::Texture> releaseTextures()
  {
    return std::move(mTextures);
  }

  std::vector<mental::rhi::vk::TextureView> releaseTextureViews()
  {
    return std::move(mTextureViews);
  }

 private:
  std::vector<VkImage> mImages;
  std::vector<mental::rhi::vk::Texture> mTextures;
  std::vector<mental::rhi::vk::TextureView> mTextureViews;
};

VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, uint32_t width, uint32_t height)
{
  if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
  {
    return surfaceCapabilities.currentExtent;
  }

  if (width == 0 || height == 0)
  {
    MENTAL_WARN("Swapchain extent was not provided, falling back to surface minimum extent");
    return surfaceCapabilities.minImageExtent;
  }

  VkExtent2D extent {};
  extent.width = std::clamp(width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
  extent.height =
    std::clamp(height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
  return extent;
}
} // namespace

mental::core::Result mental::rhi::vk::Swapchain::init(const mental::rhi::SwapchainDesc& desc)
{
  if (mIsInit)
  {
    MENTAL_INFO("Trying to initialize an already initialized vk::Swapchain");
    return core::Result::eInitializationFailed;
  }

  mDesc = desc;
  core::Result res = createSwapchain(VK_NULL_HANDLE, 0, 0);
  if (res != core::Result::eSuccess)
  {
    return res;
  }

  mIsInit = true;
  return core::Result::eSuccess;
}

mental::core::Result mental::rhi::vk::Swapchain::resize(uint32_t width, uint32_t height)
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to resize an uninitialized vk::Swapchain");
    return core::Result::eOperationFailed;
  }

  if (width == 0 || height == 0)
  {
    return core::Result::eOutOfDate;
  }

  vk::getDevice()._getGraphicsQueue().waitIdle();
  return createSwapchain(mSwapchain, width, height);
}

void mental::rhi::vk::Swapchain::destroy()
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to destroy uninitialized vk::Swapchain");
    return;
  }

  destroyImageViewsAndWrappers();
  vkDestroySwapchainKHR(vk::getDevice().getVirtualDevice(), mSwapchain, nullptr);
  mSwapchain = VK_NULL_HANDLE;
  mFormat = {};
  mPresentMode = VK_PRESENT_MODE_FIFO_KHR;
  mExtent = {};
  mDesc = {};
  mIsInit = false;
}

mental::core::Result mental::rhi::vk::Swapchain::createSwapchain(
  VkSwapchainKHR oldSwapchain, uint32_t width, uint32_t height)
{
  VkSurfaceCapabilitiesKHR surfaceCapabilities {};
  VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
    vk::getDevice().getPhysicalDevice(), vk::getDevice().getSurface(), &surfaceCapabilities);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkGetPhysicalDeviceSurfaceCapabilitiesKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat();
  VkPresentModeKHR presentMode = choosePresentMode(mDesc);
  VkExtent2D extent = chooseSwapchainExtent(surfaceCapabilities, width, height);

  uint32_t minImageCount = std::max(surfaceCapabilities.minImageCount, mDesc.textureCount);
  if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
  {
    MENTAL_WARN("Surface max image count is {}, but got {}, minImageCount is set to {}",
      surfaceCapabilities.maxImageCount,
      mDesc.textureCount,
      surfaceCapabilities.maxImageCount);
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  uint32_t graphicsQueueFamilyIndex = vk::getDevice()._getGraphicsQueue().getIndex();
  VkSwapchainCreateInfoKHR createInfo {VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
  createInfo.minImageCount = minImageCount;
  createInfo.surface = vk::getDevice().getSurface();
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 1;
  createInfo.pQueueFamilyIndices = &graphicsQueueFamilyIndex;
  createInfo.preTransform = surfaceCapabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;
  createInfo.oldSwapchain = oldSwapchain;

  VkDevice device = vk::getDevice().getVirtualDevice();
  VkSwapchainKHR newSwapchain = VK_NULL_HANDLE;
  res = vkCreateSwapchainKHR(device, &createInfo, nullptr, &newSwapchain);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateSwapchainKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  SwapchainResourceSet resourceSet;
  core::Result resourceInitRes =
    resourceSet.init(device, newSwapchain, extent, surfaceFormatToTextureFormat(surfaceFormat));
  if (resourceInitRes != core::Result::eSuccess)
  {
    vkDestroySwapchainKHR(device, newSwapchain, nullptr);
    return resourceInitRes;
  }

  destroyImageViewsAndWrappers();
  if (oldSwapchain != VK_NULL_HANDLE)
  {
    vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
  }

  mSwapchain = newSwapchain;
  mFormat = surfaceFormat;
  mPresentMode = presentMode;
  mExtent = extent;
  mTextures = resourceSet.releaseTextures();
  mTextureViews = resourceSet.releaseTextureViews();

  return core::Result::eSuccess;
}

void mental::rhi::vk::Swapchain::destroyImageViewsAndWrappers()
{
  for (TextureView& view : mTextureViews)
  {
    if (view.isValid())
    {
      view.destroy();
    }
  }
  for (Texture& texture : mTextures)
  {
    if (texture.isValid())
    {
      texture.destroy();
    }
  }
  mTextureViews.clear();
  mTextures.clear();
}

bool mental::rhi::vk::Swapchain::isValid() const
{
  return mIsInit;
}

mental::core::Result mental::rhi::vk::Swapchain::acquireNextTexture(
  uint64_t timeout, mental::rhi::ISemaphore* signalSemaphore, mental::rhi::IFence* signalFence, uint32_t& textureIndex)
{
  VkDevice device = vk::getDevice().getVirtualDevice();

  VkSemaphore semaphore = VK_NULL_HANDLE;
  if (signalSemaphore)
    semaphore = signalSemaphore->getNativeObject(core::resource::ObjectType::eVkSemaphore);

  VkFence fence = VK_NULL_HANDLE;
  if (signalFence)
    fence = signalFence->getNativeObject(core::resource::ObjectType::eVkFence);

  VkResult res = vkAcquireNextImageKHR(device, mSwapchain, timeout, semaphore, fence, &textureIndex);
  switch (res)
  {
    case VK_SUCCESS:
      return core::Result::eSuccess;
    case VK_SUBOPTIMAL_KHR:
      return core::Result::eSuboptimal;
    case VK_ERROR_OUT_OF_DATE_KHR:
      return core::Result::eOutOfDate;
    case VK_NOT_READY:
      return core::Result::eNotReady;
    case VK_TIMEOUT:
      return core::Result::eTimeout;
    default:
      return core::Result::eOperationFailed;
  }
}

mental::core::Result mental::rhi::vk::Swapchain::present(uint32_t textureIndex, mental::rhi::ISemaphore* waitSemaphore)
{
  MENTAL_ASSERT_DEBUG(textureIndex < mTextures.size());

  VkSemaphore vkWaitSemaphore = VK_NULL_HANDLE;
  if (waitSemaphore)
  {
    vkWaitSemaphore = waitSemaphore->getNativeObject(core::resource::ObjectType::eVkSemaphore);
  }

  VkSwapchainKHR swapchain = mSwapchain;
  VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
  presentInfo.swapchainCount = 1;
  presentInfo.pSwapchains = &swapchain;
  presentInfo.pImageIndices = &textureIndex;

  if (vkWaitSemaphore != VK_NULL_HANDLE)
  {
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vkWaitSemaphore;
  }

  VkQueue queue = vk::getDevice()._getGraphicsQueue().getNativeObject(core::resource::ObjectType::eVkQueue);
  VkResult res = vkQueuePresentKHR(queue, &presentInfo);
  switch (res)
  {
    case VK_SUCCESS:
      return core::Result::eSuccess;
    case VK_SUBOPTIMAL_KHR:
      return core::Result::eSuboptimal;
    case VK_ERROR_OUT_OF_DATE_KHR:
      return core::Result::eOutOfDate;
    default:
      return core::Result::eOperationFailed;
  }
}

uint32_t mental::rhi::vk::Swapchain::getTextureCount() const
{
  return static_cast<uint32_t>(mTextures.size());
}

mental::rhi::ITexture* mental::rhi::vk::Swapchain::getTexture(uint32_t index)
{
  MENTAL_ASSERT_DEBUG(index < mTextures.size());
  return &mTextures[index];
}

mental::rhi::ITextureView* mental::rhi::vk::Swapchain::getTextureView(uint32_t index)
{
  MENTAL_ASSERT_DEBUG(index < mTextureViews.size());
  return &mTextureViews[index];
}

mental::core::resource::Object mental::rhi::vk::Swapchain::getNativeObject(
  mental::core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkSwapchainKHR:
      return mSwapchain;
    default:
      return nullptr;
  }
}

VkSurfaceFormatKHR mental::rhi::vk::Swapchain::chooseSurfaceFormat() const
{
  const auto& availableFormats = vk::getDevice().getSurfaceFormats();
  MENTAL_ASSERT_MESSAGE(availableFormats.size() != 0, "No surface formats available")

  VkSurfaceFormatKHR desiredFormats[4];
  desiredFormats[0] = {VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  desiredFormats[1] = {VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  desiredFormats[2] = {VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
  desiredFormats[3] = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

  for (VkSurfaceFormatKHR desiredFormat : desiredFormats)
  {
    for (VkSurfaceFormatKHR availableFormat : availableFormats)
    {
      if (desiredFormat.format == availableFormat.format && desiredFormat.colorSpace == availableFormat.colorSpace)
      {
        MENTAL_INFO("Found desired surface format: format={}, colorSpace={}",
          static_cast<uint32_t>(desiredFormat.format),
          static_cast<uint32_t>(desiredFormat.colorSpace));
        return desiredFormat;
      }
    }
  }

  MENTAL_WARN("Desired surface format not found, falling back to format={}, colorSpace={}",
    static_cast<uint32_t>(availableFormats[0].format),
    static_cast<uint32_t>(availableFormats[0].colorSpace));
  return availableFormats[0];
}

mental::rhi::TextureFormat mental::rhi::vk::Swapchain::surfaceFormatToTextureFormat(
  VkSurfaceFormatKHR surfaceFormat) const
{
  switch (surfaceFormat.format)
  {
    case VK_FORMAT_R8G8B8A8_SRGB:
      return mental::rhi::TextureFormat::eRGBA32_SRGB;
    case VK_FORMAT_B8G8R8A8_SRGB:
      return mental::rhi::TextureFormat::eBGRA32_SRGB;
    case VK_FORMAT_R8G8B8A8_UNORM:
      return mental::rhi::TextureFormat::eRGBA32_UNORM;
    case VK_FORMAT_B8G8R8A8_UNORM:
      return mental::rhi::TextureFormat::eBGRA32_UNORM;

    default:
    {
      MENTAL_ASSERT_MESSAGE(false, "Failed to convert surface format");
      return mental::rhi::TextureFormat::eRGBA32_SRGB;
    }
  }
}

VkPresentModeKHR mental::rhi::vk::Swapchain::choosePresentMode(const SwapchainDesc& desc) const
{
  VkPresentModeKHR desiredPresentMode =
    desc.enableVerticalSync ? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR;

  const auto& availablePresentModes = vk::getDevice().getPresentModes();
  for (VkPresentModeKHR mode : availablePresentModes)
  {
    if (mode == desiredPresentMode)
    {
      return desiredPresentMode;
    }
  }

  MENTAL_WARN("Desired present mode not found, falling back to VK_PRESENT_MODE_FIFO_KHR");
  return VK_PRESENT_MODE_FIFO_KHR;
}
