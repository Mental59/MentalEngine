#include "vulkanRenderDevice.hpp"
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

  const VkCommandPoolCreateInfo commanPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = 0,
      .queueFamilyIndex = vulkanRenderDevice.graphicsFamily};

  MENTAL_VK_CHECK(vkCreateCommandPool(vulkanRenderDevice.device,
                                      &commanPoolCreateInfo, nullptr,
                                      &vulkanRenderDevice.commandPool));

  const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = vulkanRenderDevice.commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount =
          static_cast<uint32_t>(vulkanRenderDevice.swapchainImages.size()),
  };

  MENTAL_VK_CHECK(vkAllocateCommandBuffers(
      vulkanRenderDevice.device, &commandBufferAllocateInfo,
      &vulkanRenderDevice.commandBuffers[0]));

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
