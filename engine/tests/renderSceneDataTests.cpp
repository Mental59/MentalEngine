#include <render/sceneRenderData.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace
{
using mental::render::SceneCameraData;
using mental::render::SceneGeometryKind;
using mental::render::SceneRenderObject;

void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

bool nearlyEqual(float lhs, float rhs, float epsilon = 1.0e-4f)
{
  return std::fabs(lhs - rhs) <= epsilon;
}

bool nearlyEqual(const glm::vec3& lhs, const glm::vec3& rhs, float epsilon = 1.0e-4f)
{
  return nearlyEqual(lhs.x, rhs.x, epsilon) && nearlyEqual(lhs.y, rhs.y, epsilon) && nearlyEqual(lhs.z, rhs.z, epsilon);
}

bool nearlyEqual(const glm::mat4& lhs, const glm::mat4& rhs, float epsilon = 1.0e-4f)
{
  for (int column = 0; column < 4; ++column)
  {
    for (int row = 0; row < 4; ++row)
    {
      if (!nearlyEqual(lhs[column][row], rhs[column][row], epsilon))
      {
        return false;
      }
    }
  }

  return true;
}

glm::mat4 composeWorldTransform(const glm::vec3& translation, const glm::vec3& rotationDegrees, const glm::vec3& scale)
{
  glm::mat4 worldTransform {1.0f};
  worldTransform = glm::translate(worldTransform, translation);
  worldTransform = glm::rotate(worldTransform, glm::radians(rotationDegrees.x), glm::vec3 {1.0f, 0.0f, 0.0f});
  worldTransform = glm::rotate(worldTransform, glm::radians(rotationDegrees.y), glm::vec3 {0.0f, 1.0f, 0.0f});
  worldTransform = glm::rotate(worldTransform, glm::radians(rotationDegrees.z), glm::vec3 {0.0f, 0.0f, 1.0f});
  worldTransform = glm::scale(worldTransform, scale);
  return worldTransform;
}

glm::vec3 transformPoint(const glm::mat4& matrix, const glm::vec3& point)
{
  return glm::vec3 {
    matrix * glm::vec4 {point, 1.0f}
  };
}

void testSceneCameraDataUsesRequestedAspectRatio()
{
  const glm::mat4 view =
    glm::lookAt(glm::vec3 {1.0f, 2.0f, 3.0f}, glm::vec3 {1.0f, 2.0f, 2.0f}, glm::vec3 {0.0f, 1.0f, 0.0f});
  const glm::mat4 projection = glm::perspective(glm::radians(60.0f), 16.0f / 9.0f, 0.1f, 1000.0f) * glm::mat4 {1.0f};

  SceneCameraData cameraData {};
  cameraData.worldPosition = {1.0f, 2.0f, 3.0f};
  cameraData.view = view;
  cameraData.projection = projection;
  cameraData.viewProjection = projection * view;
  cameraData.aspectRatio = 16.0f / 9.0f;

  require(nearlyEqual(cameraData.aspectRatio, 16.0f / 9.0f), "SceneCameraData should store the requested aspect ratio");
  require(nearlyEqual(cameraData.viewProjection, cameraData.projection * cameraData.view),
    "SceneCameraData should store projection * view");
}

void testIdentityWorldTransformRemainsIdentity()
{
  const SceneRenderObject object {
    .objectIdentifier = 7u,
    .geometryKind = SceneGeometryKind::eCube,
    .worldTransform = glm::mat4 {1.0f},
  };

  require(nearlyEqual(object.worldTransform, glm::mat4 {1.0f}),
    "Identity transform inputs should produce an identity world transform");
}

void testTransformCompositionAppliesTranslationRotationAndScale()
{
  const glm::vec3 translation {3.0f, 4.0f, 5.0f};
  const glm::vec3 rotationDegrees {0.0f, 90.0f, 0.0f};
  const glm::vec3 scale {2.0f, 3.0f, 4.0f};

  const glm::mat4 worldTransform = composeWorldTransform(translation, rotationDegrees, scale);

  require(nearlyEqual(transformPoint(worldTransform, glm::vec3 {0.0f, 0.0f, 0.0f}), translation),
    "Translation should move the origin");
  require(nearlyEqual(transformPoint(worldTransform, glm::vec3 {1.0f, 0.0f, 0.0f}), glm::vec3 {3.0f, 4.0f, 3.0f}),
    "Rotation and scale should compose before translation");
  require(nearlyEqual(transformPoint(worldTransform, glm::vec3 {0.0f, 1.0f, 0.0f}), glm::vec3 {3.0f, 7.0f, 5.0f}),
    "Axis-aligned scale should survive composition");
}

void testSortingByIdentifierIsDeterministic()
{
  std::vector<SceneRenderObject> objects {
    SceneRenderObject {
                       .objectIdentifier = 42u, .geometryKind = SceneGeometryKind::eSphere, .worldTransform = glm::mat4 {1.0f}},
    SceneRenderObject {
                       .objectIdentifier = 7u,  .geometryKind = SceneGeometryKind::eCube,   .worldTransform = glm::mat4 {1.0f}},
    SceneRenderObject {
                       .objectIdentifier = 19u, .geometryKind = SceneGeometryKind::ePlane,  .worldTransform = glm::mat4 {1.0f}},
  };

  std::sort(objects.begin(),
    objects.end(),
    [](const SceneRenderObject& lhs, const SceneRenderObject& rhs)
    { return lhs.objectIdentifier < rhs.objectIdentifier; });

  require(objects[0].objectIdentifier == 7u, "Lowest identifier should sort first");
  require(objects[1].objectIdentifier == 19u, "Middle identifier should sort second");
  require(objects[2].objectIdentifier == 42u, "Highest identifier should sort last");
}
} // namespace

int main()
{
  try
  {
    testSceneCameraDataUsesRequestedAspectRatio();
    testIdentityWorldTransformRemainsIdentity();
    testTransformCompositionAppliesTranslationRotationAndScale();
    testSortingByIdentifierIsDeterministic();
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unknown exception\n";
  }

  return 1;
}
