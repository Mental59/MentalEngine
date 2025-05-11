#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace vkFramework {
bool createColorAndDepthFramebuffers(
    VulkanRenderDevice& vkDev, VkRenderPass renderPass,
    VkImageView depthImageView,
    std::vector<VkFramebuffer>& swapchainFramebuffers);
} // namespace vkFramework
