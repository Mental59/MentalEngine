#pragma once

#include <functional>
#include <volk.h>

namespace mental {
using PhysicalDeviceSelectorFunction = std::function<bool(VkPhysicalDevice)>;

VkResult findSuitablePhysicalDevice(VkInstance instance,
                                    PhysicalDeviceSelectorFunction selector,
                                    VkPhysicalDevice* physicalDevice);

int findQueueFamilies(VkPhysicalDevice physicalDevice,
                      VkQueueFlags desiredFlags);

int findQueueFamiliesWithPresentSupport(VkPhysicalDevice physicalDevice,
                                        VkQueueFlags desiredFlags,
                                        VkSurfaceKHR surface);
} // namespace mental