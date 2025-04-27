#pragma once

#include <functional>
#include <volk.h>

namespace mental {
VkResult
findSuitablePhysicalDevice(VkInstance instance,
                           std::function<bool(VkPhysicalDevice)> selector,
                           VkPhysicalDevice* physicalDevice);

int findQueueFamilies(VkPhysicalDevice physicalDevice,
                      VkQueueFlags desiredFlags);

int findQueueFamiliesWithPresentSupport(VkPhysicalDevice physicalDevice,
                                        VkQueueFlags desiredFlags,
                                        VkSurfaceKHR surface);
} // namespace mental