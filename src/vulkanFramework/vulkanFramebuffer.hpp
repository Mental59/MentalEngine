#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace mental {
bool createColorAndDepthFramebuffers(
    VulkanRenderDevice& vkDev, VkRenderPass renderPass,
    VkImageView depthImageView,
    std::vector<VkFramebuffer>& swapchainFramebuffers);
} // namespace mental
