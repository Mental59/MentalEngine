#pragma once
#include <volk/volk.h>
#include <render/rhi/rhi.hpp>

namespace mental::rhi::vk
{
  const char* vkResultToString(VkResult result);
}  // namespace mental::rhi::vk
