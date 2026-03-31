#pragma once

#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include <vector>

namespace mental::editor
{
struct FrameContext;
}

namespace mental::render
{
[[nodiscard]] bool isSubmitEligibleAcquireResult(core::Result acquireResult);

struct RenderSystemConfig
{
  rhi::GraphicsApi graphicsApi;
  class IRenderHostAdapter* hostAdapter;
};

class IRenderSystem : public core::resource::IResource
{
 public:
  virtual core::Result init(const RenderSystemConfig& conf) = 0;
  [[nodiscard]] virtual core::Result render(const editor::FrameContext& frameContext) = 0;
};

class RenderSystem : public IRenderSystem
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

  virtual core::Result init(const RenderSystemConfig& conf) override;
  virtual void destroy() override;

  virtual bool isValid() const override;

  void nextFrame();

  virtual core::Result render(const editor::FrameContext& frameContext) override;

 private:
  RenderSystem() = default;
  [[nodiscard]] core::Result resizeSwapchain(rhi::ISwapchain* swapchain);

  std::vector<resource::FrameDataHandle> mFrameDataHandles;

  uint32_t mCurrentFrame = 0;
  uint32_t mMaxFramesInFlight = 0;

  bool mIsInitialized = false;
  IRenderHostAdapter* mHostAdapter = nullptr;
};

inline RenderSystem& getRenderSystem()
{
  return RenderSystem::instance();
}
} // namespace mental::render
