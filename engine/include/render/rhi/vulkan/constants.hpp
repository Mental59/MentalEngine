#pragma once
#include <volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
const char* vkResultToString(VkResult result);
VkFormat convertTextureFormat(TextureFormat format);
VkImageLayout convertTextureLayout(TextureLayout layout);
VkPipelineStageFlags convertPipelineStage(PipelineStage stage);
VkImageAspectFlags getTextureAspectFlags(TextureFormat format);
VkImageTiling convertTextureTiling(TextureTiling tiling);
VkImageUsageFlags convertTextureUsageFlags(TextureUsageFlags usage);
VkImageAspectFlags getTextureAspectFlags(TextureType type);
} // namespace mental::rhi::vk
