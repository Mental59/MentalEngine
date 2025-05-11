#include "vulkanRenderDevice.hpp"
#include "vulkanCommand.hpp"
#include "vulkanDevice.hpp"
#include "vulkanSwapChain.hpp"
#include "vulkanUtils.hpp"

bool vkFramework::initVulkanRenderDevice(
    VulkanInstance& vulkanInstance, uint32_t width, uint32_t height,
    PhysicalDeviceSelectorFunction selector,
    QueueFamilySelectorFunction queueFamilySelector,
    VkPhysicalDeviceFeatures deviceFeatures, uint32_t enabledExtensionCount,
    const char* const* enabledExtensions,
    VulkanRenderDevice& vulkanRenderDevice) {
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

  VkResult crateSwapchainRes =
      createSwapchain(vulkanRenderDevice, vulkanInstance.surface,
                      vulkanRenderDevice.graphicsFamily, width, height,
                      &vulkanRenderDevice.swapchain);
  if (crateSwapchainRes != VK_SUCCESS) {
    return false;
  }

  const size_t imageCount = createSwapchainImages(
      vulkanRenderDevice.device, vulkanRenderDevice.swapchain,
      vulkanRenderDevice.swapchainImages,
      vulkanRenderDevice.swapchainImageViews);
  vulkanRenderDevice.swapchainImageCount = imageCount;
  vulkanRenderDevice.maxFramesInFlight = imageCount - 1;

  vulkanRenderDevice.commandBuffers.resize(
      vulkanRenderDevice.maxFramesInFlight);

  VkResult createSyncObjectsRes = createSyncObjects(vulkanRenderDevice);
  if (createSyncObjectsRes != VK_SUCCESS) {
    return false;
  }

  VkResult createCommandPoolRes = createCommandPool(
      vulkanRenderDevice.device, vulkanRenderDevice.graphicsFamily,
      &vulkanRenderDevice.commandPool);
  if (createCommandPoolRes != VK_SUCCESS) {
    return false;
  }

  VkResult allocateCommandBuffersRes = allocateCommandBuffers(
      vulkanRenderDevice.device, vulkanRenderDevice.commandPool,
      vulkanRenderDevice.maxFramesInFlight,
      vulkanRenderDevice.commandBuffers.data());

  return allocateCommandBuffersRes == VK_SUCCESS;
}

void vkFramework::destroyVulkanRenderDevice(VulkanRenderDevice& vkDev) {
  for (size_t i = 0; i < vkDev.swapchainImages.size(); i++) {
    vkDestroyImageView(vkDev.device, vkDev.swapchainImageViews[i], nullptr);
  }

  vkDestroySwapchainKHR(vkDev.device, vkDev.swapchain, nullptr);

  vkDestroyCommandPool(vkDev.device, vkDev.commandPool, nullptr);

  for (size_t i = 0; i < vkDev.swapchainImageSemaphores.size(); i++) {
    vkDestroySemaphore(vkDev.device, vkDev.swapchainImageSemaphores[i],
                       nullptr);
  }

  for (size_t i = 0; i < vkDev.renderSemaphores.size(); i++) {
    vkDestroySemaphore(vkDev.device, vkDev.renderSemaphores[i], nullptr);
  }

  for (size_t i = 0; i < vkDev.inflightFences.size(); i++) {
    vkDestroyFence(vkDev.device, vkDev.inflightFences[i], nullptr);
  }

  vkDestroyDevice(vkDev.device, nullptr);
}
