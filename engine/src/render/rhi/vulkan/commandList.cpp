#include <render/rhi/vulkan/commandList.hpp>
#include <render/rhi/rhi.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <vector>
#include <array>
#include "core/resource.hpp"

mental::core::Result mental::rhi::vk::CommandList::init(const mental::rhi::CommandListDesc& desc)
{
  if (mIsInit)
  {
    MENTAL_INFO("Trying to initialize an already initialized vk::CommandList");
    return core::Result::eInitializationFailed;
  }

  MENTAL_ASSERT_DEBUG(desc.commandQueue != nullptr);

  VkCommandPool cmdPool = desc.commandQueue->getNativeObject(core::resource::ObjectType::eVkCommandPool);
  VkCommandBufferAllocateInfo allocInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
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
  mIsInit = true;

  return core::Result::eSuccess;
}

void mental::rhi::vk::CommandList::destroy()
{
  if (!mIsInit)
  {
    MENTAL_INFO("Trying to destroy uninitialized vk::CommandList");
    return;
  }

  vkFreeCommandBuffers(vk::getDevice().getVirtualDevice(), mCmdPool, 1, &mCmdBuffer);

  mCmdBuffer = VK_NULL_HANDLE;
  mCmdPool = VK_NULL_HANDLE;
  mIsInit = false;
}

mental::core::resource::Object mental::rhi::vk::CommandList::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkCommandBuffer:
      return mCmdBuffer;
    case core::resource::ObjectType::eVkCommandPool:
      return mCmdPool;
    default:
      return nullptr;
  }
}

bool mental::rhi::vk::CommandList::isValid() const
{
  return mIsInit;
}

mental::core::Result mental::rhi::vk::CommandList::begin(const CommandListBegindDesc& desc)
{
  VkResult res = vkResetCommandBuffer(mCmdBuffer, 0);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkResetCommandBuffer, error: {}", vkResultToString(res));
    return core::Result::eOperationFailed;
  }

  VkCommandBufferBeginInfo beginInfo {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
  if (desc.isOneTimeSubmit)
  {
    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  }

  res = vkBeginCommandBuffer(mCmdBuffer, &beginInfo);
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
  IBuffer* srcBuffer, size_t srcOffset, IBuffer* dstBuffer, size_t dstOffset, size_t size)
{
  MENTAL_ASSERT_DEBUG(srcBuffer != nullptr);
  MENTAL_ASSERT_DEBUG(dstBuffer != nullptr);

  VkBuffer vkSrcBuffer = srcBuffer->getNativeObject(core::resource::ObjectType::eVkBuffer);
  VkBuffer vkDstBuffer = dstBuffer->getNativeObject(core::resource::ObjectType::eVkBuffer);

  MENTAL_ASSERT_DEBUG(vkSrcBuffer != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(vkDstBuffer != VK_NULL_HANDLE);

  VkBufferCopy copyRegion = {.srcOffset = srcOffset, .dstOffset = dstOffset, .size = size};
  vkCmdCopyBuffer(mCmdBuffer, vkSrcBuffer, vkDstBuffer, 1, &copyRegion);

  return core::Result::eSuccess;
}

mental::core::Result mental::rhi::vk::CommandList::copyBufferToImage(
  IBuffer* buffer, size_t bufferOffset, ITexture* texture, uint32_t mipLevel, const TextureOffset3D& textureOffset)
{
  MENTAL_ASSERT_DEBUG(buffer != nullptr);
  MENTAL_ASSERT_DEBUG(texture != nullptr);

  VkBuffer vkBuffer = buffer->getNativeObject(core::resource::ObjectType::eVkBuffer);
  MENTAL_ASSERT_DEBUG(vkBuffer != VK_NULL_HANDLE);

  VkImage vkImage = texture->getNativeObject(core::resource::ObjectType::eVkImage);
  MENTAL_ASSERT_DEBUG(vkImage != VK_NULL_HANDLE);
  MENTAL_ASSERT_DEBUG(texture->getDesc().layout == rhi::TextureLayout::eTransferDst);

  VkBufferImageCopy region {};
  region.bufferOffset = bufferOffset;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;

  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = mipLevel;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = texture->getDesc().arrayLayers;

  region.imageOffset = {textureOffset.x, textureOffset.y, textureOffset.z};
  region.imageExtent = {
    texture->getDesc().extent.width, texture->getDesc().extent.height, texture->getDesc().extent.depth};

  vkCmdCopyBufferToImage(mCmdBuffer, vkBuffer, vkImage, convertTextureLayout(texture->getDesc().layout), 1, &region);

  return core::Result::eSuccess;
}

mental::core::Result mental::rhi::vk::CommandList::transitionTexture(const TextureTransitionInfo& info)
{
  MENTAL_ASSERT_DEBUG(info.texture != nullptr);

  VkImage vkImage = info.texture->getNativeObject(core::resource::ObjectType::eVkImage);
  MENTAL_ASSERT_DEBUG(vkImage != VK_NULL_HANDLE);

  VkImageMemoryBarrier imageBarrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
  imageBarrier.oldLayout = convertTextureLayout(info.oldLayout);
  imageBarrier.newLayout = convertTextureLayout(info.newLayout);
  imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  imageBarrier.image = vkImage;
  imageBarrier.subresourceRange.aspectMask = getTextureAspectFlags(info.texture->getDesc().format);
  imageBarrier.subresourceRange.baseMipLevel = 0;
  imageBarrier.subresourceRange.levelCount = info.texture->getDesc().mipLevels;
  imageBarrier.subresourceRange.baseArrayLayer = 0;
  imageBarrier.subresourceRange.layerCount = info.texture->getDesc().arrayLayers;

  VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
  switch (info.oldLayout)
  {
    case TextureLayout::eUndefined:
      imageBarrier.srcAccessMask = 0;
      srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      break;
    case TextureLayout::ePresent:
      imageBarrier.srcAccessMask = 0;
      srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      break;
    case TextureLayout::eColorAttachment:
      imageBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      break;
    case TextureLayout::eTransferDst:
      imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case TextureLayout::eTransferSrc:
      imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case TextureLayout::eShaderReadOnly:
      imageBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
      srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      break;
    case TextureLayout::eDepthStencilAttachment:
      imageBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      break;
  }

  switch (info.newLayout)
  {
    case TextureLayout::eColorAttachment:
      imageBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
      dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      break;
    case TextureLayout::ePresent:
      imageBarrier.dstAccessMask = 0;
      dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
      break;
    case TextureLayout::eTransferDst:
      imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case TextureLayout::eTransferSrc:
      imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
      break;
    case TextureLayout::eShaderReadOnly:
      imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
      dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
      break;
    case TextureLayout::eDepthStencilAttachment:
      imageBarrier.dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
      dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
      break;
    case TextureLayout::eUndefined:
      imageBarrier.dstAccessMask = 0;
      dstStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
      break;
  }

  vkCmdPipelineBarrier(mCmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageBarrier);

  info.texture->setLayout(info.newLayout);
  return core::Result::eSuccess;
}

void mental::rhi::vk::CommandList::beginRendering(CommandListBeginRenderingInfo& info)
{
  MENTAL_ASSERT_DEBUG(info.swapchainImageView != nullptr);

  VkImageView swapchainImageView = info.swapchainImageView->getNativeObject(core::resource::ObjectType::eVkImageView);
  MENTAL_ASSERT_DEBUG(swapchainImageView != VK_NULL_HANDLE);

  VkClearValue clearColorValue {};
  clearColorValue.color.float32[0] = info.clearValue.color[0];
  clearColorValue.color.float32[1] = info.clearValue.color[1];
  clearColorValue.color.float32[2] = info.clearValue.color[2];
  clearColorValue.color.float32[3] = info.clearValue.color[3];

  VkRenderingAttachmentInfoKHR colorAttachmentInfo {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
  colorAttachmentInfo.imageView = swapchainImageView;
  colorAttachmentInfo.imageLayout = convertTextureLayout(TextureLayout::eColorAttachment);
  colorAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  colorAttachmentInfo.clearValue = clearColorValue;

  VkRenderingAttachmentInfoKHR depthAttachmentInfo {VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR};
  if (info.depthAttachmentView != nullptr)
  {
    VkImageView depthImageView = info.depthAttachmentView->getNativeObject(core::resource::ObjectType::eVkImageView);
    MENTAL_ASSERT_DEBUG(depthImageView != VK_NULL_HANDLE);

    VkClearValue clearDepthValue {};
    clearDepthValue.depthStencil.depth = info.clearValue.depth;
    clearDepthValue.depthStencil.stencil = info.clearValue.stencil;

    depthAttachmentInfo.imageView = depthImageView;
    depthAttachmentInfo.imageLayout = convertTextureLayout(TextureLayout::eDepthStencilAttachment);
    depthAttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachmentInfo.clearValue = clearDepthValue;
  }

  VkRect2D renderArea {};
  renderArea.offset.x = renderArea.offset.y = 0;
  renderArea.extent.width = info.renderArea.width;
  renderArea.extent.height = info.renderArea.height;

  VkRenderingInfoKHR renderInfo {VK_STRUCTURE_TYPE_RENDERING_INFO_KHR};
  renderInfo.renderArea = renderArea;
  renderInfo.layerCount = 1;
  renderInfo.colorAttachmentCount = 1;
  renderInfo.pColorAttachments = &colorAttachmentInfo;
  renderInfo.pDepthAttachment = info.depthAttachmentView != nullptr ? &depthAttachmentInfo : nullptr;

  vkCmdBeginRenderingKHR(mCmdBuffer, &renderInfo);

  VkViewport viewport {};
  viewport.x = 0.0f;
  viewport.y = 0.0f;
  viewport.width = static_cast<float>(info.renderArea.width);
  viewport.height = static_cast<float>(info.renderArea.height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(mCmdBuffer, 0, 1, &viewport);

  VkRect2D scissor {};
  scissor.extent = renderArea.extent;
  vkCmdSetScissor(mCmdBuffer, 0, 1, &scissor);
}

void mental::rhi::vk::CommandList::endRendering()
{
  vkCmdEndRenderingKHR(mCmdBuffer);
}

void mental::rhi::vk::CommandList::bindGraphicsPipeline(IGraphicsPipeline* pipeline)
{
  MENTAL_ASSERT_DEBUG(pipeline != nullptr);
  const VkPipeline vkPipeline = pipeline->getNativeObject(core::resource::ObjectType::eVkPipeline);
  MENTAL_ASSERT_DEBUG(vkPipeline != VK_NULL_HANDLE);
  vkCmdBindPipeline(mCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);
}

void mental::rhi::vk::CommandList::bindResourceSets(
  IGraphicsPipeline* graphicsPipeline, uint32_t firstSet, IResourceSet* const* resourceSets, uint32_t resourceSetCount)
{
  constexpr uint32_t kMaxResourceSets = 32;

  MENTAL_ASSERT(resourceSetCount <= kMaxResourceSets);
  MENTAL_ASSERT_DEBUG(graphicsPipeline != nullptr);
  MENTAL_ASSERT_DEBUG(resourceSets != nullptr);

  const VkPipelineLayout vkPipelineLayout = graphicsPipeline->getPipelineLayoutNativeObject();
  MENTAL_ASSERT_DEBUG(vkPipelineLayout != VK_NULL_HANDLE);

  std::array<VkDescriptorSet, kMaxResourceSets> vkDescriptorSets;
  for (uint32_t resourceSetIndex = 0; resourceSetIndex < resourceSetCount; ++resourceSetIndex)
  {
    MENTAL_ASSERT_DEBUG(resourceSets[resourceSetIndex] != nullptr);
    vkDescriptorSets[resourceSetIndex] =
      resourceSets[resourceSetIndex]->getNativeObject(core::resource::ObjectType::eVkDescriptorSet);
  }

  vkCmdBindDescriptorSets(mCmdBuffer,
    VK_PIPELINE_BIND_POINT_GRAPHICS,
    vkPipelineLayout,
    firstSet,
    resourceSetCount,
    vkDescriptorSets.data(),
    0,
    nullptr);
}

mental::core::Result mental::rhi::vk::CommandList::pushConstants(
  IGraphicsPipeline* graphicsPipeline, const PushConstantRangeDesc& range, const void* data)
{
  if (graphicsPipeline == nullptr || data == nullptr || range.size == 0u)
  {
    MENTAL_ERROR("Push constants require a graphics pipeline, payload, and non-zero size");
    return core::Result::eOperationFailed;
  }

  const VkPipelineLayout vkPipelineLayout = graphicsPipeline->getPipelineLayoutNativeObject();
  MENTAL_ASSERT_DEBUG(vkPipelineLayout != VK_NULL_HANDLE);
  vkCmdPushConstants(
    mCmdBuffer, vkPipelineLayout, convertShaderStageFlags(range.stageFlags), range.offset, range.size, data);
  return core::Result::eSuccess;
}

void mental::rhi::vk::CommandList::draw(
  uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
  vkCmdDraw(mCmdBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}
