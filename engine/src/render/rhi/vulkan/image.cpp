#include <render/rhi/vulkan/image.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/constants.hpp>
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
    vkDestroyImage(device, mImage, nullptr);
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
  imageInfo.arrayLayers = 1;
  imageInfo.format = convertFormat(desc.format);
  imageInfo.tiling = convertTiling(desc.tiling);
  imageInfo.initialLayout = convertLayout(desc.layout);
  imageInfo.usage = convertUsageFlags(desc.usage);
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkDevice device = vk::getDevice().getVirtualDevice();
  VkResult res = vkCreateImage(device, &imageInfo, nullptr, &mImage);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateImage, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  mDesc = desc;
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

VkFormat mental::rhi::vk::Image::convertFormat(ImageFormat format)
{
  switch (format)
  {
    case ImageFormat::eRGBA32_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
    case ImageFormat::eBGRA32_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
    case ImageFormat::eRGBA32_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
    case ImageFormat::eBGRA32_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
    case ImageFormat::eD32_SFLOAT: return VK_FORMAT_D32_SFLOAT;
    case ImageFormat::eD32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case ImageFormat::eD24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
  }
}

VkImageLayout mental::rhi::vk::Image::convertLayout(ImageLayout layout)
{
  switch (layout)
  {
    case ImageLayout::eUndefined: return VK_IMAGE_LAYOUT_UNDEFINED;
    case ImageLayout::ePresent: return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case ImageLayout::eColorAttachment: return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case ImageLayout::eDepthStencilAttachment: return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case ImageLayout::eTransferSrc: return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case ImageLayout::eTransferDst: return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case ImageLayout::eShaderReadOnly: return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }
}

VkImageTiling mental::rhi::vk::Image::convertTiling(ImageTiling tiling)
{
  switch (tiling)
  {
    case ImageTiling::eLinear: return VK_IMAGE_TILING_LINEAR;
    case ImageTiling::eOptimal: return VK_IMAGE_TILING_OPTIMAL;
  }
}

VkImageUsageFlags mental::rhi::vk::Image::convertUsageFlags(ImageUsageFlags usage)
{
  VkImageUsageFlags flags = 0;

  if (usage & ImageUsageFlagBits::eImageUsageTransferSrcBit)
  {
    flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  if (usage & ImageUsageFlagBits::eImageUsageTransferDstBit)
  {
    flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  if (usage & ImageUsageFlagBits::eImageUsageSampledBit)
  {
    flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  if (usage & ImageUsageFlagBits::eImageUsageStorageBit)
  {
    flags |= VK_IMAGE_USAGE_STORAGE_BIT;
  }

  if (usage & ImageUsageFlagBits::eImageUsageColorAttachmentBit)
  {
    flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }

  if (usage & ImageUsageFlagBits::eImageUsageDepthStencilAttachmentBit)
  {
    flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  return flags;
}

void mental::rhi::vk::ImageView::destroy()
{
  VkDevice device = vk::getDevice().getVirtualDevice();
  vkDestroyImageView(device, mImageView, nullptr);
}
