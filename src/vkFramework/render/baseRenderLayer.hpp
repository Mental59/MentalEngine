#pragma once

#include "vkFramework/vulkanImage.hpp"
#include "vkFramework/vulkanRenderDevice.hpp"
#include <cstdint>
#include <vector>
#include <volk.h>

namespace vkFramework::render {

class BaseRenderLayer {
public:
  explicit BaseRenderLayer(const VulkanRenderDevice& vkDev,
                           VulkanImage depthTexture)
      : mDevice(vkDev.device), mFramebufferExtent(vkDev.swapchainExtent),
        mDepthTexture(depthTexture) {}

  virtual ~BaseRenderLayer();
  virtual void fillCommandBuffer(VkCommandBuffer commandBuffer,
                                 uint32_t currentFrame,
                                 uint32_t currentImage) = 0;

  inline VulkanImage getDepthTexture() const { return mDepthTexture; }

protected:
  void beginRenderPass(VkCommandBuffer commandBuffer, uint32_t currentFrame,
                       uint32_t currentImage);

  void beginRenderPassDynamic(VkCommandBuffer commandBuffer,
                              uint32_t currentFrame, uint32_t currentImage);

  void cmdSetViewport(VkCommandBuffer commandBuffer);

  void cmdSetScissor(VkCommandBuffer commandBuffer);

  void endRenderPass(VkCommandBuffer commandBuffer);

  bool createUniformBuffers(VulkanRenderDevice& vkDev, size_t uniformDataSize);

  VkDevice mDevice = VK_NULL_HANDLE;

  VkExtent2D mFramebufferExtent{};

  VulkanImage mDepthTexture{};

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
