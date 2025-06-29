#pragma once

#include "vkFramework/vulkanImage.hpp"
#include "vkFramework/vulkanRenderDevice.hpp"
#include <cstdint>
#include <vector>
#include <volk.h>

namespace vkFramework::render {

class BaseRenderLayer {
public:
  explicit BaseRenderLayer() = default;
  void init(const VulkanRenderDevice* vkDev, VulkanImage* depthTexture);

  void virtual destroy();

  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) = 0;

  inline const VulkanImage* getDepthTexture() const { return mDepthTexture; }

  void destroyFramebuffers();

  bool createFramebuffers(VkImageView depthImageView);
  virtual bool createFramebuffers() = 0;

protected:
  void beginRenderPass(VkCommandBuffer commandBuffer, uint32_t currentFrame,
                       uint32_t currentImage);

  void beginRenderPassDynamic(VkCommandBuffer commandBuffer,
                              uint32_t currentFrame, uint32_t currentImage);

  void cmdSetViewport(VkCommandBuffer commandBuffer);

  void cmdSetScissor(VkCommandBuffer commandBuffer);

  void endRenderPass(VkCommandBuffer commandBuffer);

  bool createUniformBuffers(size_t uniformDataSize);

  const VulkanRenderDevice* mRenderDevice = nullptr;

  VulkanImage* mDepthTexture = nullptr;

  VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> mDescriptorSets;

  std::vector<VkFramebuffer> mSwapchainFramebuffers;

  VkRenderPass mRenderPass = VK_NULL_HANDLE;
  VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
  VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;

  std::vector<VkBuffer> mUniformBuffers;
  std::vector<VkDeviceMemory> mUniformBuffersMemory;
};

} // namespace vkFramework::render
