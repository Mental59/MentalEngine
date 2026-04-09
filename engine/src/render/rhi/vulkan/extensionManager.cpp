#include <render/rhi/vulkan/extensionManager.hpp>
#include <volk.h>
#include <core/log.hpp>

std::vector<const char*> mental::rhi::vk::ExtensionManager::getRequiredInstanceExtensions(
  const std::vector<const char*>& platformExtensions)
{
  std::vector<const char*> extensions(platformExtensions.begin(), platformExtensions.end());

#if defined(_DEBUG)
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

  extensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);

  return extensions;
}

std::vector<const char*> mental::rhi::vk::ExtensionManager::getValidationLayers()
{
  std::vector<const char*> layers;

#if defined(_DEBUG)
  layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

  return layers;
}

std::vector<const char*> mental::rhi::vk::ExtensionManager::getRequiredDeviceExtensions()
{
  return {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
}
