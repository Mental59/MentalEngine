#include <render/rhi/vulkan/texture.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/allocator.hpp>
#include <core/log.hpp>

mental::core::resource::Object mental::rhi::vk::Texture::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkImage: return mImage;
    default: return nullptr;
  }
}

void mental::rhi::vk::Texture::destroy()
{
  VkDevice device = vk::getDevice().getVirtualDevice();

  if (mShouldDestroyImage)
  {
    vmaDestroyImage(vk::getAllocator(), mImage, mAllocation);
  }
}

mental::core::Result mental::rhi::vk::Texture::init(const TextureDesc& desc)
{
  MENTAL_ASSERT_DEBUG(desc.extent.width > 0 && desc.extent.height > 0 && desc.extent.depth > 0);

  VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  imageInfo.imageType = desc.extent.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = desc.extent.width;
  imageInfo.extent.height = desc.extent.height;
  imageInfo.extent.depth = desc.extent.depth;
  imageInfo.mipLevels = desc.mipLevels;
  imageInfo.arrayLayers = desc.arrayLayers;
  imageInfo.format = convertTextureFormat(desc.format);
  imageInfo.tiling = convertTextureTiling(desc.tiling);
  imageInfo.initialLayout = convertTextureLayout(desc.layout);
  imageInfo.usage = convertTextureUsageFlags(desc.usage);
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  if (desc.cubeCompatible)
    imageInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

  VmaAllocationCreateInfo allocationCreateInfo{};
  allocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

  VkResult res = vmaCreateImage(vk::getAllocator(), &imageInfo, &allocationCreateInfo, &mImage, &mAllocation, nullptr);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vmaCreateImage, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mDesc = desc;
  mShouldDestroyImage = true;

  return core::Result::eSuccess;
}

void mental::rhi::vk::Texture::initSwapchainTexture(const SwapchainTextureDesc& desc)
{
  mShouldDestroyImage = false;
  mImage = desc.image;
  mDesc.extent = desc.extent;
  mDesc.format = desc.format;
  mDesc.layout = TextureLayout::eUndefined;
  mDesc.mipLevels = 1;
  mDesc.tiling = TextureTiling::eOptimal;
  mDesc.usage = desc.usage;
  mDesc.arrayLayers = 1;
  mDesc.cubeCompatible = false;
}

mental::core::resource::Object mental::rhi::vk::TextureView::getNativeObject(mental::core::resource::ObjectType objectType)
{
  if (objectType == core::resource::ObjectType::eVkImageView)
  {
    return mImageView;
  }
  return nullptr;
}

mental::core::Result mental::rhi::vk::TextureView::init(const mental::rhi::TextureViewDesc& desc)
{
  MENTAL_ASSERT_DEBUG(desc.texture != nullptr);

  VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  imageViewInfo.image = desc.texture->getNativeObject(core::resource::ObjectType::eVkImage);
  imageViewInfo.viewType = desc.type == TextureType::eCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
  imageViewInfo.format =
      convertTextureFormat(desc.format.has_value() ? desc.format.value() : desc.texture->getDesc().format);
  imageViewInfo.subresourceRange.baseMipLevel = 0;
  imageViewInfo.subresourceRange.baseArrayLayer = 0;
  imageViewInfo.subresourceRange.levelCount = desc.texture->getDesc().mipLevels;
  imageViewInfo.subresourceRange.layerCount = desc.texture->getDesc().arrayLayers;
  imageViewInfo.subresourceRange.aspectMask = getTextureAspectFlags(desc.type);
  MENTAL_ASSERT_DEBUG(imageViewInfo.image != VK_NULL_HANDLE);

  VkDevice device = vk::getDevice().getVirtualDevice();
  VkResult res = vkCreateImageView(device, &imageViewInfo, nullptr, &mImageView);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateImageView, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mDesc = desc;
  return core::Result::eSuccess;
}

const mental::rhi::TextureViewDesc& mental::rhi::vk::TextureView::getDesc() const
{
  return mDesc;
}

void mental::rhi::vk::TextureView::destroy()
{
  VkDevice device = vk::getDevice().getVirtualDevice();
  vkDestroyImageView(device, mImageView, nullptr);
}
