#pragma once
#include "vulkanRenderDevice.hpp"
#include <volk.h>

#ifndef VK_CHECK
#define VK_CHECK(value)                                                        \
  vkFramework::check(value == VK_SUCCESS, __FILE__, __LINE__);
#endif

#ifndef CHECK_BOOL
#define CHECK_BOOL(value) vkFramework::check(value, __FILE__, __LINE__);
#endif

namespace vkFramework {
void check(bool check, const char* fileName, int lineNumber);

VkResult createSyncObjects(VulkanRenderDevice& renderDevice);

bool hasStencilComponent(VkFormat format);

uint32_t bytesPerTexFormat(VkFormat fmt);
} // namespace vkFramework
