#pragma once

#include <array>

#include <platform/inputCodes.hpp>

namespace mental::platform
{
struct CursorPosition
{
  double x = 0.0;
  double y = 0.0;
};

struct ScrollDelta
{
  double x = 0.0;
  double y = 0.0;
};

struct InputSnapshot
{
  CursorPosition cursorPosition {};
  ScrollDelta scrollDelta {};
  std::array<bool, kKeyCodes.size()> keyDown {};
  std::array<bool, kMouseButtons.size()> mouseButtonDown {};

  [[nodiscard]] bool isKeyDown(KeyCode keyCode) const noexcept
  {
    switch (keyCode)
    {
      case KeyCode::eW:
        return keyDown[0];
      case KeyCode::eA:
        return keyDown[1];
      case KeyCode::eS:
        return keyDown[2];
      case KeyCode::eD:
        return keyDown[3];
      case KeyCode::eQ:
        return keyDown[4];
      case KeyCode::eE:
        return keyDown[5];
      case KeyCode::eR:
        return keyDown[6];
      case KeyCode::eEscape:
        return keyDown[7];
    }

    return false;
  }

  void setKeyDown(KeyCode keyCode, bool isDown) noexcept
  {
    switch (keyCode)
    {
      case KeyCode::eW:
        keyDown[0] = isDown;
        break;
      case KeyCode::eA:
        keyDown[1] = isDown;
        break;
      case KeyCode::eS:
        keyDown[2] = isDown;
        break;
      case KeyCode::eD:
        keyDown[3] = isDown;
        break;
      case KeyCode::eQ:
        keyDown[4] = isDown;
        break;
      case KeyCode::eE:
        keyDown[5] = isDown;
        break;
      case KeyCode::eR:
        keyDown[6] = isDown;
        break;
      case KeyCode::eEscape:
        keyDown[7] = isDown;
        break;
    }
  }

  [[nodiscard]] bool isMouseButtonDown(MouseButton mouseButton) const noexcept
  {
    switch (mouseButton)
    {
      case MouseButton::eLeft:
        return mouseButtonDown[0];
      case MouseButton::eRight:
        return mouseButtonDown[1];
      case MouseButton::eMiddle:
        return mouseButtonDown[2];
    }

    return false;
  }

  void setMouseButtonDown(MouseButton mouseButton, bool isDown) noexcept
  {
    switch (mouseButton)
    {
      case MouseButton::eLeft:
        mouseButtonDown[0] = isDown;
        break;
      case MouseButton::eRight:
        mouseButtonDown[1] = isDown;
        break;
      case MouseButton::eMiddle:
        mouseButtonDown[2] = isDown;
        break;
    }
  }
};
} // namespace mental::platform
