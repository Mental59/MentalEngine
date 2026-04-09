#include <render/primitiveMeshData.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>

namespace
{
using mental::render::PackedPrimitiveGeometryBlob;
using mental::render::PackedPrimitiveMeshView;
using mental::render::PackedPrimitiveVertex;
using mental::render::PrimitiveMeshData;
using mental::render::PrimitiveVertex;
using mental::render::SceneGeometryKind;

constexpr glm::vec4 kDefaultColor {1.0f, 0.5f, 0.31f, 1.0f};
constexpr float kCubeHalfExtent = 2.0f;
constexpr float kPlaneHalfExtent = 2.0f;
constexpr float kSphereRadius = 2.0f;
constexpr std::uint32_t kSphereRingCount = 32u;
constexpr std::uint32_t kSphereSegmentCount = 32u;
constexpr std::size_t kBlobAlignment = alignof(PackedPrimitiveVertex);
constexpr std::array<SceneGeometryKind, 3> kBuiltInPrimitiveOrder {
  SceneGeometryKind::eCube,
  SceneGeometryKind::ePlane,
  SceneGeometryKind::eSphere,
};

[[nodiscard]] bool areExactlyEqual(const glm::vec3& lhs, const glm::vec3& rhs) noexcept
{
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

[[nodiscard]] bool areExactlyEqual(const glm::vec4& lhs, const glm::vec4& rhs) noexcept
{
  return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

[[nodiscard]] std::size_t toMeshViewIndex(SceneGeometryKind kind) noexcept
{
  return static_cast<std::size_t>(kind);
}

void alignByteStorage(std::vector<std::byte>& bytes, std::size_t alignment)
{
  const std::size_t remainder = bytes.size() % alignment;
  if (remainder == 0u)
  {
    return;
  }

  bytes.resize(bytes.size() + (alignment - remainder), std::byte {0});
}

template <typename T> void appendObjects(std::vector<std::byte>& bytes, const std::vector<T>& objects)
{
  if (objects.empty())
  {
    return;
  }

  const std::size_t byteCount = objects.size() * sizeof(T);
  const std::size_t startOffset = bytes.size();
  bytes.resize(startOffset + byteCount);
  std::memcpy(bytes.data() + startOffset, objects.data(), byteCount);
}

[[nodiscard]] bool isFinite(const glm::vec3& value) noexcept
{
  return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
}

[[nodiscard]] bool isFinite(const glm::vec4& value) noexcept
{
  return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z) && std::isfinite(value.w);
}

[[nodiscard]] bool isValidMeshData(const PrimitiveMeshData& mesh) noexcept
{
  if (mesh.vertices.empty() || mesh.indices.empty())
  {
    return false;
  }

  for (const PrimitiveVertex& vertex : mesh.vertices)
  {
    if (!isFinite(vertex.position) || !isFinite(vertex.normal) || !isFinite(vertex.color))
    {
      return false;
    }
  }

  return std::ranges::all_of(mesh.indices, [&mesh](std::uint32_t index) { return index < mesh.vertices.size(); });
}

PackedPrimitiveVertex packVertex(const PrimitiveVertex& vertex) noexcept
{
  PackedPrimitiveVertex packedVertex {};
  packedVertex.position = glm::vec4 {vertex.position, 1.0f};
  packedVertex.normal = glm::vec4 {vertex.normal, 0.0f};
  packedVertex.color = vertex.color;
  return packedVertex;
}

void appendQuad(PrimitiveMeshData& mesh,
  const glm::vec3& normal,
  const glm::vec3& first,
  const glm::vec3& second,
  const glm::vec3& third,
  const glm::vec3& fourth)
{
  const std::uint32_t baseVertex = static_cast<std::uint32_t>(mesh.vertices.size());
  mesh.vertices.push_back({.position = first, .normal = normal, .color = kDefaultColor});
  mesh.vertices.push_back({.position = second, .normal = normal, .color = kDefaultColor});
  mesh.vertices.push_back({.position = third, .normal = normal, .color = kDefaultColor});
  mesh.vertices.push_back({.position = fourth, .normal = normal, .color = kDefaultColor});

  mesh.indices.push_back(baseVertex + 0u);
  mesh.indices.push_back(baseVertex + 1u);
  mesh.indices.push_back(baseVertex + 2u);
  mesh.indices.push_back(baseVertex + 2u);
  mesh.indices.push_back(baseVertex + 3u);
  mesh.indices.push_back(baseVertex + 0u);
}

[[nodiscard]] PrimitiveMeshData buildCubeMeshData()
{
  PrimitiveMeshData mesh {};

  appendQuad(mesh,
    glm::vec3 {1.0f, 0.0f, 0.0f},
    glm::vec3 {kCubeHalfExtent, -kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, -kCubeHalfExtent, kCubeHalfExtent});

  appendQuad(mesh,
    glm::vec3 {-1.0f, 0.0f, 0.0f},
    glm::vec3 {-kCubeHalfExtent, -kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, -kCubeHalfExtent, -kCubeHalfExtent});

  appendQuad(mesh,
    glm::vec3 {0.0f, 1.0f, 0.0f},
    glm::vec3 {-kCubeHalfExtent, kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, kCubeHalfExtent, -kCubeHalfExtent});

  appendQuad(mesh,
    glm::vec3 {0.0f, -1.0f, 0.0f},
    glm::vec3 {-kCubeHalfExtent, -kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, -kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, -kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, -kCubeHalfExtent, kCubeHalfExtent});

  appendQuad(mesh,
    glm::vec3 {0.0f, 0.0f, 1.0f},
    glm::vec3 {-kCubeHalfExtent, -kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, -kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, kCubeHalfExtent, kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, kCubeHalfExtent, kCubeHalfExtent});

  appendQuad(mesh,
    glm::vec3 {0.0f, 0.0f, -1.0f},
    glm::vec3 {kCubeHalfExtent, -kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, -kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {-kCubeHalfExtent, kCubeHalfExtent, -kCubeHalfExtent},
    glm::vec3 {kCubeHalfExtent, kCubeHalfExtent, -kCubeHalfExtent});

  return mesh;
}

[[nodiscard]] PrimitiveMeshData buildPlaneMeshData()
{
  PrimitiveMeshData mesh {};
  appendQuad(mesh,
    glm::vec3 {0.0f, 1.0f, 0.0f},
    glm::vec3 {-kPlaneHalfExtent, 0.0f, kPlaneHalfExtent},
    glm::vec3 {kPlaneHalfExtent, 0.0f, kPlaneHalfExtent},
    glm::vec3 {kPlaneHalfExtent, 0.0f, -kPlaneHalfExtent},
    glm::vec3 {-kPlaneHalfExtent, 0.0f, -kPlaneHalfExtent});
  return mesh;
}

[[nodiscard]] PrimitiveMeshData buildSphereMeshData()
{
  PrimitiveMeshData mesh {};
  mesh.vertices.reserve(
    static_cast<std::size_t>(kSphereRingCount + 1u) * static_cast<std::size_t>(kSphereSegmentCount + 1u));
  mesh.indices.reserve(static_cast<std::size_t>(kSphereRingCount) * static_cast<std::size_t>(kSphereSegmentCount) * 6u);

  constexpr float kPi = 3.14159265358979323846f;
  constexpr float kTwoPi = 2.0f * kPi;

  for (std::uint32_t ring = 0u; ring <= kSphereRingCount; ++ring)
  {
    const float v = static_cast<float>(ring) / static_cast<float>(kSphereRingCount);
    const float theta = v * kPi;
    const float sinTheta = std::sin(theta);
    const float cosTheta = std::cos(theta);

    for (std::uint32_t segment = 0u; segment <= kSphereSegmentCount; ++segment)
    {
      const float u = static_cast<float>(segment) / static_cast<float>(kSphereSegmentCount);
      const float phi = u * kTwoPi;
      const float sinPhi = std::sin(phi);
      const float cosPhi = std::cos(phi);

      const glm::vec3 normal = glm::normalize(glm::vec3 {
        sinTheta * cosPhi,
        cosTheta,
        sinTheta * sinPhi,
      });

      mesh.vertices.push_back({
        .position = normal * kSphereRadius,
        .normal = normal,
        .color = kDefaultColor,
      });
    }
  }

  const std::uint32_t stride = kSphereSegmentCount + 1u;
  for (std::uint32_t ring = 0u; ring < kSphereRingCount; ++ring)
  {
    for (std::uint32_t segment = 0u; segment < kSphereSegmentCount; ++segment)
    {
      const std::uint32_t topLeft = ring * stride + segment;
      const std::uint32_t bottomLeft = (ring + 1u) * stride + segment;
      const std::uint32_t bottomRight = bottomLeft + 1u;
      const std::uint32_t topRight = topLeft + 1u;

      mesh.indices.push_back(topLeft);
      mesh.indices.push_back(bottomRight);
      mesh.indices.push_back(bottomLeft);
      mesh.indices.push_back(topLeft);
      mesh.indices.push_back(topRight);
      mesh.indices.push_back(bottomRight);
    }
  }

  return mesh;
}
} // namespace

bool mental::render::PrimitiveVertex::operator==(const PrimitiveVertex& other) const noexcept
{
  return areExactlyEqual(position, other.position) && areExactlyEqual(normal, other.normal) &&
         areExactlyEqual(color, other.color);
}

mental::render::PrimitiveMeshData mental::render::generatePrimitiveMeshData(SceneGeometryKind kind)
{
  PrimitiveMeshData mesh {};
  switch (kind)
  {
    case SceneGeometryKind::eCube:
      mesh = buildCubeMeshData();
      break;
    case SceneGeometryKind::ePlane:
      mesh = buildPlaneMeshData();
      break;
    case SceneGeometryKind::eSphere:
      mesh = buildSphereMeshData();
      break;
  }

  if (!isValidMeshData(mesh))
  {
    return {};
  }

  return mesh;
}

mental::render::PackedPrimitiveGeometryBlob mental::render::packBuiltInPrimitiveGeometry()
{
  PackedPrimitiveGeometryBlob packedBlob {};

  for (const SceneGeometryKind kind : kBuiltInPrimitiveOrder)
  {
    const PrimitiveMeshData mesh = generatePrimitiveMeshData(kind);
    if (!isValidMeshData(mesh))
    {
      return {};
    }

    PackedPrimitiveMeshView& view = packedBlob.meshViews[toMeshViewIndex(kind)];
    view.vertexCount = static_cast<std::uint32_t>(mesh.vertices.size());
    view.indexCount = static_cast<std::uint32_t>(mesh.indices.size());

    alignByteStorage(packedBlob.bytes, alignof(std::uint32_t));
    view.indexOffsetBytes = packedBlob.bytes.size();
    appendObjects(packedBlob.bytes, mesh.indices);

    std::vector<PackedPrimitiveVertex> packedVertices {};
    packedVertices.reserve(mesh.vertices.size());
    for (const PrimitiveVertex& vertex : mesh.vertices)
    {
      packedVertices.push_back(packVertex(vertex));
    }

    alignByteStorage(packedBlob.bytes, alignof(PackedPrimitiveVertex));
    view.vertexOffsetBytes = packedBlob.bytes.size();
    appendObjects(packedBlob.bytes, packedVertices);
  }

  alignByteStorage(packedBlob.bytes, kBlobAlignment);
  return packedBlob;
}

const mental::render::PackedPrimitiveMeshView& mental::render::getPackedPrimitiveMeshView(
  const PackedPrimitiveGeometryBlob& blob, SceneGeometryKind kind)
{
  return blob.meshViews[toMeshViewIndex(kind)];
}
