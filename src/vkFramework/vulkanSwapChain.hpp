#pragma once

#include <vector>
#include <volk.h>

namespace vkFramework {
struct VulkanRenderDevice;

struct SwapChainSupportDetails {
  VkSurfaceCapabilitiesKHR capabilities{};
  std::vector<VkSurfaceFormatKHR> formats;
  std::vector<VkPresentModeKHR> presentModes;
};

SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device,
                                              VkSurfaceKHR surface);
VkSurfaceFormatKHR chooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats);

VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes);

uint32_t chooseSwapImageCount(const VkSurfaceCapabilitiesKHR& capabilities);

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
                            int pixelWidth, int pixelHeight);

VkResult createSwapchain(VulkanRenderDevice& device, VkSurfaceKHR surface,
                         uint32_t graphicsFamily, uint32_t width,
                         uint32_t height, VkSwapchainKHR* swapchain);
size_t createSwapchainImages(VkDevice device, VkSwapchainKHR swapchain,
                             std::vector<VkImage>& swapchainImages,
                             std::vector<VkImageView>& swapchainImageViews);

} // namespace vkFramework
