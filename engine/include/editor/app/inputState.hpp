#pragma once

#include <optional>

#include <editor/scene/editorState.hpp>

#include <platform/inputSnapshot.hpp>

namespace mental::editor
{
class InputState
{
 public:
  void advance(const platform::InputSnapshot& snapshot) noexcept
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

  [[nodiscard]] bool isKeyDown(platform::KeyCode keyCode) const noexcept
  {
    return mCurrentRawSnapshot.isKeyDown(keyCode);
  }

  [[nodiscard]] bool wasKeyPressed(platform::KeyCode keyCode) const noexcept
  {
    return mCurrentRawSnapshot.isKeyDown(keyCode) && !mPreviousRawSnapshot.isKeyDown(keyCode);
  }

  [[nodiscard]] bool wasKeyReleased(platform::KeyCode keyCode) const noexcept
  {
    return !mCurrentRawSnapshot.isKeyDown(keyCode) && mPreviousRawSnapshot.isKeyDown(keyCode);
  }

  [[nodiscard]] bool isMouseButtonDown(platform::MouseButton mouseButton) const noexcept
  {
    return mCurrentRawSnapshot.isMouseButtonDown(mouseButton);
  }

  [[nodiscard]] bool wasMouseButtonPressed(platform::MouseButton mouseButton) const noexcept
  {
    return mCurrentRawSnapshot.isMouseButtonDown(mouseButton) && !mPreviousRawSnapshot.isMouseButtonDown(mouseButton);
  }

  [[nodiscard]] bool wasMouseButtonReleased(platform::MouseButton mouseButton) const noexcept
  {
    return !mCurrentRawSnapshot.isMouseButtonDown(mouseButton)
      && mPreviousRawSnapshot.isMouseButtonDown(mouseButton);
  }

  [[nodiscard]] platform::CursorPosition mousePosition() const noexcept
  {
    return mCurrentCursorPosition;
  }

  [[nodiscard]] platform::CursorPosition mouseDelta() const noexcept
  {
    return {
      .x = mCurrentCursorPosition.x - mPreviousCursorPosition.x,
      .y = mCurrentCursorPosition.y - mPreviousCursorPosition.y,
    };
  }

  [[nodiscard]] platform::ScrollDelta scrollDelta() const noexcept
  {
    return mCurrentRawSnapshot.scrollDelta;
  }

  [[nodiscard]] bool isFlyLookActive() const noexcept
  {
    return isMouseButtonDown(platform::MouseButton::eRight);
  }

  [[nodiscard]] bool wantsSelectionClick() const noexcept
  {
    return wasMouseButtonPressed(platform::MouseButton::eLeft);
  }

  [[nodiscard]] std::optional<GizmoMode> requestedGizmoMode() const noexcept
  {
    if (wasKeyPressed(platform::KeyCode::eW))
    {
      return GizmoMode::eTranslate;
    }

    if (wasKeyPressed(platform::KeyCode::eE))
    {
      return GizmoMode::eRotate;
    }

    if (wasKeyPressed(platform::KeyCode::eR))
    {
      return GizmoMode::eScale;
    }

    return std::nullopt;
  }

 private:
  platform::InputSnapshot mPreviousRawSnapshot {};
  platform::InputSnapshot mCurrentRawSnapshot {};
  platform::CursorPosition mPreviousCursorPosition {};
  platform::CursorPosition mCurrentCursorPosition {};
  bool mHasCurrentCursorPosition = false;
};
} // namespace mental::editor
