#pragma once

#include <glm/glm.hpp>

namespace mental::camera
{
struct Ray
{
  glm::vec3 origin {};
  glm::vec3 direction {};
};

class Camera
{
 public:
  Camera() noexcept;

  [[nodiscard]] glm::vec3 worldPosition() const noexcept;
  void setWorldPosition(const glm::vec3& worldPosition) noexcept;

  [[nodiscard]] float yawDegrees() const noexcept;
  void setYawDegrees(float yawDegrees) noexcept;

  [[nodiscard]] float pitchDegrees() const noexcept;
  void setPitchDegrees(float pitchDegrees) noexcept;

  [[nodiscard]] float verticalFieldOfViewDegrees() const noexcept;
  void setVerticalFieldOfViewDegrees(float verticalFieldOfViewDegrees) noexcept;

  [[nodiscard]] float nearClip() const noexcept;
  void setNearClip(float nearClip) noexcept;

  [[nodiscard]] float farClip() const noexcept;
  void setFarClip(float farClip) noexcept;

  [[nodiscard]] float pitchMinimumDegrees() const noexcept;
  void setPitchMinimumDegrees(float pitchMinimumDegrees) noexcept;

  [[nodiscard]] float pitchMaximumDegrees() const noexcept;
  void setPitchMaximumDegrees(float pitchMaximumDegrees) noexcept;

  [[nodiscard]] glm::vec3 forward() const noexcept;
  [[nodiscard]] glm::vec3 right() const noexcept;
  [[nodiscard]] glm::vec3 up() const noexcept;

  [[nodiscard]] glm::mat4 viewMatrix() const noexcept;
  [[nodiscard]] glm::mat4 projectionMatrix(float aspectRatio) const noexcept;
  [[nodiscard]] Ray viewportRay(const glm::vec2& viewportPosition, const glm::vec2& viewportSize) const noexcept;

  void applyYawPitchDelta(float yawDeltaDegrees, float pitchDeltaDegrees) noexcept;
  void translateLocal(const glm::vec3& localOffset) noexcept;

 private:
  void clampPitch() noexcept;

  glm::vec3 mWorldPosition {0.0f, 0.0f, 0.0f};
  float mYawDegrees {-90.0f};
  float mPitchDegrees {0.0f};
  float mVerticalFieldOfViewDegrees {60.0f};
  float mNearClip {0.1f};
  float mFarClip {1000.0f};
  float mPitchMinimumDegrees {-89.0f};
  float mPitchMaximumDegrees {89.0f};
};
} // namespace mental::camera
