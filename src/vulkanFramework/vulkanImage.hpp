#pragma once

#include "vulkanRenderDevice.hpp"
#include <initializer_list>
#include <volk.h>

namespace mental {
struct VulkanImage final {
  VkImage image = VK_NULL_HANDLE;
  VkDeviceMemory imageMemory = VK_NULL_HANDLE;
  VkImageView imageView = VK_NULL_HANDLE;
};

VkResult createImageView(VkDevice device, VkImage image, VkFormat format,
                         VkImageAspectFlags aspectFlags, VkImageView* imageView,
                         VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,
                         uint32_t layerCount = 1, uint32_t mipLevels = 1);

bool createImage(VkDevice device, VkPhysicalDevice physicalDevice,
                 uint32_t width, uint32_t height, VkFormat format,
                 VkImageTiling tiling, VkImageUsageFlags usage,
                 VkMemoryPropertyFlags properties, VkImage& image,
                 VkDeviceMemory& imageMemory, VkImageCreateFlags flags = 0,
                 uint32_t mipLevels = 1);

bool createTextureSampler(VkDevice device, VkSampler* sampler);

void copyBufferToImage(VulkanRenderDevice& vkDev, VkBuffer buffer,
                       VkImage image, uint32_t width, uint32_t height,
                       uint32_t layerCount);

void destroyVulkanImage(VkDevice device, VulkanImage& image);

void transitionImageLayout(VulkanRenderDevice& vkDev, VkImage image,
                           VkFormat format, VkImageLayout oldLayout,
                           VkImageLayout newLayout, uint32_t layerCount = 1,
                           uint32_t mipLevels = 1);

void transitionImageLayoutCmd(VkCommandBuffer commandBuffer, VkImage image,
                              VkFormat format, VkImageLayout oldLayout,
                              VkImageLayout newLayout, uint32_t layerCount,
                              uint32_t mipLevels);

VkFormat findSupportedFormat(VkPhysicalDevice device,
                             const std::initializer_list<VkFormat>& candidates,
                             VkImageTiling tiling,
                             VkFormatFeatureFlags features);

VkFormat findDepthFormat(VkPhysicalDevice device);

bool createDepthResources(VulkanRenderDevice& vkDev, uint32_t width,
                          uint32_t height, VulkanImage& depth);
} // namespace mental