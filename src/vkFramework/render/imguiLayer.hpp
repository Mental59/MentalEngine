#pragma once
#include "baseRenderLayer.hpp"
#include "imguiLayer.hpp"
#include "vkFramework/vulkanTexture.hpp"
#include <vector>

class ImDrawData;

namespace vkFramework::render {
class ImGuiRenderer : public BaseRenderLayer {
public:
  explicit ImGuiRenderer(VulkanRenderDevice& vkDev);
  explicit ImGuiRenderer(VulkanRenderDevice& vkDev,
                         const std::vector<VulkanTexture>& textures);
  virtual ~ImGuiRenderer();

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

  void updateBuffers(VulkanRenderDevice& vkDev, uint32_t currentFrame,
                     const ImDrawData* imguiDrawData);

private:
  bool createDescriptorSet(VulkanRenderDevice& vkDev);

  // Descriptor set with multiple textures (for offscreen buffer display etc.)
  bool createMultiDescriptorSet(VulkanRenderDevice& vkDev);

  const ImDrawData* mDrawData = nullptr;

  std::vector<VulkanTexture> mExtTextures;

  VkDeviceSize mBufferSize;
  std::vector<VkBuffer> mStorageBuffer;
  std::vector<VkDeviceMemory> mStorageBufferMemory;

  VulkanImage mFont;
};
} // namespace vkFramework::render
