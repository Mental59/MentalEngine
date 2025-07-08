#pragma once
#include "camera.hpp"
#include <glm/ext.hpp>
#include <glm/glm.hpp>

namespace camera {
class FirstPersonCameraPositioner final : public ICameraPositioner {
public:
  FirstPersonCameraPositioner() = default;
  FirstPersonCameraPositioner(const glm::vec3& pos, const glm::vec3& target,
                              const glm::vec3& up = glm::vec3(0.0f, 1.0f, 0.0f),
                              const glm::vec3& right = glm::vec3(1.0f, 0.0f,
                                                                 0.0f));

  void updateRotation(const glm::vec2& mousePos);
  void updatePosition(float deltaSeconds);

  virtual glm::mat4 getViewMatrix() const override;

  inline virtual glm::vec3 getPosition() const override {
    return mCameraPosition;
  }

  inline void setPosition(const glm::vec3& pos) { mCameraPosition = pos; }

  inline void resetMousePosition(const glm::vec2& p) { mMousePos = p; };

  void lookAt(const glm::vec3& target);

public:
  struct Movement {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool fastSpeed = false;
  } mMovement;

public:
  float mMouseSpeed = 5.0f;
  float mAcceleration = 150.0f;
  float mDamping = 0.1f;
  float mMaxSpeed = 10.0f;
  float mFastCoef = 10.0f;

private:
  glm::mat4 getOrientation() const;
  glm::mat4 getTranslation() const;

  glm::vec3 forward(const glm::mat4& orientation) const;
  glm::vec3 right(const glm::mat4& orientation) const;
  glm::vec3 up(const glm::vec3& right, const glm::vec3& forward) const;

  glm::vec2 mMousePos = glm::vec2(0.0f);
  glm::vec3 mCameraPosition = glm::vec3(0.0f, 10.0f, 10.0f);
  glm::vec3 mMoveSpeed = glm::vec3(0.0f);

  const glm::vec3 mUp = glm::vec3(0.0f, 0.0f, 1.0f);
  const glm::vec3 mRight = glm::vec3(1.0f, 0.0f, 0.0f);

  glm::vec3 mAngles = glm::vec3(0.0f);
};
} // namespace camera
