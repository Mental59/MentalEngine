#include "baseRenderLayer.hpp"
#include "vkFramework/vulkanMemory.hpp"
#include <volk.h>

vkFramework::render::BaseRenderLayer::~BaseRenderLayer() {
  for (VkBuffer buf : mUniformBuffers)
    vkDestroyBuffer(mDevice, buf, nullptr);

  for (VkDeviceMemory mem : mUniformBuffersMemory)
    vkFreeMemory(mDevice, mem, nullptr);

  vkDestroyDescriptorSetLayout(mDevice, mDescriptorSetLayout, nullptr);
  vkDestroyDescriptorPool(mDevice, mDescriptorPool, nullptr);

  for (VkFramebuffer framebuffer : mSwapchainFramebuffers)
    vkDestroyFramebuffer(mDevice, framebuffer, nullptr);

  vkDestroyRenderPass(mDevice, mRenderPass, nullptr);
  vkDestroyPipelineLayout(mDevice, mPipelineLayout, nullptr);
  vkDestroyPipeline(mDevice, mGraphicsPipeline, nullptr);
}

void vkFramework::render::BaseRenderLayer::beginRenderPass(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  const VkRect2D screenRect = {.offset = {0, 0}, .extent = mFramebufferExtent};

  const VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .pNext = nullptr,
      .renderPass = mRenderPass,
      .framebuffer = mSwapchainFramebuffers[currentImage],
      .renderArea = screenRect};

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    mGraphicsPipeline);
  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame],
                          0, nullptr);
}

bool vkFramework::render::BaseRenderLayer::createUniformBuffers(
    VulkanRenderDevice& vkDev, size_t uniformDataSize) {
  mUniformBuffers.resize(vkDev.maxFramesInFlight);
  mUniformBuffersMemory.resize(vkDev.maxFramesInFlight);

  for (size_t i = 0; i < mUniformBuffers.size(); i++) {
    if (!vkFramework::createUniformBuffer(vkDev, uniformDataSize,
                                          mUniformBuffers[i],
                                          mUniformBuffersMemory[i])) {
      return false;
    }
  }
  return true;
}
