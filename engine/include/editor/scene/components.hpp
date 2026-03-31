#pragma once

#include <array>
#include <string>

#include <glm/glm.hpp>

namespace mental::editor
{
enum class PrimitiveType
{
  eCube = 0,
  ePlane,
  eSphere,
};

struct PrimitiveTypeInfo
{
  PrimitiveType type;
  const char* baseName;
};

inline constexpr std::array kPrimitiveTypeInfos {
  PrimitiveTypeInfo {PrimitiveType::eCube,   "Cube"  },
  PrimitiveTypeInfo {PrimitiveType::ePlane,  "Plane" },
  PrimitiveTypeInfo {PrimitiveType::eSphere, "Sphere"},
};

struct NameComponent
{
  std::string value {};
};

struct TransformComponent
{
  glm::vec3 position {0.0f, 0.0f, 0.0f};
  glm::vec3 rotation {0.0f, 0.0f, 0.0f};
  glm::vec3 scale {1.0f, 1.0f, 1.0f};
};

struct PrimitiveComponent
{
  PrimitiveType type {PrimitiveType::eCube};
};
} // namespace mental::editor
