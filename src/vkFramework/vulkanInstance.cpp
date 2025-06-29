#include "vulkanInstance.hpp"
#include "vulkanDebug.hpp"
#include "vulkanUtils.hpp"
#include "window/window.hpp"
#include <cstdint>
#include <cstring>
#include <vector>

namespace {
void setupLayers(std::vector<const char*>* layers,
                 VkInstanceCreateInfo* instanceCreateInfo);
void addValidationLayers(std::vector<const char*>* layers);
bool checkLayersSupport(const char** layers, uint32_t size);

void setupExtensions(std::vector<const char*>* extensions,
                     VkInstanceCreateInfo* instanceCreateInfo);
void addMainExtensions(std::vector<const char*>* extensions);
void addDebugExtensions(std::vector<const char*>* extensions);
void addPlatformSpecificExtensions(std::vector<const char*>* extensions);
bool checkExtensionsSupport(const char** extensions, uint32_t size);
} // namespace

namespace vkFramework {
VkInstance createVulkanInstance() {
  VkInstance instance;

  VkApplicationInfo appInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                            .pNext = nullptr,
                            .pApplicationName = "Mental Engine App",
                            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
                            .pEngineName = "MentalEngine",
                            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
                            .apiVersion = VK_API_VERSION_1_4};

  VkInstanceCreateInfo instanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      .pNext = nullptr,
      .flags = 0,
      .pApplicationInfo = &appInfo,
  };

  std::vector<const char*> layers;
  setupLayers(&layers, &instanceCreateInfo);

  std::vector<const char*> extensions;
  setupExtensions(&extensions, &instanceCreateInfo);

#if defined(_DEBUG)
  VkDebugUtilsMessengerCreateInfoEXT createInfo =
      vkFramework::debugUtilsMessengerCreateInfo();
  instanceCreateInfo.pNext = &createInfo;
#endif

  VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &instance));

  volkLoadInstance(instance);

  return instance;
}

void initVulkanInstance(VulkanInstance& vulkanInstance,
                        window::Window* window) {
  vulkanInstance.instance = vkFramework::createVulkanInstance();

#if defined(_DEBUG)
  vkFramework::setupDebugCallbacks(vulkanInstance.instance,
                                   &vulkanInstance.messenger,
                                   &vulkanInstance.reportCallback);
#endif // (_DEBUG)

  VK_CHECK(window->createVulkanWindowSurface(&vulkanInstance));
}

void destroyVulkanInstance(VulkanInstance& vk) {
  vkDestroySurfaceKHR(vk.instance, vk.surface, nullptr);

#if defined(_DEBUG)
  vkDestroyDebugReportCallbackEXT(vk.instance, vk.reportCallback, nullptr);
  vkDestroyDebugUtilsMessengerEXT(vk.instance, vk.messenger, nullptr);
#endif // (_DEBUG)

  vkDestroyInstance(vk.instance, nullptr);
}

} // namespace vkFramework

namespace {
void setupLayers(std::vector<const char*>* layers,
                 VkInstanceCreateInfo* instanceCreateInfo) {
#if defined(_DEBUG)
  addValidationLayers(layers);
#endif // _DEBUG

  if (layers->size() == 0)
    return;

  CHECK_BOOL(checkLayersSupport(layers->data(), layers->size()));

  instanceCreateInfo->enabledLayerCount = static_cast<uint32_t>(layers->size());
  instanceCreateInfo->ppEnabledLayerNames = layers->data();
}

void addValidationLayers(std::vector<const char*>* layers) {
  layers->push_back("VK_LAYER_KHRONOS_validation");
}

bool checkLayersSupport(const char** layers, uint32_t size) {
  uint32_t layerCount;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

  std::vector<VkLayerProperties> availableLayers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

  for (uint32_t i = 0; i < size; i++) {
    bool layerFound = false;

    for (const VkLayerProperties& layerProperties : availableLayers) {
      if (strcmp(layers[i], layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      return false;
    }
  }

  return true;
}

void setupExtensions(std::vector<const char*>* extensions,
                     VkInstanceCreateInfo* instanceCreateInfo) {
  addMainExtensions(extensions);

#if defined(_DEBUG)
  addDebugExtensions(extensions);
#endif

  addPlatformSpecificExtensions(extensions);

  CHECK_BOOL(checkExtensionsSupport(extensions->data(), extensions->size()));

  instanceCreateInfo->enabledExtensionCount =
      static_cast<uint32_t>(extensions->size());
  instanceCreateInfo->ppEnabledExtensionNames = extensions->data();
}

void addMainExtensions(std::vector<const char*>* extensions) {
  extensions->insert(extensions->end(),
                     {"VK_KHR_surface",
                      VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME});
}

void addDebugExtensions(std::vector<const char*>* extensions) {
  extensions->insert(extensions->end(), {VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
                                         VK_EXT_DEBUG_REPORT_EXTENSION_NAME});
}

void addPlatformSpecificExtensions(std::vector<const char*>* extensions) {
#if defined(_WIN32)
  extensions->push_back("VK_KHR_win32_surface");
#elif defined(__linux__)
  extensions->push_back("VK_KHR_xcb_surface");
#endif
}

bool checkExtensionsSupport(const char** extensions, uint32_t size) {
  uint32_t supportedExtensionCount = 0;
  vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount,
                                         nullptr);

  std::vector<VkExtensionProperties> supportedExtensions(
      supportedExtensionCount);
  vkEnumerateInstanceExtensionProperties(nullptr, &supportedExtensionCount,
                                         supportedExtensions.data());

  for (uint32_t i = 0; i < size; i++) {
    bool extensionFound = false;

    for (const VkExtensionProperties& extensionProperties :
         supportedExtensions) {
      if (strcmp(extensions[i], extensionProperties.extensionName) == 0) {
        extensionFound = true;
        break;
      }
    }

    if (!extensionFound) {
      return false;
    }
  }

  return true;
}
} // namespace
