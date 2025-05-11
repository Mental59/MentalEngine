#pragma once

namespace app {
class BaseApp {
public:
  BaseApp() = default;
  virtual ~BaseApp() {}

  void setFramebufferResized(bool framebufferResized) {
    mFramebufferResized = framebufferResized;
  }

protected:
  bool mFramebufferResized = false;
};
} // namespace app
