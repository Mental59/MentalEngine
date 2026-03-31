#pragma once

#include <entt/entity/entity.hpp>

namespace mental::editor
{
enum class GizmoMode
{
  eTranslate = 0,
  eRotate,
  eScale,
};

struct EditorState
{
  entt::entity selectedEntity {entt::null};
  entt::entity hoveredEntity {entt::null};
  GizmoMode gizmoMode {GizmoMode::eTranslate};
};
} // namespace mental::editor
