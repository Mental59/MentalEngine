#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <platform/glfwWindow.hpp>

int main()
{
    mental::platform::GLFWwindow window({.title = "Test app", .width = 1280, .height = 720});
    mental::render::RenderSystem renderSystem;

    if (!renderSystem.init(&window))
    {
        mental::core::log::fatal("Failed to initialize render system");
    }

    mental::core::log::info("Render system initialized");

    while (!window.shouldClose())
    {
        window.pollEvents();
    }

    return 0;
}
