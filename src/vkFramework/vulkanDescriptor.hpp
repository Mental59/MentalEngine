#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace vkFramework {
bool createDescriptorPool(const VulkanRenderDevice& vkDev,
                          uint32_t uniformBufferCount,
                          uint32_t storageBufferCount, uint32_t samplerCount,
                          VkDescriptorPool* descriptorPool);
VkDescriptorSetLayoutBinding
descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType,
                           VkShaderStageFlags stageFlags,
                           uint32_t descriptorCount = 1);

VkWriteDescriptorSet bufferWriteDescriptorSet(VkDescriptorSet ds,
                                              const VkDescriptorBufferInfo* bi,
                                              uint32_t bindIdx,
                                              VkDescriptorType dType);

VkWriteDescriptorSet imageWriteDescriptorSet(VkDescriptorSet ds,
                                             const VkDescriptorImageInfo* ii,
                                             uint32_t bindIdx);

} // namespace vkFramework
