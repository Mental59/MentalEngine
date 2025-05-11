#pragma once

#include <volk.h>

namespace vkFramework {
VkResult createDevice(VkPhysicalDevice physicalDevice,
                      VkPhysicalDeviceFeatures deviceFeatures,
                      uint32_t graphicsQueueFamily,
                      uint32_t enabledExtensionCount,
                      const char* const* enabledExtensions, VkDevice* device);
}