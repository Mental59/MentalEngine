#include "vulkanDescriptor.hpp"

bool mental::createDescriptorPool(VulkanRenderDevice& vkDev,
                                  uint32_t uniformBufferCount,
                                  uint32_t storageBufferCount,
                                  uint32_t samplerCount,
                                  VkDescriptorPool* descriptorPool) {
  const uint32_t imageCount =
      static_cast<uint32_t>(vkDev.swapchainImages.size());

  std::vector<VkDescriptorPoolSize> poolSizes;

  if (uniformBufferCount) {
    poolSizes.push_back(VkDescriptorPoolSize{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = imageCount * uniformBufferCount});
  }

  if (storageBufferCount) {
    poolSizes.push_back(VkDescriptorPoolSize{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = imageCount * storageBufferCount});
  }

  if (samplerCount) {
    poolSizes.push_back(
        VkDescriptorPoolSize{.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                             .descriptorCount = imageCount * samplerCount});
  }

  const VkDescriptorPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = static_cast<uint32_t>(imageCount),
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data()};

  return (vkCreateDescriptorPool(vkDev.device, &poolInfo, nullptr,
                                 descriptorPool) == VK_SUCCESS);
}

VkDescriptorSetLayoutBinding mental::descriptorSetLayoutBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t descriptorCount) {
  return VkDescriptorSetLayoutBinding{.binding = binding,
                                      .descriptorType = descriptorType,
                                      .descriptorCount = descriptorCount,
                                      .stageFlags = stageFlags,
                                      .pImmutableSamplers = nullptr};
}
