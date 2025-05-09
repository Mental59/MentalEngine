#include "window.hpp"

mental::Window::Window(int width, int height, const char* title) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

  mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

  if (!mWindow) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode,
                                 int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  });
}

mental::Window::~Window() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

void mental::Window::pollEvents() { glfwPollEvents(); }

bool mental::Window::shouldClose() { return glfwWindowShouldClose(mWindow); }

mental::Window::Size mental::Window::getSize() const {
  int width, height;
  glfwGetFramebufferSize(mWindow, &width, &height);
  const float ratio = width / (float)height;
  return Size{.width = width, .height = height, .ratio = ratio};
}

VkResult mental::Window::createVulkanWindowSurface(VulkanInstance* instance) {
  return glfwCreateWindowSurface(instance->instance, mWindow, nullptr,
                                 &instance->surface);
}
