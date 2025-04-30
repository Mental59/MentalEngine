#include "window.hpp"

mental::Window::Window(int width, int height, const char* title) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

  mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

mental::Window::~Window() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

void mental::Window::pollEvents() { glfwPollEvents(); }

bool mental::Window::shouldClose() { return glfwWindowShouldClose(mWindow); }

VkResult mental::Window::createVulkanWindowSurface(VulkanInstance* instance) {
  return glfwCreateWindowSurface(instance->instance, mWindow, nullptr,
                                 &instance->surface);
}
