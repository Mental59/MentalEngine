#include <editor/scene/sceneCamera.hpp>

namespace mental::editor
{
SceneCamera::SceneCamera() noexcept
{
  setWorldPosition(mSpawnTransform.position);
  setYawDegrees(mSpawnTransform.rotation.y);
  setPitchDegrees(mSpawnTransform.rotation.x);
}

float SceneCamera::moveSpeed() const noexcept
{
  return mMoveSpeed;
}

void SceneCamera::setMoveSpeed(float moveSpeed) noexcept
{
  mMoveSpeed = moveSpeed;
}

float SceneCamera::boostMultiplier() const noexcept
{
  return mBoostMultiplier;
}

void SceneCamera::setBoostMultiplier(float boostMultiplier) noexcept
{
  mBoostMultiplier = boostMultiplier;
}

float SceneCamera::mouseLookSensitivity() const noexcept
{
  return mMouseLookSensitivity;
}

void SceneCamera::setMouseLookSensitivity(float mouseLookSensitivity) noexcept
{
  mMouseLookSensitivity = mouseLookSensitivity;
}

const SceneCameraSpawnTransform& SceneCamera::spawnTransform() const noexcept
{
  return mSpawnTransform;
}
} // namespace mental::editor
