#pragma once

#include "vulkanRenderDevice.hpp"
#include <volk.h>

namespace mental {
bool createDescriptorPool(VulkanRenderDevice& vkDev,
                          uint32_t uniformBufferCount,
                          uint32_t storageBufferCount, uint32_t samplerCount,
                          VkDescriptorPool* descriptorPool);
inline VkDescriptorSetLayoutBinding
descriptorSetLayoutBinding(uint32_t binding, VkDescriptorType descriptorType,
                           VkShaderStageFlags stageFlags,
                           uint32_t descriptorCount = 1);
} // namespace mental
