#include <render/rhi/vulkan/constants.hpp>
#include <cstdio>

const char* mental::rhi::vk::vkResultToString(VkResult result)
{
  switch (result)
  {
    case VK_SUCCESS:
      return "VK_SUCCESS";
    case VK_NOT_READY:
      return "VK_NOT_READY";
    case VK_TIMEOUT:
      return "VK_TIMEOUT";
    case VK_EVENT_SET:
      return "VK_EVENT_SET";
    case VK_EVENT_RESET:
      return "VK_EVENT_RESET";
    case VK_INCOMPLETE:
      return "VK_INCOMPLETE";
    case VK_ERROR_OUT_OF_HOST_MEMORY:
      return "VK_ERROR_OUT_OF_HOST_MEMORY";
    case VK_ERROR_OUT_OF_DEVICE_MEMORY:
      return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
    case VK_ERROR_INITIALIZATION_FAILED:
      return "VK_ERROR_INITIALIZATION_FAILED";
    case VK_ERROR_DEVICE_LOST:
      return "VK_ERROR_DEVICE_LOST";
    case VK_ERROR_MEMORY_MAP_FAILED:
      return "VK_ERROR_MEMORY_MAP_FAILED";
    case VK_ERROR_LAYER_NOT_PRESENT:
      return "VK_ERROR_LAYER_NOT_PRESENT";
    case VK_ERROR_EXTENSION_NOT_PRESENT:
      return "VK_ERROR_EXTENSION_NOT_PRESENT";
    case VK_ERROR_FEATURE_NOT_PRESENT:
      return "VK_ERROR_FEATURE_NOT_PRESENT";
    case VK_ERROR_INCOMPATIBLE_DRIVER:
      return "VK_ERROR_INCOMPATIBLE_DRIVER";
    case VK_ERROR_TOO_MANY_OBJECTS:
      return "VK_ERROR_TOO_MANY_OBJECTS";
    case VK_ERROR_FORMAT_NOT_SUPPORTED:
      return "VK_ERROR_FORMAT_NOT_SUPPORTED";
    case VK_ERROR_FRAGMENTED_POOL:
      return "VK_ERROR_FRAGMENTED_POOL";
    case VK_ERROR_UNKNOWN:
      return "VK_ERROR_UNKNOWN";
    case VK_ERROR_OUT_OF_POOL_MEMORY:
      return "VK_ERROR_OUT_OF_POOL_MEMORY";
    case VK_ERROR_INVALID_EXTERNAL_HANDLE:
      return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
    case VK_ERROR_FRAGMENTATION:
      return "VK_ERROR_FRAGMENTATION";
    case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
      return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
    case VK_ERROR_SURFACE_LOST_KHR:
      return "VK_ERROR_SURFACE_LOST_KHR";
    case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
      return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
    case VK_SUBOPTIMAL_KHR:
      return "VK_SUBOPTIMAL_KHR";
    case VK_ERROR_OUT_OF_DATE_KHR:
      return "VK_ERROR_OUT_OF_DATE_KHR";
    case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
      return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
    case VK_ERROR_VALIDATION_FAILED_EXT:
      return "VK_ERROR_VALIDATION_FAILED_EXT";
    case VK_ERROR_INVALID_SHADER_NV:
      return "VK_ERROR_INVALID_SHADER_NV";
    case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
      return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
    case VK_ERROR_NOT_PERMITTED_EXT:
      return "VK_ERROR_NOT_PERMITTED_EXT";
    case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
      return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
    case VK_THREAD_IDLE_KHR:
      return "VK_THREAD_IDLE_KHR";
    case VK_THREAD_DONE_KHR:
      return "VK_THREAD_DONE_KHR";
    case VK_OPERATION_DEFERRED_KHR:
      return "VK_OPERATION_DEFERRED_KHR";
    case VK_OPERATION_NOT_DEFERRED_KHR:
      return "VK_OPERATION_NOT_DEFERRED_KHR";
    case VK_PIPELINE_COMPILE_REQUIRED_EXT:
      return "VK_PIPELINE_COMPILE_REQUIRED_EXT";

    default:
    {
      static char buf[24];
      snprintf(buf, sizeof(buf), "Unknown (%d)", result);
      return buf;
    }
  }
}

VkFormat mental::rhi::vk::convertTextureFormat(mental::rhi::TextureFormat format)
{
  switch (format)
  {
    case TextureFormat::eRGBA32_SRGB:
      return VK_FORMAT_R8G8B8A8_SRGB;
    case TextureFormat::eBGRA32_SRGB:
      return VK_FORMAT_B8G8R8A8_SRGB;
    case TextureFormat::eRGBA32_UNORM:
      return VK_FORMAT_R8G8B8A8_UNORM;
    case TextureFormat::eBGRA32_UNORM:
      return VK_FORMAT_B8G8R8A8_UNORM;
    case TextureFormat::eD32_SFLOAT:
      return VK_FORMAT_D32_SFLOAT;
    case TextureFormat::eD32_SFLOAT_S8_UINT:
      return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case TextureFormat::eD24_UNORM_S8_UINT:
      return VK_FORMAT_D24_UNORM_S8_UINT;
  }
}

VkImageLayout mental::rhi::vk::convertTextureLayout(mental::rhi::TextureLayout layout)
{
  switch (layout)
  {
    case TextureLayout::eUndefined:
      return VK_IMAGE_LAYOUT_UNDEFINED;
    case TextureLayout::ePresent:
      return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    case TextureLayout::eColorAttachment:
      return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    case TextureLayout::eDepthStencilAttachment:
      return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    case TextureLayout::eTransferSrc:
      return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    case TextureLayout::eTransferDst:
      return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    case TextureLayout::eShaderReadOnly:
      return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  }
}

VkPipelineStageFlags mental::rhi::vk::convertPipelineStage(mental::rhi::PipelineStage stage)
{
  switch (stage)
  {
    case PipelineStage::eColorAttachmentOutput:
      return VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  }
}

VkShaderStageFlags mental::rhi::vk::convertShaderStageFlags(mental::rhi::ShaderStageFlags stageFlags)
{
  VkShaderStageFlags vkStageFlags = 0;

  if ((stageFlags & ShaderStageFlagBits::eShaderStageVertexBit) != 0u)
  {
    vkStageFlags |= VK_SHADER_STAGE_VERTEX_BIT;
  }

  if ((stageFlags & ShaderStageFlagBits::eShaderStageFragmentBit) != 0u)
  {
    vkStageFlags |= VK_SHADER_STAGE_FRAGMENT_BIT;
  }

  return vkStageFlags;
}

VkDescriptorType mental::rhi::vk::convertResourceBindingType(mental::rhi::ResourceBindingType type)
{
  switch (type)
  {
    case ResourceBindingType::eUniformBuffer:
      return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case ResourceBindingType::eStorageBuffer:
      return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  }
}

VkPrimitiveTopology mental::rhi::vk::convertPrimitiveTopology(mental::rhi::PrimitiveTopology topology)
{
  switch (topology)
  {
    case PrimitiveTopology::eTriangleList:
      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
  }
}

VkPolygonMode mental::rhi::vk::convertPolygonMode(mental::rhi::PolygonMode polygonMode)
{
  switch (polygonMode)
  {
    case PolygonMode::eFill:
      return VK_POLYGON_MODE_FILL;
  }
}

VkCullModeFlags mental::rhi::vk::convertCullMode(mental::rhi::CullMode cullMode)
{
  switch (cullMode)
  {
    case CullMode::eNone:
      return VK_CULL_MODE_NONE;
    case CullMode::eBack:
      return VK_CULL_MODE_BACK_BIT;
    case CullMode::eFront:
      return VK_CULL_MODE_FRONT_BIT;
  }
}

VkFrontFace mental::rhi::vk::convertFrontFace(mental::rhi::FrontFace frontFace)
{
  switch (frontFace)
  {
    case FrontFace::eCounterClockwise:
      return VK_FRONT_FACE_COUNTER_CLOCKWISE;
    case FrontFace::eClockwise:
      return VK_FRONT_FACE_CLOCKWISE;
  }
}

VkCompareOp mental::rhi::vk::convertCompareOp(mental::rhi::CompareOp compareOp)
{
  switch (compareOp)
  {
    case CompareOp::eNever:
      return VK_COMPARE_OP_NEVER;
    case CompareOp::eLess:
      return VK_COMPARE_OP_LESS;
    case CompareOp::eLessOrEqual:
      return VK_COMPARE_OP_LESS_OR_EQUAL;
    case CompareOp::eAlways:
      return VK_COMPARE_OP_ALWAYS;
  }
}

VkImageAspectFlags mental::rhi::vk::getTextureAspectFlags(mental::rhi::TextureFormat format)
{
  switch (format)
  {
    case TextureFormat::eD32_SFLOAT:
      return VK_IMAGE_ASPECT_DEPTH_BIT;
    case TextureFormat::eD32_SFLOAT_S8_UINT:
    case TextureFormat::eD24_UNORM_S8_UINT:
      return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    case TextureFormat::eRGBA32_SRGB:
    case TextureFormat::eBGRA32_SRGB:
    case TextureFormat::eRGBA32_UNORM:
    case TextureFormat::eBGRA32_UNORM:
      return VK_IMAGE_ASPECT_COLOR_BIT;
  }
}

VkImageTiling mental::rhi::vk::convertTextureTiling(mental::rhi::TextureTiling tiling)
{
  switch (tiling)
  {
    case TextureTiling::eLinear:
      return VK_IMAGE_TILING_LINEAR;
    case TextureTiling::eOptimal:
      return VK_IMAGE_TILING_OPTIMAL;
  }
}

VkImageUsageFlags mental::rhi::vk::convertTextureUsageFlags(mental::rhi::TextureUsageFlags usage)
{
  VkImageUsageFlags flags = 0;

  if (usage & TextureUsageFlagBits::eTextureUsageTransferSrcBit)
  {
    flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
  }

  if (usage & TextureUsageFlagBits::eTextureUsageTransferDstBit)
  {
    flags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
  }

  if (usage & TextureUsageFlagBits::eTextureUsageSampledBit)
  {
    flags |= VK_IMAGE_USAGE_SAMPLED_BIT;
  }

  if (usage & TextureUsageFlagBits::eTextureUsageStorageBit)
  {
    flags |= VK_IMAGE_USAGE_STORAGE_BIT;
  }

  if (usage & TextureUsageFlagBits::eTextureUsageColorAttachmentBit)
  {
    flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  }

  if (usage & TextureUsageFlagBits::eTextureUsageDepthStencilAttachmentBit)
  {
    flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  }

  return flags;
}

VkImageAspectFlags mental::rhi::vk::getTextureAspectFlags(TextureType type)
{
  switch (type)
  {
    case TextureType::eTexture2D:
    case TextureType::eCubeMap:
      return VK_IMAGE_ASPECT_COLOR_BIT;
    case TextureType::eDepthMap:
      return VK_IMAGE_ASPECT_DEPTH_BIT;
    case TextureType::eDepthStencilMap:
      return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
  }
}
