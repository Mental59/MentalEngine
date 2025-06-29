#pragma once

#include <glm/glm.hpp>

namespace app {
struct MouseState {
  glm::vec2 pos = glm::vec2(0.0f);
  glm::vec2 posNormalized = glm::vec2(0.0f);
  bool pressedLeft = false;
  bool pressedRight = false;
  bool pressedMiddle = false;
};

class BaseApp {
public:
  BaseApp() = default;
  virtual ~BaseApp() {}

  void setFramebufferResized(bool framebufferResized) {
    mFramebufferResized = framebufferResized;
  }

  virtual void onMousePosUpdate(float x, float y, int width, int height);
  virtual void onMousePressedLeft(bool pressed);
  virtual void onMousePressedRight(bool pressed);
  virtual void onMousePressedMiddle(bool pressed);

  virtual void onKeyPressed(int key, bool pressed);

protected:
  bool mFramebufferResized = false;
  MouseState mMouseState;
};
} // namespace app
