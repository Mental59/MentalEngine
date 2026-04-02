#include <editor/app/editorApplication.hpp>

#include <algorithm>

#include <core/log.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace
{
using mental::editor::PrimitiveComponent;
using mental::editor::PrimitiveType;
using mental::editor::TransformComponent;

[[nodiscard]] mental::render::SceneGeometryKind toSceneGeometryKind(PrimitiveType type) noexcept
{
  switch (type)
  {
    case PrimitiveType::eCube:
      return mental::render::SceneGeometryKind::eCube;
    case PrimitiveType::ePlane:
      return mental::render::SceneGeometryKind::ePlane;
    case PrimitiveType::eSphere:
      return mental::render::SceneGeometryKind::eSphere;
  }

  return mental::render::SceneGeometryKind::eCube;
}

[[nodiscard]] glm::mat4 buildWorldTransform(const TransformComponent& transform) noexcept
{
  glm::mat4 worldTransform {1.0f};
  worldTransform = glm::translate(worldTransform, transform.position);
  worldTransform = glm::rotate(worldTransform, glm::radians(transform.rotation.x), glm::vec3 {1.0f, 0.0f, 0.0f});
  worldTransform = glm::rotate(worldTransform, glm::radians(transform.rotation.y), glm::vec3 {0.0f, 1.0f, 0.0f});
  worldTransform = glm::rotate(worldTransform, glm::radians(transform.rotation.z), glm::vec3 {0.0f, 0.0f, 1.0f});
  worldTransform = glm::scale(worldTransform, transform.scale);
  return worldTransform;
}
} // namespace

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
  buildSceneRenderFrame(mFrameContext.sceneRenderFrame);
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

void EditorApplication::buildSceneRenderFrame(render::SceneRenderFrame& sceneRenderFrame) const
{
  const float aspectRatio =
    static_cast<float>(mObservedFramebufferSize.width) / static_cast<float>(mObservedFramebufferSize.height);
  const auto& sceneCamera = mScene.sceneCamera();

  sceneRenderFrame.camera.worldPosition = sceneCamera.worldPosition();
  sceneRenderFrame.camera.view = sceneCamera.viewMatrix();
  sceneRenderFrame.camera.projection = sceneCamera.projectionMatrix(aspectRatio);
  sceneRenderFrame.camera.viewProjection = sceneRenderFrame.camera.projection * sceneRenderFrame.camera.view;
  sceneRenderFrame.camera.aspectRatio = aspectRatio;

  const auto primitiveView = mScene.registry().view<const TransformComponent, const PrimitiveComponent>();
  const std::size_t primitiveCount =
    static_cast<std::size_t>(std::distance(primitiveView.begin(), primitiveView.end()));
  sceneRenderFrame.objects.clear();
  sceneRenderFrame.objects.reserve(primitiveCount);

  for (const entt::entity entity : primitiveView)
  {
    const TransformComponent& transform = mScene.registry().get<TransformComponent>(entity);
    const PrimitiveComponent& primitive = mScene.registry().get<PrimitiveComponent>(entity);

    sceneRenderFrame.objects.push_back(render::SceneRenderObject {
      .objectIdentifier = static_cast<render::SceneObjectIdentifier>(entt::to_integral(entity)),
      .geometryKind = toSceneGeometryKind(primitive.type),
      .worldTransform = buildWorldTransform(transform),
    });
  }

  std::sort(sceneRenderFrame.objects.begin(),
    sceneRenderFrame.objects.end(),
    [](const render::SceneRenderObject& lhs, const render::SceneRenderObject& rhs)
    { return lhs.objectIdentifier < rhs.objectIdentifier; });
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
