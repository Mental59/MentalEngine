#include <core/log.hpp>
#include <platform/pcWindow.hpp>
#include <render/render.hpp>

int main()
{
  mental::core::log::Logger& logger = mental::core::log::Logger::getInstance();
  logger.enableOutputToDebug(true);

  mental::platform::PCWindow window;
  window.init({ .title = "Test app", .width = 1280, .height = 720 });
  mental::core::resource::ResourceGuard<mental::platform::PCWindow> windowGuard(&window);

  mental::render::getRenderSystem().init({ mental::rhi::GraphicsApi::Vulkan, &window });
  mental::core::resource::ResourceGuard<mental::render::RenderSystem> renderSystemGuard(&mental::render::getRenderSystem());

  while (!window.shouldClose())
  {
    window.pollEvents();
  }

  return 0;
}
