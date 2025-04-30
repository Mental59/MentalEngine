#include "vulkanCommand.hpp"

VkResult mental::createCommandPool(VkDevice device, uint32_t queueFamilyIndex,
                                   VkCommandPool* commandPool) {
  const VkCommandPoolCreateInfo commanPoolCreateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
      .flags = 0,
      .queueFamilyIndex = queueFamilyIndex};

  return vkCreateCommandPool(device, &commanPoolCreateInfo, nullptr,
                             commandPool);
}

VkResult mental::allocateCommandBuffers(VkDevice device,
                                        VkCommandPool commandPool,
                                        uint32_t commandBufferCount,
                                        VkCommandBuffer* commandBuffers) {
  const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = commandBufferCount,
  };

  return vkAllocateCommandBuffers(device, &commandBufferAllocateInfo,
                                  commandBuffers);
}
