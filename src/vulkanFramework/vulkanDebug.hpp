#pragma once

#include "volk.h"

namespace mental {
VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo();

VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo();

void setupDebugCallbacks(VkInstance instance,
                         VkDebugUtilsMessengerEXT* messenger,
                         VkDebugReportCallbackEXT* reportCallback);
} // namespace mental
