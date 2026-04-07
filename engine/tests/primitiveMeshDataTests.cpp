#include <render/primitiveMeshData.hpp>
#include <render/sceneRenderData.hpp>

#include <glm/geometric.hpp>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace
{
using mental::render::PackedPrimitiveGeometryBlob;
using mental::render::PackedPrimitiveMeshView;
using mental::render::PackedPrimitiveVertex;
using mental::render::PrimitiveMeshData;
using mental::render::PrimitiveVertex;
using mental::render::SceneGeometryKind;

constexpr float kEpsilon = 1.0e-4f;
constexpr float kSphereRadius = 2.0f;

void require(bool condition, const char* message)
{
  if (!condition)
  {
    throw std::runtime_error(message);
  }
}

bool nearlyEqual(float lhs, float rhs, float epsilon = kEpsilon)
{
  return std::fabs(lhs - rhs) <= epsilon;
}

bool nearlyEqual(const glm::vec3& lhs, const glm::vec3& rhs, float epsilon = kEpsilon)
{
  return nearlyEqual(lhs.x, rhs.x, epsilon) && nearlyEqual(lhs.y, rhs.y, epsilon) && nearlyEqual(lhs.z, rhs.z, epsilon);
}

bool nearlyEqual(const glm::vec4& lhs, const glm::vec4& rhs, float epsilon = kEpsilon)
{
  return nearlyEqual(lhs.x, rhs.x, epsilon) && nearlyEqual(lhs.y, rhs.y, epsilon) &&
         nearlyEqual(lhs.z, rhs.z, epsilon) && nearlyEqual(lhs.w, rhs.w, epsilon);
}

bool isFinite(float value)
{
  return std::isfinite(value);
}

bool isFinite(const glm::vec3& value)
{
  return isFinite(value.x) && isFinite(value.y) && isFinite(value.z);
}

bool isFinite(const glm::vec4& value)
{
  return isFinite(value.x) && isFinite(value.y) && isFinite(value.z) && isFinite(value.w);
}

void validateMesh(const PrimitiveMeshData& mesh)
{
  require(!mesh.vertices.empty(), "Generated primitive mesh should contain vertices");
  require(!mesh.indices.empty(), "Generated primitive mesh should contain indices");

  for (const PrimitiveVertex& vertex : mesh.vertices)
  {
    require(isFinite(vertex.position), "Vertex position should be finite");
    require(isFinite(vertex.normal), "Vertex normal should be finite");
    require(isFinite(vertex.color), "Vertex color should be finite");
  }

  for (const std::uint32_t index : mesh.indices)
  {
    require(index < mesh.vertices.size(), "Primitive mesh index should stay within vertex bounds");
  }
}

const PackedPrimitiveMeshView& getView(const PackedPrimitiveGeometryBlob& blob, SceneGeometryKind kind)
{
  return mental::render::getPackedPrimitiveMeshView(blob, kind);
}

void testPackedVertexLayoutIsStorageBufferFriendly()
{
  require(alignof(PackedPrimitiveVertex) == 16u, "Packed primitive vertex should be 16-byte aligned");
  require(sizeof(PackedPrimitiveVertex) % 16u == 0u, "Packed primitive vertex size should stay 16-byte aligned");
}

void testCubeMeshInvariants()
{
  const PrimitiveMeshData mesh = mental::render::generatePrimitiveMeshData(SceneGeometryKind::eCube);
  validateMesh(mesh);

  require(mesh.vertices.size() == 24u, "Cube should generate 24 vertices");
  require(mesh.indices.size() == 36u, "Cube should generate 36 indices");

  for (std::size_t faceIndex = 0; faceIndex < 6u; ++faceIndex)
  {
    const std::size_t vertexBase = faceIndex * 4u;
    const glm::vec3 expectedNormal = mesh.vertices[vertexBase].normal;
    require(nearlyEqual(glm::abs(expectedNormal), glm::vec3 {1.0f, 0.0f, 0.0f}) ||
              nearlyEqual(glm::abs(expectedNormal), glm::vec3 {0.0f, 1.0f, 0.0f}) ||
              nearlyEqual(glm::abs(expectedNormal), glm::vec3 {0.0f, 0.0f, 1.0f}),
      "Cube face normal should be axis-aligned");

    for (std::size_t vertexOffset = 0; vertexOffset < 4u; ++vertexOffset)
    {
      require(nearlyEqual(mesh.vertices[vertexBase + vertexOffset].normal, expectedNormal),
        "Cube face vertices should share one hard-face normal");
    }
  }
}

void testPlaneMeshInvariants()
{
  const PrimitiveMeshData mesh = mental::render::generatePrimitiveMeshData(SceneGeometryKind::ePlane);
  validateMesh(mesh);

  require(mesh.vertices.size() == 4u, "Plane should generate 4 vertices");
  require(mesh.indices.size() == 6u, "Plane should generate 6 indices");

  for (const PrimitiveVertex& vertex : mesh.vertices)
  {
    require(nearlyEqual(vertex.normal, glm::vec3 {0.0f, 1.0f, 0.0f}), "Plane normal should point along +Y");
    require(nearlyEqual(vertex.position.y, 0.0f), "Plane vertices should lie on the XZ plane");
    require(std::fabs(vertex.position.x) <= 2.0f + kEpsilon, "Plane width should stay centered in [-2.0, 2.0]");
    require(std::fabs(vertex.position.z) <= 2.0f + kEpsilon, "Plane depth should stay centered in [-2.0, 2.0]");
  }
}

void testSphereMeshInvariants()
{
  const PrimitiveMeshData mesh = mental::render::generatePrimitiveMeshData(SceneGeometryKind::eSphere);
  validateMesh(mesh);

  for (const PrimitiveVertex& vertex : mesh.vertices)
  {
    const float radius = glm::length(vertex.position);
    require(nearlyEqual(radius, kSphereRadius, 2.0e-3f), "Sphere positions should lie on the configured radius");
    require(nearlyEqual(glm::length(vertex.normal), 1.0f, 2.0e-3f), "Sphere normals should stay unit length");
    require(glm::dot(glm::normalize(vertex.position), vertex.normal) > 0.99f, "Sphere normals should face outward");
  }

  const PrimitiveMeshData secondMesh = mental::render::generatePrimitiveMeshData(SceneGeometryKind::eSphere);
  require(mesh.vertices == secondMesh.vertices, "Sphere vertex generation should be deterministic");
  require(mesh.indices == secondMesh.indices, "Sphere index generation should be deterministic");
}

void testBuiltInPrimitivePackingIsDeterministicAndAligned()
{
  const PackedPrimitiveGeometryBlob firstBlob = mental::render::packBuiltInPrimitiveGeometry();
  const PackedPrimitiveGeometryBlob secondBlob = mental::render::packBuiltInPrimitiveGeometry();

  require(!firstBlob.bytes.empty(), "Packed primitive geometry blob should contain data");
  require(firstBlob.bytes == secondBlob.bytes, "Packed primitive geometry blob should be deterministic");

  constexpr std::array<SceneGeometryKind, 3> kKinds {
    SceneGeometryKind::eCube,
    SceneGeometryKind::ePlane,
    SceneGeometryKind::eSphere,
  };

  for (const SceneGeometryKind kind : kKinds)
  {
    const PackedPrimitiveMeshView& firstView = getView(firstBlob, kind);
    const PackedPrimitiveMeshView& secondView = getView(secondBlob, kind);

    require(firstView.vertexCount > 0u, "Packed primitive view should expose a non-zero vertex count");
    require(firstView.indexCount > 0u, "Packed primitive view should expose a non-zero index count");
    require(firstView.vertexOffsetBytes % alignof(PackedPrimitiveVertex) == 0u,
      "Packed vertex data should satisfy PackedPrimitiveVertex alignment");
    require(
      firstView.indexOffsetBytes % alignof(std::uint32_t) == 0u, "Packed index data should satisfy uint32_t alignment");
    require(
      firstView.vertexOffsetBytes + firstView.vertexCount * sizeof(PackedPrimitiveVertex) <= firstBlob.bytes.size(),
      "Packed vertex view should stay within blob bounds");
    require(firstView.indexOffsetBytes + firstView.indexCount * sizeof(std::uint32_t) <= firstBlob.bytes.size(),
      "Packed index view should stay within blob bounds");

    require(
      firstView.vertexOffsetBytes == secondView.vertexOffsetBytes, "Packed vertex offsets should be deterministic");
    require(firstView.indexOffsetBytes == secondView.indexOffsetBytes, "Packed index offsets should be deterministic");
    require(firstView.vertexCount == secondView.vertexCount, "Packed vertex counts should be deterministic");
    require(firstView.indexCount == secondView.indexCount, "Packed index counts should be deterministic");
  }
}
} // namespace

int main()
{
  try
  {
    testPackedVertexLayoutIsStorageBufferFriendly();
    testCubeMeshInvariants();
    testPlaneMeshInvariants();
    testSphereMeshInvariants();
    testBuiltInPrimitivePackingIsDeterministicAndAligned();
    return 0;
  }
  catch (const std::exception& exception)
  {
    std::cerr << exception.what() << '\n';
  }
  catch (...)
  {
    std::cerr << "Unknown exception\n";
  }

  return 1;
}
