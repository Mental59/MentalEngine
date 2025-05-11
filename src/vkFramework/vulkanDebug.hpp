#pragma once

#include "volk.h"

namespace vkFramework {
VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo();

VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo();

void setupDebugCallbacks(VkInstance instance,
                         VkDebugUtilsMessengerEXT* messenger,
                         VkDebugReportCallbackEXT* reportCallback);
} // namespace vkFramework
