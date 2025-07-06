#include "clearLayer.hpp"
#include "vkFramework/vulkanFramebuffer.hpp"
#include "vkFramework/vulkanPipeline.hpp"
#include "vkFramework/vulkanUtils.hpp"

void vkFramework::render::ClearLayer::init(const VulkanRenderDevice* vkDev,
                                           VulkanImage* depthTexture)

{
  CHECK_BOOL(depthTexture != nullptr);
  BaseRenderLayer::init(vkDev, depthTexture);
  mShouldClearDepth = depthTexture->image != VK_NULL_HANDLE;
  CHECK_BOOL(createColorAndDepthRenderPass(
      *vkDev, mShouldClearDepth, &mRenderPass,
      RenderPassCreateInfo{.clearColor = true,
                           .clearDepth = true,
                           .flags = RENDER_PASS_BIT_FIRST}));

  CHECK_BOOL(createFramebuffers());
}

bool vkFramework::render::ClearLayer::createFramebuffers() {
  return BaseRenderLayer::createFramebuffers(mDepthTexture->imageView);
}

void vkFramework::render::ClearLayer::fillCommandBuffer(
    VkCommandBuffer commandBuffer, uint32_t currentFrame,
    uint32_t currentImage) {
  const VkClearValue clearValues[2] = {
      VkClearValue{.color = {0.75f, 0.75f, 0.75f, 1.0f}},
      VkClearValue{.depthStencil = {1.0f, 0}}};

  const VkRect2D screenRect = {.offset = {0, 0},
                               .extent = mRenderDevice->swapchainExtent};

  const VkRenderPassBeginInfo renderPassInfo = {
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = mRenderPass,
      .framebuffer = mSwapchainFramebuffers[currentImage],
      .renderArea = screenRect,
      .clearValueCount = static_cast<uint32_t>(mShouldClearDepth ? 2 : 1),
      .pClearValues = &clearValues[0]};

  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdEndRenderPass(commandBuffer);
}
