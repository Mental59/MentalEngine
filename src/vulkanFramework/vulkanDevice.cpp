#include "vulkanDevice.hpp"
#include <vector>

namespace {
void setupExtensions(std::vector<const char*>* extenstions);
} // namespace

namespace mental {
VkResult createDevice(VkPhysicalDevice physicalDevice,
                      VkPhysicalDeviceFeatures deviceFeatures,
                      uint32_t graphicsQueueFamily, VkDevice* device) {
  const float queuePriority = 1.0f;
  const VkDeviceQueueCreateInfo queueCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueFamilyIndex = graphicsQueueFamily,
      .queueCount = 1,
      .pQueuePriorities = &queuePriority};

  std::vector<const char*> extensions;
  setupExtensions(&extensions);

  const VkDeviceCreateInfo deviceCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .queueCreateInfoCount = 1,
      .pQueueCreateInfos = &queueCreateInfo,
      .enabledLayerCount = 0,
      .ppEnabledLayerNames = nullptr,
      .enabledExtensionCount = static_cast<uint32_t>(extensions.size()),
      .ppEnabledExtensionNames = extensions.data(),
      .pEnabledFeatures = &deviceFeatures};

  return vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, device);
}
} // namespace mental

namespace {
void setupExtensions(std::vector<const char*>* extenstions) {
  extenstions->push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}
} // namespace
