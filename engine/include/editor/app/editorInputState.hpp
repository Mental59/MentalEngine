#pragma once

#include <optional>

#include <editor/scene/editorState.hpp>

#include <input/inputState.hpp>

namespace mental::editor
{
class EditorInputState : public input::InputState
{
 public:
  [[nodiscard]] bool isFlyLookActive() const noexcept
  {
    return isMouseButtonDown(input::MouseButton::eRight);
  }

  [[nodiscard]] bool wantsSelectionClick() const noexcept
  {
    return wasMouseButtonPressed(input::MouseButton::eLeft);
  }

  [[nodiscard]] std::optional<GizmoMode> requestedGizmoMode() const noexcept
  {
    if (wasKeyPressed(input::KeyCode::eW))
    {
      return GizmoMode::eTranslate;
    }

    if (wasKeyPressed(input::KeyCode::eE))
    {
      return GizmoMode::eRotate;
    }

    if (wasKeyPressed(input::KeyCode::eR))
    {
      return GizmoMode::eScale;
    }

    return std::nullopt;
  }
};
} // namespace mental::editor
