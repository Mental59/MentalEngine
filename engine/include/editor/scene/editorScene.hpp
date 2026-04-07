#pragma once

#include <cstddef>
#include <array>
#include <cstdint>

#include <core/types.hpp>
#include <entt/entity/entity.hpp>
#include <entt/entity/registry.hpp>

#include <editor/scene/components.hpp>
#include <editor/scene/editorState.hpp>
#include <editor/scene/sceneCamera.hpp>

namespace mental::editor
{
class EditorScene
{
 public:
  [[nodiscard]] core::Result createPrimitive(PrimitiveType type, entt::entity& entity) noexcept;

  void destroyEntity(entt::entity entity);

  [[nodiscard]] bool isAlive(entt::entity entity) const noexcept
  {
    return mRegistry.valid(entity);
  }

  [[nodiscard]] const entt::registry& registry() const noexcept
  {
    return mRegistry;
  }

  [[nodiscard]] entt::registry& registry() noexcept
  {
    return mRegistry;
  }

  [[nodiscard]] const EditorState& editorState() const noexcept
  {
    return mEditorState;
  }

  [[nodiscard]] SceneCamera& sceneCamera() noexcept;

  [[nodiscard]] const SceneCamera& sceneCamera() const noexcept;

  [[nodiscard]] entt::entity selectedEntity() const noexcept
  {
    return mEditorState.selectedEntity;
  }

  [[nodiscard]] entt::entity hoveredEntity() const noexcept
  {
    return mEditorState.hoveredEntity;
  }

  [[nodiscard]] GizmoMode gizmoMode() const noexcept
  {
    return mEditorState.gizmoMode;
  }

 private:
  [[nodiscard]] bool isSceneEntityOrNull(entt::entity entity) const noexcept;
  [[nodiscard]] bool setEntityReference(entt::entity entity, entt::entity& slot) noexcept;
  void sanitizeEditorState(entt::entity entity) noexcept;

 public:
  [[nodiscard]] bool setSelectedEntity(entt::entity entity) noexcept;
  [[nodiscard]] bool setHoveredEntity(entt::entity entity) noexcept;
  void setGizmoMode(GizmoMode mode) noexcept;

 private:
  SceneCamera mSceneCamera {};
  EditorState mEditorState;
  entt::registry mRegistry;
  std::array<std::uint32_t, kPrimitiveTypeInfos.size()> mNextPrimitiveDisplayIndices = []
  {
    std::array<std::uint32_t, kPrimitiveTypeInfos.size()> indices {};
    indices.fill(1u);
    return indices;
  }();
};
} // namespace mental::editor
