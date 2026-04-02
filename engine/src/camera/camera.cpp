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
  return worldPosition_;
}

void Camera::setWorldPosition(const glm::vec3& worldPosition) noexcept
{
  worldPosition_ = worldPosition;
}

float Camera::yawDegrees() const noexcept
{
  return yawDegrees_;
}

void Camera::setYawDegrees(float yawDegrees) noexcept
{
  yawDegrees_ = yawDegrees;
}

float Camera::pitchDegrees() const noexcept
{
  return pitchDegrees_;
}

void Camera::setPitchDegrees(float pitchDegrees) noexcept
{
  pitchDegrees_ = pitchDegrees;
  clampPitch();
}

float Camera::verticalFieldOfViewDegrees() const noexcept
{
  return verticalFieldOfViewDegrees_;
}

void Camera::setVerticalFieldOfViewDegrees(float verticalFieldOfViewDegrees) noexcept
{
  verticalFieldOfViewDegrees_ = verticalFieldOfViewDegrees;
}

float Camera::nearClip() const noexcept
{
  return nearClip_;
}

void Camera::setNearClip(float nearClip) noexcept
{
  nearClip_ = nearClip;
}

float Camera::farClip() const noexcept
{
  return farClip_;
}

void Camera::setFarClip(float farClip) noexcept
{
  farClip_ = farClip;
}

float Camera::pitchMinimumDegrees() const noexcept
{
  return pitchMinimumDegrees_;
}

void Camera::setPitchMinimumDegrees(float pitchMinimumDegrees) noexcept
{
  pitchMinimumDegrees_ = pitchMinimumDegrees;
  clampPitch();
}

float Camera::pitchMaximumDegrees() const noexcept
{
  return pitchMaximumDegrees_;
}

void Camera::setPitchMaximumDegrees(float pitchMaximumDegrees) noexcept
{
  pitchMaximumDegrees_ = pitchMaximumDegrees;
  clampPitch();
}

glm::vec3 Camera::forward() const noexcept
{
  const float yawRadians = glm::radians(yawDegrees_);
  const float pitchRadians = glm::radians(pitchDegrees_);

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
  return glm::lookAt(worldPosition_, worldPosition_ + forward(), up());
}

glm::mat4 Camera::projectionMatrix(float aspectRatio) const noexcept
{
  const float safeAspectRatio = aspectRatio > 0.0001f ? aspectRatio : 1.0f;
  const float safeVerticalFieldOfViewDegrees = std::clamp(verticalFieldOfViewDegrees_, 0.0001f, 179.0f);
  const float safeNearClip = std::max(nearClip_, 0.0001f);
  const float safeFarClip = std::max(farClip_, safeNearClip + 0.0001f);

  glm::mat4 projection =
    glm::perspective(glm::radians(safeVerticalFieldOfViewDegrees), safeAspectRatio, safeNearClip, safeFarClip);
  projection[1][1] *= -1.0f;
  return projection;
}

Ray Camera::viewportRay(const glm::vec2& viewportPosition, const glm::vec2& viewportSize) const noexcept
{
  if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
  {
    return {worldPosition_, forward()};
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

  const glm::vec3 direction = glm::normalize(glm::vec3 {worldSpacePosition} - worldPosition_);
  return {worldPosition_, direction};
}

void Camera::applyYawPitchDelta(float yawDeltaDegrees, float pitchDeltaDegrees) noexcept
{
  yawDegrees_ += yawDeltaDegrees;
  pitchDegrees_ += pitchDeltaDegrees;
  clampPitch();
}

void Camera::translateLocal(const glm::vec3& localOffset) noexcept
{
  worldPosition_ += right() * localOffset.x + up() * localOffset.y + forward() * localOffset.z;
}

void Camera::clampPitch() noexcept
{
  pitchMinimumDegrees_ = std::clamp(pitchMinimumDegrees_, -maxPitchMagnitudeDegrees, maxPitchMagnitudeDegrees);
  pitchMaximumDegrees_ = std::clamp(pitchMaximumDegrees_, -maxPitchMagnitudeDegrees, maxPitchMagnitudeDegrees);
  if (pitchMinimumDegrees_ > pitchMaximumDegrees_)
  {
    std::swap(pitchMinimumDegrees_, pitchMaximumDegrees_);
  }
  pitchDegrees_ = std::clamp(pitchDegrees_, pitchMinimumDegrees_, pitchMaximumDegrees_);
}
} // namespace mental::camera
