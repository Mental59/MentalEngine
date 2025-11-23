#include <render/rhi/vulkan/swapchain.hpp>
#include "core/log.hpp"
#include "core/types.hpp"
#include "render/rhi/vulkan/constants.hpp"
#include "render/rhi/vulkan/device.hpp"

mental::core::Result mental::rhi::vk::Swapchain::init(const mental::rhi::SwapchainDesc& desc)
{
  VkSurfaceCapabilitiesKHR surfaceCapabilities{};
  VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
      vk::getDevice().getPhysicalDevice(), vk::getDevice().getSurface(), &surfaceCapabilities);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkGetPhysicalDeviceSurfaceCapabilitiesKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  VkSurfaceFormatKHR surfaceFormat;
  VkPresentModeKHR presentMode;
  VkExtent2D extent = surfaceCapabilities.currentExtent;
  uint32_t minImageCount = desc.imageCount;
  if (surfaceCapabilities.maxImageCount > 0 && minImageCount > surfaceCapabilities.maxImageCount)
  {
    MENTAL_WARN(
        "Surface max image count is {}, but got {}, minImageCount is set to {}",
        surfaceCapabilities.maxImageCount,
        desc.imageCount,
        surfaceCapabilities.maxImageCount);
    minImageCount = surfaceCapabilities.maxImageCount;
  }

  VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
  createInfo.minImageCount = minImageCount;

  res = vkCreateSwapchainKHR(vk::getDevice().getVirtualDevice(), &createInfo, nullptr, &mSwapchain);
  if (res != VK_SUCCESS)
  {
    MENTAL_ERROR("Failed to call vkCreateSwapchainKHR, error: {}", vkResultToString(res));
    return core::Result::eInitializationFailed;
  }

  return core::Result::eSuccess;
}

void mental::rhi::vk::Swapchain::destroy()
{
  vkDestroySwapchainKHR(vk::getDevice().getVirtualDevice(), mSwapchain, nullptr);
}
