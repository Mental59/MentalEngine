#pragma once

#include <input/inputSnapshot.hpp>

namespace mental::input
{
class InputState
{
 public:
  void advance(const InputSnapshot& snapshot) noexcept;

  [[nodiscard]] bool isKeyDown(KeyCode keyCode) const noexcept;

  [[nodiscard]] bool wasKeyPressed(KeyCode keyCode) const noexcept;

  [[nodiscard]] bool wasKeyReleased(KeyCode keyCode) const noexcept;

  [[nodiscard]] bool isMouseButtonDown(MouseButton mouseButton) const noexcept;

  [[nodiscard]] bool wasMouseButtonPressed(MouseButton mouseButton) const noexcept;

  [[nodiscard]] bool wasMouseButtonReleased(MouseButton mouseButton) const noexcept;

  [[nodiscard]] CursorPosition mousePosition() const noexcept;

  [[nodiscard]] CursorPosition mouseDelta() const noexcept;

  [[nodiscard]] ScrollDelta scrollDelta() const noexcept;

 protected:
  [[nodiscard]] const InputSnapshot& currentRawSnapshot() const noexcept;

  [[nodiscard]] const InputSnapshot& previousRawSnapshot() const noexcept;

 private:
  InputSnapshot mPreviousRawSnapshot {};
  InputSnapshot mCurrentRawSnapshot {};
  CursorPosition mPreviousCursorPosition {};
  CursorPosition mCurrentCursorPosition {};
  bool mHasCurrentCursorPosition = false;
};
} // namespace mental::input
