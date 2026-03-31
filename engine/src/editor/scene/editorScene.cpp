#include <editor/scene/editorScene.hpp>

#include <algorithm>
#include <string>
#include <utility>

namespace mental::editor
{
namespace
{
[[nodiscard]] const PrimitiveTypeInfo* primitiveTypeInfo(PrimitiveType type) noexcept
{
  const auto match = std::find_if(kPrimitiveTypeInfos.begin(),
    kPrimitiveTypeInfos.end(),
    [type](const PrimitiveTypeInfo& info) { return info.type == type; });

  if (match != kPrimitiveTypeInfos.end())
  {
    return &(*match);
  }

  return nullptr;
}

[[nodiscard]] std::size_t primitiveTypeIndex(PrimitiveType type) noexcept
{
  const auto match = std::find_if(kPrimitiveTypeInfos.begin(),
    kPrimitiveTypeInfos.end(),
    [type](const PrimitiveTypeInfo& info) { return info.type == type; });

  if (match != kPrimitiveTypeInfos.end())
  {
    return static_cast<std::size_t>(std::distance(kPrimitiveTypeInfos.begin(), match));
  }

  return kPrimitiveTypeInfos.size();
}
} // namespace

mental::core::Result EditorScene::createPrimitive(PrimitiveType type, entt::entity& entity) noexcept
{
  entity = entt::null;

  const PrimitiveTypeInfo* primitiveInfo = primitiveTypeInfo(type);
  const std::size_t primitiveIndex = primitiveTypeIndex(type);
  if (primitiveInfo == nullptr || primitiveIndex >= mNextPrimitiveDisplayIndices.size())
  {
    return core::Result::eOperationFailed;
  }

  entity = mRegistry.create();
  const std::uint32_t displayIndex = mNextPrimitiveDisplayIndices[primitiveIndex]++;

  std::string name = primitiveInfo->baseName;
  if (displayIndex > 1u)
  {
    name += " ";
    name += std::to_string(displayIndex);
  }

  mRegistry.emplace<NameComponent>(entity, std::move(name));
  mRegistry.emplace<TransformComponent>(entity);
  mRegistry.emplace<PrimitiveComponent>(entity, type);

  return core::Result::eSuccess;
}

void EditorScene::destroyEntity(entt::entity entity)
{
  if (!mRegistry.valid(entity))
  {
    return;
  }

  sanitizeEditorState(entity);
  mRegistry.destroy(entity);
}

bool EditorScene::setSelectedEntity(entt::entity entity) noexcept
{
  return setEntityReference(entity, mEditorState.selectedEntity);
}

bool EditorScene::setHoveredEntity(entt::entity entity) noexcept
{
  return setEntityReference(entity, mEditorState.hoveredEntity);
}

void EditorScene::setGizmoMode(GizmoMode mode) noexcept
{
  mEditorState.gizmoMode = mode;
}

bool EditorScene::isSceneEntityOrNull(entt::entity entity) const noexcept
{
  return entity == entt::null || isAlive(entity);
}

bool EditorScene::setEntityReference(entt::entity entity, entt::entity& slot) noexcept
{
  if (!isSceneEntityOrNull(entity))
  {
    return false;
  }

  slot = entity;
  return true;
}

void EditorScene::sanitizeEditorState(entt::entity entity) noexcept
{
  (void)setSelectedEntity(mEditorState.selectedEntity == entity ? entt::null : mEditorState.selectedEntity);
  (void)setHoveredEntity(mEditorState.hoveredEntity == entity ? entt::null : mEditorState.hoveredEntity);
}
} // namespace mental::editor
