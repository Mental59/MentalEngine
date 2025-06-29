#include "baseApp.hpp"

void app::BaseApp::onMousePosUpdate(float x, float y, int width, int height) {
  mMouseState.pos = glm::vec2(x, y);

  mMouseState.posNormalized.x = static_cast<float>(x / width);
  mMouseState.posNormalized.y = static_cast<float>(y / height);
}

void app::BaseApp::onMousePressedLeft(bool pressed) {
  mMouseState.pressedLeft = pressed;
}

void app::BaseApp::onMousePressedRight(bool pressed) {
  mMouseState.pressedRight = pressed;
}

void app::BaseApp::onMousePressedMiddle(bool pressed) {
  mMouseState.pressedMiddle = pressed;
}

void app::BaseApp::onKeyPressed(int key, bool pressed) {}
