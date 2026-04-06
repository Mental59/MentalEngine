#pragma once

#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace mental::render
{
using SceneObjectIdentifier = std::uint64_t;

enum class SceneGeometryKind
{
  eCube = 0,
  ePlane,
  eSphere,
};

struct SceneCameraData
{
  glm::vec3 worldPosition {0.0f, 0.0f, 0.0f};
  glm::mat4 view {1.0f};
  glm::mat4 projection {1.0f};
  glm::mat4 viewProjection {1.0f};
  float aspectRatio {1.0f};
};

struct SceneRenderObject
{
  SceneObjectIdentifier objectIdentifier {0u};
  SceneGeometryKind geometryKind {SceneGeometryKind::eCube};
  glm::mat4 worldTransform {1.0f};
  glm::mat4 normalMatrix {1.0f};
};

struct SceneRenderFrame
{
  SceneCameraData camera {};
  std::vector<SceneRenderObject> objects {};
};
} // namespace mental::render
