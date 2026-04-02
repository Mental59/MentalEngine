#include <camera/camera.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <cmath>
#include <limits>

namespace mental::camera
{
namespace
{
constexpr float maxPitchMagnitudeDegrees = 89.0f;
}

Camera::Camera() noexcept = default;

glm::vec3 Camera::worldPosition() const noexcept
{
  return mWorldPosition;
}

void Camera::setWorldPosition(const glm::vec3& worldPosition) noexcept
{
  mWorldPosition = worldPosition;
}

float Camera::yawDegrees() const noexcept
{
  return mYawDegrees;
}

void Camera::setYawDegrees(float yawDegrees) noexcept
{
  mYawDegrees = yawDegrees;
}

float Camera::pitchDegrees() const noexcept
{
  return mPitchDegrees;
}

void Camera::setPitchDegrees(float pitchDegrees) noexcept
{
  mPitchDegrees = pitchDegrees;
  clampPitch();
}

float Camera::verticalFieldOfViewDegrees() const noexcept
{
  return mVerticalFieldOfViewDegrees;
}

void Camera::setVerticalFieldOfViewDegrees(float verticalFieldOfViewDegrees) noexcept
{
  mVerticalFieldOfViewDegrees = verticalFieldOfViewDegrees;
}

float Camera::nearClip() const noexcept
{
  return mNearClip;
}

void Camera::setNearClip(float nearClip) noexcept
{
  mNearClip = nearClip;
}

float Camera::farClip() const noexcept
{
  return mFarClip;
}

void Camera::setFarClip(float farClip) noexcept
{
  mFarClip = farClip;
}

float Camera::pitchMinimumDegrees() const noexcept
{
  return mPitchMinimumDegrees;
}

void Camera::setPitchMinimumDegrees(float pitchMinimumDegrees) noexcept
{
  mPitchMinimumDegrees = pitchMinimumDegrees;
  clampPitch();
}

float Camera::pitchMaximumDegrees() const noexcept
{
  return mPitchMaximumDegrees;
}

void Camera::setPitchMaximumDegrees(float pitchMaximumDegrees) noexcept
{
  mPitchMaximumDegrees = pitchMaximumDegrees;
  clampPitch();
}

glm::vec3 Camera::forward() const noexcept
{
  const float yawRadians = glm::radians(mYawDegrees);
  const float pitchRadians = glm::radians(mPitchDegrees);

  const glm::vec3 direction {
    std::cos(pitchRadians) * std::cos(yawRadians),
    std::sin(pitchRadians),
    std::cos(pitchRadians) * std::sin(yawRadians),
  };

  return glm::normalize(direction);
}

glm::vec3 Camera::right() const noexcept
{
  constexpr glm::vec3 worldUp {0.0f, 1.0f, 0.0f};
  const glm::vec3 cameraRight = glm::cross(forward(), worldUp);
  return glm::normalize(cameraRight);
}

glm::vec3 Camera::up() const noexcept
{
  return glm::normalize(glm::cross(right(), forward()));
}

glm::mat4 Camera::viewMatrix() const noexcept
{
  return glm::lookAt(mWorldPosition, mWorldPosition + forward(), up());
}

glm::mat4 Camera::projectionMatrix(float aspectRatio) const noexcept
{
  const float safeAspectRatio = aspectRatio > 0.0001f ? aspectRatio : 1.0f;
  const float safeVerticalFieldOfViewDegrees = std::clamp(mVerticalFieldOfViewDegrees, 0.0001f, 179.0f);
  const float safeNearClip = std::max(mNearClip, 0.0001f);
  const float safeFarClip = std::max(mFarClip, safeNearClip + 0.0001f);

  glm::mat4 projection =
    glm::perspective(glm::radians(safeVerticalFieldOfViewDegrees), safeAspectRatio, safeNearClip, safeFarClip);
  projection[1][1] *= -1.0f;
  return projection;
}

Ray Camera::viewportRay(const glm::vec2& viewportPosition, const glm::vec2& viewportSize) const noexcept
{
  if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
  {
    return {mWorldPosition, forward()};
  }

  const float aspectRatio = viewportSize.x / viewportSize.y;
  const glm::mat4 projection = projectionMatrix(aspectRatio);
  const glm::mat4 view = viewMatrix();
  const glm::mat4 inverseViewProjection = glm::inverse(projection * view);

  const glm::vec2 normalizedPosition {
    (2.0f * viewportPosition.x / viewportSize.x) - 1.0f,
    (2.0f * viewportPosition.y / viewportSize.y) - 1.0f,
  };

  const glm::vec4 clipSpacePosition {normalizedPosition.x, normalizedPosition.y, -1.0f, 1.0f};
  glm::vec4 worldSpacePosition = inverseViewProjection * clipSpacePosition;
  if (worldSpacePosition.w != 0.0f)
  {
    worldSpacePosition /= worldSpacePosition.w;
  }

  const glm::vec3 direction = glm::normalize(glm::vec3 {worldSpacePosition} - mWorldPosition);
  return {mWorldPosition, direction};
}

void Camera::applyYawPitchDelta(float yawDeltaDegrees, float pitchDeltaDegrees) noexcept
{
  mYawDegrees += yawDeltaDegrees;
  mPitchDegrees += pitchDeltaDegrees;
  clampPitch();
}

void Camera::translateLocal(const glm::vec3& localOffset) noexcept
{
  mWorldPosition += right() * localOffset.x + up() * localOffset.y + forward() * localOffset.z;
}

void Camera::clampPitch() noexcept
{
  mPitchMinimumDegrees = std::clamp(mPitchMinimumDegrees, -maxPitchMagnitudeDegrees, maxPitchMagnitudeDegrees);
  mPitchMaximumDegrees = std::clamp(mPitchMaximumDegrees, -maxPitchMagnitudeDegrees, maxPitchMagnitudeDegrees);
  if (mPitchMinimumDegrees > mPitchMaximumDegrees)
  {
    std::swap(mPitchMinimumDegrees, mPitchMaximumDegrees);
  }
  mPitchDegrees = std::clamp(mPitchDegrees, mPitchMinimumDegrees, mPitchMaximumDegrees);
}
} // namespace mental::camera
