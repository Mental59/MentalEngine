#pragma once
#include <platform/window.hpp>
#include <core/types.hpp>

class GLFWwindow;

namespace mental::platform
{
class PCWindow : public IWindow
{
 public:
  PCWindow() = default;
  virtual core::resource::Object getNativeObject(core::resource::ObjectType objectType) override;
  virtual core::Result init(const WindowDesc& desc) override;
  virtual void destroy() override;

  virtual WindowSize getWindowSize() const override;
  [[nodiscard]] virtual input::InputSnapshot sampleInput() const override;
  virtual void setCursorMode(CursorMode mode) override;
  virtual void setRawMouseMotionEnabled(bool enabled) override;

  virtual bool isValid() const override;

  virtual void pollEvents() const override;
  virtual void waitEvents() const override;
  virtual double getTime() const override;
  virtual bool shouldClose() const override;

 private:
  static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

  GLFWwindow* mWindow = nullptr;
  mutable input::ScrollDelta mAccumulatedScrollDelta {};
  CursorMode mCursorMode = CursorMode::eNormal;
  bool mRawMouseMotionEnabled = false;
  bool mIsInitialized = false;
};

} // namespace mental::platform
