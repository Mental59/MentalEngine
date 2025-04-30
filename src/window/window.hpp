#pragma once
#include "vulkanFramework/vulkanInstance.hpp"
#include <GLFW/glfw3.h>
#include <volk.h>

namespace mental {
class Window {
public:
  Window(int width, int height, const char* title);
  ~Window();

  void pollEvents();
  bool shouldClose();

  VkResult createVulkanWindowSurface(VulkanInstance* instance);

private:
  GLFWwindow* mWindow = nullptr;
};
} // namespace mental
