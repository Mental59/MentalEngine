#pragma once
#include "vulkanFramework/vulkanInstance.hpp"
#include <GLFW/glfw3.h>
#include <volk.h>

namespace mental {
class BaseApp;

class Window {
public:
  struct Size {
    int width = 0;
    int height = 0;
    float ratio = 0.0f;
  };

  Window(int width, int height, const char* title, bool fullScreen = false,
         BaseApp* pApp = nullptr);
  ~Window();

  void pollEvents();
  void waitEvents();

  bool shouldClose();
  Size getSize() const;

  static void toggleFullscreen(GLFWwindow* window);

  VkResult createVulkanWindowSurface(VulkanInstance* instance);

private:
  GLFWwindow* mWindow = nullptr;
};
} // namespace mental
