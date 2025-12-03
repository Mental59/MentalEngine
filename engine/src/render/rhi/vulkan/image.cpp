#include <render/rhi/vulkan/image.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/allocator.hpp>
#include <core/log.hpp>

mental::core::resource::Object mental::rhi::vk::Image::getNativeObject(core::resource::ObjectType objectType)
{
  switch (objectType)
  {
    case core::resource::ObjectType::eVkImage: return mImage;
    default: return nullptr;
  }
}

void mental::rhi::vk::Image::destroy()
{
  VkDevice device = vk::getDevice().getVirtualDevice();

  if (mShouldDestroyImage)
  {
    vmaDestroyImage(vk::getAllocator(), mImage, mAllocation);
  }
}

mental::core::Result mental::rhi::vk::Image::init(const ImageDesc& desc)
{
  MENTAL_ASSERT_DEBUG(desc.extent.width > 0 && desc.extent.height > 0 && desc.extent.depth > 0);

  VkImageCreateInfo imageInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
  imageInfo.imageType = desc.extent.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = desc.extent.width;
  imageInfo.extent.height = desc.extent.height;
  imageInfo.extent.depth = desc.extent.depth;
  imageInfo.mipLevels = desc.mipLevels;
  imageInfo.arrayLayers = desc.arrayLayers;
  imageInfo.format = convertImageFormat(desc.format);
  imageInfo.tiling = convertImageTiling(desc.tiling);
  imageInfo.initialLayout = convertImageLayout(desc.layout);
  imageInfo.usage = convertImageUsageFlags(desc.usage);
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

void mental::rhi::vk::Image::initSwapchainImage(const SwapchainImageDesc& desc)
{
  mShouldDestroyImage = false;
  mImage = desc.image;
  mDesc.extent = desc.extent;
  mDesc.format = desc.format;
  mDesc.layout = ImageLayout::eUndefined;
  mDesc.mipLevels = 1;
  mDesc.tiling = ImageTiling::eOptimal;
  mDesc.usage = desc.usage;
}

mental::core::Result mental::rhi::vk::ImageView::init(const mental::rhi::ImageViewDesc& desc)
{
  MENTAL_ASSERT_DEBUG(desc.image != nullptr);

  VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
  imageViewInfo.image = desc.image->getNativeObject(core::resource::ObjectType::eVkImage);
  imageViewInfo.viewType = desc.type == ImageViewType::eCubeMap ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
  imageViewInfo.format = convertImageFormat(desc.format.has_value() ? desc.format.value() : desc.image->getDesc().format);
  imageViewInfo.subresourceRange.baseMipLevel = 0;
  imageViewInfo.subresourceRange.baseArrayLayer = 0;
  imageViewInfo.subresourceRange.levelCount = desc.image->getDesc().mipLevels;
  imageViewInfo.subresourceRange.layerCount = desc.image->getDesc().arrayLayers;
  switch (desc.type)
  {
    case ImageViewType::eTexture:
    case ImageViewType::eCubeMap:
    {
      imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
      break;
    }
    case ImageViewType::eDepthMap:
    {
      imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
      break;
    }
    case ImageViewType::eDepthStencilMap:
    {
      imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
      break;
    }
  }
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

const mental::rhi::ImageViewDesc& mental::rhi::vk::ImageView::getDesc() const
{
  return mDesc;
}

void mental::rhi::vk::ImageView::destroy()
{
  VkDevice device = vk::getDevice().getVirtualDevice();
  vkDestroyImageView(device, mImageView, nullptr);
}
