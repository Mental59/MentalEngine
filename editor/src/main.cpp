#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <platform/glfwWindow.hpp>

int main()
{
    mental::platform::GLFWwindow* windowPtr = new mental::platform::GLFWwindow({.title = "Test app", .width = 1280, .height = 720});
    mental::platform::WindowHandle window = mental::platform::WindowHandle::Create(windowPtr);

    mental::render::RenderSystem renderSystem;
    if (!renderSystem.init(window))
    {
        mental::core::log::fatal("Failed to initialize render system");
    }

    mental::core::log::info("Render system initialized");

    while (!window->shouldClose())
    {
        window->pollEvents();
    }

    return 0;
}
