#include <editor/app/editorApplication.hpp>

#include <core/log.hpp>

namespace mental::editor
{
EditorApplication::EditorApplication(platform::IWindow* window, render::IRenderSystem* renderSystem)
  : mWindow(window)
  , mRenderSystem(renderSystem)
{
}

core::Result EditorApplication::init()
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized EditorApplication");
    return core::Result::eInitializationFailed;
  }

  if (mWindow == nullptr || mRenderSystem == nullptr || !mWindow->isValid() || !mRenderSystem->isValid())
  {
    MENTAL_ERROR("EditorApplication requires a valid window and render system");
    return core::Result::eInitializationFailed;
  }

  mObservedFramebufferSize = mWindow->getWindowSize();
  mFramebufferResized = false;
  mAbsoluteTimeSeconds = mWindow->getTime();
  mDeltaTimeSeconds = 0.0;
  mRenderedFrameCount = 0;
  mFrameContext = {};
  mLastFrameTimeSeconds = mAbsoluteTimeSeconds;
  mShutdownRequested = false;
  mShouldAttemptFrame = mObservedFramebufferSize.width > 0 && mObservedFramebufferSize.height > 0;
  mLastError = bootstrapScene();
  if (mLastError != core::Result::eSuccess)
  {
    return mLastError;
  }

  mIsInitialized = true;
  return core::Result::eSuccess;
}

core::Result EditorApplication::run()
{
  if (!mIsInitialized)
  {
    MENTAL_ERROR("Trying to run an uninitialized EditorApplication");
    mLastError = core::Result::eInitializationFailed;
    return mLastError;
  }

  while (!mShutdownRequested && !mWindow->shouldClose())
  {
    if (const core::Result res = executePhase(&EditorApplication::updatePlatform); res != core::Result::eSuccess)
    {
      return res;
    }

    if (const core::Result res = executePhase(&EditorApplication::collectInput); res != core::Result::eSuccess)
    {
      return res;
    }

    if (const core::Result res = executePhase(&EditorApplication::updateEditor); res != core::Result::eSuccess)
    {
      return res;
    }

    if (const core::Result res = executePhase(&EditorApplication::renderFrame); res != core::Result::eSuccess)
    {
      return res;
    }
  }

  mLastError = core::Result::eSuccess;
  return mLastError;
}

void EditorApplication::shutdown()
{
  mShutdownRequested = true;
}

core::Result EditorApplication::updatePlatform()
{
  const platform::WindowSize previousSize = mObservedFramebufferSize;
  const platform::WindowSize currentSize = mWindow->getWindowSize();

  mObservedFramebufferSize = currentSize;
  mFramebufferResized = currentSize.width != previousSize.width || currentSize.height != previousSize.height;

  const double currentTimeSeconds = mWindow->getTime();
  mAbsoluteTimeSeconds = currentTimeSeconds;
  mDeltaTimeSeconds = currentTimeSeconds - mLastFrameTimeSeconds;
  mLastFrameTimeSeconds = currentTimeSeconds;

  mShouldAttemptFrame = currentSize.width > 0 && currentSize.height > 0;
  if (mShouldAttemptFrame)
  {
    mWindow->pollEvents();
  }
  else
  {
    mWindow->waitEvents();
  }

  return core::Result::eSuccess;
}

core::Result EditorApplication::collectInput()
{
  return core::Result::eSuccess;
}

core::Result EditorApplication::updateEditor()
{
  if (!mShouldAttemptFrame)
  {
    return core::Result::eSuccess;
  }

  return core::Result::eSuccess;
}

core::Result EditorApplication::renderFrame()
{
  if (!mShouldAttemptFrame)
  {
    return core::Result::eSuccess;
  }

  mFrameContext.absoluteTimeSeconds = mAbsoluteTimeSeconds;
  mFrameContext.deltaTimeSeconds = mDeltaTimeSeconds;
  mFrameContext.framebufferSize = mObservedFramebufferSize;
  mFrameContext.framebufferResized = mFramebufferResized;
  mFrameContext.frameIndex = mRenderedFrameCount;

  const core::Result res = mRenderSystem->render(mFrameContext);
  if (res == core::Result::eSuccess)
  {
    ++mRenderedFrameCount;
  }

  return res;
}

core::Result EditorApplication::bootstrapScene()
{
  entt::entity cube = entt::null;
  return mScene.createPrimitive(PrimitiveType::eCube, cube);
}

core::Result EditorApplication::executePhase(core::Result (EditorApplication::*phase)())
{
  mLastError = (this->*phase)();
  if (mLastError != core::Result::eSuccess)
  {
    shutdown();
  }
  return mLastError;
}
} // namespace mental::editor
