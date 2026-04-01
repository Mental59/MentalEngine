#pragma once

#include <editor/app/editorInputState.hpp>
#include <editor/scene/editorScene.hpp>

#include <core/types.hpp>
#include <platform/window.hpp>
#include <render/frameContext.hpp>
#include <render/render.hpp>

namespace mental::editor
{
class EditorApplication
{
 public:
  EditorApplication(platform::IWindow* window, render::IRenderSystem* renderSystem);
  virtual ~EditorApplication() = default;

  [[nodiscard]] core::Result init();
  [[nodiscard]] core::Result run();
  void shutdown();

 protected:
  [[nodiscard]] virtual core::Result updatePlatform();
  [[nodiscard]] virtual core::Result collectInput();
  [[nodiscard]] virtual core::Result updateEditor();
  [[nodiscard]] virtual core::Result renderFrame();

  [[nodiscard]] core::Result bootstrapScene();

  [[nodiscard]] EditorScene& scene() noexcept
  {
    return mScene;
  }

  [[nodiscard]] const EditorScene& scene() const noexcept
  {
    return mScene;
  }

  [[nodiscard]] const render::FrameContext& frameContext() const noexcept
  {
    return mFrameContext;
  }

  [[nodiscard]] EditorInputState& inputState() noexcept
  {
    return mInputState;
  }

  [[nodiscard]] const EditorInputState& inputState() const noexcept
  {
    return mInputState;
  }

  [[nodiscard]] bool isShutdownRequested() const noexcept
  {
    return mShutdownRequested;
  }

  [[nodiscard]] core::Result lastError() const noexcept
  {
    return mLastError;
  }

 private:
  [[nodiscard]] core::Result executePhase(core::Result (EditorApplication::*phase)());

  platform::IWindow* mWindow = nullptr;
  render::IRenderSystem* mRenderSystem = nullptr;
  EditorScene mScene;
  EditorInputState mInputState {};
  render::FrameContext mFrameContext {};
  platform::WindowSize mObservedFramebufferSize {};
  double mAbsoluteTimeSeconds = 0.0;
  double mDeltaTimeSeconds = 0.0;
  std::uint64_t mRenderedFrameCount = 0;
  double mLastFrameTimeSeconds = 0.0;
  bool mShutdownRequested = false;
  bool mIsInitialized = false;
  bool mShouldAttemptFrame = true;
  bool mFramebufferResized = false;
  core::Result mLastError = core::Result::eSuccess;
};
} // namespace mental::editor
