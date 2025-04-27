#pragma once

#include <volk.h>

namespace mental {
VkResult createDevice(VkPhysicalDevice physicalDevice,
                      VkPhysicalDeviceFeatures deviceFeatures,
                      uint32_t graphicsQueueFamily, VkDevice* device);
}