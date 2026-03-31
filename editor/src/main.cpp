#include <editor/scene/editorScene.hpp>

#include <core/log.hpp>
#include <platform/pcWindow.hpp>
#include <render/render.hpp>
#include "render/rhi/rhi.hpp"

int main()
{
  mental::core::log::Logger& logger = mental::core::log::Logger::getInstance();
  logger.enableOutputToDebug(true);

  mental::editor::EditorScene editorScene;
  entt::entity cube = entt::null;
  if (editorScene.createPrimitive(mental::editor::PrimitiveType::eCube, cube) != mental::core::Result::eSuccess)
  {
    return 1;
  }
  (void)cube;

  mental::platform::PCWindow window;
  window.init({.title = "Editor", .width = 1280, .height = 720});
  mental::core::resource::ResourceGuard<mental::platform::PCWindow> windowGuard(&window);

  mental::render::RenderSystem* renderSystem = &mental::render::getRenderSystem();
  renderSystem->init({mental::rhi::GraphicsApi::Vulkan, &window});
  mental::core::resource::ResourceGuard<mental::render::RenderSystem> renderSystemGuard(renderSystem);

  while (!window.shouldClose())
  {
    mental::core::Result res = renderSystem->render();
    if (res != mental::core::Result::eSuccess)
    {
      break;
    }

    window.pollEvents();
  }

  return 0;
}
