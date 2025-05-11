#pragma once

#include "vulkanInstance.hpp"
#include "vulkanPhysicalDevice.hpp"
#include <cstdint>
#include <functional>
#include <vector>
#include <volk.h>

namespace vkFramework {
using QueueFamilySelectorFunction = std::function<int(VkPhysicalDevice)>;

struct VulkanRenderDevice final {
  uint32_t swapchainImageCount = 0;
  uint32_t maxFramesInFlight = 0;

  VkDevice device = VK_NULL_HANDLE;
  VkQueue graphicsQueue = VK_NULL_HANDLE;
  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  uint32_t graphicsFamily = 0;

  VkSwapchainKHR swapchain = VK_NULL_HANDLE;

  std::vector<VkSemaphore> swapchainImageSemaphores;
  std::vector<VkSemaphore> renderSemaphores;
  std::vector<VkFence> inflightFences;

  std::vector<VkImage> swapchainImages;
  std::vector<VkImageView> swapchainImageViews;
  std::vector<VkFramebuffer> swapchainFramebuffers;

  VkCommandPool commandPool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> commandBuffers;

  VkExtent2D swapchainExtent;
};

bool initVulkanRenderDevice(VulkanInstance& vulkanInstance, uint32_t width,
                            uint32_t height,
                            PhysicalDeviceSelectorFunction selector,
                            QueueFamilySelectorFunction queueFamilySelector,
                            VkPhysicalDeviceFeatures deviceFeatures,
                            uint32_t enabledExtensionCount,
                            const char* const* enabledExtensions,
                            VulkanRenderDevice& vulkanRenderDevice);

void destroyVulkanRenderDevice(VulkanRenderDevice& vkDev);
} // namespace vkFramework
