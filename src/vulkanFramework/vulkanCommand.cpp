#include "vulkanCommand.hpp"
#include "vulkanRenderDevice.hpp"

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

VkCommandBuffer mental::beginSingleTimeCommands(VulkanRenderDevice& vkDev) {
  const VkCommandBufferAllocateInfo allocInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .pNext = nullptr,
      .commandPool = vkDev.commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1};

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(vkDev.device, &allocInfo, &commandBuffer);

  const VkCommandBufferBeginInfo beginInfo = {
      .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .pNext = nullptr,
      .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
      .pInheritanceInfo = nullptr};

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

void mental::endSingleTimeCommands(VulkanRenderDevice& vkDev,
                                   VkCommandBuffer commandBuffer) {
  vkEndCommandBuffer(commandBuffer);

  const VkSubmitInfo submitInfo = {.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
                                   .pNext = nullptr,
                                   .waitSemaphoreCount = 0,
                                   .pWaitSemaphores = nullptr,
                                   .pWaitDstStageMask = nullptr,
                                   .commandBufferCount = 1,
                                   .pCommandBuffers = &commandBuffer,
                                   .signalSemaphoreCount = 0,
                                   .pSignalSemaphores = nullptr};

  vkQueueSubmit(vkDev.graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(vkDev.graphicsQueue);

  vkFreeCommandBuffers(vkDev.device, vkDev.commandPool, 1, &commandBuffer);
}
