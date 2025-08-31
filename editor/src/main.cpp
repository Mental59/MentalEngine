#include <engine/include/render/render.hpp>
#include <engine/include/core/log.hpp>

mental::render::RenderSystem gRenderSystem;

int main()
{
    if (!gRenderSystem.init())
    {
        mental::core::log::fatal("Failed to initialize render system");
    }

    mental::core::log::info("Render system initialized");
    return 0;
}
