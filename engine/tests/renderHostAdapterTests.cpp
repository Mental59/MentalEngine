#include <core/types.hpp>
#include <input/inputSnapshot.hpp>
#include <platform/window.hpp>
#include <render/render.hpp>
#include <render/renderHostAdapter.hpp>

#include <exception>
#include <iostream>
#include <stdexcept>

namespace
{
using mental::core::Result;

struct FakeWindow final : mental::platform::IWindow
{
  Result init(const mental::platform::WindowDesc&) override
  {
    return Result::eSuccess;
  }

  void pollEvents() const override
  {
  }

  void waitEvents() const override
  {
    ++waitEventsCount;
    if (waitEventsCount <= scriptedSizesCount)
    {
      currentSize = scriptedSizes[waitEventsCount - 1];
    }
  }

  double getTime() const override
  {
    return 0.0;
  }

  mental::input::InputSnapshot sampleInput() const override
  {
    return {};
  }

  bool shouldClose() const override
  {
    return closeRequested;
  }

  mental::platform::WindowSize getWindowSize() const override
  {
    return currentSize;
  }

  bool isValid() const override
  {
    return valid;
  }

  void destroy() override
  {
    valid = false;
  }

  mutable mental::platform::WindowSize scriptedSizes[4] {};
  int scriptedSizesCount = 0;
  mutable int waitEventsCount = 0;
  mutable mental::platform::WindowSize currentSize {1280u, 720u};
  bool closeRequested = false;
  bool valid = true;
};

void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

void testRenderSystemConfigAcceptsRenderHostAdapter()
{
  FakeWindow window;
  mental::render::WindowRenderHostAdapter adapter(&window);

  mental::render::RenderSystemConfig config {
    .graphicsApi = mental::rhi::GraphicsApi::Vulkan,
    .hostAdapter = &adapter,
  };

  require(config.hostAdapter == &adapter, "RenderSystemConfig should store the render host adapter");
}

void testCreateDeviceInitInputProvidesSurfaceFactory()
{
  FakeWindow window;
  mental::render::WindowRenderHostAdapter adapter(&window);

  const mental::rhi::DeviceInitInput initInput = adapter.createDeviceInitInput(mental::rhi::GraphicsApi::Vulkan);

  require(initInput.vulkanSurface.createSurface != nullptr, "Vulkan init input should provide a surface callback");
  require(initInput.vulkanSurface.userData == &window, "Vulkan init input should retain the window as callback context");
}

void testValidateDeviceInitInputAcceptsWindowAdapterInput()
{
  FakeWindow window;
  mental::render::WindowRenderHostAdapter adapter(&window);

  const mental::rhi::DeviceInitInput initInput = adapter.createDeviceInitInput(mental::rhi::GraphicsApi::Vulkan);

  require(mental::rhi::validateDeviceInitInput(mental::rhi::GraphicsApi::Vulkan, initInput) == Result::eSuccess,
    "Window-backed init input should validate successfully");
}

void testValidateDeviceInitInputRejectsMissingSurfaceFactory()
{
  const mental::rhi::DeviceInitInput initInput {};

  require(
    mental::rhi::validateDeviceInitInput(mental::rhi::GraphicsApi::Vulkan, initInput) == Result::eInitializationFailed,
    "Missing Vulkan surface creation input should fail validation");
}

void testValidateDeviceInitInputRejectsNullWindowContext()
{
  mental::render::WindowRenderHostAdapter adapter(nullptr);
  const mental::rhi::DeviceInitInput initInput = adapter.createDeviceInitInput(mental::rhi::GraphicsApi::Vulkan);

  require(
    mental::rhi::validateDeviceInitInput(mental::rhi::GraphicsApi::Vulkan, initInput) == Result::eInitializationFailed,
    "A window-backed adapter without a window should fail validation");
}

void testFenceResetOccursOnlyForSubmitEligibleAcquireResult()
{
  require(mental::render::isSubmitEligibleAcquireResult(Result::eSuccess),
    "Successful acquire should remain eligible for submission");
  require(!mental::render::isSubmitEligibleAcquireResult(Result::eSuboptimal),
    "Suboptimal acquire should not reset the frame fence before resize handling");
  require(!mental::render::isSubmitEligibleAcquireResult(Result::eOutOfDate),
    "Out-of-date acquire should not reset the frame fence before resize handling");
}

void testRecoverNextFramebufferExtentReturnsCurrentExtent()
{
  FakeWindow window;
  window.currentSize = {1600u, 900u};

  mental::render::WindowRenderHostAdapter adapter(&window);
  const mental::render::FramebufferExtentRecoveryResult result = adapter.recoverNextFramebufferExtent();

  require(result.status == mental::render::FramebufferExtentRecoveryStatus::eUsableExtent,
    "Usable framebuffer should be reported when the extent is already available");
  require(result.extent.width == 1600u, "Recovered width should match the current framebuffer width");
  require(result.extent.height == 900u, "Recovered height should match the current framebuffer height");
  require(window.waitEventsCount == 0, "No event wait should occur when the framebuffer is already usable");
}

void testRecoverNextFramebufferExtentWaitsForUsableExtent()
{
  FakeWindow window;
  window.currentSize = {};
  window.scriptedSizes[0] = {};
  window.scriptedSizes[1] = {1920u, 1080u};
  window.scriptedSizesCount = 2;

  mental::render::WindowRenderHostAdapter adapter(&window);
  const mental::render::FramebufferExtentRecoveryResult result = adapter.recoverNextFramebufferExtent();

  require(result.status == mental::render::FramebufferExtentRecoveryStatus::eUsableExtent,
    "Adapter should keep waiting until a usable framebuffer extent exists");
  require(result.extent.width == 1920u, "Recovered width should come from the first usable extent");
  require(result.extent.height == 1080u, "Recovered height should come from the first usable extent");
  require(window.waitEventsCount == 2, "Adapter should wait once per minimized extent observation");
}

void testRecoverNextFramebufferExtentReturnsClosing()
{
  FakeWindow window;
  window.currentSize = {};
  window.closeRequested = true;

  mental::render::WindowRenderHostAdapter adapter(&window);
  const mental::render::FramebufferExtentRecoveryResult result = adapter.recoverNextFramebufferExtent();

  require(result.status == mental::render::FramebufferExtentRecoveryStatus::eClosing,
    "Close requests should stop framebuffer recovery");
  require(result.extent.width == 0u, "Closing should not report a usable width");
  require(result.extent.height == 0u, "Closing should not report a usable height");
  require(window.waitEventsCount == 0, "Adapter should not wait for events after a close request");
}
} // namespace

int main()
{
  try
  {
    testRenderSystemConfigAcceptsRenderHostAdapter();
    testCreateDeviceInitInputProvidesSurfaceFactory();
    testValidateDeviceInitInputAcceptsWindowAdapterInput();
    testValidateDeviceInitInputRejectsMissingSurfaceFactory();
    testValidateDeviceInitInputRejectsNullWindowContext();
    testFenceResetOccursOnlyForSubmitEligibleAcquireResult();
    testRecoverNextFramebufferExtentReturnsCurrentExtent();
    testRecoverNextFramebufferExtentWaitsForUsableExtent();
    testRecoverNextFramebufferExtentReturnsClosing();
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
