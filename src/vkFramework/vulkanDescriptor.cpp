#include "vulkanDescriptor.hpp"

bool vkFramework::createDescriptorPool(VulkanRenderDevice& vkDev,
                                       uint32_t uniformBufferCount,
                                       uint32_t storageBufferCount,
                                       uint32_t samplerCount,
                                       VkDescriptorPool* descriptorPool) {
  std::vector<VkDescriptorPoolSize> poolSizes;
  poolSizes.reserve(3);

  if (uniformBufferCount) {
    poolSizes.push_back(VkDescriptorPoolSize{
        .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = vkDev.maxFramesInFlight * uniformBufferCount});
  }

  if (storageBufferCount) {
    poolSizes.push_back(VkDescriptorPoolSize{
        .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = vkDev.maxFramesInFlight * storageBufferCount});
  }

  if (samplerCount) {
    poolSizes.push_back(VkDescriptorPoolSize{
        .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = vkDev.maxFramesInFlight * samplerCount});
  }

  const VkDescriptorPoolCreateInfo poolInfo = {
      .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .maxSets = vkDev.maxFramesInFlight,
      .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
      .pPoolSizes = poolSizes.empty() ? nullptr : poolSizes.data()};

  return (vkCreateDescriptorPool(vkDev.device, &poolInfo, nullptr,
                                 descriptorPool) == VK_SUCCESS);
}

VkDescriptorSetLayoutBinding vkFramework::descriptorSetLayoutBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t descriptorCount) {
  return VkDescriptorSetLayoutBinding{.binding = binding,
                                      .descriptorType = descriptorType,
                                      .descriptorCount = descriptorCount,
                                      .stageFlags = stageFlags,
                                      .pImmutableSamplers = nullptr};
}

VkWriteDescriptorSet vkFramework::bufferWriteDescriptorSet(
    VkDescriptorSet ds, const VkDescriptorBufferInfo* bi, uint32_t bindIdx,
    VkDescriptorType dType) {
  return VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                              .pNext = nullptr,
                              .dstSet = ds,
                              .dstBinding = bindIdx,
                              .dstArrayElement = 0,
                              .descriptorCount = 1,
                              .descriptorType = dType,
                              .pImageInfo = nullptr,
                              .pBufferInfo = bi,
                              .pTexelBufferView = nullptr};
}

VkWriteDescriptorSet vkFramework::imageWriteDescriptorSet(
    VkDescriptorSet ds, const VkDescriptorImageInfo* ii, uint32_t bindIdx) {
  return VkWriteDescriptorSet{.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                              .pNext = nullptr,
                              .dstSet = ds,
                              .dstBinding = bindIdx,
                              .dstArrayElement = 0,
                              .descriptorCount = 1,
                              .descriptorType =
                                  VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                              .pImageInfo = ii,
                              .pBufferInfo = nullptr,
                              .pTexelBufferView = nullptr};
}
