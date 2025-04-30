#pragma once

#include <volk.h>

namespace mental {
VkResult createCommandPool(VkDevice device, uint32_t queueFamilyIndex,
                           VkCommandPool* commandPool);

VkResult allocateCommandBuffers(VkDevice device, VkCommandPool commandPool,
                                uint32_t commandBufferCount,
                                VkCommandBuffer* commandBuffers);
} // namespace mental