#include "baseRenderLayer.hpp"
#include "vkFramework/vulkanFramebuffer.hpp"
#include "vkFramework/vulkanMemory.hpp"
#include "vkFramework/vulkanUtils.hpp"
#include <volk.h>

void vkFramework::render::BaseRenderLayer::init(const VulkanRenderDevice* vkDev,
                                                VulkanImage* depthTexture) {
  CHECK_BOOL(vkDev != nullptr);
  mRenderDevice = vkDev;
  mDepthTexture = depthTexture;
}

void vkFramework::render::BaseRenderLayer::destroy() {
  for (VkBuffer buf : mUniformBuffers)
    vkDestroyBuffer(mRenderDevice->device, buf, nullptr);

  for (VkDeviceMemory mem : mUniformBuffersMemory)
    vkFreeMemory(mRenderDevice->device, mem, nullptr);

  if (mDescriptorSetLayout) {
    vkDestroyDescriptorSetLayout(mRenderDevice->device, mDescriptorSetLayout,
                                 nullptr);
  }

  if (mDescriptorPool) {
    vkDestroyDescriptorPool(mRenderDevice->device, mDescriptorPool, nullptr);
  }

  destroyFramebuffers();

  if (mRenderPass) {
    vkDestroyRenderPass(mRenderDevice->device, mRenderPass, nullptr);
  }
  if (mPipelineLayout) {
    vkDestroyPipelineLayout(mRenderDevice->device, mPipelineLayout, nullptr);
  }
  if (mGraphicsPipeline) {
    vkDestroyPipeline(mRenderDevice->device, mGraphicsPipeline, nullptr);
  }
}

void vkFramework::render::BaseRenderLayer::destroyFramebuffers() {
  for (VkFramebuffer framebuffer : mSwapchainFramebuffers) {
    vkDestroyFramebuffer(mRenderDevice->device, framebuffer, nullptr);
  }
}

bool vkFramework::render::BaseRenderLayer::createFramebuffers(
    VkImageView depthImageView) {
  return createColorAndDepthFramebuffers(
      *mRenderDevice, mRenderPass, depthImageView, mSwapchainFramebuffers);
}

void vkFramework::render::BaseRenderLayer::beginRenderPass(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  const VkRect2D screenRect = {.offset = {0, 0},
                               .extent = mRenderDevice->swapchainExtent};

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

void vkFramework::render::BaseRenderLayer::beginRenderPassDynamic(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  beginRenderPass(commandBuffer, currentFrame, currentImage);
  cmdSetViewport(commandBuffer);
  cmdSetScissor(commandBuffer);
}

void vkFramework::render::BaseRenderLayer::cmdSetViewport(
    VkCommandBuffer commandBuffer) {
  VkViewport viewport{
      .x = 0.0f,
      .y = 0.0f,
      .width = static_cast<float>(mRenderDevice->swapchainExtent.width),
      .height = static_cast<float>(mRenderDevice->swapchainExtent.height),
      .minDepth = 0.0f,
      .maxDepth = 1.0f};
  vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void vkFramework::render::BaseRenderLayer::cmdSetScissor(
    VkCommandBuffer commandBuffer) {
  VkRect2D scissor{.offset = {0, 0}, .extent = mRenderDevice->swapchainExtent};
  vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void vkFramework::render::BaseRenderLayer::endRenderPass(
    VkCommandBuffer commandBuffer) {
  vkCmdEndRenderPass(commandBuffer);
}

bool vkFramework::render::BaseRenderLayer::createUniformBuffers(
    size_t uniformDataSize) {
  mUniformBuffers.resize(mRenderDevice->maxFramesInFlight);
  mUniformBuffersMemory.resize(mRenderDevice->maxFramesInFlight);

  for (size_t i = 0; i < mUniformBuffers.size(); i++) {
    if (!vkFramework::createUniformBuffer(*mRenderDevice, uniformDataSize,
                                          mUniformBuffers[i],
                                          mUniformBuffersMemory[i])) {
      return false;
    }
  }
  return true;
}
