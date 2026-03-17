#include <cstdint>
#include <render/rhi/vulkan/swapchain.hpp>
#include "core/log.hpp"
#include "core/types.hpp"
#include "render/rhi/vulkan/constants.hpp"
#include "render/rhi/vulkan/device.hpp"
#include "render/rhi/rhi.hpp"

mental::core::Result mental::rhi::vk::Swapchain::init(const mental::rhi::SwapchainDesc& desc)
{
  if (mIsInit)
  {
    MENTAL_INFO("Trying to initialize an already initialized vk::Swapchain");
    return core::Result::eInitializationFailed;
  }

  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      vk::getDevice().getPhysicalDevice(), vk::getDevice().getSurface(), &surfaceCapabilities);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkGetPhysicalDeviceSurfaceCapabilitiesKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat();
  VkPresentModeKHR presentMode = choosePresentMode(desc);
  VkExtent2D extent = surfaceCapabilities.currentExtent;
  uint32_t minImageCount = desc.textureCount;
  if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
  {
    MENTAL_WARN(
        "Surface max image count is {}, but got {}, minImageCount is set to {}",
        surfaceCapabilities.maxImageCount,
        desc.textureCount,
        surfaceCapabilities.maxImageCount);
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  uint32_t graphicsQueueFamilyIndex = vk::getDevice()._getGraphicsQueue().getIndex();
  VkImageUsageFlags imageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

  VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  createInfo.minImageCount = minImageCount;
  createInfo.surface = vk::getDevice().getSurface();
  createInfo.imageFormat = surfaceFormat.format;
  createInfo.imageColorSpace = surfaceFormat.colorSpace;
  createInfo.imageExtent = extent;
  createInfo.imageArrayLayers = 1;
  createInfo.imageUsage = imageUsageFlags;
  createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  createInfo.queueFamilyIndexCount = 1;
  createInfo.pQueueFamilyIndices = &graphicsQueueFamilyIndex;
  createInfo.preTransform = surfaceCapabilities.currentTransform;
  createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  createInfo.presentMode = presentMode;
  createInfo.clipped = VK_TRUE;

  VkDevice device = vk::getDevice().getVirtualDevice();
  res = vkCreateSwapchainKHR(device, &createInfo, nullptr, &mSwapchain);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateSwapchainKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mFormat = surfaceFormat;
  mPresentMode = presentMode;
  mExtent = extent;

  uint32_t swapchainImageCount;
  vkGetSwapchainImagesKHR(device, mSwapchain, &swapchainImageCount, nullptr);
  std::vector<VkImage> vulkanImages(swapchainImageCount);
  vkGetSwapchainImagesKHR(device, mSwapchain, &swapchainImageCount, vulkanImages.data());

  mTextures.resize(swapchainImageCount);
  mTextureViews.resize(swapchainImageCount);
  for (uint32_t i = 0; i < swapchainImageCount; i++)
  {
    SwapchainTextureDesc desc{};
    desc.image = vulkanImages[i];
    desc.extent = { .width = extent.width, .height = extent.height, .depth = 1 };
    desc.format = surfaceFormatToTextureFormat(surfaceFormat);
    desc.usage = rhi::TextureUsageFlagBits::eTextureUsageColorAttachmentBit;
    mTextures[i].initSwapchainTexture(desc);

    TextureViewDesc viewDesc{};
    viewDesc.texture = &mTextures[i];
    viewDesc.type = TextureType::eTexture2D;
    mTextureViews[i].init(viewDesc);
  }

  mIsInit = true;
  return core::Result::eSuccess;
}

void mental::rhi::vk::Swapchain::destroy()
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to destroy uninitialized vk::Swapchain");
    return;
  }

  vkDestroySwapchainKHR(vk::getDevice().getVirtualDevice(), mSwapchain, nullptr);
  for (TextureView& view : mTextureViews)
  {
    view.destroy();
  }
  for (Texture& texture : mTextures)
  {
    texture.destroy();
  }
  mIsInit = false;
}

bool mental::rhi::vk::Swapchain::isValid() const
{
  return false;
}

mental::core::Result mental::rhi::vk::Swapchain::acquireNextTexture(
    uint64_t timeout,
    mental::rhi::ISemaphore* signalSemaphore,
    mental::rhi::IFence* signalFence,
    uint32_t& textureIndex)
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
    case VK_SUCCESS: return core::Result::eSuccess;
    case VK_SUBOPTIMAL_KHR: return core::Result::eSuboptimal;
    case VK_ERROR_OUT_OF_DATE_KHR: return core::Result::eOutOfDate;
    case VK_NOT_READY: return core::Result::eNotReady;
    case VK_TIMEOUT: return core::Result::eTimeout;
    default: return core::Result::eOperationFailed;
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

mental::core::resource::Object mental::rhi::vk::Swapchain::getNativeObject(mental::core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkSwapchainKHR: return mSwapchain;
    default: return nullptr;
  }
}

VkSurfaceFormatKHR mental::rhi::vk::Swapchain::chooseSurfaceFormat() const
{
  const auto& availableFormats = vk::getDevice().getSurfaceFormats();
  MENTAL_ASSERT_MESSAGE(availableFormats.size() != 0, "No surface formats available")

  VkSurfaceFormatKHR desiredFormats[4];
  desiredFormats[0] = { VK_FORMAT_R8G8B8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  desiredFormats[1] = { VK_FORMAT_B8G8R8A8_SRGB, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  desiredFormats[2] = { VK_FORMAT_R8G8B8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
  desiredFormats[3] = { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

  for (VkSurfaceFormatKHR desiredFormat : desiredFormats)
  {
    for (VkSurfaceFormatKHR availableFormat : availableFormats)
    {
      if (desiredFormat.format == availableFormat.format && desiredFormat.colorSpace == availableFormat.colorSpace)
      {
        MENTAL_INFO(
            "Found desired surface format: format={}, colorSpace={}",
            static_cast<uint32_t>(desiredFormat.format),
            static_cast<uint32_t>(desiredFormat.colorSpace));
        return desiredFormat;
      }
    }
  }

  MENTAL_WARN(
      "Desired surface format not found, falling back to format={}, colorSpace={}",
      static_cast<uint32_t>(availableFormats[0].format),
      static_cast<uint32_t>(availableFormats[0].colorSpace));
  return availableFormats[0];
}

mental::rhi::TextureFormat mental::rhi::vk::Swapchain::surfaceFormatToTextureFormat(VkSurfaceFormatKHR surfaceFormat) const
{
  switch (surfaceFormat.format)
  {
    case VK_FORMAT_R8G8B8A8_SRGB: return mental::rhi::TextureFormat::eRGBA32_SRGB;
    case VK_FORMAT_B8G8R8A8_SRGB: return mental::rhi::TextureFormat::eBGRA32_SRGB;
    case VK_FORMAT_R8G8B8A8_UNORM: return mental::rhi::TextureFormat::eRGBA32_UNORM;
    case VK_FORMAT_B8G8R8A8_UNORM: return mental::rhi::TextureFormat::eBGRA32_UNORM;

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
