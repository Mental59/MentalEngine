#include "vulkanRenderDevice.hpp"
#include "vulkanCommand.hpp"
#include "vulkanDevice.hpp"
#include "vulkanSwapChain.hpp"
#include "vulkanUtils.hpp"

bool mental::initVulkanRenderDevice(VulkanInstance& vulkanInstance,
                                    VulkanRenderDevice& vulkanRenderDevice,
                                    uint32_t width, uint32_t height,
                                    PhysicalDeviceSelectorFunction selector,
                                    VkPhysicalDeviceFeatures deviceFeatures) {
  vulkanRenderDevice.framebufferWidth = width;
  vulkanRenderDevice.framebufferHeight = height;

  MENTAL_VK_CHECK(findSuitablePhysicalDevice(
      vulkanInstance.instance, selector, &vulkanRenderDevice.physicalDevice));

  int graphicsFamily = findQueueFamiliesWithPresentSupport(
      vulkanRenderDevice.physicalDevice, VK_QUEUE_GRAPHICS_BIT,
      vulkanInstance.surface);
  if (graphicsFamily < 0) {
    MENTAL_VK_CHECK_BOOL(false);
  }
  vulkanRenderDevice.graphicsFamily = static_cast<uint32_t>(graphicsFamily);

  MENTAL_VK_CHECK(createDevice(
      vulkanRenderDevice.physicalDevice, deviceFeatures,
      vulkanRenderDevice.graphicsFamily, &vulkanRenderDevice.device));

  vkGetDeviceQueue(vulkanRenderDevice.device, vulkanRenderDevice.graphicsFamily,
                   0, &vulkanRenderDevice.graphicsQueue);
  if (vulkanRenderDevice.graphicsQueue == nullptr) {
    MENTAL_VK_CHECK_BOOL(false);
  }

  MENTAL_VK_CHECK(createSwapchain(
      vulkanRenderDevice.device, vulkanRenderDevice.physicalDevice,
      vulkanInstance.surface, vulkanRenderDevice.graphicsFamily, width, height,
      &vulkanRenderDevice.swapchain));
  const size_t imageCount = createSwapchainImages(
      vulkanRenderDevice.device, vulkanRenderDevice.swapchain,
      vulkanRenderDevice.swapchainImages,
      vulkanRenderDevice.swapchainImageViews);
  vulkanRenderDevice.commandBuffers.resize(imageCount);

  MENTAL_VK_CHECK(createSemaphore(vulkanRenderDevice.device,
                                  &vulkanRenderDevice.semaphore));
  MENTAL_VK_CHECK(createSemaphore(vulkanRenderDevice.device,
                                  &vulkanRenderDevice.renderSemaphore));

  MENTAL_VK_CHECK(createCommandPool(vulkanRenderDevice.device,
                                    vulkanRenderDevice.graphicsFamily,
                                    &vulkanRenderDevice.commandPool))

  uint32_t commandBufferCount =
      static_cast<uint32_t>(vulkanRenderDevice.swapchainImages.size());
  MENTAL_VK_CHECK(allocateCommandBuffers(
      vulkanRenderDevice.device, vulkanRenderDevice.commandPool,
      commandBufferCount, vulkanRenderDevice.commandBuffers.data()));

  return true;
}

void mental::destroyVulkanRenderDevice(VulkanRenderDevice& vkDev) {
  for (size_t i = 0; i < vkDev.swapchainImages.size(); i++) {
    vkDestroyImageView(vkDev.device, vkDev.swapchainImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(vkDev.device, vkDev.swapchain, nullptr);

  vkDestroyCommandPool(vkDev.device, vkDev.commandPool, nullptr);

  vkDestroySemaphore(vkDev.device, vkDev.semaphore, nullptr);
  vkDestroySemaphore(vkDev.device, vkDev.renderSemaphore, nullptr);

  vkDestroyDevice(vkDev.device, nullptr);
}
