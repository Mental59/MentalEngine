#include <render/primitiveMeshLibrary.hpp>

#include <core/log.hpp>

namespace
{
[[nodiscard]] std::size_t toMeshViewIndex(mental::render::SceneGeometryKind kind) noexcept
{
  return static_cast<std::size_t>(kind);
}
} // namespace

bool mental::render::PrimitiveMeshView::isValid() const noexcept
{
  return storageBufferHandle.isValid() && vertexCount > 0u && indexCount > 0u;
}

mental::core::Result mental::render::PrimitiveMeshLibrary::init()
{
  if (mIsInitialized)
  {
    MENTAL_WARN("Trying to initialize an already initialized PrimitiveMeshLibrary");
    return core::Result::eInitializationFailed;
  }

  const PackedPrimitiveGeometryBlob packedGeometry = packBuiltInPrimitiveGeometry();
  if (packedGeometry.bytes.empty())
  {
    MENTAL_ERROR("Failed to pack built-in primitive geometry");
    return core::Result::eInitializationFailed;
  }

  const core::Result uploadResult = uploadPackedGeometry(packedGeometry);
  if (uploadResult != core::Result::eSuccess)
  {
    destroy();
    return uploadResult;
  }

  for (SceneGeometryKind kind : {SceneGeometryKind::eCube, SceneGeometryKind::ePlane, SceneGeometryKind::eSphere})
  {
    const PackedPrimitiveMeshView& packedView = getPackedPrimitiveMeshView(packedGeometry, kind);
    PrimitiveMeshView& meshView = mMeshViews[toMeshViewIndex(kind)];
    meshView.storageBufferHandle = mStorageBufferHandle;
    meshView.vertexOffsetBytes = packedView.vertexOffsetBytes;
    meshView.indexOffsetBytes = packedView.indexOffsetBytes;
    meshView.vertexCount = packedView.vertexCount;
    meshView.indexCount = packedView.indexCount;

    if (!meshView.isValid())
    {
      MENTAL_ERROR(
        "Primitive mesh library produced an invalid mesh view for kind={}", static_cast<std::uint32_t>(kind));
      destroy();
      return core::Result::eInitializationFailed;
    }
  }

  mIsInitialized = true;
  return core::Result::eSuccess;
}

void mental::render::PrimitiveMeshLibrary::destroy()
{
  clearViews();

  if (mStorageBufferHandle.isValid())
  {
    mStorageBufferHandle.destroy();
    mStorageBufferHandle = resource::BufferHandle::invalid();
  }

  mIsInitialized = false;
}

bool mental::render::PrimitiveMeshLibrary::isValid() const noexcept
{
  return mIsInitialized && mStorageBufferHandle.isValid();
}

const mental::render::PrimitiveMeshView* mental::render::PrimitiveMeshLibrary::findMeshView(
  SceneGeometryKind kind) const noexcept
{
  const PrimitiveMeshView& meshView = mMeshViews[toMeshViewIndex(kind)];
  if (!meshView.isValid())
  {
    return nullptr;
  }

  return &meshView;
}

void mental::render::PrimitiveMeshLibrary::clearViews() noexcept
{
  for (PrimitiveMeshView& meshView : mMeshViews)
  {
    meshView = {};
  }
}

mental::core::Result mental::render::PrimitiveMeshLibrary::uploadPackedGeometry(
  const PackedPrimitiveGeometryBlob& packedGeometry)
{
  rhi::BufferDesc bufferDesc {};
  bufferDesc.usage = rhi::BufferUsageFlagBits::eBufferUsageStorageBit;
  bufferDesc.cpuAccess = rhi::BufferCpuAccess::Write;
  bufferDesc.byteSize = packedGeometry.bytes.size();

  mStorageBufferHandle = resource::getResourceManager().createBuffer(bufferDesc);
  if (!mStorageBufferHandle.isValid())
  {
    MENTAL_ERROR("Failed to create primitive mesh storage buffer");
    return core::Result::eInitializationFailed;
  }

  rhi::IBuffer* storageBuffer = mStorageBufferHandle.get();
  if (storageBuffer == nullptr)
  {
    MENTAL_ERROR("Primitive mesh storage buffer handle resolved to null");
    return core::Result::eInitializationFailed;
  }

  void* mappedData = nullptr;
  core::Result result = storageBuffer->map(&mappedData);
  if (result != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to map primitive mesh storage buffer");
    return core::Result::eOperationFailed;
  }

  result = storageBuffer->copy(const_cast<std::byte*>(packedGeometry.bytes.data()), packedGeometry.bytes.size());
  storageBuffer->unmap();
  if (result != core::Result::eSuccess)
  {
    MENTAL_ERROR("Failed to upload primitive mesh geometry blob");
    return core::Result::eOperationFailed;
  }

  return core::Result::eSuccess;
}
