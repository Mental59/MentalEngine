#include <input/inputState.hpp>

namespace mental::input
{
void InputState::advance(const InputSnapshot& snapshot) noexcept
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

bool InputState::isKeyDown(KeyCode keyCode) const noexcept
{
  return mCurrentRawSnapshot.isKeyDown(keyCode);
}

bool InputState::wasKeyPressed(KeyCode keyCode) const noexcept
{
  return mCurrentRawSnapshot.isKeyDown(keyCode) && !mPreviousRawSnapshot.isKeyDown(keyCode);
}

bool InputState::wasKeyReleased(KeyCode keyCode) const noexcept
{
  return !mCurrentRawSnapshot.isKeyDown(keyCode) && mPreviousRawSnapshot.isKeyDown(keyCode);
}

bool InputState::isMouseButtonDown(MouseButton mouseButton) const noexcept
{
  return mCurrentRawSnapshot.isMouseButtonDown(mouseButton);
}

bool InputState::wasMouseButtonPressed(MouseButton mouseButton) const noexcept
{
  return mCurrentRawSnapshot.isMouseButtonDown(mouseButton) && !mPreviousRawSnapshot.isMouseButtonDown(mouseButton);
}

bool InputState::wasMouseButtonReleased(MouseButton mouseButton) const noexcept
{
  return !mCurrentRawSnapshot.isMouseButtonDown(mouseButton) && mPreviousRawSnapshot.isMouseButtonDown(mouseButton);
}

CursorPosition InputState::mousePosition() const noexcept
{
  return mCurrentCursorPosition;
}

CursorPosition InputState::mouseDelta() const noexcept
{
  return {
    .x = mCurrentCursorPosition.x - mPreviousCursorPosition.x,
    .y = mCurrentCursorPosition.y - mPreviousCursorPosition.y,
  };
}

ScrollDelta InputState::scrollDelta() const noexcept
{
  return mCurrentRawSnapshot.scrollDelta;
}

const InputSnapshot& InputState::currentRawSnapshot() const noexcept
{
  return mCurrentRawSnapshot;
}

const InputSnapshot& InputState::previousRawSnapshot() const noexcept
{
  return mPreviousRawSnapshot;
}
} // namespace mental::input
