#include <platform/pcWindow.hpp>
#include <GLFW/glfw3.h>
#include <core/log.hpp>
#include "core/resource.hpp"

mental::core::resource::Object mental::platform::PCWindow::getNativeObject(core::resource::ObjectType objectType)
{
  if (objectType == core::resource::ObjectType::eGLFWwindow)
  {
    return mWindow;
  }
  return mWindow;
}

mental::core::Result mental::platform::PCWindow::init(const mental::platform::WindowDesc& desc)
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized PCWindow");
    return core::Result::eInitializationFailed;
  }

  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

  mWindow = glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);
  MENTAL_ASSERT_MESSAGE(mWindow != nullptr, "Failed to create GLFW window");

  glfwSetKeyCallback(
      mWindow,
      [](::GLFWwindow* window, int key, int scancode, int action, int mods)
      {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
          glfwSetWindowShouldClose(window, GLFW_TRUE);
      });

  MENTAL_INFO("GLFW window initialized");
  mIsInitialized = true;

  return core::Result::eSuccess;
}

void mental::platform::PCWindow::destroy()
{
  if (!mIsInitialized)
  {
    MENTAL_WARN("Trying to destroy an uninitialized PCWindow");
    return;
  }

  glfwDestroyWindow(mWindow);
  glfwTerminate();

  MENTAL_INFO("GLFW window destroyed");
}

void mental::platform::PCWindow::pollEvents() const
{
  glfwPollEvents();
}

double mental::platform::PCWindow::getTime() const
{
  return glfwGetTime();
}

bool mental::platform::PCWindow::shouldClose() const
{
  return glfwWindowShouldClose(mWindow);
}

#ifdef MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/constants.hpp>

mental::core::Result
mental::platform::createVulkanSurface(mental::platform::IWindow* window, VkInstance instance, VkSurfaceKHR* surface)
{
  GLFWwindow* glfwWindow = window->getNativeObject(core::resource::ObjectType::eGLFWwindow);
  VkResult res = glfwCreateWindowSurface(instance, glfwWindow, VK_NULL_HANDLE, surface);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call glfwCreateWindowSurface, error: {}", mental::rhi::vk::vkResultToString(res));
    return core::Result::eInitializationFailed;
  }
  return core::Result::eSuccess;
}

#endif
