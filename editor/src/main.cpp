#include <render/render.hpp>
#include <platform/window.hpp>
#include <platform/glfwWindow.hpp>

int main()
{
    mental::platform::GLFWwindow window({.title = "Test app", .width = 1280, .height = 720});
    mental::render::RenderSystem renderSystem(&window);

    while (!window.shouldClose())
    {
        window.pollEvents();
    }

    return 0;
}
