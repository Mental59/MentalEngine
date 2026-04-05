#pragma once

#include <camera/camera.hpp>

#include <glm/glm.hpp>

namespace mental::editor
{
struct SceneCameraSpawnTransform
{
  glm::vec3 position {0.0f, 1.0f, 5.0f};
  glm::vec3 rotation {0.0f, -90.0f, 0.0f};
};

class SceneCamera : public camera::Camera
{
 public:
  SceneCamera() noexcept;

  [[nodiscard]] float moveSpeed() const noexcept;
  void setMoveSpeed(float moveSpeed) noexcept;

  [[nodiscard]] float boostMultiplier() const noexcept;
  void setBoostMultiplier(float boostMultiplier) noexcept;

  [[nodiscard]] float mouseLookSensitivity() const noexcept;
  void setMouseLookSensitivity(float mouseLookSensitivity) noexcept;

  [[nodiscard]] const SceneCameraSpawnTransform& spawnTransform() const noexcept;

 private:
  SceneCameraSpawnTransform mSpawnTransform {};
  float mMoveSpeed {5.0f};
  float mBoostMultiplier {4.0f};
  float mMouseLookSensitivity {0.1f};
};
} // namespace mental::editor
