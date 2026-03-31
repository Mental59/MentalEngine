#include <editor/scene/components.hpp>
#include <editor/scene/editorScene.hpp>
#include <editor/scene/editorState.hpp>

#include <core/types.hpp>
#include <exception>
#include <iostream>
#include <stdexcept>

#include <entt/entity/entity.hpp>
#include <glm/vec3.hpp>

int main()
{
  try {
    auto require = [](bool condition, const char* message) {
      if(!condition) {
        throw std::runtime_error(message);
      }
    };

    const mental::editor::TransformComponent transform{};
    require(transform.position == glm::vec3(0.0f), "TransformComponent.position default");

    require(transform.rotation == glm::vec3(0.0f), "TransformComponent.rotation default");

    require(transform.scale == glm::vec3(1.0f), "TransformComponent.scale default");

    const mental::editor::EditorState state{};
    require(state.selectedEntity == entt::null, "EditorState.selectedEntity default");

    require(state.hoveredEntity == entt::null, "EditorState.hoveredEntity default");

    require(state.gizmoMode == mental::editor::GizmoMode::eTranslate, "EditorState.gizmoMode default");

    mental::editor::EditorScene scene{};
    entt::entity firstCube = entt::null;
    require(scene.createPrimitive(mental::editor::PrimitiveType::eCube, firstCube) == mental::core::Result::eSuccess,
      "Creating first cube should succeed");
    require(scene.isAlive(firstCube), "Created primitive should be alive");

    const auto& firstCubeName = scene.registry().get<mental::editor::NameComponent>(firstCube).value;
    require(firstCubeName == "Cube", "First cube name");

    const auto& firstCubeTransform = scene.registry().get<mental::editor::TransformComponent>(firstCube);
    require(firstCubeTransform.position == glm::vec3(0.0f), "Primitive TransformComponent.position default");
    require(firstCubeTransform.rotation == glm::vec3(0.0f), "Primitive TransformComponent.rotation default");
    require(firstCubeTransform.scale == glm::vec3(1.0f), "Primitive TransformComponent.scale default");

    const auto& firstCubePrimitive = scene.registry().get<mental::editor::PrimitiveComponent>(firstCube);
    require(firstCubePrimitive.type == mental::editor::PrimitiveType::eCube, "Primitive type matches request");

    entt::entity secondCube = entt::null;
    require(scene.createPrimitive(mental::editor::PrimitiveType::eCube, secondCube) == mental::core::Result::eSuccess,
      "Creating second cube should succeed");
    require(scene.registry().get<mental::editor::NameComponent>(secondCube).value == "Cube 2", "Second cube name");

    entt::entity plane = entt::null;
    require(scene.createPrimitive(mental::editor::PrimitiveType::ePlane, plane) == mental::core::Result::eSuccess,
      "Creating plane should succeed");
    require(scene.isAlive(plane), "Plane primitive should be alive");
    require(scene.registry().get<mental::editor::NameComponent>(plane).value == "Plane", "Plane name");
    require(scene.registry().get<mental::editor::PrimitiveComponent>(plane).type == mental::editor::PrimitiveType::ePlane, "Plane type matches request");

    entt::entity destroyedSphere = entt::null;
    require(scene.createPrimitive(mental::editor::PrimitiveType::eSphere, destroyedSphere) == mental::core::Result::eSuccess,
      "Creating destroyed sphere should succeed");
    scene.destroyEntity(destroyedSphere);
    require(!scene.isAlive(destroyedSphere), "Destroyed primitive should not be alive");

    entt::entity invalidPrimitiveEntity = entt::entity{999};
    require(scene.createPrimitive(static_cast<mental::editor::PrimitiveType>(-1), invalidPrimitiveEntity)
        == mental::core::Result::eOperationFailed,
      "Invalid PrimitiveType should fail with eOperationFailed");
    require(invalidPrimitiveEntity == entt::null, "Invalid PrimitiveType should clear the output entity");

    entt::entity selectedEntity = entt::null;
    require(scene.createPrimitive(mental::editor::PrimitiveType::eCube, selectedEntity) == mental::core::Result::eSuccess,
      "Creating selected entity should succeed");
    require(scene.setSelectedEntity(selectedEntity), "Selecting a live entity should succeed");
    require(scene.setHoveredEntity(selectedEntity), "Hovering a live entity should succeed");
    require(scene.selectedEntity() == selectedEntity, "selectedEntity getter returns current selection");
    require(scene.hoveredEntity() == selectedEntity, "hoveredEntity getter returns current hover");

    scene.destroyEntity(selectedEntity);
    require(scene.selectedEntity() == entt::null, "Destroying a selected entity clears selectedEntity");
    require(scene.hoveredEntity() == entt::null, "Destroying a selected entity clears hoveredEntity");

    entt::entity preservedSelection = entt::null;
    entt::entity destroyedOtherEntity = entt::null;
    require(scene.createPrimitive(mental::editor::PrimitiveType::ePlane, preservedSelection) == mental::core::Result::eSuccess,
      "Creating preserved selection should succeed");
    require(scene.createPrimitive(mental::editor::PrimitiveType::eSphere, destroyedOtherEntity)
        == mental::core::Result::eSuccess,
      "Creating destroyed other entity should succeed");
    require(scene.setSelectedEntity(preservedSelection), "Selecting a different live entity should succeed");

    scene.destroyEntity(destroyedOtherEntity);
    require(scene.selectedEntity() == preservedSelection, "Destroying a different entity preserves selection");

    require(!scene.setSelectedEntity(entt::entity{999}), "Selecting a dead entity should fail");
    require(scene.selectedEntity() == preservedSelection, "Failed selection update preserves previous selection");

    require(scene.setHoveredEntity(entt::null), "Clearing hovered entity should succeed");
    require(scene.hoveredEntity() == entt::null, "Hovered entity can be cleared explicitly");

    require(!scene.setHoveredEntity(entt::entity{999}), "Hovering a dead entity should fail");
    require(scene.hoveredEntity() == entt::null, "Failed hover update preserves previous hover");

    require(scene.gizmoMode() == mental::editor::GizmoMode::eTranslate, "Gizmo mode getter returns default");
    scene.setGizmoMode(mental::editor::GizmoMode::eScale);
    require(scene.gizmoMode() == mental::editor::GizmoMode::eScale, "Gizmo mode setter updates mode");

    const auto& editorStateView = scene.editorState();
    require(editorStateView.selectedEntity == preservedSelection, "Const editorState view reflects selection");
    require(editorStateView.hoveredEntity == entt::null, "Const editorState view reflects hover");
    require(editorStateView.gizmoMode == mental::editor::GizmoMode::eScale, "Const editorState view reflects gizmo mode");

    return 0;
  } catch(const std::exception& exception) {
    std::cerr << exception.what() << '\n';
  } catch(...) {
    std::cerr << "Unknown exception\n";
  }

  return 1;
}
