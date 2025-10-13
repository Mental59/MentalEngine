#include <render/rhi/vulkan/factory.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/extensionManager.hpp>
#include <Volk/volk.h>
#include <vulkan/vulkan.hpp>
#include <core/log.hpp>
#include <set>
#include <vector>

namespace mental::rhi::vk
{

#if defined(_DEBUG)
static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    mental::core::log::info("Debug callback: %s", pCallbackData->pMessage);
    return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL reportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
    size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* UserData)
{
    mental::core::log::info("Report callback (%s): %s", pLayerPrefix, pMessage);
    return VK_FALSE;
}

static ::vk::DebugUtilsMessengerCreateInfoEXT getDebugMessengerCreateInfo()
{
    ::vk::DebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo
        .setMessageSeverity(::vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                            ::vk::DebugUtilsMessageSeverityFlagBitsEXT::eError)
        .setMessageType(::vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | ::vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                        ::vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
        .setPfnUserCallback(debugCallback);

    return createInfo;
}

static ::vk::DebugReportCallbackCreateInfoEXT getReportCallbackCreateInfo()
{
    ::vk::DebugReportCallbackCreateInfoEXT createInfo;
    createInfo
        .setFlags(::vk::DebugReportFlagBitsEXT::eWarning | ::vk::DebugReportFlagBitsEXT::ePerformanceWarning |
                  ::vk::DebugReportFlagBitsEXT::eError | ::vk::DebugReportFlagBitsEXT::eDebug)
        .setPfnCallback(reportCallback);

    return createInfo;
}

DebugMessenger DeviceFactory::createDebugMessenger(::vk::Instance instance) const
{
    ::vk::DebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo = getDebugMessengerCreateInfo();
    ::vk::ResultValue<::vk::DebugUtilsMessengerEXT> debugMessengerRes = instance.createDebugUtilsMessengerEXT(createDebugMessengerInfo);
    if (debugMessengerRes.result != ::vk::Result::eSuccess)
    {
        mental::core::log::error("Failed to call createDebugUtilsMessengerEXT");
    }

    ::vk::DebugReportCallbackCreateInfoEXT createReportCallbackInfo = getReportCallbackCreateInfo();
    ::vk::ResultValue<::vk::DebugReportCallbackEXT> reportCallbackRes = instance.createDebugReportCallbackEXT(createReportCallbackInfo);
    if (reportCallbackRes.result != ::vk::Result::eSuccess)
    {
        mental::core::log::error("Failed to call createDebugReportCallbackEXT");
    }

    return {debugMessengerRes.value, reportCallbackRes.value};
}
#endif

Device* DeviceFactory::create(const InstanceInfo& instanceInfo, const ::vk::SurfaceKHR& surface) const
{
    DebugMessenger debugMessenger{};
#if defined(_DEBUG)
    debugMessenger = createDebugMessenger(instanceInfo.getInstance());
#endif

    PhysicalDeviceInfo physicalDeviceInfo = choosePhysicalDevice(instanceInfo.getInstance(), surface);
    ::vk::Device device = createLogicalDevice(physicalDeviceInfo);

    DeviceDesc desc{.instance = instanceInfo.getInstance(),
        .surface = surface,
        .physicalDevice = physicalDeviceInfo.getPhysicalDevice(),
        .device = device,
        .graphicsQueue = device.getQueue(physicalDeviceInfo.getGraphicsQueueFamily(), 0),
        .graphicsQueueIndex = physicalDeviceInfo.getGraphicsQueueFamily(),
        .debugUtilsMessenger = debugMessenger.utilsMessenger,
        .debugReportCallback = debugMessenger.reportCallback,
        .instanceExtensions = instanceInfo.getExtensions(),
        .deviceExtensions = physicalDeviceInfo.getRequiredExtensions()};

    return Device::create(desc);
}

InstanceInfo DeviceFactory::createInstance() const
{
    VkResult volkInitRes = volkInitialize();
    if (volkInitRes != VK_SUCCESS)
    {
        mental::core::log::fatal("Failed to initialize volk");
    }

    VULKAN_HPP_DEFAULT_DISPATCHER.init(vkGetInstanceProcAddr);

    ::vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("Mental App")
        .setApplicationVersion(::vk::makeVersion(1, 0, 0))
        .setPEngineName("Mental Engine")
        .setEngineVersion(::vk::makeVersion(1, 0, 0))
        .setPNext(nullptr);

    ::vk::Result res = ::vk::enumerateInstanceVersion(&appInfo.apiVersion);
    const uint32_t minimumVulkanVersion = VK_MAKE_API_VERSION(0, 1, 3, 0);

    if (appInfo.apiVersion < minimumVulkanVersion)
    {
        mental::core::log::error("The Vulkan API version supported on the system (%d.%d.%d) is too low, at least %d.%d.%d is required.",
            VK_API_VERSION_MAJOR(appInfo.apiVersion), VK_API_VERSION_MINOR(appInfo.apiVersion), VK_API_VERSION_PATCH(appInfo.apiVersion),
            VK_API_VERSION_MAJOR(minimumVulkanVersion), VK_API_VERSION_MINOR(minimumVulkanVersion),
            VK_API_VERSION_PATCH(minimumVulkanVersion));

        return {};
    }

    std::vector<const char*> instanceExtensions = ExtensionManager::getRequiredInstanceExtensions();
    std::vector<const char*> validationLayers = ExtensionManager::getValidationLayers();

    if (!checkInstanceExtensionSupport(instanceExtensions))
    {
        mental::core::log::fatal("Required extensions not supported");
    }
    if (!checkInstanceLayerSupport(validationLayers))
    {
        mental::core::log::fatal("Validation layers not supported");
    }

    ::vk::InstanceCreateInfo instanceCreateInfo;
    instanceCreateInfo.setPApplicationInfo(&appInfo).setPEnabledExtensionNames(instanceExtensions).setPEnabledLayerNames(validationLayers);
#if defined(_DEBUG)
    ::vk::DebugUtilsMessengerCreateInfoEXT createDebugMessengerInfo = getDebugMessengerCreateInfo();
    instanceCreateInfo.setPNext(&createDebugMessengerInfo);
#endif

    ::vk::ResultValue<::vk::Instance> createInstanceRes = ::vk::createInstance(instanceCreateInfo);
    if (createInstanceRes.result != ::vk::Result::eSuccess)
    {
        return {};
    }

    ::vk::Instance instance = createInstanceRes.value;
    volkLoadInstance(instance);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

    return InstanceInfo(instance, instanceExtensions);
}

bool DeviceFactory::checkInstanceExtensionSupport(const std::vector<const char*>& extensions) const
{
    auto availableExtensions = ::vk::enumerateInstanceExtensionProperties();
    if (availableExtensions.result != ::vk::Result::eSuccess)
    {
        mental::core::log::fatal("Failed to call enumerateInstanceExtensionProperties");
    }

    std::set<std::string> requiredSet(extensions.begin(), extensions.end());
    for (const ::vk::ExtensionProperties& extension : availableExtensions.value)
    {
        requiredSet.erase(extension.extensionName);
    }

    return requiredSet.empty();
}

bool DeviceFactory::checkInstanceLayerSupport(const std::vector<const char*>& layers) const
{
    auto availableLayers = ::vk::enumerateInstanceLayerProperties();
    if (availableLayers.result != ::vk::Result::eSuccess)
    {
        mental::core::log::fatal("Failed to call enumerateInstanceLayerProperties");
    }

    std::set<std::string> requiredSet(layers.begin(), layers.end());
    for (const ::vk::LayerProperties& layer : availableLayers.value)
    {
        requiredSet.erase(layer.layerName);
    }

    return requiredSet.empty();
}

PhysicalDeviceInfo DeviceFactory::choosePhysicalDevice(const ::vk::Instance& instance, const ::vk::SurfaceKHR& surface) const
{
    std::vector<const char*> extensions = ExtensionManager::getRequiredDeviceExtensions();

    auto physicalDevices = instance.enumeratePhysicalDevices();
    if (physicalDevices.result != ::vk::Result::eSuccess)
    {

        mental::core::log::fatal("Failed to call enumeratePhysicalDevices");
    }

    PhysicalDeviceInfo bestPhysicalDeviceInfo;
    std::vector<const char*> requiredExtensions = ExtensionManager::getRequiredDeviceExtensions();
    ::vk::PhysicalDeviceFeatures requiredFeatures{};
    requiredFeatures.setGeometryShader(::vk::True);

    for (const ::vk::PhysicalDevice& physicalDevice : physicalDevices.value)
    {
        PhysicalDeviceInfo physicalDeviceInfo(physicalDevice, surface, requiredExtensions, requiredFeatures);
        if (physicalDeviceInfo.getScore() > bestPhysicalDeviceInfo.getScore())
        {
            bestPhysicalDeviceInfo = physicalDeviceInfo;
            break;
        }
    }

    if (!bestPhysicalDeviceInfo.isSuitable())
    {
        mental::core::log::fatal("Failed to find suitable physical device");
    }

    return bestPhysicalDeviceInfo;
}

::vk::Device DeviceFactory::createLogicalDevice(const PhysicalDeviceInfo& physicalDeviceInfo) const
{
    ::vk::DeviceQueueCreateInfo graphicsQueueCreateInfo{};
    float graphicsQueuePriority = 1.0f;
    graphicsQueueCreateInfo.setQueueFamilyIndex(physicalDeviceInfo.getGraphicsQueueFamily())
        .setQueueCount(1)
        .setPQueuePriorities(&graphicsQueuePriority);

    ::vk::DeviceCreateInfo createInfo{};
    createInfo.setQueueCreateInfoCount(1)
        .setPQueueCreateInfos(&graphicsQueueCreateInfo)
        .setPEnabledExtensionNames(physicalDeviceInfo.getRequiredExtensions())
        .setPEnabledFeatures(&physicalDeviceInfo.getRequiredFeatures());

    auto createDeviceRes = physicalDeviceInfo.getPhysicalDevice().createDevice(createInfo);
    if (createDeviceRes.result != ::vk::Result::eSuccess)
    {
        mental::core::log::fatal("Failed to create logical device");
    }

    ::vk::Device device = createDeviceRes.value;
    volkLoadDevice(device);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(device);

    return device;
}

PhysicalDeviceInfo::PhysicalDeviceInfo(const ::vk::PhysicalDevice& physicalDevice, const ::vk::SurfaceKHR& surface,
    const std::vector<const char*>& requiredExtensions, const ::vk::PhysicalDeviceFeatures& features)
    : mPhysicalDevice(physicalDevice), mSurface(surface), mAreExtensionsSupported(checkDeviceExtensionSupport(requiredExtensions)),
      mGraphicsQueueFamily(findGraphicsQueueFamily()), mSwapchainSupportDetails(querySwapchainSupport()), mScore(calculateScore()),
      mRequiredExtensions(requiredExtensions), mRequiredFeatures(features)
{
}

bool PhysicalDeviceInfo::checkDeviceExtensionSupport(const std::vector<const char*>& extensions) const
{
    auto extensionProperties = mPhysicalDevice.enumerateDeviceExtensionProperties();
    if (extensionProperties.result != ::vk::Result::eSuccess)
    {
        mental::core::log::fatal("Failed to call enumerateDeviceExtensionProperties");
    }

    std::set<std::string> requiredSet(extensions.begin(), extensions.end());
    for (const ::vk::ExtensionProperties& property : extensionProperties.value)
    {
        requiredSet.erase(property.extensionName);
    }

    return requiredSet.empty();
}

int PhysicalDeviceInfo::findGraphicsQueueFamily() const
{
    auto queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();
    ::vk::QueueFlags desiredFlags = ::vk::QueueFlagBits::eGraphics;
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        auto isPresentSupported = mPhysicalDevice.getSurfaceSupportKHR(i, mSurface);
        if (isPresentSupported.result != ::vk::Result::eSuccess)
        {
            continue;
        }

        if (queueFamilyProperties[i].queueCount > 0 && queueFamilyProperties[i].queueFlags & desiredFlags && isPresentSupported.value)
        {
            return i;
        }
    }

    return -1;
}

SwapchainSupportDetails PhysicalDeviceInfo::querySwapchainSupport() const
{
    auto surfaceCapabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(mSurface);
    auto surfaceFormats = mPhysicalDevice.getSurfaceFormatsKHR(mSurface);
    auto presentModes = mPhysicalDevice.getSurfacePresentModesKHR(mSurface);

    if (surfaceCapabilities.result != ::vk::Result::eSuccess || surfaceFormats.result != ::vk::Result::eSuccess ||
        presentModes.result != ::vk::Result::eSuccess)
    {
        return {};
    }

    return {surfaceCapabilities.value, surfaceFormats.value, presentModes.value};
}

int PhysicalDeviceInfo::calculateScore() const
{
    if (!mAreExtensionsSupported || !isGPU() || mGraphicsQueueFamily < 0 || mSwapchainSupportDetails.formats.empty() ||
        mSwapchainSupportDetails.presentModes.empty())
    {
        return 0;
    }

    // TODO: should be expanded, only checking geometry shader support
    ::vk::PhysicalDeviceFeatures features = mPhysicalDevice.getFeatures();
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
