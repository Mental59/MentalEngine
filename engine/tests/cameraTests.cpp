#include <camera/camera.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>

namespace
{
using mental::camera::Camera;

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

bool nearlyEqual(const glm::vec4& lhs, const glm::vec4& rhs, float epsilon = 1.0e-4f)
{
  return nearlyEqual(lhs.x, rhs.x, epsilon) && nearlyEqual(lhs.y, rhs.y, epsilon) && nearlyEqual(lhs.z, rhs.z, epsilon)
    && nearlyEqual(lhs.w, rhs.w, epsilon);
}

bool isFinite(const glm::mat4& matrix)
{
  for (int column = 0; column < 4; ++column)
  {
    for (int row = 0; row < 4; ++row)
    {
      if (!std::isfinite(matrix[column][row]))
      {
        return false;
      }
    }
  }

  return true;
}

glm::vec4 transformPoint(const glm::mat4& matrix, const glm::vec3& point)
{
  return matrix * glm::vec4 {point, 1.0f};
}

void testDefaultConstructionProducesFiniteMatrices()
{
  const Camera camera {};

  require(isFinite(camera.viewMatrix()), "Default view matrix should contain finite values");
  require(isFinite(camera.projectionMatrix(16.0f / 9.0f)), "Default projection matrix should contain finite values");
}

void testYawPitchChangeForwardVectorAsExpected()
{
  Camera camera {};

  camera.applyYawPitchDelta(90.0f, 30.0f);

  const glm::vec3 expectedForward {
    std::cos(glm::radians(30.0f)),
    0.5f,
    0.0f,
  };

  require(nearlyEqual(camera.forward(), expectedForward),
    "Yaw and pitch deltas should rotate the forward vector around the world axes");
}

void testPitchClampPreventsInversion()
{
  Camera camera {};
  camera.setPitchDegrees(85.0f);

  camera.applyYawPitchDelta(0.0f, 10.0f);

  require(nearlyEqual(camera.pitchDegrees(), camera.pitchMaximumDegrees()),
    "Pitch should clamp to the configured maximum");
  require(camera.forward().y > 0.0f, "Clamped pitch should still look upward");
  require(camera.forward().z < 0.0f, "Clamped pitch should not invert the camera");
}

void testConfigurablePitchLimitsStayBelowVertical()
{
  Camera camera {};
  camera.setPitchMinimumDegrees(95.0f);
  camera.setPitchMaximumDegrees(80.0f);
  camera.setPitchDegrees(90.0f);

  require(camera.pitchMinimumDegrees() <= camera.pitchMaximumDegrees(),
    "Swapped pitch limits should be normalized into an ordered range");
  require(camera.pitchMaximumDegrees() < 90.0f, "Pitch maximum should stay below vertical");
  require(camera.pitchDegrees() == camera.pitchMaximumDegrees(), "Pitch should clamp to the configured maximum");
  require(std::isfinite(camera.right().x) && std::isfinite(camera.right().y) && std::isfinite(camera.right().z),
    "Right vector should remain finite when pitch limits are configured near vertical");
  require(std::isfinite(camera.up().x) && std::isfinite(camera.up().y) && std::isfinite(camera.up().z),
    "Up vector should remain finite when pitch limits are configured near vertical");
}

void testLocalTranslationUsesCameraAxes()
{
  Camera camera {};
  camera.setWorldPosition(glm::vec3 {10.0f, 20.0f, 30.0f});

  camera.translateLocal(glm::vec3 {2.0f, 3.0f, 4.0f});

  require(nearlyEqual(camera.worldPosition(), glm::vec3 {12.0f, 23.0f, 26.0f}),
    "Local translation should use the camera axes");
}

void testViewMatrixAndProjectionMatrixUseCameraMath()
{
  Camera camera {};
  camera.setWorldPosition(glm::vec3 {1.0f, 2.0f, 3.0f});
  camera.setVerticalFieldOfViewDegrees(200.0f);

  const glm::mat4 view = camera.viewMatrix();
  const glm::mat4 projection = camera.projectionMatrix(16.0f / 9.0f);

  require(nearlyEqual(transformPoint(view, camera.worldPosition()), glm::vec4 {0.0f, 0.0f, 0.0f, 1.0f}),
    "View matrix should bring the camera position to the origin");
  require(std::isfinite(projection[0][0]) && std::isfinite(projection[1][1]),
    "Projection matrix should stay finite for oversized field-of-view values");
  require(projection[0][0] > 0.0f && projection[1][1] < 0.0f,
    "Projection matrix should not invert when the field of view is oversized");
  require(nearlyEqual(projection[2][3], -1.0f), "Projection matrix should contain a perspective divide term");
  require(nearlyEqual(projection[3][3], 0.0f), "Projection matrix should contain a non-identity bottom-right term");
}

void testViewportRayUsesViewportPositionAndSize()
{
  Camera camera {};

  const mental::camera::Ray centerRay = camera.viewportRay(glm::vec2 {640.0f, 360.0f}, glm::vec2 {1280.0f, 720.0f});
  const mental::camera::Ray topEdgeRay = camera.viewportRay(glm::vec2 {640.0f, 0.0f}, glm::vec2 {1280.0f, 720.0f});
  const mental::camera::Ray rightEdgeRay = camera.viewportRay(glm::vec2 {1280.0f, 360.0f}, glm::vec2 {1280.0f, 720.0f});

  require(nearlyEqual(centerRay.origin, camera.worldPosition()), "Viewport ray should originate at the camera position");
  require(nearlyEqual(glm::normalize(centerRay.direction), camera.forward()),
    "Center viewport ray should align with the camera forward direction");
  require(topEdgeRay.direction.y > centerRay.direction.y,
    "Top-of-viewport rays should tilt upward relative to the center ray");
  require(rightEdgeRay.direction.x > centerRay.direction.x,
    "An off-center viewport ray should tilt toward the camera right direction");
}
} // namespace

int main()
{
  try
  {
    testDefaultConstructionProducesFiniteMatrices();
    testYawPitchChangeForwardVectorAsExpected();
    testPitchClampPreventsInversion();
    testConfigurablePitchLimitsStayBelowVertical();
    testLocalTranslationUsesCameraAxes();
    testViewMatrixAndProjectionMatrixUseCameraMath();
    testViewportRayUsesViewportPositionAndSize();
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
