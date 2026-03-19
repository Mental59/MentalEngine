#pragma once

#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include <vector>

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
  static RenderSystem& instance()
  {
    static RenderSystem renderSystem;
    return renderSystem;
  }

  RenderSystem(const RenderSystem&) = delete;
  RenderSystem(const RenderSystem&&) = delete;
  RenderSystem& operator=(const RenderSystem&) = delete;
  RenderSystem& operator=(const RenderSystem&&) = delete;

  core::Result init(const RenderSystemConfig& conf);
  virtual void destroy() override;

  virtual bool isValid() const override;

  void nextFrame();

  core::Result render();

 private:
  RenderSystem() = default;
  void resizeSwapchain(rhi::ISwapchain* swapchain);

  std::vector<resource::FrameDataHandle> mFrameDataHandles;

  uint32_t mCurrentFrame = 0;
  uint32_t mMaxFramesInFlight = 0;

  bool mIsInitialized = false;
  mental::platform::IWindow* mWindow;
};

inline RenderSystem& getRenderSystem()
{
  return RenderSystem::instance();
}
} // namespace mental::render
