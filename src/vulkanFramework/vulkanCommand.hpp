#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace mental {
VkResult createCommandPool(VkDevice device, uint32_t queueFamilyIndex,
                           VkCommandPool* commandPool);

VkResult allocateCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                uint32_t commandBufferCount,
                                VkCommandBuffer* commandBuffers);

VkCommandBuffer beginSingleTimeCommands(VulkanRenderDevice& vkDev);

void endSingleTimeCommands(VulkanRenderDevice& vkDev,
                           VkCommandBuffer commandBuffer);
} // namespace mental