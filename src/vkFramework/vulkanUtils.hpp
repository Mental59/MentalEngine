#pragma once
#include "vulkanRenderDevice.hpp"
#include <volk.h>

#define MENTAL_VK_CHECK(value)                                                 \
  vkFramework::check(value == VK_SUCCESS, __FILE__, __LINE__);
#define MENTAL_CHECK_BOOL(value) vkFramework::check(value, __FILE__, __LINE__);

namespace vkFramework {
void check(bool check, const char* fileName, int lineNumber);

VkResult createSyncObjects(VulkanRenderDevice& renderDevice);

bool hasStencilComponent(VkFormat format);

uint32_t bytesPerTexFormat(VkFormat fmt);
} // namespace vkFramework
