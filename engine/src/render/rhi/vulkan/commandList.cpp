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

mental::core::Result mental::rhi::vk::CommandList::copyBufferToImage(
    IBuffer* buffer,
    size_t bufferOffset,
    ITexture* texture,
    uint32_t mipLevel,
    const TextureOffset3D& textureOffset)
{
  MENTAL_ASSERT_DEBUG(buffer != nullptr);
  MENTAL_ASSERT_DEBUG(texture != nullptr);

  VkBuffer vkBuffer = buffer->getNativeObject(core::resource::ObjectType::eVkBuffer);
  MENTAL_ASSERT_DEBUG(vkBuffer != VK_NULL_HANDLE);

  VkImage vkImage = texture->getNativeObject(core::resource::ObjectType::eVkImage);
  MENTAL_ASSERT_DEBUG(vkImage != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(texture->getDesc().layout == rhi::TextureLayout::eTransferDst);

  VkBufferImageCopy region{};
  region.bufferOffset = bufferOffset;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = mipLevel;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = texture->getDesc().arrayLayers;

  region.imageOffset = { textureOffset.x, textureOffset.y, textureOffset.z };
  region.imageExtent = { texture->getDesc().extent.width,
                         texture->getDesc().extent.height,
                         texture->getDesc().extent.depth };

  vkCmdCopyBufferToImage(mCmdBuffer, vkBuffer, vkImage, convertTextureLayout(texture->getDesc().layout), 1, &region);

  return core::Result::eSuccess;
}
