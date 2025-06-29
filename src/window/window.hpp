#pragma once
#include "vkFramework/includes.hpp"
#include <GLFW/glfw3.h>
#include <volk.h>

namespace app {
class BaseApp;
}

namespace window {

class Window {
public:
  struct Size {
    int width = 0;
    int height = 0;
    float ratio = 0.0f;
  };

  Window(int width, int height, const char* title, bool fullScreen = false,
         app::BaseApp* pApp = nullptr);
  ~Window();

  void pollEvents();
  void waitEvents();

  double getTime() const;

  bool shouldClose();
  Size getSize() const;

  static void toggleFullscreen(GLFWwindow* window);

  VkResult createVulkanWindowSurface(vkFramework::VulkanInstance* instance);

private:
  GLFWwindow* mWindow = nullptr;
};
} // namespace window
