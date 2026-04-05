#pragma once

#include <render/sceneRenderData.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include <glm/glm.hpp>

namespace mental::render
{
struct PrimitiveVertex
{
  glm::vec3 position {0.0f, 0.0f, 0.0f};
  glm::vec3 normal {0.0f, 1.0f, 0.0f};
  glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};

  [[nodiscard]] bool operator==(const PrimitiveVertex& other) const noexcept;
};

struct alignas(16) PackedPrimitiveVertex
{
  glm::vec4 position {0.0f, 0.0f, 0.0f, 1.0f};
  glm::vec4 normal {0.0f, 1.0f, 0.0f, 0.0f};
  glm::vec4 color {1.0f, 1.0f, 1.0f, 1.0f};
  glm::vec4 reserved {0.0f, 0.0f, 0.0f, 0.0f};
};

struct PrimitiveMeshData
{
  std::vector<PrimitiveVertex> vertices {};
  std::vector<std::uint32_t> indices {};
};

struct PackedPrimitiveMeshView
{
  std::size_t vertexOffsetBytes {0u};
  std::size_t indexOffsetBytes {0u};
  std::uint32_t vertexCount {0u};
  std::uint32_t indexCount {0u};
};

struct PackedPrimitiveGeometryBlob
{
  std::vector<std::byte> bytes {};
  std::array<PackedPrimitiveMeshView, 3> meshViews {};
};

[[nodiscard]] PrimitiveMeshData generatePrimitiveMeshData(SceneGeometryKind kind);
[[nodiscard]] PackedPrimitiveGeometryBlob packBuiltInPrimitiveGeometry();
[[nodiscard]] const PackedPrimitiveMeshView& getPackedPrimitiveMeshView(
  const PackedPrimitiveGeometryBlob& blob, SceneGeometryKind kind);
} // namespace mental::render
