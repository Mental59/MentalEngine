#include "firstPersonCameraPositioner.hpp"

namespace {
constexpr float gMaxPitch = glm::radians(90.0f);
constexpr float gMinPitch = glm::radians(-90.0f);
} // namespace

camera::FirstPersonCameraPositioner::FirstPersonCameraPositioner(
    const glm::vec3& pos, const glm::vec3& target, const glm::vec3& up)
    : mCameraPosition(pos), mUp(up) {
  lookAt(target);
}

void camera::FirstPersonCameraPositioner::updateRotation(
    const glm::vec2& mousePos) {
  const glm::vec2 delta = mousePos - mMousePos;
  mMousePos = mousePos;

  float pitch = mMouseSpeed * delta.y;
  float yaw = mMouseSpeed * delta.x;

  mAngles.x += pitch;
  mAngles.y += yaw;
  mAngles.x = glm::clamp(mAngles.x, gMinPitch, gMaxPitch);
}

void camera::FirstPersonCameraPositioner::updatePosition(float deltaSeconds) {
  const glm::mat4 v = getOrientation();

  const glm::vec3 forwardV = forward(v);
  const glm::vec3 rightV = right(v);
  const glm::vec3 upV = up(rightV, forwardV);

  glm::vec3 accel(0.0f);

  if (mMovement.forward)
    accel += forwardV;
  if (mMovement.backward)
    accel -= forwardV;

  if (mMovement.left)
    accel -= rightV;
  if (mMovement.right)
    accel += rightV;

  if (mMovement.up)
    accel += upV;
  if (mMovement.down)
    accel -= upV;

  if (mMovement.fastSpeed)
    accel *= mFastCoef;

  if (accel == glm::vec3(0)) {
    // decelerate naturally according to the damping value
    mMoveSpeed -= mMoveSpeed * std::min((1.0f / mDamping) * deltaSeconds, 1.0f);
  } else {
    // acceleration
    mMoveSpeed += accel * mAcceleration * deltaSeconds;
    const float maxSpeed =
        mMovement.fastSpeed ? mMaxSpeed * mFastCoef : mMaxSpeed;
    if (glm::length(mMoveSpeed) > maxSpeed)
      mMoveSpeed = glm::normalize(mMoveSpeed) * maxSpeed;
  }

  mCameraPosition += mMoveSpeed * deltaSeconds;
}

glm::mat4 camera::FirstPersonCameraPositioner::getViewMatrix() const {
  return getOrientation() * getTranslation();
}

void camera::FirstPersonCameraPositioner::lookAt(const glm::vec3& target) {
  // Calculate direction vector from camera to target
  const glm::vec3 direction = -glm::normalize(target - mCameraPosition);

  // Calculate yaw (horizontal angle)
  mAngles.y = -atan2f(direction.x, direction.z);

  // Calculate pitch (vertical angle)
  const float distanceXZ =
      sqrtf(direction.x * direction.x + direction.z * direction.z);
  mAngles.x = atan2f(direction.y, distanceXZ);
}

glm::mat4 camera::FirstPersonCameraPositioner::getOrientation() const {
  glm::quat pitchQuat = glm::angleAxis(mAngles.x, glm::vec3(1.0f, 0.0f, 0.0f));
  glm::quat yawQuat = glm::angleAxis(mAngles.y, mUp);

  return glm::mat4_cast(pitchQuat * yawQuat);
}

glm::mat4 camera::FirstPersonCameraPositioner::getTranslation() const {
  return glm::translate(glm::mat4(1.0f), -mCameraPosition);
}

glm::vec3 camera::FirstPersonCameraPositioner::forward(
    const glm::mat4& orientation) const {
  return -glm::vec3(orientation[0][2], orientation[1][2], orientation[2][2]);
}

glm::vec3
camera::FirstPersonCameraPositioner::right(const glm::mat4& orientation) const {
  return glm::vec3(orientation[0][0], orientation[1][0], orientation[2][0]);
}

glm::vec3
camera::FirstPersonCameraPositioner::up(const glm::vec3& right,
                                        const glm::vec3& forward) const {
  return glm::cross(right, forward);
}
