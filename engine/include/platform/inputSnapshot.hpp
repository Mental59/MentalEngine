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
    return keyDown[toIndex(keyCode)];
  }

  void setKeyDown(KeyCode keyCode, bool isDown) noexcept
  {
    keyDown[toIndex(keyCode)] = isDown;
  }

  [[nodiscard]] bool isMouseButtonDown(MouseButton mouseButton) const noexcept
  {
    return mouseButtonDown[toIndex(mouseButton)];
  }

  void setMouseButtonDown(MouseButton mouseButton, bool isDown) noexcept
  {
    mouseButtonDown[toIndex(mouseButton)] = isDown;
  }
};
} // namespace mental::platform
