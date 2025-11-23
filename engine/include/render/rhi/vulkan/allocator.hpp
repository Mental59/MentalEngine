#pragma once
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include "core/types.hpp"

namespace mental::rhi::vk
{
  struct AllocatorDesc
  {
    uint32_t vulkanApiVersion;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkInstance instance;
  };

  core::Result initAllocator(const AllocatorDesc& desc);
  void destroyAllocator();

  VmaAllocator getAllocator();
}  // namespace mental::rhi::vk
