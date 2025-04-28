#include "vulkanFramework/vulkanFramework.hpp"
#include <GLFW/glfw3.h>

int main() {
  volkInitialize();

  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  GLFWwindow* window =
      glfwCreateWindow(1280, 720, "Mental engine editor", nullptr, nullptr);

  mental::VulkanInstance vkInstance;
  mental::initVulkanInstanceGLFW(vkInstance, window);

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  mental::destroyVulkanInstance(vkInstance);

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
