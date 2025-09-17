#include <platform/glfwWindow.hpp>
#include <GLFW/glfw3.h>
#include <core/log.hpp>

mental::platform::GLFWwindow::GLFWwindow(const WindowDesc& desc)
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    mWindow = glfwCreateWindow(desc.width, desc.height, desc.title, nullptr, nullptr);

    if (!mWindow)
    {
        core::log::fatal("Failed to create GLFW window");
    }

    core::log::info("GLFW window initialized");
}

mental::platform::GLFWwindow::~GLFWwindow()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();

    core::log::info("GLFW window destroyed");
}

void mental::platform::GLFWwindow::pollEvents() const
{
    glfwPollEvents();
}

double mental::platform::GLFWwindow::getTime() const
{
    return glfwGetTime();
}

bool mental::platform::GLFWwindow::shouldClose() const
{
    return glfwWindowShouldClose(mWindow);
}

#if defined(MENTAL_WITH_VULKAN)
::vk::SurfaceKHR mental::platform::GLFWwindow::createSurface(const ::vk::Instance& instance) const
{
    VkSurfaceKHR surface;

    VkResult res = glfwCreateWindowSurface(instance, mWindow, nullptr, &surface);
    if (res != VK_SUCCESS)
    {
        core::log::fatal("Failed to create Vulkan surface");
    }

    return ::vk::SurfaceKHR(surface);
}
#endif
