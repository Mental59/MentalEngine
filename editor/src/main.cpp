#include <core/log.hpp>
#include <platform/pcWindow.hpp>
#include <render/render.hpp>
#include "render/rhi/rhi.hpp"

int main()
{
  mental::core::log::Logger& logger = mental::core::log::Logger::getInstance();
  logger.enableOutputToDebug(true);

  mental::platform::PCWindow window;
  window.init({ .title = "Editor", .width = 1280, .height = 720 });
  mental::core::resource::ResourceGuard<mental::platform::PCWindow> windowGuard(&window);

  mental::render::RenderSystem* renderSystem = &mental::render::getRenderSystem();
  renderSystem->init({ mental::rhi::GraphicsApi::Vulkan, &window });
  mental::core::resource::ResourceGuard<mental::render::RenderSystem> renderSystemGuard(renderSystem);

  while (!window.shouldClose())
  {
    renderSystem->render();
    window.pollEvents();
  }

  return 0;
}
