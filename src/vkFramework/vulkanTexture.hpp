#pragma once

#include "vulkanImage.hpp"
#include <volk.h>

namespace vkFramework {
struct VulkanTexture final {
  uint32_t width;
  uint32_t height;
  uint32_t depth;
  VkFormat format;

  VulkanImage image;
  VkSampler sampler;

  // Offscreen buffers require VK_IMAGE_LAYOUT_GENERAL && static textures have
  // VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
  VkImageLayout desiredLayout;
};

bool updateTextureImage(
    VulkanRenderDevice& vkDev, VkImage& textureImage,
    VkDeviceMemory& textureImageMemory, uint32_t texWidth, uint32_t texHeight,
    VkFormat texFormat, uint32_t layerCount, const void* imageData,
    VkImageLayout sourceImageLayout = VK_IMAGE_LAYOUT_UNDEFINED);

void uploadBufferData(VulkanRenderDevice& vkDev,
                      const VkDeviceMemory& bufferMemory,
                      VkDeviceSize deviceOffset, const void* data,
                      const size_t dataSize);

bool createTextureImageFromData(VulkanRenderDevice& vkDev,
                                VkImage& textureImage,
                                VkDeviceMemory& textureImageMemory,
                                void* imageData, uint32_t texWidth,
                                uint32_t texHeight, VkFormat texFormat,
                                uint32_t layerCount = 1,
                                VkImageCreateFlags flags = 0);

bool loadTextureFromFile(VulkanRenderDevice& vkDev, const char* filename,
                         VkImage& textureImage, VkFormat imageFormat,
                         VkDeviceMemory& textureImageMemory,
                         uint32_t* outTexWidth = nullptr,
                         uint32_t* outTexHeight = nullptr);

bool createVulkanImage(VulkanRenderDevice& vkDev, const char* filename,
                       VulkanImage& image);
} // namespace vkFramework