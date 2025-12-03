#include <render/rhi/vulkan/constants.hpp>
#include <cstdio>

const char* mental::rhi::vk::vkResultToString(VkResult result)
{
  switch (result)
  {
    case VK_SUCCESS: return "VK_SUCCESS";
    case VK_NOT_READY: return "VK_NOT_READY";
    case VK_TIMEOUT: return "VK_TIMEOUT";
    case VK_EVENT_SET: return "VK_EVENT_SET";
    case VK_EVENT_RESET: return "VK_EVENT_RESET";
    case VK_INCOMPLETE: return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY: return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY: return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED: return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST: return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED: return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT: return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT: return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT: return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER: return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS: return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED: return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL: return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN: return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY: return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE: return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION: return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS: return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR: return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR: return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR: return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT: return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV: return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
      return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT: return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT: return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR: return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR: return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR: return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR: return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_PIPELINE_COMPILE_REQUIRED_EXT: return "VK_PIPELINE_COMPILE_REQUIRED_EXT";

    default:
    {
      static char buf[24];
      snprintf(buf, sizeof(buf), "Unknown (%d)", result);
      return buf;
    }
  }
}

VkFormat mental::rhi::vk::convertImageFormat(mental::rhi::ImageFormat format)
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

VkImageLayout mental::rhi::vk::convertImageLayout(mental::rhi::ImageLayout layout)
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

VkImageTiling mental::rhi::vk::convertImageTiling(mental::rhi::ImageTiling tiling)
{
  switch (tiling)
  {
    case ImageTiling::eLinear: return VK_IMAGE_TILING_LINEAR;
    case ImageTiling::eOptimal: return VK_IMAGE_TILING_OPTIMAL;
  }
}

VkImageUsageFlags mental::rhi::vk::convertImageUsageFlags(mental::rhi::ImageUsageFlags usage)
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
