#pragma once

#include <core/types.hpp>
#include <render/primitiveMeshData.hpp>
#include <resource/resourceManager.hpp>

#include <array>

namespace mental::render
{
struct PrimitiveMeshView
{
  resource::BufferHandle storageBufferHandle = resource::BufferHandle::invalid();
  std::size_t vertexOffsetBytes {0u};
  std::size_t indexOffsetBytes {0u};
  std::uint32_t vertexCount {0u};
  std::uint32_t indexCount {0u};

  [[nodiscard]] bool isValid() const noexcept;
};

class PrimitiveMeshLibrary
{
 public:
  [[nodiscard]] core::Result init();
  void destroy();

  [[nodiscard]] bool isValid() const noexcept;
  [[nodiscard]] const PrimitiveMeshView* findMeshView(SceneGeometryKind kind) const noexcept;

 private:
  void clearViews() noexcept;
  [[nodiscard]] core::Result uploadPackedGeometry(const PackedPrimitiveGeometryBlob& packedGeometry);

  resource::BufferHandle mStorageBufferHandle = resource::BufferHandle::invalid();
  std::array<PrimitiveMeshView, 3> mMeshViews {};
  bool mIsInitialized = false;
};
} // namespace mental::render
