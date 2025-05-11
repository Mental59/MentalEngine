#include "finishLayer.hpp"
#include "vkFramework/vulkanFramebuffer.hpp"
#include "vkFramework/vulkanPipeline.hpp"
#include "vkFramework/vulkanUtils.hpp"
#include <volk.h>

vkFramework::render::FinishLayer::FinishLayer(VulkanRenderDevice& vkDev,
                                              VulkanImage depthTexture)
    : BaseRenderLayer(vkDev, depthTexture) {
  bool useDepth = mDepthTexture.image != VK_NULL_HANDLE;
  CHECK_BOOL(createColorAndDepthRenderPass(
      vkDev, useDepth, &mRenderPass,
      RenderPassCreateInfo{.clearColor = false,
                           .clearDepth = false,
                           .flags = RENDER_PASS_BIT_LAST}));

  CHECK_BOOL(createColorAndDepthFramebuffers(
      vkDev, mRenderPass, depthTexture.imageView, mSwapchainFramebuffers));
}

void vkFramework::render::FinishLayer::fillCommandBuffer(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  const VkRect2D screenRect = {.offset = {0, 0}, .extent = mFramebufferExtent};

  const VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = mRenderPass,
      .framebuffer = mSwapchainFramebuffers[currentImage],
      .renderArea = screenRect};

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  vkCmdEndRenderPass(commandBuffer);
}
