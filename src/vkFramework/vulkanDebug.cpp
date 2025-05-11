#include "vulkanDebug.hpp"

#include <volk.h>

#include <cstdio>

#include "vulkanUtils.hpp"

namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

VKAPI_ATTR VkBool32 VKAPI_CALL reportCallback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    uint64_t object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* UserData);
} // namespace

namespace vkFramework {
VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo() {
  VkDebugUtilsMessengerCreateInfoEXT createInfo{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
      .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
      .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                     VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
      .pfnUserCallback = debugCallback};

  return createInfo;
}

VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo() {
  VkDebugReportCallbackCreateInfoEXT createInfo{
      .sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
      .pNext = nullptr,
      .flags = VK_DEBUG_REPORT_WARNING_BIT_EXT |
               VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
               VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT,
      .pfnCallback = reportCallback,
      .pUserData = nullptr};

  return createInfo;
}

void setupDebugCallbacks(VkInstance instance,
                         VkDebugUtilsMessengerEXT* messenger,
                         VkDebugReportCallbackEXT* reportCallback) {
  const VkDebugUtilsMessengerCreateInfoEXT messengerCreateInfo =
      debugUtilsMessengerCreateInfo();
  MENTAL_VK_CHECK(vkCreateDebugUtilsMessengerEXT(instance, &messengerCreateInfo,
                                                 nullptr, messenger));

  const VkDebugReportCallbackCreateInfoEXT reportCallbackCreateInfo =
      debugReportCallbackCreateInfo();
  MENTAL_VK_CHECK(vkCreateDebugReportCallbackEXT(
      instance, &reportCallbackCreateInfo, nullptr, reportCallback));
}

} // namespace vkFramework

namespace {
VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
  printf("Validation layer: %s\n", pCallbackData->pMessage);
  return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL reportCallback(
    VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
    uint64_t object, size_t location, int32_t messageCode,
    const char* pLayerPrefix, const char* pMessage, void* UserData) {
  printf("Debug callback (%s): %s\n", pLayerPrefix, pMessage);
  return VK_FALSE;
}
} // namespace
