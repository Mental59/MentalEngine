#include <editor/app/editorApplication.hpp>

#include <core/log.hpp>

namespace
{
void applySceneCameraFlyLook(
  mental::editor::SceneCamera& camera, const mental::editor::EditorInputState& inputState, double deltaTimeSeconds)
{
  const mental::input::CursorPosition mouseDelta = inputState.mouseDelta();
  camera.applyYawPitchDelta(static_cast<float>(mouseDelta.x) * camera.mouseLookSensitivity(),
    static_cast<float>(-mouseDelta.y) * camera.mouseLookSensitivity());

  float movementDistance = camera.moveSpeed() * static_cast<float>(deltaTimeSeconds);
  if (inputState.isCameraBoostActive())
  {
    movementDistance *= camera.boostMultiplier();
  }

  glm::vec3 localOffset {0.0f, 0.0f, 0.0f};
  if (inputState.isKeyDown(mental::input::KeyCode::eW))
  {
    localOffset.z += movementDistance;
  }
  if (inputState.isKeyDown(mental::input::KeyCode::eS))
  {
    localOffset.z -= movementDistance;
  }
  if (inputState.isKeyDown(mental::input::KeyCode::eD))
  {
    localOffset.x += movementDistance;
  }
  if (inputState.isKeyDown(mental::input::KeyCode::eA))
  {
    localOffset.x -= movementDistance;
  }
  if (inputState.isKeyDown(mental::input::KeyCode::eE))
  {
    localOffset.y += movementDistance;
  }
  if (inputState.isKeyDown(mental::input::KeyCode::eQ))
  {
    localOffset.y -= movementDistance;
  }

  camera.translateLocal(localOffset);
}
} // namespace

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
    mLastError = core::Result::eInitializationFailed;
    return core::Result::eInitializationFailed;
  }

  if (mWindow == nullptr || mRenderSystem == nullptr || !mWindow->isValid() || !mRenderSystem->isValid())
  {
    MENTAL_ERROR("EditorApplication requires a valid window and render system");
    mLastError = core::Result::eInitializationFailed;
    return core::Result::eInitializationFailed;
  }

  mObservedFramebufferSize = mWindow->getWindowSize();
  mFramebufferResized = false;
  mAbsoluteTimeSeconds = mWindow->getTime();
  mDeltaTimeSeconds = 0.0;
  mRenderedFrameCount = 0;
  mInputState = {};
  mFrameContext = {};
  mLastFrameTimeSeconds = mAbsoluteTimeSeconds;
  mShutdownRequested = false;
  mSceneCameraControlActive = false;
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

  setSceneCameraControlActive(false);
  mLastError = core::Result::eSuccess;
  return mLastError;
}

void EditorApplication::shutdown()
{
  mShutdownRequested = true;
  setSceneCameraControlActive(false);
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
  mInputState.advance(mWindow->sampleInput());

  if (!mInputState.isFlyLookActive())
  {
    if (const std::optional<GizmoMode> requestedGizmoMode = mInputState.requestedGizmoMode();
      requestedGizmoMode.has_value())
    {
      mScene.setGizmoMode(requestedGizmoMode.value());
    }
  }

  return core::Result::eSuccess;
}

core::Result EditorApplication::updateEditor()
{
  return updateSceneCameraControl();
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

  const render::RenderFrameOutcome outcome = mRenderSystem->render(mFrameContext);
  if (outcome.result == core::Result::eSuccess && outcome.submitted)
  {
    ++mRenderedFrameCount;
  }

  return outcome.result;
}

core::Result EditorApplication::bootstrapScene()
{
  entt::entity cube = entt::null;
  return mScene.createPrimitive(PrimitiveType::eCube, cube);
}

core::Result EditorApplication::updateSceneCameraControl()
{
  const bool flyLookActive = mInputState.isFlyLookActive();
  setSceneCameraControlActive(flyLookActive);

  if (!flyLookActive || !mShouldAttemptFrame)
  {
    return core::Result::eSuccess;
  }

  applySceneCameraFlyLook(mScene.sceneCamera(), mInputState, mDeltaTimeSeconds);
  return core::Result::eSuccess;
}

void EditorApplication::setSceneCameraControlActive(bool active) noexcept
{
  if (mWindow == nullptr || mSceneCameraControlActive == active)
  {
    return;
  }

  mWindow->setCursorMode(active ? platform::CursorMode::eDisabled : platform::CursorMode::eNormal);
  mWindow->setRawMouseMotionEnabled(active);
  mSceneCameraControlActive = active;
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
