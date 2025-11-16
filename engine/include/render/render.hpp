#pragma once

#include <render/rhi/rhi.hpp>
#include "core/resource.hpp"

namespace mental::platform
{
  class IWindow;
}

namespace mental::render
{
  struct RenderSystemConfig
  {
    rhi::GraphicsApi graphicsApi;
    mental::platform::IWindow* window;
  };

  class RenderSystem : public core::resource::IResource
  {
   public:
    RenderSystem() = default;
    RenderSystem(const RenderSystem&) = delete;
    RenderSystem(const RenderSystem&&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem& operator=(const RenderSystem&&) = delete;

    core::Result init(const RenderSystemConfig& conf);
    virtual void destroy() override;

   private:
    bool mIsInitialized = false;
  };

  inline RenderSystem& getRenderSystem()
  {
    static RenderSystem renderSystem;
    return renderSystem;
  }
}  // namespace mental::render
