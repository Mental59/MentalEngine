#include <render/rhi/vulkan/commandList.hpp>
#include <render/rhi/rhi.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include "core/resource.hpp"

mental::core::Result mental::rhi::vk::CommandList::init(const mental::rhi::CommandListDesc& desc)
{
  MENTAL_ASSERT_DEBUG(desc.commandQueue != nullptr);

  VkCommandPool cmdPool = desc.commandQueue->getNativeObject(core::resource::ObjectType::eVkCommandPool);
  VkCommandBufferAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = cmdPool;
  allocInfo.commandBufferCount = 1;

  VkResult res = vkAllocateCommandBuffers(vk::getDevice().getVirtualDevice(), &allocInfo, &mCmdBuffer);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkAllocateCommandBuffers, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mCmdPool = cmdPool;

  return core::Result::eSuccess;
}

void mental::rhi::vk::CommandList::destroy()
{
  vkFreeCommandBuffers(vk::getDevice().getVirtualDevice(), mCmdPool, 1, &mCmdBuffer);
}

mental::core::Result mental::rhi::vk::CommandList::begin(const CommandListBegindDesc& desc)
{
  VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
  if (desc.isOneTimeSubmit)
  {
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }

  VkResult res = vkBeginCommandBuffer(mCmdBuffer, &beginInfo);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkBeginCommandBuffer, error: {}", vkResultToString(res));
    return core::Result::eOperationFailed;
  }

  return core::Result::eSuccess;
}

mental::core::Result mental::rhi::vk::CommandList::end()
{
  VkResult res = vkEndCommandBuffer(mCmdBuffer);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkEndCommandBuffer, error: {}", vkResultToString(res));
    return core::Result::eOperationFailed;
  }

  return core::Result::eSuccess;
}

mental::core::Result mental::rhi::vk::CommandList::copyBuffer(
    IBuffer* srcBuffer,
    size_t srcOffset,
    IBuffer* dstBuffer,
    size_t dstOffset,
    size_t size)
{
  MENTAL_ASSERT_DEBUG(srcBuffer != nullptr);
  MENTAL_ASSERT_DEBUG(dstBuffer != nullptr);

  VkBuffer vkSrcBuffer = srcBuffer->getNativeObject(core::resource::ObjectType::eVkBuffer);
  VkBuffer vkDstBuffer = dstBuffer->getNativeObject(core::resource::ObjectType::eVkBuffer);

  MENTAL_ASSERT_DEBUG(vkSrcBuffer != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(vkDstBuffer != VK_NULL_HANDLE);

  VkBufferCopy copyRegion = { .srcOffset = srcOffset, .dstOffset = dstOffset, .size = size };
  vkCmdCopyBuffer(mCmdBuffer, vkSrcBuffer, vkDstBuffer, 1, &copyRegion);

  return core::Result::eSuccess;
}
