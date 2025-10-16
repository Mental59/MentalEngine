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

    glfwSetKeyCallback(mWindow,
        [](::GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, GLFW_TRUE);
        });

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

mental::rhi::Result mental::platform::GLFWwindow::createSurface(VkInstance instance, VkSurfaceKHR& surface) const
{
    VkResult res = glfwCreateWindowSurface(instance, mWindow, VK_NULL_HANDLE, &surface);
    if (res != VK_SUCCESS) return rhi::Result::eSurfaceInitializationFailed;
    return rhi::Result::eSuccess;
}

#if defined(MENTAL_WITH_VULKAN)

#endif
