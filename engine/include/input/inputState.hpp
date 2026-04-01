#pragma once

#include <input/inputSnapshot.hpp>

namespace mental::input
{
class InputState
{
 public:
  void advance(const InputSnapshot& snapshot) noexcept
  {
    mPreviousRawSnapshot = mCurrentRawSnapshot;
    mCurrentRawSnapshot = snapshot;

    if (mHasCurrentCursorPosition)
    {
      mPreviousCursorPosition = mCurrentCursorPosition;
    }
    else
    {
      mPreviousCursorPosition = snapshot.cursorPosition;
      mHasCurrentCursorPosition = true;
    }

    mCurrentCursorPosition = snapshot.cursorPosition;
  }

  [[nodiscard]] bool isKeyDown(KeyCode keyCode) const noexcept
  {
    return mCurrentRawSnapshot.isKeyDown(keyCode);
  }

  [[nodiscard]] bool wasKeyPressed(KeyCode keyCode) const noexcept
  {
    return mCurrentRawSnapshot.isKeyDown(keyCode) && !mPreviousRawSnapshot.isKeyDown(keyCode);
  }

  [[nodiscard]] bool wasKeyReleased(KeyCode keyCode) const noexcept
  {
    return !mCurrentRawSnapshot.isKeyDown(keyCode) && mPreviousRawSnapshot.isKeyDown(keyCode);
  }

  [[nodiscard]] bool isMouseButtonDown(MouseButton mouseButton) const noexcept
  {
    return mCurrentRawSnapshot.isMouseButtonDown(mouseButton);
  }

  [[nodiscard]] bool wasMouseButtonPressed(MouseButton mouseButton) const noexcept
  {
    return mCurrentRawSnapshot.isMouseButtonDown(mouseButton) && !mPreviousRawSnapshot.isMouseButtonDown(mouseButton);
  }

  [[nodiscard]] bool wasMouseButtonReleased(MouseButton mouseButton) const noexcept
  {
    return !mCurrentRawSnapshot.isMouseButtonDown(mouseButton) && mPreviousRawSnapshot.isMouseButtonDown(mouseButton);
  }

  [[nodiscard]] CursorPosition mousePosition() const noexcept
  {
    return mCurrentCursorPosition;
  }

  [[nodiscard]] CursorPosition mouseDelta() const noexcept
  {
    return {
      .x = mCurrentCursorPosition.x - mPreviousCursorPosition.x,
      .y = mCurrentCursorPosition.y - mPreviousCursorPosition.y,
    };
  }

  [[nodiscard]] ScrollDelta scrollDelta() const noexcept
  {
    return mCurrentRawSnapshot.scrollDelta;
  }

 protected:
  [[nodiscard]] const InputSnapshot& currentRawSnapshot() const noexcept
  {
    return mCurrentRawSnapshot;
  }

  [[nodiscard]] const InputSnapshot& previousRawSnapshot() const noexcept
  {
    return mPreviousRawSnapshot;
  }

 private:
  InputSnapshot mPreviousRawSnapshot {};
  InputSnapshot mCurrentRawSnapshot {};
  CursorPosition mPreviousCursorPosition {};
  CursorPosition mCurrentCursorPosition {};
  bool mHasCurrentCursorPosition = false;
};
} // namespace mental::input
