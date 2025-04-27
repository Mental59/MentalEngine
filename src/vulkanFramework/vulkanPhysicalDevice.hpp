#pragma once

#include <functional>
#include <volk.h>

namespace mental {
VkResult
findSuitablePhysicalDevice(VkInstance instance,
                           std::function<bool(VkPhysicalDevice)> selector,
                           VkPhysicalDevice* physicalDevice);

uint32_t findQueueFamilies(VkPhysicalDevice physicalDevice,
                           VkQueueFlags desiredFlags);

uint32_t findQueueFamiliesWithPresentSupport(VkPhysicalDevice physicalDevice,
                                             VkQueueFlags desiredFlags,
                                             VkSurfaceKHR surface);
} // namespace mental