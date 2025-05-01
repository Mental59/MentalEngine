#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace mental {
uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties);

bool createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer& buffer,
                  VkDeviceMemory& bufferMemory);

void copyBuffer(VulkanRenderDevice& vkDev, VkBuffer srcBuffer,
                VkBuffer dstBuffer, VkDeviceSize size);
} // namespace mental