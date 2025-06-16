#pragma once
#include <glm/glm.hpp>

namespace camera {
class ICameraPositioner {
public:
  virtual ~ICameraPositioner() = default;
  virtual glm::mat4 getViewMatrix() const = 0;
  virtual glm::vec3 getPosition() const = 0;
};

class Camera final {
public:
  explicit Camera(const ICameraPositioner& positioner)
      : mPositioner(&positioner) {}

  Camera(const Camera&) = default;
  Camera& operator=(const Camera&) = default;

  inline glm::mat4 getViewMatrix() const {
    return mPositioner->getViewMatrix();
  }
  inline glm::vec3 getPosition() const { return mPositioner->getPosition(); }

private:
  const ICameraPositioner* mPositioner;
};
} // namespace camera
