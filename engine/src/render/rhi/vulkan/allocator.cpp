#define VMA_IMPLEMENTATION
#include <Volk/volk.h>
#include <vma/vk_mem_alloc.h>
#include "core/log.hpp"
#include "core/types.hpp"
#include <render/rhi/vulkan/allocator.hpp>

static VmaAllocator gAllocator;

mental::core::Result mental::rhi::vk::initAllocator(const mental::rhi::vk::AllocatorDesc& desc)
{
  VmaAllocatorCreateInfo allocatorCreateInfo{};
  allocatorCreateInfo.vulkanApiVersion = desc.vulkanApiVersion;
  allocatorCreateInfo.physicalDevice = desc.physicalDevice;
  allocatorCreateInfo.device = desc.device;
  allocatorCreateInfo.instance = desc.instance;

  VmaVulkanFunctions vulkanFunctions;
  VkResult importRes = vmaImportVulkanFunctionsFromVolk(&allocatorCreateInfo, &vulkanFunctions);
  if (importRes != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to import vulkan functions");
    return core::Result::eInitializationFailed;
  }

  allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;

  VkResult createAllocatorRes = vmaCreateAllocator(&allocatorCreateInfo, &gAllocator);
  if (createAllocatorRes != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to create vulkan allocator");
    return core::Result::eInitializationFailed;
  }

  return core::Result::eSuccess;
}

void mental::rhi::vk::destroyAllocator()
{
  vmaDestroyAllocator(gAllocator);
}

VmaAllocator mental::rhi::vk::getAllocator()
{
  return gAllocator;
}
