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

  virtual bool isValid() const override;

  virtual void pollEvents() const override;
  virtual void waitEvents() const override;
  virtual double getTime() const override;
  virtual bool shouldClose() const override;

 private:
  GLFWwindow* mWindow;
  bool mIsInitialized = false;
};

} // namespace mental::platform
