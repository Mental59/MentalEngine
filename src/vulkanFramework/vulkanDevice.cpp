#include "vulkanDevice.hpp"
#include <vector>

namespace mental {
VkResult createDevice(VkPhysicalDevice physicalDevice,
                      VkPhysicalDeviceFeatures deviceFeatures,
                      uint32_t graphicsQueueFamily,
                      uint32_t enabledExtensionCount,
                      const char* const* enabledExtensions, VkDevice* device) {
  const float queuePriority = 1.0f;
  const VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = graphicsQueueFamily,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  const VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = enabledExtensionCount,
      .ppEnabledExtensionNames = enabledExtensions,
      .pEnabledFeatures = &deviceFeatures};

  return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, device);
}
} // namespace mental
