#pragma once
#include "baseRenderLayer.hpp"
#include "imguiLayer.hpp"
#include "vkFramework/vulkanTexture.hpp"
#include <vector>

class ImDrawData;

namespace vkFramework::render {
class ImGuiLayer : public BaseRenderLayer {
public:
  explicit ImGuiLayer() = default;

  void init(const VulkanRenderDevice* vkDev);
  void init(const VulkanRenderDevice* vkDev,
            const std::vector<VulkanTexture>& textures);

  void virtual destroy() override;

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) override;

  virtual bool createFramebuffers() override;

  void updateBuffers(uint32_t currentFrame, const ImDrawData* imguiDrawData);

private:
  bool createDescriptorSet();

  // Descriptor set with multiple textures (for offscreen buffer display etc.)
  bool createMultiDescriptorSet();

  const ImDrawData* mDrawData = nullptr;

  std::vector<VulkanTexture> mExtTextures;

  VkDeviceSize mBufferSize;
  std::vector<VkBuffer> mStorageBuffer;
  std::vector<VkDeviceMemory> mStorageBufferMemory;

  VulkanImage mFont;
};
} // namespace vkFramework::render
