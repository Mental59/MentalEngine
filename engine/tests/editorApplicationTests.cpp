#include <editor/app/editorApplication.hpp>
#include <editor/scene/components.hpp>
#include <render/frameContext.hpp>
#include <render/sceneRenderData.hpp>

#include <cmath>
#include <core/types.hpp>
#include <input/inputCodes.hpp>
#include <input/inputSnapshot.hpp>
#include <platform/window.hpp>

#include <entt/entity/entity.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{
using mental::core::Result;
using mental::editor::EditorApplication;
using mental::render::FrameContext;

bool nearlyEqual(float lhs, float rhs, float epsilon = 1.0e-4f)
{
  return std::fabs(lhs - rhs) <= epsilon;
}

bool nearlyEqual(const glm::vec3& lhs, const glm::vec3& rhs, float epsilon = 1.0e-4f)
{
  return nearlyEqual(lhs.x, rhs.x, epsilon) && nearlyEqual(lhs.y, rhs.y, epsilon) && nearlyEqual(lhs.z, rhs.z, epsilon);
}

bool nearlyEqual(const glm::mat4& lhs, const glm::mat4& rhs, float epsilon = 1.0e-4f)
{
  for (int column = 0; column < 4; ++column)
  {
    for (int row = 0; row < 4; ++row)
    {
      if (!nearlyEqual(lhs[column][row], rhs[column][row], epsilon))
      {
        return false;
      }
    }
  }

  return true;
}

mental::input::InputSnapshot makeInputSnapshot()
{
  return {};
}

struct FakeWindow final : mental::platform::IWindow
{
  mutable int pollEventsCount = 0;
  mutable int waitEventsCount = 0;
  mutable int cursorModeSetCount = 0;
  mutable int rawMouseMotionSetCount = 0;
  bool valid = true;
  bool closeRequested = false;
  mental::platform::WindowSize size {1280u, 720u};
  mental::platform::CursorMode cursorMode = mental::platform::CursorMode::eNormal;
  bool rawMouseMotionEnabled = false;
  mental::input::InputSnapshot inputSnapshot {};
  double timeSeconds = 0.0;

  Result init(const mental::platform::WindowDesc&) override
  {
    return Result::eSuccess;
  }

  void pollEvents() const override
  {
    ++pollEventsCount;
  }

  void waitEvents() const override
  {
    ++waitEventsCount;
  }

  double getTime() const override
  {
    return timeSeconds;
  }

  mental::input::InputSnapshot sampleInput() const override
  {
    return inputSnapshot;
  }

  bool shouldClose() const override
  {
    return closeRequested;
  }

  mental::platform::WindowSize getWindowSize() const override
  {
    return size;
  }

  void setCursorMode(mental::platform::CursorMode mode) override
  {
    ++cursorModeSetCount;
    cursorMode = mode;
  }

  void setRawMouseMotionEnabled(bool enabled) override
  {
    ++rawMouseMotionSetCount;
    rawMouseMotionEnabled = enabled;
  }

  bool isValid() const override
  {
    return valid;
  }

  void destroy() override
  {
    valid = false;
  }
};

struct FakeRenderSystem final : mental::render::IRenderSystem
{
  Result init(const mental::render::RenderSystemConfig&) override
  {
    initialized = true;
    return Result::eSuccess;
  }

  void destroy() override
  {
    destroyed = true;
  }

  bool isValid() const override
  {
    return initialized && !destroyed;
  }

  mental::render::RenderFrameOutcome render(const FrameContext& frameContext) override
  {
    frames.push_back(frameContext);
    return {
      .result = renderResult,
      .submitted = submitted,
    };
  }

  mental::core::resource::Object getNativeObject(mental::core::resource::ObjectType) override
  {
    return nullptr;
  }

  bool initialized = true;
  bool destroyed = false;
  Result renderResult = Result::eSuccess;
  bool submitted = true;
  std::vector<FrameContext> frames;
};

struct TestEditorApplication : EditorApplication
{
  using EditorApplication::bootstrapScene;
  using EditorApplication::collectInput;
  using EditorApplication::EditorApplication;
  using EditorApplication::frameContext;
  using EditorApplication::inputState;
  using EditorApplication::isShutdownRequested;
  using EditorApplication::lastError;
  using EditorApplication::renderFrame;
  using EditorApplication::scene;
  using EditorApplication::updateEditor;
  using EditorApplication::updatePlatform;
};

void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

void testDefaultConstruction()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(!app.isShutdownRequested(), "Construction should not request shutdown");
  require(app.lastError() == Result::eSuccess, "Construction should start without an error");
  require(app.scene().registry().view<mental::editor::PrimitiveComponent>().size() == 0u,
    "Construction should not bootstrap the scene");
}

void testCollectInputReportsFirstFrameKeyPress()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for first-frame key press test");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for first-frame key press test");

  require(app.inputState().isKeyDown(mental::input::KeyCode::eW), "First frame W should be down");
  require(app.inputState().wasKeyPressed(mental::input::KeyCode::eW), "First frame W should report pressed");
  require(!app.inputState().wasKeyReleased(mental::input::KeyCode::eW), "First frame W should not report released");
}

void testCollectInputDoesNotRepeatPressedWhileHeld()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for held key test");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for held key test");
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for held key test");

  require(app.inputState().isKeyDown(mental::input::KeyCode::eW), "Held W should remain down");
  require(!app.inputState().wasKeyPressed(mental::input::KeyCode::eW), "Held W should not re-report pressed");
  require(!app.inputState().wasKeyReleased(mental::input::KeyCode::eW), "Held W should not report released");
}

void testCollectInputReportsKeyRelease()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for key release test");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for key release test");

  window.inputSnapshot = makeInputSnapshot();
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for key release test");

  require(!app.inputState().isKeyDown(mental::input::KeyCode::eW), "Released W should not remain down");
  require(!app.inputState().wasKeyPressed(mental::input::KeyCode::eW), "Released W should not report pressed");
  require(app.inputState().wasKeyReleased(mental::input::KeyCode::eW), "Released W should report released");
}

void testCollectInputTracksMouseDeltaFromFrameToFrameMovement()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for mouse delta test");

  window.inputSnapshot.cursorPosition = {100.0, 150.0};
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for mouse delta test");
  require(app.inputState().mouseDelta().x == 0.0, "First sampled frame should not invent mouse delta X");
  require(app.inputState().mouseDelta().y == 0.0, "First sampled frame should not invent mouse delta Y");

  window.inputSnapshot.cursorPosition = {112.5, 132.0};
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for mouse delta test");

  require(app.inputState().mousePosition().x == 112.5, "Mouse position X should match the current snapshot");
  require(app.inputState().mousePosition().y == 132.0, "Mouse position Y should match the current snapshot");
  require(app.inputState().mouseDelta().x == 12.5, "Mouse delta X should reflect frame-to-frame movement");
  require(app.inputState().mouseDelta().y == -18.0, "Mouse delta Y should reflect frame-to-frame movement");
}

void testCollectInputConsumesOneFrameScrollDelta()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for scroll delta test");

  window.inputSnapshot.scrollDelta = {1.5, -2.0};
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for scroll delta test");
  require(app.inputState().scrollDelta().x == 1.5, "Scroll delta X should be available on the sampled frame");
  require(app.inputState().scrollDelta().y == -2.0, "Scroll delta Y should be available on the sampled frame");

  window.inputSnapshot = makeInputSnapshot();
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for scroll delta test");
  require(app.inputState().scrollDelta().x == 0.0, "Scroll delta X should clear on the next frame");
  require(app.inputState().scrollDelta().y == 0.0, "Scroll delta Y should clear on the next frame");
}

void testCollectInputRequestsTranslateModeWithW()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for W hotkey test");

  app.scene().setGizmoMode(mental::editor::GizmoMode::eScale);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);

  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for W hotkey test");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eTranslate, "W should request translate gizmo mode");
}

void testCollectInputRequestsRotateModeWithE()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for E hotkey test");

  app.scene().setGizmoMode(mental::editor::GizmoMode::eScale);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eE, true);

  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for E hotkey test");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eRotate, "E should request rotate gizmo mode");
}

void testCollectInputRequestsScaleModeWithR()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for R hotkey test");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eR, true);

  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for R hotkey test");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eScale, "R should request scale gizmo mode");
}

void testCollectInputReportsFlyLookAndSelectionIntents()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for interaction intent test");

  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eLeft, true);

  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for interaction intent test");
  require(app.inputState().isFlyLookActive(), "RMB held should report fly-look active");
  require(app.inputState().wantsSelectionClick(), "LMB press should request a selection click");
}

void testCollectInputReportsCameraBoostWithLeftShift()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for camera boost test");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eLeftShift, true);

  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for camera boost test");
  require(app.inputState().isCameraBoostActive(), "Left shift should report camera boost active");
}

void testCollectInputSuppressesGizmoHotkeysWhileFlyLookActive()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for fly-look hotkey suppression test");

  app.scene().setGizmoMode(mental::editor::GizmoMode::eScale);
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed while fly-look is active");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eScale,
    "W should not change gizmo mode while fly-look is active");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, false);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eE, true);
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for fly-look E hotkey suppression");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eScale,
    "E should not change gizmo mode while fly-look is active");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eE, false);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eR, true);
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for fly-look R hotkey suppression");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eScale,
    "R should not change gizmo mode while fly-look is active");
}

void testUpdateEditorEntersAndLeavesFlyLookCursorMode()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for fly-look cursor mode test");

  window.timeSeconds = 0.25;
  require(
    app.updatePlatform() == Result::eSuccess, "First updatePlatform should succeed for fly-look cursor mode test");
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for fly-look cursor mode test");
  require(app.updateEditor() == Result::eSuccess, "First updateEditor should succeed for fly-look cursor mode test");
  require(
    window.cursorMode == mental::platform::CursorMode::eNormal, "Cursor should remain normal before fly-look begins");
  require(window.cursorModeSetCount == 0, "Cursor mode should not change before fly-look begins");

  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 0.5;
  require(
    app.updatePlatform() == Result::eSuccess, "Second updatePlatform should succeed for fly-look cursor mode test");
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for fly-look cursor mode test");
  require(app.updateEditor() == Result::eSuccess, "Second updateEditor should succeed for fly-look cursor mode test");
  require(
    window.cursorMode == mental::platform::CursorMode::eDisabled, "Fly-look should disable the cursor while active");
  require(window.cursorModeSetCount == 1, "Fly-look start should request cursor mode once");
  require(window.rawMouseMotionEnabled, "Fly-look should enable raw mouse motion while active");
  require(window.rawMouseMotionSetCount == 1, "Fly-look start should request raw mouse motion once");

  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, false);
  window.timeSeconds = 0.75;
  require(
    app.updatePlatform() == Result::eSuccess, "Third updatePlatform should succeed for fly-look cursor mode test");
  require(app.collectInput() == Result::eSuccess, "Third collectInput should succeed for fly-look cursor mode test");
  require(app.updateEditor() == Result::eSuccess, "Third updateEditor should succeed for fly-look cursor mode test");
  require(
    window.cursorMode == mental::platform::CursorMode::eNormal, "Ending fly-look should restore the normal cursor");
  require(window.cursorModeSetCount == 2, "Fly-look end should request cursor mode once more");
  require(!window.rawMouseMotionEnabled, "Ending fly-look should disable raw mouse motion");
  require(window.rawMouseMotionSetCount == 2, "Fly-look end should request raw mouse motion once more");
}

void testUpdateEditorAppliesMouseDeltaToSceneCameraOrientation()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for mouse-look test");

  auto& camera = app.scene().sceneCamera();
  camera.setMouseLookSensitivity(0.25f);

  window.inputSnapshot.cursorPosition = {100.0, 100.0};
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 1.0;
  require(app.updatePlatform() == Result::eSuccess, "First updatePlatform should succeed for mouse-look test");
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for mouse-look test");
  require(app.updateEditor() == Result::eSuccess, "First updateEditor should succeed for mouse-look test");

  window.inputSnapshot.cursorPosition = {116.0, 92.0};
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 1.5;
  require(app.updatePlatform() == Result::eSuccess, "Second updatePlatform should succeed for mouse-look test");
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for mouse-look test");
  require(app.updateEditor() == Result::eSuccess, "Second updateEditor should succeed for mouse-look test");

  require(nearlyEqual(camera.yawDegrees(), -86.0f), "Mouse delta X should adjust yaw by sensitivity");
  require(nearlyEqual(camera.pitchDegrees(), 2.0f), "Mouse delta Y should adjust pitch by sensitivity");
}

void testUpdateEditorMovesSceneCameraInLocalSpaceWithDeltaTimeAndBoost()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for movement test");

  auto& camera = app.scene().sceneCamera();
  camera.setMoveSpeed(8.0f);
  camera.setBoostMultiplier(2.0f);

  window.inputSnapshot.cursorPosition = {50.0, 50.0};
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 1.0;
  require(app.updatePlatform() == Result::eSuccess, "First updatePlatform should succeed for movement test");
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for movement test");
  require(app.updateEditor() == Result::eSuccess, "First updateEditor should succeed for movement test");

  window.inputSnapshot.cursorPosition = {50.0, 50.0};
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eD, true);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eLeftShift, true);
  window.timeSeconds = 1.5;
  require(app.updatePlatform() == Result::eSuccess, "Second updatePlatform should succeed for movement test");
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for movement test");
  require(app.updateEditor() == Result::eSuccess, "Second updateEditor should succeed for movement test");

  require(nearlyEqual(camera.worldPosition(), glm::vec3 {8.0f, 0.0f, -3.0f}),
    "Fly-look movement should use local axes, delta time, and boost");
}

void testUpdateEditorDoesNotMoveSceneCameraWithoutFlyLookActive()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for no-fly-look movement test");

  auto& camera = app.scene().sceneCamera();
  camera.setMoveSpeed(4.0f);
  const glm::vec3 initialPosition = camera.worldPosition();

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
  window.timeSeconds = 0.5;
  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for no-fly-look movement test");
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for no-fly-look movement test");
  require(app.updateEditor() == Result::eSuccess, "updateEditor should succeed for no-fly-look movement test");

  require(nearlyEqual(camera.worldPosition(), initialPosition),
    "W should not move the scene camera unless fly-look is active");
}

void testUpdateEditorBoostMovesSceneCameraFartherThanUnboostedMovement()
{
  auto measureTravelDistance = [](bool boosted)
  {
    FakeWindow window;
    FakeRenderSystem renderer;
    TestEditorApplication app {&window, &renderer};
    require(app.init() == Result::eSuccess, "Application init should succeed for boost comparison test");

    auto& camera = app.scene().sceneCamera();
    camera.setMoveSpeed(4.0f);
    camera.setBoostMultiplier(2.0f);

    window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
    window.inputSnapshot.setKeyDown(mental::input::KeyCode::eW, true);
    if (boosted)
    {
      window.inputSnapshot.setKeyDown(mental::input::KeyCode::eLeftShift, true);
    }
    window.timeSeconds = 0.5;
    require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for boost comparison test");
    require(app.collectInput() == Result::eSuccess, "collectInput should succeed for boost comparison test");
    require(app.updateEditor() == Result::eSuccess, "updateEditor should succeed for boost comparison test");

    return camera.worldPosition().z;
  };

  const float initialZ = 5.0f;
  const float unboostedZ = measureTravelDistance(false);
  const float boostedZ = measureTravelDistance(true);
  const float unboostedTravel = initialZ - unboostedZ;
  const float boostedTravel = initialZ - boostedZ;

  require(nearlyEqual(unboostedTravel, 2.0f), "Unboosted movement should travel the base distance over the frame");
  require(nearlyEqual(boostedTravel, 4.0f), "Boosted movement should travel twice the base distance over the frame");
  require(boostedTravel > unboostedTravel, "Boost should increase travel distance over the same delta time");
}

void testUpdateEditorMovesSceneCameraVerticallyWithQAndE()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for vertical movement test");

  auto& camera = app.scene().sceneCamera();
  camera.setMoveSpeed(4.0f);

  window.inputSnapshot.cursorPosition = {25.0, 25.0};
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 0.5;
  require(app.updatePlatform() == Result::eSuccess, "First updatePlatform should succeed for vertical movement test");
  require(app.collectInput() == Result::eSuccess, "First collectInput should succeed for vertical movement test");
  require(app.updateEditor() == Result::eSuccess, "First updateEditor should succeed for vertical movement test");

  window.inputSnapshot.cursorPosition = {25.0, 25.0};
  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eE, true);
  window.timeSeconds = 0.75;
  require(app.updatePlatform() == Result::eSuccess, "Second updatePlatform should succeed for vertical movement test");
  require(app.collectInput() == Result::eSuccess, "Second collectInput should succeed for vertical movement test");
  require(app.updateEditor() == Result::eSuccess, "Second updateEditor should succeed for vertical movement test");
  require(nearlyEqual(camera.worldPosition(), glm::vec3 {0.0f, 1.0f, 5.0f}),
    "E should move the fly camera upward in local space");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eE, false);
  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eQ, true);
  window.timeSeconds = 1.0;
  require(app.updatePlatform() == Result::eSuccess, "Third updatePlatform should succeed for vertical movement test");
  require(app.collectInput() == Result::eSuccess, "Third collectInput should succeed for vertical movement test");
  require(app.updateEditor() == Result::eSuccess, "Third updateEditor should succeed for vertical movement test");
  require(nearlyEqual(camera.worldPosition(), glm::vec3 {0.0f, 0.0f, 5.0f}),
    "Q should move the fly camera downward in local space");
}

void testWindowContractTracksCursorAndRawMouseRequests()
{
  FakeWindow window;

  require(window.cursorMode == mental::platform::CursorMode::eNormal, "Window should start in normal cursor mode");
  require(!window.rawMouseMotionEnabled, "Window should start with raw mouse motion disabled");

  window.setCursorMode(mental::platform::CursorMode::eDisabled);
  window.setRawMouseMotionEnabled(true);
  window.setRawMouseMotionEnabled(false);
  window.setCursorMode(mental::platform::CursorMode::eNormal);

  require(
    window.cursorMode == mental::platform::CursorMode::eNormal, "Window should store the requested normal cursor mode");
  require(!window.rawMouseMotionEnabled, "Window should store the requested raw mouse motion disable state");
  require(window.cursorModeSetCount == 2, "Cursor mode requests should be tracked");
  require(window.rawMouseMotionSetCount == 2, "Raw mouse motion requests should be tracked");
}

void testShutdownRestoresFlyLookInputState()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for shutdown cleanup test");

  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 0.25;
  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for shutdown cleanup test");
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for shutdown cleanup test");
  require(app.updateEditor() == Result::eSuccess, "updateEditor should succeed for shutdown cleanup test");
  require(
    window.cursorMode == mental::platform::CursorMode::eDisabled, "Fly-look should be active before shutdown cleanup");
  require(window.rawMouseMotionEnabled, "Fly-look should enable raw mouse motion before shutdown cleanup");

  app.shutdown();

  require(window.cursorMode == mental::platform::CursorMode::eNormal,
    "Shutdown should restore the normal cursor immediately");
  require(!window.rawMouseMotionEnabled, "Shutdown should disable raw mouse motion immediately");
  require(window.cursorModeSetCount == 2, "Shutdown cleanup should issue one extra cursor request");
  require(window.rawMouseMotionSetCount == 2, "Shutdown cleanup should issue one extra raw mouse request");
}

void testRunFailureRestoresFlyLookInputState()
{
  FakeWindow window;
  FakeRenderSystem renderer;

  struct FailingRenderApp final : TestEditorApplication
  {
    using TestEditorApplication::TestEditorApplication;

    Result renderFrame() override
    {
      return Result::eOperationFailed;
    }
  };

  FailingRenderApp app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for failure cleanup test");

  window.inputSnapshot.setMouseButtonDown(mental::input::MouseButton::eRight, true);
  window.timeSeconds = 0.25;
  require(app.run() == Result::eOperationFailed, "Render failure should propagate from run for cleanup test");
  require(window.cursorMode == mental::platform::CursorMode::eNormal, "Run failure should restore the normal cursor");
  require(!window.rawMouseMotionEnabled, "Run failure should disable raw mouse motion");
}

void testInitFailureUpdatesLastError()
{
  FakeWindow window;
  window.valid = false;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eInitializationFailed, "Init should fail for an invalid window");
  require(app.lastError() == Result::eInitializationFailed, "Failed init should update lastError");
}

void testCollectInputSamplesInputWhileMinimized()
{
  FakeWindow window;
  window.size = {};
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed for minimized input test");

  window.inputSnapshot.setKeyDown(mental::input::KeyCode::eE, true);

  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for minimized input test");
  require(app.collectInput() == Result::eSuccess, "collectInput should succeed for minimized input test");
  require(window.waitEventsCount == 1, "Minimized input test should still wait for events");
  require(app.scene().gizmoMode() == mental::editor::GizmoMode::eRotate,
    "collectInput should still process hotkeys while minimized");
}

void testBootstrapOccursInApplicationInit()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed");
  require(app.scene().registry().view<mental::editor::PrimitiveComponent>().size() == 1u,
    "Application init should bootstrap the scene");

  entt::entity bootstrapEntity = entt::null;
  const auto view = app.scene().registry().view<mental::editor::PrimitiveComponent>();
  require(view.size() == 1u, "Application init should create one bootstrap primitive");
  for (const entt::entity entity : view)
  {
    bootstrapEntity = entity;
  }

  require(bootstrapEntity != entt::null, "Bootstrap primitive should exist");
  require(app.scene().registry().get<mental::editor::PrimitiveComponent>(bootstrapEntity).type ==
            mental::editor::PrimitiveType::eCube,
    "Bootstrap primitive should be a cube");
}

void testRunCallsPhasesInOrder()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed");

  std::vector<std::string> phases;
  struct OrderedApp final : TestEditorApplication
  {
    using TestEditorApplication::TestEditorApplication;

    std::vector<std::string>* phases = nullptr;

    Result updatePlatform() override
    {
      phases->push_back("updatePlatform");
      return Result::eSuccess;
    }

    Result collectInput() override
    {
      phases->push_back("collectInput");
      return Result::eSuccess;
    }

    Result updateEditor() override
    {
      phases->push_back("updateEditor");
      return Result::eSuccess;
    }

    Result renderFrame() override
    {
      phases->push_back("renderFrame");
      shutdown();
      return Result::eSuccess;
    }
  };

  OrderedApp orderedApp {&window, &renderer};
  orderedApp.phases = &phases;
  require(orderedApp.init() == Result::eSuccess, "Ordered app init should succeed");
  require(orderedApp.run() == Result::eSuccess, "Ordered app run should succeed");
  require(phases.size() == 4u, "One frame should call four phases");
  require(phases[0] == "updatePlatform", "Phase order 1");
  require(phases[1] == "collectInput", "Phase order 2");
  require(phases[2] == "updateEditor", "Phase order 3");
  require(phases[3] == "renderFrame", "Phase order 4");
}

void testFailureStopsLoop()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};
  require(app.init() == Result::eSuccess, "Application init should succeed");

  struct FailingApp final : TestEditorApplication
  {
    using TestEditorApplication::TestEditorApplication;

    std::vector<std::string>* phases = nullptr;

    Result updatePlatform() override
    {
      phases->push_back("updatePlatform");
      return Result::eSuccess;
    }

    Result collectInput() override
    {
      phases->push_back("collectInput");
      return Result::eSuccess;
    }

    Result updateEditor() override
    {
      phases->push_back("updateEditor");
      return Result::eOperationFailed;
    }

    Result renderFrame() override
    {
      phases->push_back("renderFrame");
      return Result::eSuccess;
    }
  };

  std::vector<std::string> phases;
  FailingApp failingApp {&window, &renderer};
  failingApp.phases = &phases;
  require(failingApp.init() == Result::eSuccess, "Failing app init should succeed");
  require(failingApp.run() == Result::eOperationFailed, "Failure should propagate from the loop");
  require(phases.size() == 3u, "Failure should stop before renderFrame and the next loop");
  require(phases[0] == "updatePlatform", "Failure order 1");
  require(phases[1] == "collectInput", "Failure order 2");
  require(phases[2] == "updateEditor", "Failure order 3");
}

void testMinimizedFrameWaitsWithoutRendering()
{
  FakeWindow window;
  window.size = {};
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed for minimized test");
  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for minimized window");
  require(window.waitEventsCount == 1, "Minimized frame should wait for events");
  require(window.pollEventsCount == 0, "Minimized frame should not poll events");
  require(app.updateEditor() == Result::eSuccess, "updateEditor should no-op for minimized window");
  require(app.renderFrame() == Result::eSuccess, "renderFrame should no-op for minimized window");
  require(renderer.frames.empty(), "Renderer should not be called for minimized frame");
}

void testRenderFrameBuildsFrameContext()
{
  FakeWindow window;
  window.timeSeconds = 5.0;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed for frame context test");

  window.timeSeconds = 5.5;
  require(app.updatePlatform() == Result::eSuccess, "First updatePlatform should succeed");
  require(app.renderFrame() == Result::eSuccess, "First renderFrame should succeed");
  require(renderer.frames.size() == 1u, "Renderer should receive first frame");
  require(renderer.frames[0].absoluteTimeSeconds == 5.5, "Frame absolute time should match observed time");
  require(renderer.frames[0].deltaTimeSeconds == 0.5, "Frame delta time should match elapsed time");
  require(renderer.frames[0].framebufferSize.width == 1280u, "Frame width should match window");
  require(renderer.frames[0].framebufferSize.height == 720u, "Frame height should match window");
  require(!renderer.frames[0].framebufferResized, "First observed frame should not report resize");
  require(renderer.frames[0].frameIndex.has_value() && renderer.frames[0].frameIndex.value() == 0u,
    "First frame index should be zero");

  window.size = {1920u, 1080u};
  window.timeSeconds = 6.0;
  require(app.updatePlatform() == Result::eSuccess, "Second updatePlatform should succeed");
  require(app.renderFrame() == Result::eSuccess, "Second renderFrame should succeed");
  require(renderer.frames.size() == 2u, "Renderer should receive second frame");
  require(renderer.frames[1].absoluteTimeSeconds == 6.0, "Second frame absolute time should match observed time");
  require(renderer.frames[1].deltaTimeSeconds == 0.5, "Second frame delta time should match elapsed time");
  require(renderer.frames[1].framebufferSize.width == 1920u, "Second frame width should match resized window");
  require(renderer.frames[1].framebufferSize.height == 1080u, "Second frame height should match resized window");
  require(renderer.frames[1].framebufferResized, "Second frame should report resize");
  require(renderer.frames[1].frameIndex.has_value() && renderer.frames[1].frameIndex.value() == 1u,
    "Second frame index should increment");
}

void testRenderFramePopulatesSceneRenderPayload()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed for scene payload test");
  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for scene payload test");
  require(app.renderFrame() == Result::eSuccess, "renderFrame should succeed for scene payload test");
  require(renderer.frames.size() == 1u, "Renderer should receive one frame for scene payload test");

  const FrameContext& frameContext = renderer.frames[0];
  const mental::render::SceneRenderFrame& sceneRenderFrame = frameContext.sceneRenderFrame;
  const float aspectRatio = 1280.0f / 720.0f;
  const auto& sceneCamera = app.scene().sceneCamera();

  require(nearlyEqual(sceneRenderFrame.camera.aspectRatio, aspectRatio),
    "Scene camera payload should use the framebuffer aspect ratio");
  require(nearlyEqual(sceneRenderFrame.camera.worldPosition, sceneCamera.worldPosition()),
    "Scene camera payload should copy the camera world position");
  require(nearlyEqual(sceneRenderFrame.camera.view, sceneCamera.viewMatrix()),
    "Scene camera payload should copy the camera view matrix");
  require(nearlyEqual(sceneRenderFrame.camera.projection, sceneCamera.projectionMatrix(aspectRatio)),
    "Scene camera payload should copy the camera projection matrix");
  require(nearlyEqual(
            sceneRenderFrame.camera.viewProjection, sceneRenderFrame.camera.projection * sceneRenderFrame.camera.view),
    "Scene camera payload should precompute viewProjection as projection * view");
  require(sceneRenderFrame.objects.size() == 1u, "Bootstrap cube should be submitted as one render object");
  require(sceneRenderFrame.objects[0].geometryKind == mental::render::SceneGeometryKind::eCube,
    "Bootstrap cube should package as cube geometry");
  require(nearlyEqual(sceneRenderFrame.objects[0].worldTransform, glm::mat4 {1.0f}),
    "Bootstrap cube should use an identity world transform");
}

void testRenderFrameIncludesNewPrimitiveInScenePayload()
{
  FakeWindow window;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed for primitive count test");

  entt::entity plane = entt::null;
  require(app.scene().createPrimitive(mental::editor::PrimitiveType::ePlane, plane) == Result::eSuccess,
    "Creating a new primitive should succeed");

  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for primitive count test");
  require(app.renderFrame() == Result::eSuccess, "renderFrame should succeed for primitive count test");
  require(renderer.frames.size() == 1u, "Renderer should receive one frame for primitive count test");
  require(renderer.frames[0].sceneRenderFrame.objects.size() == 2u,
    "Adding a primitive should increase the submitted render object count");
}

void testRenderFramePreservesScenePayloadAcrossResize()
{
  FakeWindow window;
  window.timeSeconds = 1.0;
  FakeRenderSystem renderer;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed for resize payload test");

  window.timeSeconds = 1.5;
  require(app.updatePlatform() == Result::eSuccess, "First updatePlatform should succeed for resize payload test");
  require(app.renderFrame() == Result::eSuccess, "First renderFrame should succeed for resize payload test");
  require(renderer.frames.size() == 1u, "Renderer should receive the first frame for resize payload test");

  window.size = {1920u, 1080u};
  window.timeSeconds = 2.0;
  require(app.updatePlatform() == Result::eSuccess, "Second updatePlatform should succeed for resize payload test");
  require(app.renderFrame() == Result::eSuccess, "Second renderFrame should succeed for resize payload test");
  require(renderer.frames.size() == 2u, "Renderer should receive the second frame for resize payload test");

  const FrameContext& resizedFrame = renderer.frames[1];
  const mental::render::SceneRenderFrame& sceneRenderFrame = resizedFrame.sceneRenderFrame;
  const float aspectRatio = 1920.0f / 1080.0f;

  require(resizedFrame.absoluteTimeSeconds == 2.0, "Resize frame should update the absolute time");
  require(resizedFrame.deltaTimeSeconds == 0.5, "Resize frame should update the delta time");
  require(resizedFrame.framebufferSize.width == 1920u, "Resize frame should update the framebuffer width");
  require(resizedFrame.framebufferSize.height == 1080u, "Resize frame should update the framebuffer height");
  require(resizedFrame.framebufferResized, "Resize frame should report the framebuffer resize");
  require(sceneRenderFrame.objects.size() == 1u, "Resize should preserve the bootstrap cube render payload");
  require(nearlyEqual(sceneRenderFrame.camera.aspectRatio, aspectRatio),
    "Resize should update the scene camera aspect ratio");
  require(nearlyEqual(
            sceneRenderFrame.camera.viewProjection, sceneRenderFrame.camera.projection * sceneRenderFrame.camera.view),
    "Resize should keep the camera viewProjection consistent");
}

void testRenderFrameDoesNotAdvanceIndexForNoOpSuccess()
{
  FakeWindow window;
  window.timeSeconds = 1.0;
  FakeRenderSystem renderer;
  renderer.submitted = false;
  TestEditorApplication app {&window, &renderer};

  require(app.init() == Result::eSuccess, "Application init should succeed for no-op render test");

  window.timeSeconds = 2.0;
  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for first no-op frame");
  require(app.renderFrame() == Result::eSuccess, "renderFrame should succeed for first no-op frame");
  require(renderer.frames.size() == 1u, "Renderer should still observe the first no-op frame");
  require(renderer.frames[0].frameIndex.has_value() && renderer.frames[0].frameIndex.value() == 0u,
    "First no-op success should not advance the rendered frame index");

  window.timeSeconds = 3.0;
  require(app.updatePlatform() == Result::eSuccess, "updatePlatform should succeed for second no-op frame");
  require(app.renderFrame() == Result::eSuccess, "renderFrame should succeed for second no-op frame");
  require(renderer.frames.size() == 2u, "Renderer should still observe the second no-op frame");
  require(renderer.frames[1].frameIndex.has_value() && renderer.frames[1].frameIndex.value() == 0u,
    "Second no-op success should reuse the same rendered frame index");
}
} // namespace

int main()
{
  try
  {
    testDefaultConstruction();
    testCollectInputReportsFirstFrameKeyPress();
    testCollectInputDoesNotRepeatPressedWhileHeld();
    testCollectInputReportsKeyRelease();
    testCollectInputTracksMouseDeltaFromFrameToFrameMovement();
    testCollectInputConsumesOneFrameScrollDelta();
    testBootstrapOccursInApplicationInit();
    testCollectInputRequestsTranslateModeWithW();
    testCollectInputRequestsRotateModeWithE();
    testCollectInputRequestsScaleModeWithR();
    testCollectInputSuppressesGizmoHotkeysWhileFlyLookActive();
    testCollectInputReportsFlyLookAndSelectionIntents();
    testCollectInputReportsCameraBoostWithLeftShift();
    testUpdateEditorEntersAndLeavesFlyLookCursorMode();
    testUpdateEditorAppliesMouseDeltaToSceneCameraOrientation();
    testUpdateEditorMovesSceneCameraInLocalSpaceWithDeltaTimeAndBoost();
    testUpdateEditorDoesNotMoveSceneCameraWithoutFlyLookActive();
    testUpdateEditorBoostMovesSceneCameraFartherThanUnboostedMovement();
    testUpdateEditorMovesSceneCameraVerticallyWithQAndE();
    testWindowContractTracksCursorAndRawMouseRequests();
    testShutdownRestoresFlyLookInputState();
    testRunFailureRestoresFlyLookInputState();
    testInitFailureUpdatesLastError();
    testRunCallsPhasesInOrder();
    testFailureStopsLoop();
    testMinimizedFrameWaitsWithoutRendering();
    testCollectInputSamplesInputWhileMinimized();
    testRenderFrameBuildsFrameContext();
    testRenderFramePopulatesSceneRenderPayload();
    testRenderFrameIncludesNewPrimitiveInScenePayload();
    testRenderFramePreservesScenePayloadAcrossResize();
    testRenderFrameDoesNotAdvanceIndexForNoOpSuccess();
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unknown exception\n";
  }

  return 1;
}
