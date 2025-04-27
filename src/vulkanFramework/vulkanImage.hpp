#pragma once

#include <volk.h>

namespace mental {
VkResult createImageView(VkDevice device, VkImage image, VkFormat format,
                         VkImageAspectFlags aspectFlags, VkImageView* imageView,
                         VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
                         uint32_t layerCount = 1, uint32_t mipLevels = 1);
}