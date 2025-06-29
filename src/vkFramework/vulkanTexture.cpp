#include "vulkanTexture.hpp"
#include "vulkanImage.hpp"
#include "vulkanMemory.hpp"
#include "vulkanUtils.hpp"
#include <stb_image.h>
#include <volk.h>

bool vkFramework::updateTextureImage(const VulkanRenderDevice& vkDev,
                                     VkImage& textureImage,
                                     VkDeviceMemory& textureImageMemory,
                                     uint32_t texWidth, uint32_t texHeight,
                                     VkFormat texFormat, uint32_t layerCount,
                                     const void* imageData,
                                     VkImageLayout sourceImageLayout) {
  uint32_t bytesPerPixel = bytesPerTexFormat(texFormat);

  VkDeviceSize layerSize = texWidth * texHeight * bytesPerPixel;
  VkDeviceSize imageSize = layerSize * layerCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;
  if (!createBuffer(vkDev.device, vkDev.physicalDevice, imageSize,
                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    stagingBuffer, stagingBufferMemory)) {
    return false;
  }

  uploadBufferData(vkDev, stagingBufferMemory, 0, imageData, imageSize);

  transitionImageLayout(vkDev, textureImage, texFormat, sourceImageLayout,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, layerCount);
  copyBufferToImage(vkDev, stagingBuffer, textureImage,
                    static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight), layerCount);
  transitionImageLayout(vkDev, textureImage, texFormat,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, layerCount);

  vkDestroyBuffer(vkDev.device, stagingBuffer, nullptr);
  vkFreeMemory(vkDev.device, stagingBufferMemory, nullptr);

  return true;
}

void vkFramework::uploadBufferData(const VulkanRenderDevice& vkDev,
                                   const VkDeviceMemory& bufferMemory,
                                   VkDeviceSize deviceOffset, const void* data,
                                   const size_t dataSize) {
  void* mappedData = nullptr;
  vkMapMemory(vkDev.device, bufferMemory, deviceOffset, dataSize, 0,
              &mappedData);
  memcpy(mappedData, data, dataSize);
  vkUnmapMemory(vkDev.device, bufferMemory);
}

bool vkFramework::createTextureImageFromData(
    const VulkanRenderDevice& vkDev, VkImage& textureImage,
    VkDeviceMemory& textureImageMemory, void* imageData, uint32_t texWidth,
    uint32_t texHeight, VkFormat texFormat, uint32_t layerCount,
    VkImageCreateFlags flags) {
  if (!createImage(vkDev.device, vkDev.physicalDevice, texWidth, texHeight,
                   texFormat, VK_IMAGE_TILING_OPTIMAL,
                   VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                   VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage,
                   textureImageMemory, flags)) {
    return false;
  }

  return updateTextureImage(vkDev, textureImage, textureImageMemory, texWidth,
                            texHeight, texFormat, layerCount, imageData);
}

bool vkFramework::loadTextureFromFile(const VulkanRenderDevice& vkDev,
                                      const char* filename,
                                      VkImage& textureImage,
                                      VkFormat imageFormat,
                                      VkDeviceMemory& textureImageMemory,
                                      uint32_t* outTexWidth,
                                      uint32_t* outTexHeight) {
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels =
      stbi_load(filename, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

  if (!pixels) {
    return false;
  }

  bool result =
      createTextureImageFromData(vkDev, textureImage, textureImageMemory,
                                 pixels, texWidth, texHeight, imageFormat);

  stbi_image_free(pixels);

  if (outTexWidth && outTexHeight) {
    *outTexWidth = (uint32_t)texWidth;
    *outTexHeight = (uint32_t)texHeight;
  }

  return result;
}

bool vkFramework::createVulkanImage(const VulkanRenderDevice& vkDev,
                                    const char* filename, VulkanImage& image) {
  VkFormat imageFormat = VK_FORMAT_R8G8B8A8_UNORM;

  if (!vkFramework::loadTextureFromFile(vkDev, filename, image.image,
                                        imageFormat, image.imageMemory,
                                        &image.width, &image.height)) {
    return false;
  }

  if (vkFramework::createImageView(vkDev.device, image.image, imageFormat,
                                   VK_IMAGE_ASPECT_COLOR_BIT,
                                   &image.imageView) != VK_SUCCESS) {
    return false;
  }

  bool isSamplerCreated =
      vkFramework::createTextureSampler(vkDev.device, &image.sampler);

  if (!isSamplerCreated) {
    destroyVulkanImage(vkDev.device, &image);
    return false;
  }

  return true;
}
