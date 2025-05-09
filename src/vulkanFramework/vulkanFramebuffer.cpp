#include "vulkanFramebuffer.hpp"
#include "vulkanUtils.hpp"
#include <array>

bool mental::createColorAndDepthFramebuffers(
    VulkanRenderDevice& vkDev, VkRenderPass renderPass,
    VkImageView depthImageView,
    std::vector<VkFramebuffer>& swapchainFramebuffers) {

  uint32_t numSwapchainImages =
      static_cast<uint32_t>(vkDev.swapchainImageViews.size());

  swapchainFramebuffers.resize(numSwapchainImages);

  for (size_t i = 0; i < swapchainFramebuffers.size(); i++) {
    std::array<VkImageView, 2> attachments = {vkDev.swapchainImageViews[i],
                                              depthImageView};

    const VkFramebufferCreateInfo framebufferInfo = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .renderPass = renderPass,
        .attachmentCount =
            static_cast<uint32_t>((depthImageView == VK_NULL_HANDLE) ? 1 : 2),
        .pAttachments = attachments.data(),
        .width = vkDev.framebufferWidth,
        .height = vkDev.framebufferHeight,
        .layers = 1};

    MENTAL_VK_CHECK(vkCreateFramebuffer(vkDev.device, &framebufferInfo, nullptr,
                                        &swapchainFramebuffers[i]));
  }

  return true;
}
