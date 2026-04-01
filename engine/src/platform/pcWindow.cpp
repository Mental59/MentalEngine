#include <platform/pcWindow.hpp>
#include <GLFW/glfw3.h>
#include <core/log.hpp>
#include "core/resource.hpp"

namespace
{
constexpr int toGlfwKey(mental::input::KeyCode keyCode)
{
  switch (keyCode)
  {
    case mental::input::KeyCode::eW:
      return GLFW_KEY_W;
    case mental::input::KeyCode::eA:
      return GLFW_KEY_A;
    case mental::input::KeyCode::eS:
      return GLFW_KEY_S;
    case mental::input::KeyCode::eD:
      return GLFW_KEY_D;
    case mental::input::KeyCode::eQ:
      return GLFW_KEY_Q;
    case mental::input::KeyCode::eE:
      return GLFW_KEY_E;
    case mental::input::KeyCode::eR:
      return GLFW_KEY_R;
    case mental::input::KeyCode::eEscape:
      return GLFW_KEY_ESCAPE;
  }

  return GLFW_KEY_UNKNOWN;
}

constexpr int toGlfwMouseButton(mental::input::MouseButton mouseButton)
{
  switch (mouseButton)
  {
    case mental::input::MouseButton::eLeft:
      return GLFW_MOUSE_BUTTON_LEFT;
    case mental::input::MouseButton::eRight:
      return GLFW_MOUSE_BUTTON_RIGHT;
    case mental::input::MouseButton::eMiddle:
      return GLFW_MOUSE_BUTTON_MIDDLE;
  }

  return GLFW_MOUSE_BUTTON_LEFT;
}
} // namespace

mental::core::resource::Object mental::platform::PCWindow::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eGLFWwindow:
      return mWindow;
    default:
      return nullptr;
  }
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

  mAccumulatedScrollDelta = {};
  glfwSetWindowUserPointer(mWindow, this);
  glfwSetScrollCallback(mWindow, &PCWindow::scrollCallback);

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

  glfwSetWindowUserPointer(mWindow, nullptr);
  glfwDestroyWindow(mWindow);
  mWindow = nullptr;
  mAccumulatedScrollDelta = {};
  glfwTerminate();

  mIsInitialized = false;

  MENTAL_INFO("GLFW window destroyed");
}

mental::platform::WindowSize mental::platform::PCWindow::getWindowSize() const
{
  int width, height;
  glfwGetFramebufferSize(mWindow, &width, &height);

  WindowSize windowSize {};
  windowSize.width = static_cast<uint32_t>(width);
  windowSize.height = static_cast<uint32_t>(height);

  return windowSize;
}

mental::input::InputSnapshot mental::platform::PCWindow::sampleInput() const
{
  input::InputSnapshot snapshot {};

  if (mWindow == nullptr)
  {
    return snapshot;
  }

  double cursorX = 0.0;
  double cursorY = 0.0;
  glfwGetCursorPos(mWindow, &cursorX, &cursorY);

  snapshot.cursorPosition = {cursorX, cursorY};
  snapshot.scrollDelta = mAccumulatedScrollDelta;

  for (const input::KeyCode keyCode : input::kKeyCodes)
  {
    snapshot.setKeyDown(keyCode, glfwGetKey(mWindow, toGlfwKey(keyCode)) == GLFW_PRESS);
  }

  for (const input::MouseButton mouseButton : input::kMouseButtons)
  {
    snapshot.setMouseButtonDown(
      mouseButton, glfwGetMouseButton(mWindow, toGlfwMouseButton(mouseButton)) == GLFW_PRESS);
  }

  mAccumulatedScrollDelta = {};

  return snapshot;
}

bool mental::platform::PCWindow::isValid() const
{
  return mIsInitialized;
}

void mental::platform::PCWindow::pollEvents() const
{
  glfwPollEvents();
}

void mental::platform::PCWindow::waitEvents() const
{
  glfwWaitEvents();
}

double mental::platform::PCWindow::getTime() const
{
  return glfwGetTime();
}

bool mental::platform::PCWindow::shouldClose() const
{
  return glfwWindowShouldClose(mWindow);
}

void mental::platform::PCWindow::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
  if (auto* self = static_cast<PCWindow*>(glfwGetWindowUserPointer(window)); self != nullptr)
  {
    self->mAccumulatedScrollDelta.x += xoffset;
    self->mAccumulatedScrollDelta.y += yoffset;
  }
}

#ifdef MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/constants.hpp>

mental::core::Result mental::platform::createVulkanSurface(
  mental::platform::IWindow* window, VkInstance instance, VkSurfaceKHR* surface)
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
