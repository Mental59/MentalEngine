#include <editor/app/editorApplication.hpp>

#include <core/log.hpp>
#include <platform/pcWindow.hpp>
#include <render/render.hpp>
#include "render/rhi/rhi.hpp"

int main()
{
  mental::core::log::Logger& logger = mental::core::log::Logger::getInstance();
  logger.enableOutputToDebug(true);

  mental::platform::PCWindow window;
  if (window.init({.title = "Editor", .width = 1280, .height = 720}) != mental::core::Result::eSuccess)
  {
    return 1;
  }
  mental::core::resource::ResourceGuard<mental::platform::PCWindow> windowGuard(&window);

  mental::render::RenderSystem* renderSystem = &mental::render::getRenderSystem();
  if (renderSystem->init({mental::rhi::GraphicsApi::Vulkan, &window}) != mental::core::Result::eSuccess)
  {
    return 1;
  }
  mental::core::resource::ResourceGuard<mental::render::RenderSystem> renderSystemGuard(renderSystem);

  mental::editor::EditorApplication editorApplication(&window, renderSystem);
  if (editorApplication.init() != mental::core::Result::eSuccess)
  {
    return 1;
  }

  return editorApplication.run() == mental::core::Result::eSuccess ? 0 : 1;
}
