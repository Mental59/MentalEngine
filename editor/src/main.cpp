#include <render/render.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#include <platform/glfwWindow.hpp>

int main()
{
    mental::core::log::Logger& logger = mental::core::log::Logger::getInstance();
    logger.enableOutputToDebug(true);

    mental::platform::GLFWwindow window({.title = "Test app", .width = 1280, .height = 720});
    mental::render::RenderSystem renderSystem(&window);

    while (!window.shouldClose())
    {
        window.pollEvents();
    }

    return 0;
}
