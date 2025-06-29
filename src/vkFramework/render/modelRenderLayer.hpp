#pragma once
#include "baseRenderLayer.hpp"

namespace vkFramework::render {

class ModelRenderLayer : public BaseRenderLayer {
public:
  explicit ModelRenderLayer() = default;

  void init(const VulkanRenderDevice* vkDev, const char* modelFile,
            const char* textureFile, uint32_t uniformDataSize,
            VulkanImage* depthTexture);

  void init(const VulkanRenderDevice* vkDev, bool useDepth,
            VkBuffer storageBuffer, VkDeviceMemory storageBufferMemory,
            uint32_t vertexBufferSize, uint32_t indexBufferSize,
            VulkanImage texture, const std::vector<const char*>& shaderFiles,
            uint32_t uniformDataSize, bool useGeneralTextureLayout = true,
            VulkanImage* externalDepth = nullptr, bool deleteMeshData = true);

  void virtual destroy() override;

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

  virtual bool createFramebuffers() override;

  void updateUniformBuffer(uint32_t currentFrame, const void* data,
                           const size_t dataSize);

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

  bool createDescriptorSet(uint32_t uniformDataSize);
};

} // namespace vkFramework::render