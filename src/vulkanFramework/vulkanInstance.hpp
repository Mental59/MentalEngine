#pragma once

#include <glfw/glfw3.h>
#include <volk.h>

namespace mental {

struct VulkanInstance final {
  VkInstance instance = VK_NULL_HANDLE;
  VkSurfaceKHR surface = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT messenger = VK_NULL_HANDLE;
  VkDebugReportCallbackEXT reportCallback = VK_NULL_HANDLE;
};

VkInstance createVulkanInstance();

void initVulkanInstanceGLFW(VulkanInstance& vulkanInstance, GLFWwindow* window);

void destroyVulkanInstance(VulkanInstance& vk);
} // namespace mental
