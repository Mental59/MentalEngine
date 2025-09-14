#include <Volk/volk.h>
#include <render/rhi/vulkan/factory.hpp>
#include <render/rhi/vulkan/device.hpp>
#include <render/rhi/vulkan/extensionManager.hpp>
#include <core/log.hpp>
#include <vulkan/vulkan.hpp>
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

void DeviceFactory::setupDebugMessenger(::vk::Instance instance) const
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
}
#endif

DeviceHandle DeviceFactory::create() const
{
    ::vk::Instance instance = createInstance();

    return createDevice({.instance = instance});
}

::vk::Instance DeviceFactory::createInstance() const
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

        return nullptr;
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
        return nullptr;
    }

    ::vk::Instance instance = createInstanceRes.value;
    volkLoadInstance(instance);
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#if defined(_DEBUG)
    setupDebugMessenger(instance);
#endif

    return instance;
}

bool DeviceFactory::checkInstanceExtensionSupport(const std::vector<const char*> extensions) const
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

bool DeviceFactory::checkInstanceLayerSupport(const std::vector<const char*> layers) const
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

}  // namespace mental::rhi::vk
