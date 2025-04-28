#pragma once

#include "vulkanInstance.hpp"
#include "vulkanPhysicalDevice.hpp"
#include <cstdint>
#include <vector>
#include <volk.h>

namespace mental {

struct VulkanRenderDevice final {
  uint32_t framebufferWidth = 0;
  uint32_t framebufferHeight = 0;

  VkDevice device = VK_NULL_HANDLE;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  uint32_t graphicsFamily = 0;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  VkSemaphore semaphore = VK_NULL_HANDLE;
  VkSemaphore renderSemaphore = VK_NULL_HANDLE;

  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;

  VkCommandPool commandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> commandBuffers;
};

bool initVulkanRenderDevice(VulkanInstance& vulkanInstance,
                            VulkanRenderDevice& vulkanRenderDevice,
                            uint32_t width, uint32_t height,
                            PhysicalDeviceSelectorFunction selector,
                            VkPhysicalDeviceFeatures deviceFeatures);

void destroyVulkanRenderDevice(VulkanRenderDevice& vkDev);
} // namespace mental
