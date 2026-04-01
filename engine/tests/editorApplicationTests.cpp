#include <editor/app/editorApplication.hpp>
#include <editor/scene/components.hpp>
#include <render/frameContext.hpp>

#include <core/types.hpp>
#include <platform/inputCodes.hpp>
#include <platform/inputSnapshot.hpp>
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

struct FakeWindow final : mental::platform::IWindow
{
  mutable int pollEventsCount = 0;
  mutable int waitEventsCount = 0;
  bool valid = true;
  bool closeRequested = false;
  mental::platform::WindowSize size {1280u, 720u};
  mental::platform::InputSnapshot inputSnapshot {};
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

  mental::platform::InputSnapshot sampleInput() const override
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
  using EditorApplication::EditorApplication;
  using EditorApplication::bootstrapScene;
  using EditorApplication::collectInput;
  using EditorApplication::frameContext;
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

void testInputSnapshotDefaultsToReleasedStatesAndZeroDeltas()
{
  const mental::platform::InputSnapshot snapshot {};

  require(snapshot.cursorPosition.x == 0.0, "Default cursor X should be zero");
  require(snapshot.cursorPosition.y == 0.0, "Default cursor Y should be zero");
  require(snapshot.scrollDelta.x == 0.0, "Default scroll X should be zero");
  require(snapshot.scrollDelta.y == 0.0, "Default scroll Y should be zero");
  require(!snapshot.isKeyDown(mental::platform::KeyCode::eW), "Default W state should be released");
  require(!snapshot.isKeyDown(mental::platform::KeyCode::eEscape), "Default Escape state should be released");
  require(!snapshot.isMouseButtonDown(mental::platform::MouseButton::eLeft), "Default left button should be released");
  require(!snapshot.isMouseButtonDown(mental::platform::MouseButton::eMiddle),
    "Default middle button should be released");
}

void testFakeWindowReturnsConfiguredInputSnapshot()
{
  FakeWindow window;
  window.inputSnapshot.cursorPosition = {320.0, 240.0};
  window.inputSnapshot.scrollDelta = {-1.0, 2.0};
  window.inputSnapshot.setKeyDown(mental::platform::KeyCode::eA, true);
  window.inputSnapshot.setKeyDown(mental::platform::KeyCode::eEscape, true);
  window.inputSnapshot.setMouseButtonDown(mental::platform::MouseButton::eRight, true);

  const mental::platform::InputSnapshot snapshot = window.sampleInput();

  require(snapshot.cursorPosition.x == 320.0, "Sampled cursor X should match configured value");
  require(snapshot.cursorPosition.y == 240.0, "Sampled cursor Y should match configured value");
  require(snapshot.scrollDelta.x == -1.0, "Sampled scroll X should match configured value");
  require(snapshot.scrollDelta.y == 2.0, "Sampled scroll Y should match configured value");
  require(snapshot.isKeyDown(mental::platform::KeyCode::eA), "Sampled A state should match configured value");
  require(snapshot.isKeyDown(mental::platform::KeyCode::eEscape),
    "Sampled Escape state should match configured value");
  require(snapshot.isMouseButtonDown(mental::platform::MouseButton::eRight),
    "Sampled right mouse state should match configured value");
  require(!snapshot.isMouseButtonDown(mental::platform::MouseButton::eLeft),
    "Unconfigured left mouse state should remain released");
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
  require(app.scene().registry().get<mental::editor::PrimitiveComponent>(bootstrapEntity).type
      == mental::editor::PrimitiveType::eCube,
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
    testInputSnapshotDefaultsToReleasedStatesAndZeroDeltas();
    testFakeWindowReturnsConfiguredInputSnapshot();
    testBootstrapOccursInApplicationInit();
    testRunCallsPhasesInOrder();
    testFailureStopsLoop();
    testMinimizedFrameWaitsWithoutRendering();
    testRenderFrameBuildsFrameContext();
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
