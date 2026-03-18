#include <render/rhi/vulkan/extensionManager.hpp>
#include <volk/volk.h>
#include <core/log.hpp>

std::vector<const char*> mental::rhi::vk::ExtensionManager::getRequiredInstanceExtensions()
{
  std::vector<const char*> extensions;

#if defined(MENTAL_WIN32)
  extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(MENTAL_LINUX)
  extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#else
  MENTAL_ASSERT_MESSAGE(false, "Unsupported platform");
#endif

#if defined(_DEBUG)
  extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

  extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
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
