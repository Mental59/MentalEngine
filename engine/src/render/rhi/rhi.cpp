#include <render/rhi/rhi.hpp>
#include <core/log.hpp>
#include <platform/window.hpp>
#if defined MENTAL_WITH_VULKAN
#include <render/rhi/vulkan/factory.hpp>
#endif

namespace mental::rhi
{
static IDevice* gDevice = nullptr;

const char* resultToString(Result res)
{
    switch (res)
    {
        case mental::rhi::Result::eSuccess: return "RHI_SUCCESS";
        case mental::rhi::Result::eDeviceInitializationFailed: return "RHI_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eInstanceInitializationFailed: return "RHI_INSTANCE_INITIALIZATION_FAILED";
        case mental::rhi::Result::ePhysicalDeviceInitializationFailed: return "RHI_PHYSICAL_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eLogicalDeviceInitializationFailed: return "RHI_LOGICAL_DEVICE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eSurfaceInitializationFailed: return "RHI_SURFACE_INITIALIZATION_FAILED";
        case mental::rhi::Result::eBufferInitializationFailed: return "RHI_BUFFER_INITIALIZATION_FAILED";
        case mental::rhi::Result::eBufferMapFailed: return "RHI_BUFFER_MAP_FAILED";
        case mental::rhi::Result::eBufferUploadFailed: return "RHI_BUFFER_UPLOAD_FAILED";

        default:
        {
            static char buf[24];
            snprintf(buf, sizeof(buf), "Unknown (%d)", res);
            return buf;
        }
    }
}

const char* graphicsApiToString(GraphicsApi api)
{
    switch (api)
    {
        case GraphicsApi::Vulkan:
        {
            return "Vulkan";
        }
        default:
        {
            return "";
        }
    }
}

void initDevice(GraphicsApi api, const mental::platform::IWindow* const window)
{
    switch (api)
    {
#if defined MENTAL_WITH_VULKAN
        case GraphicsApi::Vulkan:
        {
            rhi::Result res;
            rhi::vk::DeviceFactory factory;

            rhi::vk::InstanceInfo instanceInfo;
            res = factory.createInstance(instanceInfo);
            if (res != rhi::Result::eSuccess) mental::core::log::fatal("Failed to create vulkan instance. Error: %s", resultToString(res));

            VkSurfaceKHR surface;
            res = window->createSurface(instanceInfo.getInstance(), surface);
            if (res != rhi::Result::eSuccess) mental::core::log::fatal("Failed to create vulkan surface. Error: %s", resultToString(res));

            res = factory.initDevice(instanceInfo, surface);
            if (res != rhi::Result::eSuccess) mental::core::log::fatal("Failed to create vulkan device. Error: %s", resultToString(res));

            gDevice = &rhi::vk::Device::instance();
            break;
        }
#endif

        default:
        {
            mental::core::log::fatal("Unsupported graphics api %s", graphicsApiToString(api));
        }
    }
}

IDevice& getDevice()
{
    return *gDevice;
}

}  // namespace mental::rhi
