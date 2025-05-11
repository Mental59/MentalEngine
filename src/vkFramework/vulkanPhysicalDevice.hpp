#pragma once

#include <functional>
#include <volk.h>

namespace vkFramework {
using PhysicalDeviceSelectorFunction = std::function<bool(VkPhysicalDevice)>;

VkResult findSuitablePhysicalDevice(VkInstance instance,
                                    PhysicalDeviceSelectorFunction selector,
                                    VkPhysicalDevice* physicalDevice);

int findQueueFamilies(VkPhysicalDevice physicalDevice,
                      VkQueueFlags desiredFlags);

int findQueueFamiliesWithPresentSupport(VkPhysicalDevice physicalDevice,
                                        VkQueueFlags desiredFlags,
                                        VkSurfaceKHR surface);

bool checkDeviceExtensionSupport(const VkPhysicalDevice device,
                                 const char* const* extensions,
                                 uint32_t numExtensions);
} // namespace vkFramework