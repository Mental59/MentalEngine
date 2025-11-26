#include <render/rhi/vulkan/factory.hpp>
#include <Volk/volk.h>
#include <core/log.hpp>
#include <render/rhi/vulkan/constants.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/extensionManager.hpp>
#include <set>
#include <vector>

namespace mental::rhi::vk
{

#if defined(_DEBUG)
  static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
      VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
      VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
      void* pUserData)
  {
    MENTAL_DEBUG("Debug callback: {}", pCallbackData->pMessage);
    return VK_FALSE;
  }

  static VKAPI_ATTR VkBool32 VKAPI_CALL reportCallback(
      VkDebugReportFlagsEXT flags,
      VkDebugReportObjectTypeEXT objectType,
      uint64_t object,
      size_t location,
      int32_t messageCode,
      const char* pLayerPrefix,
      const char* pMessage,
      void* UserData)
  {
    MENTAL_DEBUG("Report callback ({}): {}", pLayerPrefix, pMessage);
    return VK_FALSE;
  }

  static VkDebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo()
  {
    VkDebugUtilsMessengerCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT };
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    return createInfo;
  }

  static VkDebugReportCallbackCreateInfoEXT getReportCallbackCreateInfo()
  {
    VkDebugReportCallbackCreateInfoEXT createInfo{ VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT };
    createInfo.flags = VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
                       VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    createInfo.pfnCallback = reportCallback;
    return createInfo;
  }

  DebugMessenger DeviceFactory::createDebugMessenger(VkInstance instance) const
  {
    VkResult vkRes;

    VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo = getDebugMessengerCreateInfo();
    VkDebugUtilsMessengerEXT debugMessenger;
    vkRes = vkCreateDebugUtilsMessengerEXT(instance, &createDebugMessengerInfo, VK_NULL_HANDLE, &debugMessenger);
    if (vkRes != VK_SUCCESS)
      MENTAL_ERROR("Failed to call createDebugUtilsMessengerEXT");

    VkDebugReportCallbackCreateInfoEXT createReportCallbackInfo = getReportCallbackCreateInfo();
    VkDebugReportCallbackEXT reportCallback;
    vkRes = vkCreateDebugReportCallbackEXT(instance, &createReportCallbackInfo, VK_NULL_HANDLE, &reportCallback);
    if (vkRes != VK_SUCCESS)
      MENTAL_ERROR("Failed to call createDebugReportCallbackEXT");

    return { debugMessenger, reportCallback };
  }
#endif

  core::Result DeviceFactory::initDevice(const InstanceInfo& instanceInfo, VkSurfaceKHR surface) const
  {
    core::Result res;

    DebugMessenger debugMessenger{};
#if defined(_DEBUG)
    debugMessenger = createDebugMessenger(instanceInfo.getInstance());
#endif

    PhysicalDeviceInfo physicalDeviceInfo;
    res = choosePhysicalDevice(instanceInfo.getInstance(), surface, physicalDeviceInfo);
    if (res != core::Result::eSuccess)
      return res;

    VkDevice vkDevice;
    res = createLogicalDevice(physicalDeviceInfo, vkDevice);
    if (res != core::Result::eSuccess)
      return res;

    VkQueue graphicsQueue;
    vkGetDeviceQueue(vkDevice, physicalDeviceInfo.getGraphicsQueueFamily(), 0, &graphicsQueue);

    DeviceDesc desc{ .instance = instanceInfo.getInstance(),
                     .surface = surface,
                     .physicalDevice = physicalDeviceInfo.getPhysicalDevice(),
                     .device = vkDevice,
                     .formats = physicalDeviceInfo.getSwapchainSupportDetails().formats,
                     .presentModes = physicalDeviceInfo.getSwapchainSupportDetails().presentModes,
                     .graphicsQueue = graphicsQueue,
                     .graphicsQueueIndex = physicalDeviceInfo.getGraphicsQueueFamily(),
                     .debugUtilsMessenger = debugMessenger.utilsMessenger,
                     .debugReportCallback = debugMessenger.reportCallback,
                     .instanceExtensions = instanceInfo.getExtensions(),
                     .deviceExtensions = physicalDeviceInfo.getRequiredExtensions() };

    return getDevice().init(desc);
  }

  core::Result DeviceFactory::createInstance(InstanceInfo& instanceInfo) const
  {
    VkResult vkRes;

    vkRes = volkInitialize();
    if (vkRes != VK_SUCCESS)
      return core::Result::eInitializationFailed;

    VkApplicationInfo appInfo{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
    appInfo.pApplicationName = "Mental App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Mental Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);

    vkRes = vkEnumerateInstanceVersion(&appInfo.apiVersion);
    if (vkRes != VK_SUCCESS)
      return core::Result::eInitializationFailed;

    const uint32_t minimumVulkanVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    if (appInfo.apiVersion < minimumVulkanVersion)
    {
      MENTAL_ERROR(
          "The Vulkan API version supported on the system ({}.{}.{}) is too low, at least {}.{}.{} is required.",
          VK_API_VERSION_MAJOR(appInfo.apiVersion),
          VK_API_VERSION_MINOR(appInfo.apiVersion),
          VK_API_VERSION_PATCH(appInfo.apiVersion),
          VK_API_VERSION_MAJOR(minimumVulkanVersion),
          VK_API_VERSION_MINOR(minimumVulkanVersion),
          VK_API_VERSION_PATCH(minimumVulkanVersion));
      return core::Result::eInitializationFailed;
    }

    std::vector<const char*> instanceExtensions = ExtensionManager::getRequiredInstanceExtensions();
    std::vector<const char*> validationLayers = ExtensionManager::getValidationLayers();

    if (!checkInstanceExtensionSupport(instanceExtensions))
    {
      MENTAL_ERROR("Required extensions not supported");
      return core::Result::eInitializationFailed;
    }

    if (!checkInstanceLayerSupport(validationLayers))
    {
      MENTAL_ERROR("Validation layers not supported");
      return core::Result::eInitializationFailed;
    }

    VkInstanceCreateInfo instanceCreateInfo{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
    instanceCreateInfo.pApplicationInfo = &appInfo;
    instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
    instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(instanceExtensions.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());

#if defined(_DEBUG)
    VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo = getDebugMessengerCreateInfo();
    instanceCreateInfo.pNext = &createDebugMessengerInfo;
#endif

    VkInstance instance;
    vkRes = vkCreateInstance(&instanceCreateInfo, VK_NULL_HANDLE, &instance);
    if (vkRes != VK_SUCCESS)
      return core::Result::eInitializationFailed;

    volkLoadInstance(instance);

    instanceInfo = InstanceInfo(instance, instanceExtensions);
    return core::Result::eSuccess;
  }

  bool DeviceFactory::checkInstanceExtensionSupport(const std::vector<const char*>& extensions) const
  {
    VkResult vkRes;

    uint32_t availableExtensionCount = 0;
    vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, nullptr);
    if (vkRes != VK_SUCCESS)
      return false;

    std::vector<VkExtensionProperties> availableExtensions(availableExtensionCount);
    vkRes = vkEnumerateInstanceExtensionProperties(nullptr, &availableExtensionCount, availableExtensions.data());
    if (vkRes != VK_SUCCESS)
      return false;

    std::set<std::string> requiredSet(extensions.begin(), extensions.end());
    for (const VkExtensionProperties& extension : availableExtensions)
    {
      requiredSet.erase(extension.extensionName);
    }

    return requiredSet.empty();
  }

  bool DeviceFactory::checkInstanceLayerSupport(const std::vector<const char*>& layers) const
  {
    VkResult vkRes;

    uint32_t layersCount;
    vkRes = vkEnumerateInstanceLayerProperties(&layersCount, nullptr);
    if (vkRes != VK_SUCCESS)
      return false;

    std::vector<VkLayerProperties> availableLayers(layersCount);
    vkRes = vkEnumerateInstanceLayerProperties(&layersCount, availableLayers.data());
    if (vkRes != VK_SUCCESS)
      return false;

    std::set<std::string> requiredSet(layers.begin(), layers.end());
    for (const VkLayerProperties& layer : availableLayers)
    {
      requiredSet.erase(layer.layerName);
    }

    return requiredSet.empty();
  }

  core::Result DeviceFactory::choosePhysicalDevice(
      VkInstance instance,
      VkSurfaceKHR surface,
      PhysicalDeviceInfo& physicalDeviceInfo) const
  {
    VkResult vkRes;

    std::vector<const char*> extensions = ExtensionManager::getRequiredDeviceExtensions();

    uint32_t physicalDeviceCount = 0;
    vkRes = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    if (!physicalDeviceCount || vkRes != VK_SUCCESS)
      return core::Result::eInitializationFailed;

    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkRes = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    if (vkRes != VK_SUCCESS)
      return core::Result::eInitializationFailed;

    PhysicalDeviceInfo bestPhysicalDeviceInfo;
    std::vector<const char*> requiredExtensions = ExtensionManager::getRequiredDeviceExtensions();
    VkPhysicalDeviceFeatures requiredFeatures{};
    requiredFeatures.geometryShader = VK_TRUE;

    for (const VkPhysicalDevice& physicalDevice : physicalDevices)
    {
      PhysicalDeviceInfo physicalDeviceInfo(physicalDevice, surface, requiredExtensions, requiredFeatures);
      if (physicalDeviceInfo.getScore() > bestPhysicalDeviceInfo.getScore())
      {
        bestPhysicalDeviceInfo = physicalDeviceInfo;
        break;
      }
    }

    if (!bestPhysicalDeviceInfo.isSuitable())
      return core::Result::eInitializationFailed;

    physicalDeviceInfo = bestPhysicalDeviceInfo;
    return core::Result::eSuccess;
  }

  core::Result DeviceFactory::createLogicalDevice(const PhysicalDeviceInfo& physicalDeviceInfo, VkDevice& device) const
  {
    VkDeviceQueueCreateInfo graphicsQueueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
    float graphicsQueuePriority = 1.0f;
    graphicsQueueCreateInfo.queueFamilyIndex = physicalDeviceInfo.getGraphicsQueueFamily();
    graphicsQueueCreateInfo.queueCount = 1;
    graphicsQueueCreateInfo.pQueuePriorities = &graphicsQueuePriority;

    std::vector<const char*> physicalDeviceExtensions = physicalDeviceInfo.getRequiredExtensions();
    VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &graphicsQueueCreateInfo;
    createInfo.ppEnabledExtensionNames = physicalDeviceExtensions.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(physicalDeviceExtensions.size());
    createInfo.pEnabledFeatures = &physicalDeviceInfo.getRequiredFeatures();

    VkResult res = vkCreateDevice(physicalDeviceInfo.getPhysicalDevice(), &createInfo, VK_NULL_HANDLE, &device);
    if (res != VK_SUCCESS)
      return core::Result::eInitializationFailed;

    volkLoadDevice(device);

    return core::Result::eSuccess;
  }

  PhysicalDeviceInfo::PhysicalDeviceInfo(
      VkPhysicalDevice physicalDevice,
      VkSurfaceKHR surface,
      const std::vector<const char*>& requiredExtensions,
      const VkPhysicalDeviceFeatures& features)
      : mPhysicalDevice(physicalDevice),
        mSurface(surface),
        mAreExtensionsSupported(checkDeviceExtensionSupport(requiredExtensions)),
        mGraphicsQueueFamily(findGraphicsQueueFamily()),
        mSwapchainSupportDetails(querySwapchainSupport()),
        mScore(calculateScore()),
        mRequiredExtensions(requiredExtensions),
        mRequiredFeatures(features)
  {
  }

  bool PhysicalDeviceInfo::isDiscreteGPU() const
  {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
  }

  bool PhysicalDeviceInfo::isIntegratedGPU() const
  {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(mPhysicalDevice, &properties);
    return properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU;
  }

  bool PhysicalDeviceInfo::checkDeviceExtensionSupport(const std::vector<const char*>& extensions) const
  {
    VkResult vkRes;

    uint32_t extensionCount;
    vkRes = vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, nullptr);
    if (vkRes != VK_SUCCESS)
      return false;

    std::vector<VkExtensionProperties> extensionProperties(extensionCount);
    vkRes = vkEnumerateDeviceExtensionProperties(mPhysicalDevice, nullptr, &extensionCount, extensionProperties.data());
    if (vkRes != VK_SUCCESS)
      return false;

    std::set<std::string> requiredSet(extensions.begin(), extensions.end());
    for (const VkExtensionProperties& property : extensionProperties)
    {
      requiredSet.erase(property.extensionName);
    }

    return requiredSet.empty();
  }

  int PhysicalDeviceInfo::findGraphicsQueueFamily() const
  {
    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &familyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(mPhysicalDevice, &familyCount, queueFamilyProperties.data());

    VkQueueFlags desiredFlags = VK_QUEUE_GRAPHICS_BIT;
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
    {
      VkBool32 isPresentSupported = VK_FALSE;
      vkGetPhysicalDeviceSurfaceSupportKHR(mPhysicalDevice, i, mSurface, &isPresentSupported);

      if (queueFamilyProperties[i].queueCount > 0 && queueFamilyProperties[i].queueFlags & desiredFlags &&
          isPresentSupported)
      {
        return i;
      }
    }

    return -1;
  }

  SwapchainSupportDetails PhysicalDeviceInfo::querySwapchainSupport() const
  {
    SwapchainSupportDetails supportDetails{};

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(mPhysicalDevice, mSurface, &supportDetails.capabilities);

    uint32_t formatsCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatsCount, VK_NULL_HANDLE);
    if (formatsCount > 0)
    {
      supportDetails.formats.resize(formatsCount);
      vkGetPhysicalDeviceSurfaceFormatsKHR(mPhysicalDevice, mSurface, &formatsCount, supportDetails.formats.data());
    }

    uint32_t modesCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &modesCount, VK_NULL_HANDLE);
    if (modesCount > 0)
    {
      supportDetails.presentModes.resize(modesCount);
      vkGetPhysicalDeviceSurfacePresentModesKHR(mPhysicalDevice, mSurface, &modesCount, supportDetails.presentModes.data());
    }

    return supportDetails;
  }

  int PhysicalDeviceInfo::calculateScore() const
  {
    if (!mAreExtensionsSupported || !isGPU() || mGraphicsQueueFamily < 0 || mSwapchainSupportDetails.formats.empty() ||
        mSwapchainSupportDetails.presentModes.empty())
    {
      return 0;
    }

    // TODO: should be expanded, only checking geometry shader support
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(mPhysicalDevice, &features);
    if (mRequiredFeatures.geometryShader && !features.geometryShader)
    {
      return 0;
    }

    int score = 1;
    if (isDiscreteGPU())
    {
      score += 10;
    }

    return score;
  }

}  // namespace mental::rhi::vk
