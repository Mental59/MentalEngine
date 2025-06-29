#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace vkFramework {
VkResult createCommandPool(VkDevice device, uint32_t queueFamilyIndex,
                           VkCommandPool* commandPool);

VkResult allocateCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                uint32_t commandBufferCount,
                                VkCommandBuffer* commandBuffers);

VkCommandBuffer beginSingleTimeCommands(const VulkanRenderDevice& vkDev);

void endSingleTimeCommands(const VulkanRenderDevice& vkDev,
                           VkCommandBuffer commandBuffer);
} // namespace vkFramework