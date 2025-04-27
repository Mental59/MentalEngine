#include "vulkanImage.hpp"

VkResult mental::createImageView(VkDevice device, VkImage image,
                                 VkFormat format,
                                 VkImageAspectFlags aspectFlags,
                                 VkImageView* imageView,
                                 VkImageViewType viewType, uint32_t layerCount,
                                 uint32_t mipLevels) {
  const VkImageViewCreateInfo viewInfo = {
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .image = image,
      .viewType = viewType,
      .format = format,
      .subresourceRange = {.aspectMask = aspectFlags,
                           .baseMipLevel = 0,
                           .levelCount = mipLevels,
                           .baseArrayLayer = 0,
                           .layerCount = layerCount}};

  return vkCreateImageView(device, &viewInfo, nullptr, imageView);
}
