#pragma once
#include "baseRenderLayer.hpp"

namespace vkFramework::render {

class ModelRenderLayer : public BaseRenderLayer {
public:
  ModelRenderLayer(VulkanRenderDevice& vkDev, const char* modelFile,
                   const char* textureFile, uint32_t uniformDataSize);

  ModelRenderLayer(VulkanRenderDevice& vkDev, bool useDepth,
                   VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory,
                   uint32_t vertexBufferSize, uint32_t indexBufferSize,
                   VulkanImage texture,
                   const std::vector<const char*>& shaderFiles,
                   uint32_t uniformDataSize,
                   bool useGeneralTextureLayout = true,
                   VulkanImage externalDepth = {}, bool deleteMeshData = true);

  virtual ~ModelRenderLayer();

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

  void updateUniformBuffer(VulkanRenderDevice& vkDev, uint32_t currentFrame,
                           const void* data, const size_t dataSize);

  void freeTextureSampler();

private:
  bool mUseGeneralTextureLayout = false;
  bool mIsExternalDepth = false;
  bool mDeleteMeshData = true;

  size_t mVertexBufferSize;
  size_t mIndexBufferSize;

  VkBuffer mStorageBuffer;
  VkDeviceMemory mStorageBufferMemory;

  VulkanImage mTexture;

  bool createDescriptorSet(VulkanRenderDevice& vkDev, uint32_t uniformDataSize);
};

} // namespace vkFramework::render