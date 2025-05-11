#pragma once

#include "vulkanRenderDevice.hpp"
#include <glm/glm.hpp>
#include <volk.h>

namespace vkFramework {
struct VertexData {
  glm::vec3 pos;
  glm::vec2 tc;
};

uint32_t findMemoryType(VkPhysicalDevice device, uint32_t typeFilter,
                        VkMemoryPropertyFlags properties);

bool createBuffer(VkDevice device, VkPhysicalDevice physicalDevice,
                  VkDeviceSize size, VkBufferUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkBuffer& buffer,
                  VkDeviceMemory& bufferMemory);

void copyBuffer(VulkanRenderDevice& vkDev, VkBuffer srcBuffer,
                VkBuffer dstBuffer, VkDeviceSize size);

bool createTexturedVertexBuffer(VulkanRenderDevice& vkDev, const char* filename,
                                VkBuffer* storageBuffer,
                                VkDeviceMemory* storageBufferMemory,
                                size_t* vertexBufferSize,
                                size_t* indexBufferSize);

size_t allocateVertexBuffer(VulkanRenderDevice& vkDev, VkBuffer* storageBuffer,
                            VkDeviceMemory* storageBufferMemory,
                            size_t vertexDataSize, const void* vertexData,
                            size_t indexDataSize, const void* indexData);
} // namespace vkFramework