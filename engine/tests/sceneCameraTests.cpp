#include <editor/scene/sceneCamera.hpp>

#include <camera/camera.hpp>

#include <glm/glm.hpp>

#include <cmath>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace
{
using mental::camera::Camera;
using mental::editor::SceneCamera;

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

void testSceneCameraIsConstructibleAsBaseCamera()
{
  SceneCamera sceneCamera {};
  const Camera& camera = sceneCamera;

  require(nearlyEqual(camera.worldPosition(), sceneCamera.worldPosition()),
    "SceneCamera should be usable through the base camera interface");
}

void testSceneCameraDefaultsShowOrigin()
{
  const SceneCamera sceneCamera {};
  const auto& spawnTransform = sceneCamera.spawnTransform();

  require(nearlyEqual(sceneCamera.worldPosition(), glm::vec3 {0.0f, 1.0f, 5.0f}),
    "SceneCamera should start above the ground plane so the default grid is visible");
  require(nearlyEqual(sceneCamera.yawDegrees(), -90.0f), "SceneCamera should look toward the origin by default");
  require(nearlyEqual(sceneCamera.pitchDegrees(), 0.0f), "SceneCamera should start level by default");
  require(nearlyEqual(sceneCamera.forward(), glm::vec3 {0.0f, 0.0f, -1.0f}),
    "SceneCamera should face the bootstrap cube at the origin");
  require(nearlyEqual(spawnTransform.position, glm::vec3 {0.0f, 1.0f, 5.0f}),
    "SceneCamera should store the default spawn position");
  require(nearlyEqual(spawnTransform.rotation, glm::vec3 {0.0f, -90.0f, 0.0f}),
    "SceneCamera should store the default spawn rotation");
}

void testSceneCameraTuningStateIsSeparateFromBaseCameraMathState()
{
  SceneCamera sceneCamera {};
  const glm::vec3 initialWorldPosition = sceneCamera.worldPosition();
  const float initialYaw = sceneCamera.yawDegrees();
  const float initialPitch = sceneCamera.pitchDegrees();

  sceneCamera.setMoveSpeed(12.5f);
  sceneCamera.setBoostMultiplier(3.0f);
  sceneCamera.setMouseLookSensitivity(0.25f);

  require(nearlyEqual(sceneCamera.worldPosition(), initialWorldPosition),
    "Changing editor tuning values should not move the camera");
  require(nearlyEqual(sceneCamera.yawDegrees(), initialYaw), "Changing editor tuning values should not change yaw");
  require(
    nearlyEqual(sceneCamera.pitchDegrees(), initialPitch), "Changing editor tuning values should not change pitch");
  require(nearlyEqual(sceneCamera.moveSpeed(), 12.5f), "Move speed should be stored independently");
  require(nearlyEqual(sceneCamera.boostMultiplier(), 3.0f), "Boost multiplier should be stored independently");
  require(
    nearlyEqual(sceneCamera.mouseLookSensitivity(), 0.25f), "Mouse look sensitivity should be stored independently");
}
} // namespace

int main()
{
  try
  {
    static_assert(std::is_base_of_v<Camera, SceneCamera>, "SceneCamera should derive from camera::Camera");
    testSceneCameraIsConstructibleAsBaseCamera();
    testSceneCameraDefaultsShowOrigin();
    testSceneCameraTuningStateIsSeparateFromBaseCameraMathState();
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
