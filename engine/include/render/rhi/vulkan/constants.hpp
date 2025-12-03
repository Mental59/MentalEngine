#pragma once
#include <volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  const char* vkResultToString(VkResult result);
  VkFormat convertImageFormat(ImageFormat format);
  VkImageLayout convertImageLayout(ImageLayout layout);
  VkImageTiling convertImageTiling(ImageTiling tiling);
  VkImageUsageFlags convertImageUsageFlags(ImageUsageFlags usage);
}  // namespace mental::rhi::vk
