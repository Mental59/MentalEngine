#pragma once

#include <render/frameContext.hpp>
#include <render/primitiveMeshLibrary.hpp>
#include <render/scenePipelineLibrary.hpp>
#include <render/rhi/rhi.hpp>
#include <resource/resourceManager.hpp>
#include <vector>

namespace mental::render
{
[[nodiscard]] bool isSubmitEligibleAcquireResult(core::Result acquireResult);

struct RenderFrameOutcome
{
  core::Result result = core::Result::eSuccess;
  bool submitted = false;
};

struct RenderSystemConfig
{
  rhi::GraphicsApi graphicsApi;
  class IRenderHostAdapter* hostAdapter;
};

class IRenderSystem : public core::resource::IResource
{
 public:
  virtual core::Result init(const RenderSystemConfig& conf) = 0;
  [[nodiscard]] virtual RenderFrameOutcome render(const FrameContext& frameContext) = 0;
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

  virtual RenderFrameOutcome render(const FrameContext& frameContext) override;

 private:
  RenderSystem() = default;
  [[nodiscard]] core::Result resizeSwapchain(rhi::ISwapchain* swapchain);
  [[nodiscard]] core::Result createCameraUploadBuffers();
  void destroyCameraUploadBuffers();
  [[nodiscard]] core::Result ensureDepthTarget(const platform::WindowSize& framebufferSize);
  void destroyDepthTarget();

  std::vector<resource::FrameDataHandle> mFrameDataHandles;
  std::vector<resource::BufferHandle> mCameraBufferHandles;
  resource::TextureHandle mDepthTextureHandle = resource::TextureHandle::invalid();
  resource::TextureViewHandle mDepthTextureViewHandle = resource::TextureViewHandle::invalid();
  PrimitiveMeshLibrary mPrimitiveMeshLibrary {};
  ScenePipelineLibrary mScenePipelineLibrary {};
  platform::WindowSize mDepthExtent {};

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
