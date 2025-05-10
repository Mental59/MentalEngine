#pragma once

namespace mental {
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
} // namespace mental
