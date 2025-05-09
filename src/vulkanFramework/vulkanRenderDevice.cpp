#include "vulkanRenderDevice.hpp"
#include "vulkanCommand.hpp"
#include "vulkanDevice.hpp"
#include "vulkanSwapChain.hpp"
#include "vulkanUtils.hpp"

bool mental::initVulkanRenderDevice(
    VulkanInstance& vulkanInstance, uint32_t width, uint32_t height,
    PhysicalDeviceSelectorFunction selector,
    QueueFamilySelectorFunction queueFamilySelector,
    VkPhysicalDeviceFeatures deviceFeatures, uint32_t enabledExtensionCount,
    const char* const* enabledExtensions,
    VulkanRenderDevice& vulkanRenderDevice) {
  vulkanRenderDevice.framebufferWidth = width;
  vulkanRenderDevice.framebufferHeight = height;

  VkResult findSuitableDeviceRes = findSuitablePhysicalDevice(
      vulkanInstance.instance, selector, &vulkanRenderDevice.physicalDevice);
  if (findSuitableDeviceRes != VK_SUCCESS) {
    return false;
  }

  int graphicsFamily = queueFamilySelector(vulkanRenderDevice.physicalDevice);
  if (graphicsFamily < 0) {
    return false;
  }
  vulkanRenderDevice.graphicsFamily = static_cast<uint32_t>(graphicsFamily);

  VkResult createDeviceRes =
      createDevice(vulkanRenderDevice.physicalDevice, deviceFeatures,
                   vulkanRenderDevice.graphicsFamily, enabledExtensionCount,
                   enabledExtensions, &vulkanRenderDevice.device);
  if (createDeviceRes != VK_SUCCESS) {
    return false;
  }

  vkGetDeviceQueue(vulkanRenderDevice.device, vulkanRenderDevice.graphicsFamily,
                   0, &vulkanRenderDevice.graphicsQueue);
  if (vulkanRenderDevice.graphicsQueue == nullptr) {
    return false;
  }

  VkResult crateSwapchainRes = createSwapchain(
      vulkanRenderDevice.device, vulkanRenderDevice.physicalDevice,
      vulkanInstance.surface, vulkanRenderDevice.graphicsFamily, width, height,
      &vulkanRenderDevice.swapchain);
  if (crateSwapchainRes != VK_SUCCESS) {
    return false;
  }

  const size_t imageCount = createSwapchainImages(
      vulkanRenderDevice.device, vulkanRenderDevice.swapchain,
      vulkanRenderDevice.swapchainImages,
      vulkanRenderDevice.swapchainImageViews);
  vulkanRenderDevice.commandBuffers.resize(imageCount);

  VkResult createSemaphoreRes =
      createSemaphore(vulkanRenderDevice.device, &vulkanRenderDevice.semaphore);
  VkResult createRenderSemaphore = createSemaphore(
      vulkanRenderDevice.device, &vulkanRenderDevice.renderSemaphore);
  if (createSemaphoreRes != VK_SUCCESS || createRenderSemaphore != VK_SUCCESS) {
    return false;
  }

  VkResult createCommandPoolRes = createCommandPool(
      vulkanRenderDevice.device, vulkanRenderDevice.graphicsFamily,
      &vulkanRenderDevice.commandPool);
  if (createCommandPoolRes != VK_SUCCESS) {
    return false;
  }

  uint32_t commandBufferCount =
      static_cast<uint32_t>(vulkanRenderDevice.swapchainImages.size());
  VkResult allocateCommandBuffersRes = allocateCommandBuffers(
      vulkanRenderDevice.device, vulkanRenderDevice.commandPool,
      commandBufferCount, vulkanRenderDevice.commandBuffers.data());

  return allocateCommandBuffersRes == VK_SUCCESS;
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
