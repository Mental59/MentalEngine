#include "window.hpp"
#include "app/baseApp.hpp"

mental::Window::Window(int width, int height, const char* title,
                       bool fullScreen, BaseApp* pApp) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  mWindow =
      glfwCreateWindow(width, height, title,
                       fullScreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
  if (pApp) {
    glfwSetWindowUserPointer(mWindow, pApp);
  }

  if (!mWindow) {
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode,
                                 int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
      toggleFullscreen(window);
    }
  });

  glfwSetFramebufferSizeCallback(
      mWindow, [](GLFWwindow* window, int width, int height) {
        if (BaseApp* app =
                reinterpret_cast<BaseApp*>(glfwGetWindowUserPointer(window))) {
          app->setFramebufferResized(true);
        }
      });
}

mental::Window::~Window() {
  glfwDestroyWindow(mWindow);
  glfwTerminate();
}

void mental::Window::pollEvents() { glfwPollEvents(); }
void mental::Window::waitEvents() { glfwWaitEvents(); }

bool mental::Window::shouldClose() { return glfwWindowShouldClose(mWindow); }

mental::Window::Size mental::Window::getSize() const {
  int width, height;
  glfwGetFramebufferSize(mWindow, &width, &height);
  const float ratio = width / (float)height;
  return Size{.width = width, .height = height, .ratio = ratio};
}

void mental::Window::toggleFullscreen(GLFWwindow* window) {
  static int savedWidth, savedHeight;
  static int savedPosX, savedPosY;

  if (!glfwGetWindowMonitor(window)) {
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    glfwGetWindowPos(window, &savedPosX, &savedPosY);
    glfwGetWindowSize(window, &savedWidth, &savedHeight);

    glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height,
                         mode->refreshRate);
  } else {
    glfwSetWindowMonitor(window, nullptr, savedPosX, savedPosY, savedWidth,
                         savedHeight, GLFW_DONT_CARE);
  }
}

VkResult mental::Window::createVulkanWindowSurface(VulkanInstance* instance) {
  return glfwCreateWindowSurface(instance->instance, mWindow, nullptr,
                                 &instance->surface);
}
