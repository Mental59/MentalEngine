#include <cstdint>
#include <render/rhi/vulkan/swapchain.hpp>
#include "core/log.hpp"
#include "core/types.hpp"
#include "render/rhi/vulkan/constants.hpp"
#include "render/rhi/vulkan/device.hpp"

mental::core::Result mental::rhi::vk::Swapchain::init(const mental::rhi::SwapchainDesc& desc)
{
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
  uint32_t minImageCount = desc.imageCount;
  if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
  {
    MENTAL_WARN(
        "Surface max image count is {}, but got {}, minImageCount is set to {}",
        surfaceCapabilities.maxImageCount,
        desc.imageCount,
        surfaceCapabilities.maxImageCount);
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  uint32_t graphicsQueueFamilyIndex = vk::getDevice()._getGraphicsQueue().getIndex();
  VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
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

  res = vkCreateSwapchainKHR(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mSwapchain);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateSwapchainKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mFormat = surfaceFormat;
  mPresentMode = presentMode;
  mExtent = extent;

  return core::Result::eSuccess;
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

void mental::rhi::vk::Swapchain::destroy()
{
  vkDestroySwapchainKHR(vk::getDevice().getVirtualDevice(), mSwapchain, nullptr);
}
