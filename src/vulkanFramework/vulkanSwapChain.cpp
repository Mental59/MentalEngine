#include "vulkanSwapChain.hpp"
#include "vulkanImage.hpp"
#include "vulkanRenderDevice.hpp"
#include "vulkanUtils.hpp"
#include <algorithm>

namespace mental {
SwapChainSupportDetails mental::querySwapChainSupport(VkPhysicalDevice device,
                                                      VkSurfaceKHR surface) {
  SwapChainSupportDetails details;

  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
                                            &details.capabilities);

  uint32_t formatCount = 0;
  vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

  if (formatCount != 0) {
    details.formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount,
                                         details.formats.data());
  }

  uint32_t presentModeCount;
  vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount,
                                            nullptr);

  if (presentModeCount != 0) {
    details.presentModes.resize(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(
        device, surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}

VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
  for (const VkSurfaceFormatKHR availableFormat : availableFormats) {
    if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
        availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
      return availableFormat;
    }
  }

  return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
}

VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) {
  for (const VkPresentModeKHR availablePresentMode : availablePresentModes) {
    if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
      return availablePresentMode;
    }
  }

  return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities) {
  const uint32_t imageCount = capabilities.minImageCount + 1;

  const bool imageCountExceeded =
      capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount;

  return imageCountExceeded ? capabilities.maxImageCount : imageCount;
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            int pixelWidth, int pixelHeight) {
  if (capabilities.currentExtent.width !=
      std::numeric_limits<uint32_t>::max()) {
    return capabilities.currentExtent;
  } else {
    VkExtent2D actualExtent{static_cast<uint32_t>(pixelWidth),
                            static_cast<uint32_t>(pixelHeight)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
  }
}
VkResult createSwapchain(VulkanRenderDevice& device, VkSurfaceKHR surface,
                         uint32_t graphicsFamily, uint32_t width,
                         uint32_t height, VkSwapchainKHR* swapchain) {
  SwapChainSupportDetails swapchainSupport =
      querySwapChainSupport(device.physicalDevice, surface);
  VkSurfaceFormatKHR surfaceFormat =
      chooseSwapSurfaceFormat(swapchainSupport.formats);
  VkPresentModeKHR presentMode =
      chooseSwapPresentMode(swapchainSupport.presentModes);
  VkExtent2D extent =
      chooseSwapExtent(swapchainSupport.capabilities, width, height);
  uint32_t minImageCount = chooseSwapImageCount(swapchainSupport.capabilities);

  VkSwapchainKHR oldSwapchain = device.swapchain;
  const VkSwapchainCreateInfoKHR createSwapchainInfo = {
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .flags = 0,
      .surface = surface,
      .minImageCount = minImageCount,
      .imageFormat = surfaceFormat.format,
      .imageColorSpace = surfaceFormat.colorSpace,
      .imageExtent = extent,
      .imageArrayLayers = 1,
      .imageUsage =
          VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .queueFamilyIndexCount = 1,
      .pQueueFamilyIndices = &graphicsFamily,
      .preTransform = swapchainSupport.capabilities.currentTransform,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = presentMode,
      .clipped = VK_TRUE,
      .oldSwapchain = oldSwapchain};

  VkResult result = vkCreateSwapchainKHR(device.device, &createSwapchainInfo,
                                         nullptr, swapchain);

  device.swapchainExtent = extent;

  return result;
}
size_t createSwapchainImages(VkDevice device, VkSwapchainKHR swapchain,
                             std::vector<VkImage>& swapchainImages,
                             std::vector<VkImageView>& swapchainImageViews) {
  uint32_t imageCount = 0;
  MENTAL_VK_CHECK(
      vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr));

  swapchainImages.resize(imageCount);
  swapchainImageViews.resize(imageCount);

  MENTAL_VK_CHECK(vkGetSwapchainImagesKHR(device, swapchain, &imageCount,
                                          swapchainImages.data()));

  for (unsigned i = 0; i < imageCount; i++) {
    MENTAL_VK_CHECK(
        createImageView(device, swapchainImages[i], VK_FORMAT_B8G8R8A8_UNORM,
                        VK_IMAGE_ASPECT_COLOR_BIT, &swapchainImageViews[i]));
  }

  return static_cast<size_t>(imageCount);
}
} // namespace mental
